/* ============================================================================
 * NOM DU SCRIPT   : src/engine/sentry_engine.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Cœur cinétique de surveillance passive et d'interception
 *                   permissionnelle synchrone anti-rançongiciel (fanotify).
 *                   Déploiement d'appâts denses (canaris), écoute bloquante
 *                   à 0% CPU, gel réflexe SIGSTOP et invisibilité mémoire.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Élimination de la dépendance externe rap_check_safety_pid (cause du SIGSEGV).
 *   - Intégration autonome de sentry_is_protected_pid (PID <= 1, self, parent).
 *   - Alignement 8 octets garanti sur le buffer fanotify (__attribute__((aligned(8)))).
 *   - Permissions 0666 forcées via chmod sur les canaris pour piéger tout compte.
 *   - Dimensionnement sécurisé du tampon d'alerte (512 octets).
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *   111. sentry_deploy_canary    : Génère et dépose un fichier leurre binaire dense.
 *   112. sentry_init_fanotify    : Initialise fanotify et marque les leurres cibles.
 *   113. sentry_listen_loop      : Guet passif bloquant sur poll() et lecture d'événements.
 *   114. sentry_trigger_reflex   : Riposte cinétique (SIGSTOP, FAN_DENY, levée d'alerte).
 *   115. sentry_enter_stealth    : Verrouillage mémoire physique, immunité OOM et prctl.
 * ============================================================================
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <sys/fanotify.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sentry_engine.h"

/* ============================================================================
 * GARDE-FOU INTERNE : Protection anti-suicide et anti-panique noyau
 * ============================================================================
 */
static inline bool sentry_is_protected_pid(pid_t pid)
{
    /* Ne jamais frapper PID 0 ou PID 1 (init/systemd) */
    if (pid <= 1) {
        return true;
    }
    /* Ne jamais frapper la sentinelle elle-même */
    if (pid == getpid()) {
        return true;
    }
    /* Ne jamais frapper le parent direct (shell ou wrapper) */
    if (pid == getppid()) {
        return true;
    }
    return false;
}

/* ============================================================================
 * 111. sentry_deploy_canary
 * ============================================================================
 */
