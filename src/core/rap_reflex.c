/* ==============================================================================
 * NOM DU SCRIPT : src/core/rap_reflex.c
 * RÉVISION      : Rev 1.1 (2026-10-25)
 * PROJET        : RAPTOR DEFENSE v2.0
 * DESCRIPTION   : Moteur séquentiel d'action réflexe chirurgicale.
 *                 Prend en charge la paralysie immédiate (SIGSTOP), la fermeture
 *                 des sockets réseau, l'extraction de métadonnées et la destruction (SIGKILL).
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Dimensionnement strict des tampons de chemin système à 512 octets (RAP_PATH_MAX_LOCAL)
 *     pour éliminer l'avertissement compilateur GCC -Wformat-truncation sur link_path.
 *   - Maintien du modèle d'allocation statique sur la pile (0 heap).
 * FONCTIONS NUMÉROTÉES :
 *   1. rap_reflex_init()      - Initialisation et configuration du mode initial.
 *   2. rap_reflex_set_mode()  - Ajustement dynamique du niveau d'armement.
 *   3. rap_reflex_dispatch()  - Séquence ordonnancée d'interception et neutralisation.
 *   4. rap_reflex_get_stats() - Lecture des métriques d'intervention du module.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "rap_reflex.h"
#include "rap_blackbox.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

#define RAP_PATH_MAX_LOCAL 512

/* --- État interne statique (0-Heap) --- */
static reflex_mode_t g_reflex_mode        = REFLEX_MODE_PASSIVE;
static uint64_t      g_neutralized_count  = 0;
static bool          g_reflex_initialized = false;

/* --- Fonctions auxiliaires internes privées --- */

static void reflex_extract_cmdline(pid_t pid, char *out_buf, size_t max_len)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    out_buf[0] = '\0';

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return;
    }

    ssize_t bytes = read(fd, out_buf, max_len - 1);
    close(fd);

    if (bytes > 0) {
        out_buf[bytes] = '\0';
        /* Remplacement des délimiteurs nuls par des espaces pour lisibilité */
        for (ssize_t i = 0; i < bytes - 1; i++) {
            if (out_buf[i] == '\0') {
                out_buf[i] = ' ';
            }
        }
    }
}

static void reflex_sever_sockets(pid_t pid)
{
    char fd_dir_path[64];
    snprintf(fd_dir_path, sizeof(fd_dir_path), "/proc/%d/fd", pid);

    DIR *d = opendir(fd_dir_path);
    if (!d) {
        return;
    }

    struct dirent *dir_entry;
    while ((dir_entry = readdir(d)) != NULL) {
        if (dir_entry->d_name[0] == '.') {
            continue;
        }

        char link_target[RAP_PATH_MAX_LOCAL];
        char link_path[RAP_PATH_MAX_LOCAL];
        snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir_path, dir_entry->d_name);

        ssize_t len = readlink(link_path, link_target, sizeof(link_target) - 1);
        if (len > 0) {
            link_target[len] = '\0';
            /* Si le descripteur référence une socket active, on isole le flux */
            if (strncmp(link_target, "socket:[", 8) == 0) {
                int target_fd = 0;
                const char *p = dir_entry->d_name;
                while (*p >= '0' && *p <= '9') {
                    target_fd = target_fd * 10 + (*p - '0');
                    p++;
                }
                /* Fermeture forcée des flux bidirectionnels de la socket */
                shutdown(target_fd, SHUT_RDWR);
            }
        }
    }
    closedir(d);
}

/* ==============================================================================
 * 1. rap_reflex_init
 * ============================================================================== */
int rap_reflex_init(reflex_mode_t initial_mode)
{
    g_reflex_mode = initial_mode;
    g_neutralized_count = 0;

    /* Liaison avec la boîte noire en mémoire vive */
    if (rap_blackbox_open() != 0) {
        return -1;
    }

    g_reflex_initialized = true;
    return 0;
}

/* ==============================================================================
 * 2. rap_reflex_set_mode
 * ============================================================================== */
void rap_reflex_set_mode(reflex_mode_t mode)
{
    g_reflex_mode = mode;
}

/* ==============================================================================
 * 3. rap_reflex_dispatch
 * ============================================================================== */
int rap_reflex_dispatch(pid_t target_pid, reflex_trigger_t trigger)
{
    if (!g_reflex_initialized) {
        if (rap_reflex_init(REFLEX_MODE_PASSIVE) != 0) {
            return -1;
        }
    }

    if (g_reflex_mode == REFLEX_MODE_PASSIVE || target_pid <= 1) {
        return 0; /* Ne jamais frapper PID 0, PID 1 ou opérer en mode passif */
    }

    struct timespec start_time;
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    reflex_record_t record;
    memset(&record, 0, sizeof(record));
    clock_gettime(CLOCK_REALTIME, &record.timestamp);

    record.pid = target_pid;
    record.trigger = trigger;
    record.kill_confirmed = false;

    /* Extraction de la ligne de commande du suspect */
    reflex_extract_cmdline(target_pid, record.cmdline, sizeof(record.cmdline));

    /* --- ÉTAPE 1 : PARALYSIE IMMÉDIATE (SIGSTOP) --- */
    if (g_reflex_mode == REFLEX_MODE_ARMED) {
        kill(target_pid, SIGSTOP);
        kill(-target_pid, SIGSTOP); /* Arrêt de l'ensemble du groupe de threads */
    }

    /* --- ÉTAPE 2 : RÉVOCATION FORCÉE DES SOCKETS --- */
    if (g_reflex_mode == REFLEX_MODE_ARMED) {
        reflex_sever_sockets(target_pid);
    }

    /* --- ÉTAPE 3 : DESTRUCTION PHYSIQUE (SIGKILL) --- */
    if (g_reflex_mode == REFLEX_MODE_ARMED) {
        kill(target_pid, SIGKILL);
        kill(-target_pid, SIGKILL);

        int status = 0;
        waitpid(target_pid, &status, WNOHANG);
        record.kill_confirmed = true;
        g_neutralized_count++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    uint64_t diff_sec = (uint64_t)(end_time.tv_sec - start_time.tv_sec);
    uint64_t diff_nsec = (uint64_t)(end_time.tv_nsec - start_time.tv_nsec);
    record.latency_ns = (uint32_t)((diff_sec * 1000000000ULL) + diff_nsec);

    /* --- ÉTAPE 4 : ENREGISTREMENT MÉDICO-LÉGAL BOÎTE NOIRE --- */
    rap_blackbox_push(&record);

    return 0;
}

/* ==============================================================================
 * 4. rap_reflex_get_stats
 * ============================================================================== */
reflex_mode_t rap_reflex_get_stats(uint64_t *out_neutralized_count)
{
    if (out_neutralized_count != NULL) {
        *out_neutralized_count = g_neutralized_count;
    }
    return g_reflex_mode;
}
