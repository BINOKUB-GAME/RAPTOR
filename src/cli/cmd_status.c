/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_status.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon de diagnostic vital instantané ('rap status').
 *                   Audite la mémoire vive réelle via /proc/meminfo, recense
 *                   les processus actifs sous /proc et vérifie la présence
 *                   du fichier d'altération globale /etc/ld.so.preload.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 89)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   89. rap_cmd_status
 * ============================================================================
 */

#include <dirent.h>
#include <ctype.h>
#include "rap_cli.h"

/* ============================================================================
 * FONCTION 89 : rap_cmd_status
 * ----------------------------------------------------------------------------
 * Triage d'urgence en moins de 15 ms :
 *  - Mémoire totale et réellement disponible (MemAvailable).
 *  - Dénombrement des processus actifs dans le pseudo-système /proc.
 *  - Présence critique de /etc/ld.so.preload.
 * ============================================================================
 */
int rap_cmd_status(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    long mem_total = 0;
    long mem_avail = 0;

    FILE *fmem = fopen("/proc/meminfo", "r");
    if (fmem) {
        char line[128];
        while (fgets(line, sizeof(line), fmem)) {
            if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1) continue;
            if (sscanf(line, "MemAvailable: %ld kB", &mem_avail) == 1) continue;
        }
        fclose(fmem);
    }

    int proc_count = 0;
    DIR *dir = opendir("/proc");
    if (dir) {
        struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            if (isdigit(de->d_name[0])) {
                proc_count++;
            }
        }
        closedir(dir);
    }

    int preload_active = (access("/etc/ld.so.preload", F_OK) == 0);

    /* Sortie condensée pour pipeline ou script de surveillance */
    if (cfg->quiet) {
        printf("%ld\t%ld\t%d\t%d\n",
               mem_total / 1024,
               mem_avail / 1024,
               proc_count,
               preload_active);
        return preload_active ? RAP_EXIT_THREAT : RAP_EXIT_OK;
    }

    /* Sortie console formatée */
    printf("%s[ RAPTOR DEFENSE // RAP STATUS ]%s\n",
           cfg->is_tty ? C_CYAN : "",
           cfg->is_tty ? C_RESET : "");
    printf(" - Mémoire Totale      : %ld Mo\n", mem_total / 1024);
    printf(" - Mémoire Disponible  : %ld Mo (%ld %%)\n",
           mem_avail / 1024,
           mem_total ? (mem_avail * 100 / mem_total) : 0);
    printf(" - Processus Hôte      : %d actifs\n", proc_count);
    printf(" - /etc/ld.so.preload  : %s%s%s\n",
           preload_active ? (cfg->is_tty ? C_RED "PRÉSENT (ALERTE ROOTKIT)" : "PRÉSENT") : "Non configuré (Intègre)",
           preload_active ? (cfg->is_tty ? C_RESET : "") : "",
           preload_active ? "" : "");

    return preload_active ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}
