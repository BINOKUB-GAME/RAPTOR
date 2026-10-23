/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/preload_hunter.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Sonde forensique de détection des détournements du chargeur
 *                   dynamique sous Linux (Section 14).
 *                   Traque le préchargement clandestin de bibliothèques partagées
 *                   (.so) au niveau global de l'hôte (/etc/ld.so.preload) et au
 *                   niveau ciblé de chaque processus (/proc/[PID]/environ).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Sécurisation du tampon d'inspection /proc/[PID]/environ contre les
 *     débordements et lecture stricte sur les délimiteurs nuls (\0).
 *   - Nettoyage des espaces et retours chariot dans le parseur /etc/ld.so.preload.
 *   - Maintien des deux fonctions numérotées au dictionnaire (84 et 85).
 *   - Zéro allocation dynamique lourde pour un audit temps réel à 0 ms.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   84. preload_hunter_check_global
 *   85. preload_hunter_scan_process_env
 * ============================================================================
 */

#include "preload_hunter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* ------------------------------------------------------------------------- */
/* 84. AUDIT DE COMPROMISSION GLOBALE VIA /etc/ld.so.preload                 */
/* ------------------------------------------------------------------------- */
bool preload_hunter_check_global(char* out_path, size_t out_len) {
    if (out_path && out_len > 0) {
        out_path[0] = '\0';
    }

    if (access(LD_PRELOAD_GLOBAL_PATH, F_OK) != 0) {
        return false;
    }

    FILE* fp = fopen(LD_PRELOAD_GLOBAL_PATH, "r");
    if (!fp) {
        return false;
    }

    char line[512];
    bool infection_found = false;

    while (fgets(line, sizeof(line), fp)) {
        char* cur = line;
        while (*cur == ' ' || *cur == '\t') {
            cur++;
        }

        if (*cur == '#' || *cur == '\n' || *cur == '\r' || *cur == '\0') {
            continue;
        }

        size_t len = strlen(cur);
        while (len > 0 && (cur[len - 1] == '\n' || cur[len - 1] == '\r' || cur[len - 1] == ' ' || cur[len - 1] == '\t')) {
            cur[len - 1] = '\0';
            len--;
        }

        if (len > 0) {
            if (out_path && out_len > 0) {
                size_t copy_len = (len < out_len - 1) ? len : out_len - 1;
                memcpy(out_path, cur, copy_len);
                out_path[copy_len] = '\0';
            }
            infection_found = true;
            break;
        }
    }

    fclose(fp);
    return infection_found;
}

/* ------------------------------------------------------------------------- */
/* 85. AUDIT CHIRURGICAL DES VARIABLES D'ENVIRONNEMENT (/proc/[PID]/environ) */
/* ------------------------------------------------------------------------- */
bool preload_hunter_scan_process_env(pid_t pid, char* out_preload_lib, size_t out_len) {
    if (pid <= 1) {
        return false;
    }

    if (out_preload_lib && out_len > 0) {
        out_preload_lib[0] = '\0';
    }

    char env_path[64];
    snprintf(env_path, sizeof(env_path), "/proc/%d/environ", (int)pid);

    int fd = open(env_path, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read <= 0) {
        return false;
    }

    buffer[bytes_read] = '\0';

    const char* ptr = buffer;
    const char* end = buffer + bytes_read;
    static const char s_target_key[] = "LD_PRELOAD=";
    size_t key_len = sizeof(s_target_key) - 1;

    while (ptr < end) {
        if (strncmp(ptr, s_target_key, key_len) == 0) {
            const char* val = ptr + key_len;
            while (*val == ' ' || *val == '\t') {
                val++;
            }
            size_t val_len = strlen(val);

            if (val_len > 0) {
                if (out_preload_lib && out_len > 0) {
                    size_t copy_len = (val_len < out_len - 1) ? val_len : out_len - 1;
                    memcpy(out_preload_lib, val, copy_len);
                    out_preload_lib[copy_len] = '\0';
                }
                return true;
            }
        }
        ptr += strlen(ptr) + 1;
    }

    return false;
}
