/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_rx.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon d'investigation mémoire chirurgicale ('rap rx').
 *                   Inspecte les tables d'allocation virtuelle (/proc/[PID]/maps)
 *                   pour détecter les mutations clandestines anonymes Inode 0
 *                   converties en exécutable (r-xp) sans ancrage sur le disque.
 *                   Raccorde la détection active au moteur de riposte 'rap_reflex'.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Prise en charge des drapeaux d'armement réflexe active :
 *       '--arm' ou '-a'       : Neutralisation immédiate (SIGSTOP, coupure socket, SIGKILL).
 *       '--dry-run'           : Simulation de riposte avec trace en boîte noire RAM.
 *       (Par défaut)          : Surveillance passive sans altération des cibles.
 *   - Routage automatique des menaces non whitelistées vers rap_reflex_dispatch
 *     avec le motif TRIGGER_MEMORY_MUTATION.
 *   - Mise à jour contextuelle de la colonne anomalie (SIGKILL / DRY-RUN / INODE 0).
 *   - Protection anti-suicide : exclusion de PID <= 1 et du PID de rap lui-même.
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
#include <string.h>
#include <unistd.h>
#include "rap_cli.h"
#include "rap_reflex.h"
#include "rap_oom.h"
#include "rap_blackbox.h"

/* ============================================================================
 * FONCTION 92 : rap_cmd_rx
 * ----------------------------------------------------------------------------
 * Parcourt l'arborescence /proc pour traquer les pages anonymes R-X (Inode 0) :
 *  - Configure le mode réflexe (PASSIVE, ARMED, DRYRUN) selon les arguments CLI.
 *  - Filtre les composants légitimes via mem_whitelist.conf sauf si -v.
 *  - Achemine toute mutation hostile non autorisée vers rap_reflex_dispatch.
 *  - Renvoie RAP_EXIT_THREAT (1) en cas d'anomalie non listée.
 * ============================================================================
 */
int rap_cmd_rx(int argc, char **argv, const RapConfig *cfg)
{
    reflex_mode_t reflex_mode = REFLEX_MODE_PASSIVE;
    bool mode_explicit = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--arm") == 0 || strcmp(argv[i], "-a") == 0) {
            reflex_mode = REFLEX_MODE_ARMED;
            mode_explicit = true;
            break;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            reflex_mode = REFLEX_MODE_DRYRUN;
            mode_explicit = true;
            break;
        }
    }

    /* Si appelé unitairement via 'rap rx' avec un mode explicite */
    if (mode_explicit) {
        rap_reflex_init(reflex_mode);
        rap_reflex_set_mode(reflex_mode);
        if (reflex_mode == REFLEX_MODE_ARMED) {
            rap_oom_protect();
            rap_oom_lock_memory();
        }
    }

    int real_threat_found = 0;
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("rap rx: ouverture /proc impossible");
        return RAP_EXIT_ERROR;
    }

    pid_t self_pid = getpid();

    if (!cfg->quiet) {
        rap_output_formatter(cfg, "PID", "NOM", "PLAGE MÉMOIRE", "ANOMALIE");
        if (cfg->is_tty && !cfg->raw) {
            printf(" ----------------------------------------------------------------------\n");
        }
    }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (!isdigit((unsigned char)de->d_name[0])) {
            continue;
        }

        pid_t pid = (pid_t)atoi(de->d_name);

        /* Garde-fous : ne jamais analyser ni frapper PID 0, PID 1 ou le processus rap */
        if (pid <= 1 || pid == self_pid) {
            continue;
        }

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

                        /* Déclenchement de l'action réflexe (SIGSTOP -> Sockets -> Forensique -> SIGKILL) */
                        rap_reflex_dispatch(pid, TRIGGER_MEMORY_MUTATION);

                        reflex_mode_t current_mode = rap_reflex_get_stats(NULL);
                        if (current_mode == REFLEX_MODE_ARMED) {
                            rap_output_formatter(cfg, spid, scomm, srange, "MUTATION R-X [NEUTRALISÉ SIGKILL]");
                        } else if (current_mode == REFLEX_MODE_DRYRUN) {
                            rap_output_formatter(cfg, spid, scomm, srange, "MUTATION R-X [SIMULATION BOÎTE NOIRE]");
                        } else {
                            rap_output_formatter(cfg, spid, scomm, srange, "MUTATION R-X (Inode 0)");
                        }
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
    } else if (real_threat_found > 0 && !cfg->quiet && cfg->is_tty) {
        reflex_mode_t current_mode = rap_reflex_get_stats(NULL);
        if (current_mode == REFLEX_MODE_ARMED) {
            printf("\n%s[!] ALERTE RÉFLEXE : %d processus hostile(s) neutralisé(s) en mémoire vive.%s\n",
                   C_RED, real_threat_found, C_RESET);
        } else if (current_mode == REFLEX_MODE_DRYRUN) {
            printf("\n%s[*] SIMULATION RÉFLEXE : %d anomalie(s) consignée(s) dans la boîte noire RAM.%s\n",
                   C_YELLOW, real_threat_found, C_RESET);
        }
    }

    return real_threat_found ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}