int sentry_deploy_canary(const char *dir_path)
{
    if (!dir_path || dir_path[0] == '\0') {
        return -1;
    }

    char canary_path[SENTRY_PATH_MAX];
    int ret = snprintf(canary_path, sizeof(canary_path), "%s/%s", dir_path, SENTRY_CANARY_NAME);
    if (ret <= 0 || (size_t)ret >= sizeof(canary_path)) {
        return -1;
    }

    int fd = open(canary_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (fd < 0) {
        return -1;
    }

    /* Force les droits de lecture/écriture pour piéger même les scripts non-root */
    fchmod(fd, 0666);

    uint8_t chunk[512];
    const uint8_t kdbx_magic[12] = {
        0x03, 0xD9, 0xA2, 0x9A,
        0x67, 0xFB, 0x4B, 0xB5,
        0x00, 0x00, 0x04, 0x00
    };

    uint32_t seed = 0x1337BEEF;
    for (int block = 0; block < 64; ++block) {
        for (int i = 0; i < 512; ++i) {
            seed = (seed * 1103515245U + 12345U) & 0x7FFFFFFFU;
            chunk[i] = (uint8_t)(seed ^ (block + i));
        }

        if (block == 0) {
            memcpy(chunk, kdbx_magic, sizeof(kdbx_magic));
        }

        ssize_t written = write(fd, chunk, sizeof(chunk));
        if (written != sizeof(chunk)) {
            close(fd);
            return -1;
        }
    }

    fdatasync(fd);
    close(fd);
    return 0;
}

/* ============================================================================
 * 112. sentry_init_fanotify
 * ============================================================================
 */
int sentry_init_fanotify(const SentryConfig *config)
{
    if (!config || config->target_count == 0) {
        return -1;
    }

    int fan_fd = fanotify_init(FAN_CLASS_CONTENT | FAN_NONBLOCK | FAN_CLOEXEC,
                               O_RDONLY | O_LARGEFILE | O_CLOEXEC);
    if (fan_fd < 0) {
        return -1;
    }

    uint32_t marks_applied = 0;
    char canary_full_path[SENTRY_PATH_MAX];

    for (uint32_t i = 0; i < config->target_count; ++i) {
        const char *dir = config->target_dirs[i];
        if (dir[0] == '\0') {
            continue;
        }

        snprintf(canary_full_path, sizeof(canary_full_path), "%s/%s", dir, SENTRY_CANARY_NAME);
        if (access(canary_full_path, F_OK) != 0) {
            if (sentry_deploy_canary(dir) != 0) {
                continue;
            }
        }

        int mark_ret = fanotify_mark(fan_fd,
                                     FAN_MARK_ADD,
                                     FAN_OPEN_PERM,
                                     AT_FDCWD,
                                     canary_full_path);
        if (mark_ret == 0) {
            marks_applied++;
        }
    }

    if (marks_applied == 0) {
        close(fan_fd);
        return -1;
    }

    return fan_fd;
}

/* ============================================================================
 * 113. sentry_listen_loop
 * ============================================================================
 */
int sentry_listen_loop(int fanotify_fd, const SentryConfig *config)
{
    if (fanotify_fd < 0 || !config) {
        return -1;
    }

    struct pollfd fds[1];
    fds[0].fd = fanotify_fd;
    fds[0].events = POLLIN;

    /* Alignement mémoire strict pour struct fanotify_event_metadata */
    uint8_t event_buf[SENTRY_EVENT_BUF_SIZE] __attribute__((aligned(8)));

    while (1) {
        int poll_ret = poll(fds, 1, -1);
        if (poll_ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        if (fds[0].revents & POLLIN) {
            ssize_t len = read(fanotify_fd, event_buf, sizeof(event_buf));
            if (len <= 0) {
                if (errno == EAGAIN || errno == EINTR) {
                    continue;
                }
                return -1;
            }

            const struct fanotify_event_metadata *metadata = 
                (const struct fanotify_event_metadata *)event_buf;

            while (FAN_EVENT_OK(metadata, len)) {
                if (metadata->vers < FANOTIFY_METADATA_VERSION) {
                    break;
                }

                if (metadata->mask & FAN_OPEN_PERM) {
                    sentry_trigger_reflex(fanotify_fd, metadata->fd, metadata->pid);
                } else if (metadata->fd >= 0) {
                    close(metadata->fd);
                }

                metadata = FAN_EVENT_NEXT(metadata, len);
            }
        }
    }

    return 0;
}

/* ============================================================================
 * 114. sentry_trigger_reflex
 * ============================================================================
 */
int sentry_trigger_reflex(int fanotify_fd, int fd_event, pid_t pid)
{
    struct fanotify_response resp;
    resp.fd = fd_event;

    /* Filtrage par le garde-fou autonome */
    if (sentry_is_protected_pid(pid)) {
        resp.response = FAN_ALLOW;
        write(fanotify_fd, &resp, sizeof(resp));
        if (fd_event >= 0) {
            close(fd_event);
        }
        return -1;
    }

    /* 1. RIPOSTE CINÉTIQUE : Gel immédiat du processus pirate (SIGSTOP) */
    int kill_ret = kill(pid, SIGSTOP);

    /* 2. REJET SYSTÈME : Refus d'ouverture du fichier canari par le noyau */
    resp.response = FAN_DENY;
    write(fanotify_fd, &resp, sizeof(resp));

    if (fd_event >= 0) {
        close(fd_event);
    }

    /* Notification visuelle d'alerte */
    char alert_msg[512];
    int msg_len = snprintf(alert_msg, sizeof(alert_msg),
        "\n\033[1;31m[ALERTE SENTINELLE]\033[0m ATTEINTE AU CANARI DETECTEE !\n"
        "  -> PID AGRESSEUR       : %d\n"
        "  -> ETAT DU PROCESSUS   : SUSPENDU CONSERVATOIRE (SIGSTOP)\n"
        "  -> ACCES FICHIER       : BLOQUE (FAN_DENY)\n"
        "  -> ACTION REQUISE      : rap hunt OU rap kill %d\n\n",
        (int)pid, (int)pid);

    if (msg_len > 0) {
        write(STDERR_FILENO, alert_msg, (size_t)msg_len);
    }

    return (kill_ret == 0) ? 0 : -1;
}

/* ============================================================================
 * 115. sentry_enter_stealth
 * ============================================================================
 */
int sentry_enter_stealth(const char *fake_name)
{
    if (fake_name && fake_name[0] != '\0') {
        prctl(PR_SET_NAME, (unsigned long)fake_name, 0, 0, 0);
    } else {
        prctl(PR_SET_NAME, (unsigned long)"[kworker/u16:3]", 0, 0, 0);
    }

    int oom_fd = open("/proc/self/oom_score_adj", O_WRONLY | O_CLOEXEC);
    if (oom_fd >= 0) {
        const char oom_val[] = "-1000\n";
        write(oom_fd, oom_val, sizeof(oom_val) - 1);
        close(oom_fd);
    }

    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        return -1;
    }

    return 0;
}
