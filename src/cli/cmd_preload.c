/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_preload.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon de traque du chargeur dynamique ('rap preload').
 *                   Détecte les rootkits userland via le contrôle passif de
 *                   /etc/ld.so.preload et l'extraction chirurgicale des variables
 *                   d'environnement (LD_PRELOAD) dans /proc/[PID]/environ.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 93)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   93. rap_cmd_preload
 * ============================================================================
 */

#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include "rap_cli.h"

/* ============================================================================
 * FONCTION 93 : rap_cmd_preload
 * ----------------------------------------------------------------------------
 * Exécute l'audit double palier du chargeur dynamique Linux :
 *  1. Palier Global : Vérifie la présence physique de /etc/ld.so.preload.
 *  2. Palier Unitaire : Lit les blocs mémoires délimités par des octets nuls
 *     dans /proc/[PID]/environ pour intercepter les injections furtives.
 * ============================================================================
 */
int rap_cmd_preload(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    int threat_found = 0;

    /* Étape 1 : Contrôle de l'infection globale du système */
    if (access("/etc/ld.so.preload", F_OK) == 0) {
        threat_found++;
        if (!cfg->quiet) {
            fprintf(stderr, "%s[ALERTE CRITIQUE] Fichier actif : /etc/ld.so.preload (Rootkit global potentiel)%s\n",
                    cfg->is_tty ? C_RED : "", cfg->is_tty ? C_RESET : "");
        } else {
            printf("GLOBAL\t/etc/ld.so.preload\tACTIF\n");
        }
    }

    /* Étape 2 : Inspection des processus actifs */
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("rap preload: ouverture /proc impossible");
        return RAP_EXIT_ERROR;
    }

    if (!cfg->quiet) {
        rap_output_formatter(cfg, "PID", "NOM", "BIBLIOTHÈQUE INJECTÉE", "STATUT");
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

        char env_path[64];
        snprintf(env_path, sizeof(env_path), "/proc/%d/environ", pid);
        int fd = open(env_path, O_RDONLY);
        if (fd < 0) {
            continue;
        }

        char env_buf[2048];
        ssize_t n = read(fd, env_buf, sizeof(env_buf) - 1);
        close(fd);

        if (n <= 0) {
            continue;
        }
        env_buf[n] = '\0';

        /* Découpage sur les octets nuls de l'environnement */
        char *ptr = env_buf;
        while (ptr < env_buf + n) {
            if (strncmp(ptr, "LD_PRELOAD=", 11) == 0) {
                threat_found++;
                char *lib_path = ptr + 11;

                char spid[16], scomm[64] = "inconnu";
                snprintf(spid, sizeof(spid), "%d", pid);

                /* Résolution du nom court de la cible */
                char comm_path[64];
                snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);
                FILE *fcomm = fopen(comm_path, "r");
                if (fcomm) {
                    if (fgets(scomm, sizeof(scomm), fcomm)) {
                        scomm[strcspn(scomm, "\r\n")] = '\0';
                    }
                    fclose(fcomm);
                }

                rap_output_formatter(cfg, spid, scomm, lib_path, "HIJACK LD_PRELOAD");
                break;
            }
            ptr += strlen(ptr) + 1;
        }
    }
    closedir(dir);

    if (threat_found == 0 && !cfg->quiet) {
        printf("%s[+] Aucun détournement LD_PRELOAD détecté sur les processus actifs%s\n",
               cfg->is_tty ? C_GREEN : "", cfg->is_tty ? C_RESET : "");
    }

    return threat_found ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}
