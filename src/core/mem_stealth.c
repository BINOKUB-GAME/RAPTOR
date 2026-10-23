/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/mem_stealth.c
 * RÉVISION        : Rev 1.7
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Sonde d'inspection forensique mémoire et traque de mutations.
 *                   Repère l'exécution sans ancrage binaire (mémoire anonyme R-X)
 *                   consécutive à une bascule mprotect() clandestine et injecte
 *                   l'anomalie directement dans le registre de l'Onglet 4.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.7) :
 *   - Suppression définitive de l'avertissement GCC -Wstringop-truncation.
 *   - Remplacement de tous les appels strncpy par des copies bornées memcpy
 *     avec assignation explicite de l'octet nul terminal.
 *   - Sécurisation du parseur de assets/rules/mem_whitelist.conf.
 *   - Maintien strict des 4 fonctions numérotées (80 à 83).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   80. mem_stealth_whitelist_load
 *   81. mem_stealth_is_whitelisted
 *   82. mem_stealth_scan_maps_anon_rx
 *   83. mem_stealth_dispatch_diff_event
 * ============================================================================
 */

#include "mem_stealth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Structure du cache mémoire de la whitelist */
typedef struct {
    char pattern[64];
} WhitelistEntry;

static WhitelistEntry s_whitelist[MAX_MEM_WHITELIST_ENTRIES];
static int s_whitelist_count = 0;
static bool s_whitelist_loaded = false;

/* Registre interne des alertes forensiques pour l'Onglet 4 */
static StealthEvent s_stealth_history[MAX_STEALTH_EVENTS];
static int s_stealth_history_count = 0;

/* ------------------------------------------------------------------------- */
/* 80. CHARGEMENT DU REGISTRE SOUVERAIN (assets/rules/mem_whitelist.conf)     */
/* ------------------------------------------------------------------------- */
bool mem_stealth_whitelist_load(const char* filepath) {
    s_whitelist_count = 0;

    const char* paths_to_try[3] = {
        filepath,
        "assets/rules/mem_whitelist.conf",
        "../assets/rules/mem_whitelist.conf"
    };

    FILE* fp = NULL;
    for (int p = 0; p < 3; ++p) {
        if (paths_to_try[p]) {
            fp = fopen(paths_to_try[p], "r");
            if (fp) {
                break;
            }
        }
    }

    if (!fp) {
        s_whitelist_loaded = true;
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        char type[16] = {0};
        char pattern[64] = {0};

        char* pipe1 = strchr(line, '|');
        if (!pipe1) {
            continue;
        }

        /* Extraction sécurisée du champ TYPE */
        size_t type_len = (size_t)(pipe1 - line);
        if (type_len >= sizeof(type)) {
            type_len = sizeof(type) - 1;
        }
        memcpy(type, line, type_len);
        type[type_len] = '\0';

        /* Extraction sécurisée du champ MOTIF */
        char* start_pat = pipe1 + 1;
        while (*start_pat == ' ' || *start_pat == '\t') {
            start_pat++;
        }

        char* pipe2 = strchr(start_pat, '|');
        char* end_pat = pipe2 ? pipe2 : (start_pat + strlen(start_pat));

        while (end_pat > start_pat && (*(end_pat - 1) == ' ' || *(end_pat - 1) == '\t' ||
               *(end_pat - 1) == '\n' || *(end_pat - 1) == '\r')) {
            end_pat--;
        }

        size_t pat_len = (size_t)(end_pat - start_pat);
        if (pat_len > 0 && pat_len < sizeof(pattern)) {
            memcpy(pattern, start_pat, pat_len);
            pattern[pat_len] = '\0';

            for (size_t i = 0; pattern[i]; ++i) {
                pattern[i] = (char)tolower((unsigned char)pattern[i]);
            }

            if (s_whitelist_count < MAX_MEM_WHITELIST_ENTRIES) {
                size_t plen = strlen(pattern);
                if (plen >= sizeof(s_whitelist[s_whitelist_count].pattern)) {
                    plen = sizeof(s_whitelist[s_whitelist_count].pattern) - 1;
                }
                memcpy(s_whitelist[s_whitelist_count].pattern, pattern, plen);
                s_whitelist[s_whitelist_count].pattern[plen] = '\0';
                s_whitelist_count++;
            }
        }
    }

    fclose(fp);
    s_whitelist_loaded = true;
    return (s_whitelist_count > 0);
}

