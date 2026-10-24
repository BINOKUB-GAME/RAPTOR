/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_heur.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon d'analyse heuristique des attaques ('rap heur').
 *                   Inspecte les liens /proc/[PID]/exe pour traquer :
 *                   - Les binaires supprimés du disque pendant l'exécution (deleted).
 *                   - Les exécutions clandestines depuis les zones temporaires (/tmp, /dev/shm).
 *                   - L'usurpation d'identité de démons système critiques (Masquerading).
 *                   Opère sans allocation dynamique (0 heap, buffers pile bornés).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Neutralisation des faux positifs de masquerading lorsque /proc/[PID]/exe
 *     est inaccessible (exécution en utilisateur non-root sans sudo).
 *   - Élargissement des préfixes d'autorité système (/usr/, /bin/, /sbin/, /lib/)
 *     pour intégrer les distributions de type Debian / Linux Mint (/lib/systemd/).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonctions 109 et 110)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   109. rap_cmd_heur
 *   110. rap_heur_check_process
 * ============================================================================
 */

#include <dirent.h>
#include <ctype.h>
#include "rap_cli.h"

/* ============================================================================
 * FONCTION 110 : rap_heur_check_process
 * ----------------------------------------------------------------------------
 * Évalue unitairement la posture de sécurité d'un exécutable :
 *  - Détection (deleted) : l'attaquant efface le fichier pour masquer ses traces.
 *  - Détection chemins volatils : /tmp, /var/tmp, /dev/shm, /run/user.
 *  - Détection usurpation : un binaire nommé 'systemd', 'sshd' ou 'kworker'
 *    qui s'exécute hors des répertoires d'autorité (/usr, /bin, /sbin, /lib).
 * ============================================================================
 */
int rap_heur_check_process(pid_t pid, const char *comm, const char *exe_path, char *out_anomaly, size_t out_sz)
{
    (void)pid;

    /* Si le chemin est inaccessible (droits root restreints sans sudo), on ignore */
    if (!exe_path || exe_path[0] == '\0') {
        return 0;
    }

    /* 1. Traque des exécutables fantômes (supprimés du disque après lancement) */
    if (strstr(exe_path, " (deleted)")) {
        snprintf(out_anomaly, out_sz, "BINAIRE FANTÔME (Supprimé sur disque)");
        return 1;
    }

    /* 2. Traque des chemins volatils suspects */
    if (strncmp(exe_path, "/tmp/", 5) == 0 ||
        strncmp(exe_path, "/var/tmp/", 9) == 0 ||
        strncmp(exe_path, "/dev/shm/", 9) == 0 ||
        strstr(exe_path, "/run/user/") != NULL) {
        snprintf(out_anomaly, out_sz, "CHEMIN VOLATIL SUSPECT (/tmp, /dev/shm...)");
        return 1;
    }

    /* 3. Traque de l'usurpation de threads noyau (kworker avec binaire utilisateur) */
    if (strncmp(comm, "kworker/", 8) == 0 ||
        strncmp(comm, "ksoftirqd/", 10) == 0 ||
        strncmp(comm, "migration/", 10) == 0) {
        snprintf(out_anomaly, out_sz, "USURPATION NOYAU (Faux thread kworker)");
        return 1;
    }

    /* 4. Traque de l'usurpation de démons système critiques */
    static const char * const s_system_daemons[] = {
        "systemd", "sshd", "cron", "crond", "rsyslogd", "udevd", "init", NULL
    };

    for (int i = 0; s_system_daemons[i] != NULL; i++) {
        if (strcmp(comm, s_system_daemons[i]) == 0) {
            /* Un démon système officiel réside sous /usr, /bin, /sbin ou /lib */
            if (strncmp(exe_path, "/usr/", 5) != 0 &&
                strncmp(exe_path, "/bin/", 5) != 0 &&
                strncmp(exe_path, "/sbin/", 6) != 0 &&
                strncmp(exe_path, "/lib/", 5) != 0) {
                snprintf(out_anomaly, out_sz, "USURPATION SYSTÈME (Masquerading %s)", comm);
                return 1;
            }
        }
    }

    return 0;
}

/* ============================================================================
 * FONCTION 109 : rap_cmd_heur
 * ----------------------------------------------------------------------------
 * Parcourt /proc pour traquer l'ensemble des anomalies comportementales :
 *  - Lit le lien symbolique /proc/[PID]/exe via readlink.
 *  - Applique l'analyseur rap_heur_check_process sur chaque cible.
 *  - Formate les alertes avec coloration ANSI et retourne RAP_EXIT_THREAT si requis.
 * ============================================================================
 */
int rap_cmd_heur(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("rap heur: ouverture /proc impossible");
        return RAP_EXIT_ERROR;
    }

    if (!cfg->quiet) {
        rap_output_formatter(cfg, "PID", "NOM", "CHEMIN EXÉCUTABLE", "ANOMALIE HEURISTIQUE");
        if (cfg->is_tty && !cfg->raw) {
            printf(" --------------------------------------------------------------------------------\n");
        }
    }

    struct dirent *de;
    int threat_found = 0;

    while ((de = readdir(dir)) != NULL) {
        if (!isdigit(de->d_name[0])) {
            continue;
        }

        pid_t pid = (pid_t)atoi(de->d_name);

        /* Résolution du nom court (comm) */
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

        /* Résolution du lien symbolique de l'exécutable */
        char exe_link[64];
        char real_exe[256] = {0};
        snprintf(exe_link, sizeof(exe_link), "/proc/%d/exe", pid);
        ssize_t len = readlink(exe_link, real_exe, sizeof(real_exe) - 1);
        if (len > 0) {
            real_exe[len] = '\0';
        }

        char anomaly[128] = {0};
        if (rap_heur_check_process(pid, scomm, real_exe, anomaly, sizeof(anomaly))) {
            threat_found++;
            char spid[16];
            snprintf(spid, sizeof(spid), "%d", pid);

            char formatted_anomaly[160];
            if (cfg->is_tty && !cfg->raw) {
                snprintf(formatted_anomaly, sizeof(formatted_anomaly), "%s%s%s", C_RED, anomaly, C_RESET);
            } else {
                snprintf(formatted_anomaly, sizeof(formatted_anomaly), "%s", anomaly);
            }

            rap_output_formatter(cfg, spid, scomm, real_exe, formatted_anomaly);
        }
    }
    closedir(dir);

    if (threat_found == 0 && !cfg->quiet) {
        printf("%s[+] Aucune anomalie heuristique détectée (Arborescence et binaires sains)%s\n",
               cfg->is_tty ? C_GREEN : "", cfg->is_tty ? C_RESET : "");
    }

    return threat_found ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}
