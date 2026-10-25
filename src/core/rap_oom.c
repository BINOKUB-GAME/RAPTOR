/* ==============================================================================
 * NOM DU SCRIPT : src/core/rap_oom.c
 * RÉVISION      : Rev 1.0 (2026-10-25)
 * PROJET        : RAPTOR DEFENSE v2.0
 * DESCRIPTION   : Implémentation du sous-système de survie sous contrainte mémoire.
 *                 Fonctionnement déterministe C99 sans allocation dynamique.
 * NOUVELLES INTÉGRATIONS :
 *   - Écriture atomique dans /proc/self/oom_score_adj sans stdio dynamique.
 *   - Verrouillage total MCL_CURRENT | MCL_FUTURE sous POSIX.
 * FONCTIONS NUMÉROTÉES :
 *   1. rap_oom_protect()     - Écriture du score -1000 dans oom_score_adj.
 *   2. rap_oom_lock_memory() - Exécution de l'appel système mlockall.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "rap_oom.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define OOM_SCORE_PATH "/proc/self/oom_score_adj"
#define OOM_SCORE_STR  "-1000\n"

/* ==============================================================================
 * 1. rap_oom_protect
 * ============================================================================== */
int rap_oom_protect(void)
{
    int fd = open(OOM_SCORE_PATH, O_WRONLY);
    if (fd < 0) {
        return -1;
    }

    const char payload[] = OOM_SCORE_STR;
    ssize_t len = sizeof(payload) - 1;
    ssize_t written = write(fd, payload, (size_t)len);
    close(fd);

    return (written == len) ? 0 : -1;
}

/* ==============================================================================
 * 2. rap_oom_lock_memory
 * ============================================================================== */
int rap_oom_lock_memory(void)
{
    /* Verrouille l'ensemble des pages du processus en RAM (texte, données, BSS, pile) */
    return mlockall(MCL_CURRENT | MCL_FUTURE);
}