/* ------------------------------------------------------------------------- */
/* 81. ÉVALUATION CONTRE LES SIGNATURES DE LA WHITELIST EXTERNE             */
/* ------------------------------------------------------------------------- */
bool mem_stealth_is_whitelisted(const char* comm, const char* exe) {
    if (!s_whitelist_loaded) {
        mem_stealth_whitelist_load("assets/rules/mem_whitelist.conf");
    }

    if (s_whitelist_count == 0 || (!comm && !exe)) {
        return false;
    }

    char lower_comm[128] = {0};
    char lower_exe[256] = {0};

    if (comm) {
        for (size_t i = 0; i < sizeof(lower_comm) - 1 && comm[i]; ++i) {
            lower_comm[i] = (char)tolower((unsigned char)comm[i]);
        }
    }

    if (exe) {
        for (size_t i = 0; i < sizeof(lower_exe) - 1 && exe[i]; ++i) {
            lower_exe[i] = (char)tolower((unsigned char)exe[i]);
        }
    }

    for (int i = 0; i < s_whitelist_count; ++i) {
        const char* pat = s_whitelist[i].pattern;
        if (pat[0] != '\0') {
            if ((lower_comm[0] && strstr(lower_comm, pat) != NULL) ||
                (lower_exe[0]  && strstr(lower_exe, pat) != NULL)) {
                return true;
            }
        }
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* 82. TRAQUE DE MÉMOIRE ANONYME EXÉCUTABLE (R-X SANS FICHIER SOURCE)         */
/* ------------------------------------------------------------------------- */
bool mem_stealth_scan_maps_anon_rx(pid_t pid, const char* comm, const char* exe, StealthEvent* ev_out) {
    if (pid <= 1 || !ev_out) {
        return false;
    }

    if (mem_stealth_is_whitelisted(comm, exe)) {
        return false;
    }

    char maps_path[64];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", (int)pid);

    FILE* fp = fopen(maps_path, "r");
    if (!fp) {
        return false;
    }

    char line[512];
    bool anomaly_detected = false;

    while (fgets(line, sizeof(line), fp)) {
        unsigned long start = 0, end = 0;
        char perms[5] = {0};
        unsigned long offset = 0;
        int dev_major = 0, dev_minor = 0;
        unsigned long inode = 0;
        int path_idx = 0;

        int items = sscanf(line, "%lx-%lx %4s %lx %x:%x %lu %n",
                           &start, &end, perms, &offset, &dev_major, &dev_minor, &inode, &path_idx);

        if (items < 7) {
            continue;
        }

        if (perms[2] == 'x') {
            const char* path_str = (path_idx > 0) ? (line + path_idx) : "";
            while (*path_str == ' ' || *path_str == '\t') {
                path_str++;
            }

            if (inode == 0 &&
                strstr(path_str, "[vdso]") == NULL &&
                strstr(path_str, "[vsyscall]") == NULL) {

                ev_out->pid = pid;

                const char* src_comm = comm ? comm : "inconnu";
                size_t clen = strlen(src_comm);
                if (clen >= sizeof(ev_out->comm)) {
                    clen = sizeof(ev_out->comm) - 1;
                }
                memcpy(ev_out->comm, src_comm, clen);
                ev_out->comm[clen] = '\0';

                ev_out->start_addr = start;
                ev_out->end_addr = end;
                ev_out->size_kb = (size_t)((end - start) / 1024);

                snprintf(ev_out->detail, sizeof(ev_out->detail),
                         "Mutation anonyme R-X (Inode 0) : %lu Ko sans binaire",
                         (unsigned long)ev_out->size_kb);

                mem_stealth_dispatch_diff_event(ev_out);

                anomaly_detected = true;
                break;
            }
        }
    }

    fclose(fp);
    return anomaly_detected;
}

/* ------------------------------------------------------------------------- */
/* 83. DISPATCH & ENREGISTREMENT DANS LE REGISTRE DU CAHIER DIFFÉRENTIEL     */
/* ------------------------------------------------------------------------- */
void mem_stealth_dispatch_diff_event(const StealthEvent* ev) {
    if (!ev) {
        return;
    }

    for (int i = 0; i < s_stealth_history_count; ++i) {
        if (s_stealth_history[i].pid == ev->pid &&
            s_stealth_history[i].start_addr == ev->start_addr) {
            return;
        }
    }

    if (s_stealth_history_count < MAX_STEALTH_EVENTS) {
        s_stealth_history[s_stealth_history_count++] = *ev;
    } else {
        for (int i = 0; i < MAX_STEALTH_EVENTS - 1; ++i) {
            s_stealth_history[i] = s_stealth_history[i + 1];
        }
        s_stealth_history[MAX_STEALTH_EVENTS - 1] = *ev;
    }

    fprintf(stderr, "[RAPTOR FORENSIC] ALERTE ENREGISTRÉE : PID %d (%s) -> 0x%lx (%lu Ko)\n",
            ev->pid, ev->comm, ev->start_addr, (unsigned long)ev->size_kb);
}

/* ------------------------------------------------------------------------- */
/* ACCESSEURS POUR LA VUE GRAPHIQUE RAYLIB                                    */
/* ------------------------------------------------------------------------- */
int mem_stealth_get_diff_events(StealthEvent* out_events, int max_count) {
    if (!out_events || max_count <= 0) {
        return 0;
    }

    int count = (s_stealth_history_count < max_count) ? s_stealth_history_count : max_count;
    for (int i = 0; i < count; ++i) {
        out_events[i] = s_stealth_history[i];
    }
    return count;
}

void mem_stealth_clear_diff_events(void) {
    s_stealth_history_count = 0;
}
