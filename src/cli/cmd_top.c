/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_top.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon d'analyse des consommateurs de mémoire vive ('rap top').
 *                   Extrait l'empreinte résidente réelle (VmRSS) sous /proc/[PID]/status,
 *                   exclut systématiquement les threads noyau inactifs en RAM physique
 *                   et applique un tri d'insertion statique borné (zéro heap).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Correction de l'avertissement GCC -Wstringop-truncation.
 *   - Remplacement de strncpy par snprintf pour sécuriser la copie de 'comm'
 *     avec terminaison NUL garantie et conformité C99 stricte.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 90)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   90. rap_cmd_top
 * ============================================================================
 */

#include <dirent.h>
#include <ctype.h>
#include "rap_cli.h"

#define RAP_TOP_MAX_ENTRIES 32

typedef struct {
    pid_t pid;
    long  rss_kb;
    char  name[64];
} RapTopEntry;

/* ============================================================================
 * FONCTION 90 : rap_cmd_top
 * ----------------------------------------------------------------------------
 * Parcourt /proc pour collecter et ordonner les processus les plus gourmands :
 *  - Lecture ciblée des champs Name et VmRSS dans /proc/[PID]/status.
 *  - Élimination des processus ayant rss == 0 (threads noyau / démons dormants).
 *  - Tri d'insertion à capacité fixe dans la pile (0 allocation dynamique).
 *  - Projection tabulaire ou TSV selon les drapeaux -q, -r et -n.
 * ============================================================================
 */
int rap_cmd_top(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    RapTopEntry top[RAP_TOP_MAX_ENTRIES];
    int count = 0;
    int limit = (cfg->limit > 0 && cfg->limit <= RAP_TOP_MAX_ENTRIES) ? cfg->limit : 5;

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("rap top: ouverture /proc impossible");
        return RAP_EXIT_ERROR;
    }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (!isdigit(de->d_name[0])) {
            continue;
        }

        pid_t pid = (pid_t)atoi(de->d_name);

        char status_path[64];
        snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
        FILE *fstatus = fopen(status_path, "r");
        if (!fstatus) {
            continue;
        }

        char line[128];
        char comm[64] = "inconnu";
        long rss = 0;

        while (fgets(line, sizeof(line), fstatus)) {
            if (strncmp(line, "Name:", 5) == 0) {
                sscanf(line, "Name:\t%63s", comm);
            } else if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line, "VmRSS:\t%ld", &rss);
            }
        }
        fclose(fstatus);

        if (rss <= 0) {
            continue; /* Élimination stricte des threads noyau */
        }

        /* Insertion ordonnée décroissante */
        int insert_idx = (count < limit) ? count : -1;
        for (int i = 0; i < count; i++) {
            if (rss > top[i].rss_kb) {
                insert_idx = i;
                break;
            }
        }

        if (insert_idx != -1) {
            int shift_end = (count < limit) ? count : (limit - 1);
            for (int j = shift_end; j > insert_idx; j--) {
                top[j] = top[j - 1];
            }
            top[insert_idx].pid = pid;
            top[insert_idx].rss_kb = rss;
            snprintf(top[insert_idx].name, sizeof(top[insert_idx].name), "%s", comm);

            if (count < limit) {
                count++;
            }
        }
    }
    closedir(dir);

    if (!cfg->quiet) {
        rap_output_formatter(cfg, "PID", "NOM", "RAM (RSS)", "STATUT");
        if (cfg->is_tty && !cfg->raw) {
            printf(" ----------------------------------------------------------------------\n");
        }
    }

    for (int i = 0; i < count; i++) {
        char spid[16], smem[32];
        snprintf(spid, sizeof(spid), "%d", top[i].pid);
        snprintf(smem, sizeof(smem), "%ld Mo", top[i].rss_kb / 1024);
        rap_output_formatter(cfg, spid, top[i].name, smem, "Résident");
    }

    return RAP_EXIT_OK;
}
