/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_rx.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon d'investigation mémoire chirurgicale ('rap rx').
 *                   Inspecte les tables d'allocation virtuelle (/proc/[PID]/maps)
 *                   pour détecter les mutations clandestines anonymes Inode 0
 *                   converties en exécutable (r-xp) sans ancrage sur le disque.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Intégration du filtrage de liste blanche via rap_is_mem_whitelisted.
 *   - Par défaut : neutralise la pollution visuelle des moteurs JIT et GUI.
 *   - Mode -v / --verbose : projette les éléments approuvés avec tag dédié
 *     sans lever d'alerte critique sur le code de retour POSIX.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 92)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   92. rap_cmd_rx
 * ============================================================================
 */

#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include "rap_cli.h"

/* ============================================================================
 * FONCTION 92 : rap_cmd_rx
 * ----------------------------------------------------------------------------
 * Parcourt l'arborescence /proc pour traquer les pages anonymes R-X (Inode 0) :
 *  - Filtre les composants légitimes via mem_whitelist.conf sauf si -v.
 *  - Formate la sortie selon le mode interactif TTY ou pipeline machine TSV.
 *  - Renvoie RAP_EXIT_THREAT (1) uniquement en cas d'anomalie non listée.
 * ============================================================================
 */
int rap_cmd_rx(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    int real_threat_found = 0;
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("rap rx: ouverture /proc impossible");
        return RAP_EXIT_ERROR;
    }

    if (!cfg->quiet) {
        rap_output_formatter(cfg, "PID", "NOM", "PLAGE MÉMOIRE", "ANOMALIE");
        if (cfg->is_tty && !cfg->raw) {
            printf(" ----------------------------------------------------------------------\n");
        }
    }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (!isdigit(de->d_name[0])) {
            continue;
        }

        pid_t pid = (pid_t)atoi(de->d_name);

        char maps_path[64];
        snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
        FILE *fmaps = fopen(maps_path, "r");
        if (!fmaps) {
            continue;
        }

        char line[256];
        while (fgets(line, sizeof(line), fmaps)) {
            unsigned long start = 0, end = 0;
            char perms[5] = {0};
            unsigned long offset = 0;
            int dev_maj = 0, dev_min = 0;
            unsigned long inode = 0;
            char path_buf[128] = "";

            int matched = sscanf(line, "%lx-%lx %4s %lx %x:%x %lu %127[^\n]",
                                 &start, &end, perms, &offset, &dev_maj, &dev_min, &inode, path_buf);

            if (matched >= 7) {
                if (perms[0] == 'r' && perms[2] == 'x' && inode == 0 && offset == 0) {
                    if (strstr(path_buf, "[vdso]") || strstr(path_buf, "[vsyscall]")) {
                        continue;
                    }

                    /* Résolution du nom de la tâche */
                    char comm_path[64];
                    char scomm[64] = "inconnu";
                    snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);
                    FILE *fcomm = fopen(comm_path, "r");
                    if (fcomm) {
                        if (fgets(scomm, sizeof(scomm), fcomm)) {
                            scomm[strcspn(scomm, "\r\n")] = '\0';
                        }
                        fclose(fcomm);
                    }

                    int whitelisted = rap_is_mem_whitelisted(scomm);

                    /* Élimination du bruit si pas de mode verbose demandé */
                    if (whitelisted && !cfg->verbose) {
                        break;
                    }

                    char spid[16], srange[64];
                    snprintf(spid, sizeof(spid), "%d", pid);
                    snprintf(srange, sizeof(srange), "0x%lx-0x%lx", start, end);

                    if (whitelisted) {
                        rap_output_formatter(cfg, spid, scomm, srange, "MUTATION R-X [LISTE BLANCHE]");
                    } else {
                        real_threat_found++;
                        rap_output_formatter(cfg, spid, scomm, srange, "MUTATION R-X (Inode 0)");
                    }
                    break;
                }
            }
        }
        fclose(fmaps);
    }
    closedir(dir);

    if (real_threat_found == 0 && !cfg->quiet) {
        printf("%s[+] Aucune mutation hostile Inode 0 détectée (Système intègre)%s\n",
               cfg->is_tty ? C_GREEN : "", cfg->is_tty ? C_RESET : "");
    }

    return real_threat_found ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}
