/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_sentry.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Interface utilisateur en ligne de commande pour la sentinelle
 *                   anti-rançongiciel ('rap sentry'). Gestion de l'armement,
 *                   du contrôle d'intégrité des canaris, du détachement furtif
 *                   et de la boucle de veille passive noyau.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Intégration des sous-commandes 'arm', 'disarm', 'status' et 'run'.
 *   - Prise en charge des répertoires par défaut critiques (/home, /var/www, /var/lib, /etc).
 *   - Mode démon autonome (-d / --daemon) avec détachement de session (setsid/fork)
 *     et redirection silencieuse des flux standards.
 *   - Zéro allocation dynamique (0 heap, structures bornées sur la pile).
 *   - Codes de sortie POSIX normalisés (0 = Nominal, 1 = Alerte, 2 = Erreur/Droits).
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *   116. cmd_sentry_arm    : Déploie les canaris dans les cibles spécifiées ou par défaut.
 *   117. cmd_sentry_disarm : Nettoie et supprime physiquement les fichiers canaris.
 *   118. cmd_sentry_status : Contrôle la présence et l'état des leurres sur le disque.
 *   119. cmd_sentry_run    : Lance le guet passif fanotify (interactif ou démon furtif).
 *   120. cmd_sentry        : Point d'entrée principal et aiguilleur d'arguments CLI.
 * ============================================================================
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../engine/sentry_engine.h"

/* Palette ANSI souveraine */
#define ANSI_RED      "\033[1;31m"
#define ANSI_GREEN    "\033[1;32m"
#define ANSI_YELLOW   "\033[1;33m"
#define ANSI_DIM      "\033[2m"
#define ANSI_RESET    "\033[0m"

static const char *DEFAULT_TARGETS[] = {
    "/home",
    "/var/www",
    "/var/lib",
    "/etc"
};
#define DEFAULT_TARGETS_COUNT (sizeof(DEFAULT_TARGETS) / sizeof(DEFAULT_TARGETS[0]))

/* ============================================================================
 * 116. cmd_sentry_arm
 * Déploie les fichiers canaris dans les répertoires fournis en argument,
 * ou applique les répertoires par défaut du système si aucun chemin n'est passé.
 * ============================================================================
 */
static int cmd_sentry_arm(int argc, char **argv)
{
    printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Armement des fichiers canaris denses...\n");

    int deployed = 0;
    int failed = 0;

    if (argc <= 2) {
        /* Utilisation des cibles par défaut */
        for (size_t i = 0; i < DEFAULT_TARGETS_COUNT; ++i) {
            if (access(DEFAULT_TARGETS[i], F_OK) == 0) {
                if (sentry_deploy_canary(DEFAULT_TARGETS[i]) == 0) {
                    printf("  " ANSI_GREEN "[ARME]" ANSI_RESET " %s/%s\n", DEFAULT_TARGETS[i], SENTRY_CANARY_NAME);
                    deployed++;
                } else {
                    printf("  " ANSI_RED "[ECHEC]" ANSI_RESET " %s (droits root requis ?)\n", DEFAULT_TARGETS[i]);
                    failed++;
                }
            }
        }
    } else {
        /* Cibles manuelles fournies en ligne de commande */
        for (int i = 2; i < argc; ++i) {
            if (sentry_deploy_canary(argv[i]) == 0) {
                printf("  " ANSI_GREEN "[ARME]" ANSI_RESET " %s/%s\n", argv[i], SENTRY_CANARY_NAME);
                deployed++;
            } else {
                printf("  " ANSI_RED "[ECHEC]" ANSI_RESET " %s (chemin invalide ou droits insuffisants)\n", argv[i]);
                failed++;
            }
        }
    }

    printf("\nBilan : " ANSI_GREEN "%d canari(s) actif(s)" ANSI_RESET ", " ANSI_YELLOW "%d echec(s)" ANSI_RESET ".\n", deployed, failed);
    return (deployed > 0) ? 0 : 2;
}

/* ============================================================================
 * 117. cmd_sentry_disarm
 * Supprime proprement les fichiers leurres pour préparer une maintenance
 * ou désactiver la sentinelle sans déclencher de fausse alerte.
 * ============================================================================
 */
static int cmd_sentry_disarm(int argc, char **argv)
{
    printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Desarmement et purge des canaris...\n");

    char canary_path[SENTRY_PATH_MAX];
    int removed = 0;

    if (argc <= 2) {
        for (size_t i = 0; i < DEFAULT_TARGETS_COUNT; ++i) {
            snprintf(canary_path, sizeof(canary_path), "%s/%s", DEFAULT_TARGETS[i], SENTRY_CANARY_NAME);
            if (unlink(canary_path) == 0) {
                printf("  " ANSI_YELLOW "[PURGE]" ANSI_RESET " %s\n", canary_path);
                removed++;
            }
        }
    } else {
        for (int i = 2; i < argc; ++i) {
            snprintf(canary_path, sizeof(canary_path), "%s/%s", argv[i], SENTRY_CANARY_NAME);
            if (unlink(canary_path) == 0) {
                printf("  " ANSI_YELLOW "[PURGE]" ANSI_RESET " %s\n", canary_path);
                removed++;
            }
        }
    }

    printf("Total canaris neutralises : %d\n", removed);
    return 0;
}

/* ============================================================================
 * 118. cmd_sentry_status
 * Vérifie l'existence et la taille physique des appâts sur le disque.
 * ============================================================================
 */
static int cmd_sentry_status(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Audit d'integrite des leurres anti-rancongiciel :\n\n");
    printf("  %-35s %-12s %-10s\n", "REPERTOIRE CIBLE", "STATUT", "TAILLE");
    printf("  -------------------------------------------------------------\n");

    char canary_path[SENTRY_PATH_MAX];
    struct stat st;

    for (size_t i = 0; i < DEFAULT_TARGETS_COUNT; ++i) {
        if (access(DEFAULT_TARGETS[i], F_OK) != 0) {
            continue;
        }

        snprintf(canary_path, sizeof(canary_path), "%s/%s", DEFAULT_TARGETS[i], SENTRY_CANARY_NAME);

        if (stat(canary_path, &st) == 0) {
            if (st.st_size == SENTRY_CANARY_SIZE) {
                printf("  %-35s " ANSI_GREEN "[ACTIF]" ANSI_RESET "     %lu octets\n", DEFAULT_TARGETS[i], (unsigned long)st.st_size);
            } else {
                printf("  %-35s " ANSI_RED "[ALTERE]" ANSI_RESET "    %lu octets (suspect !)\n", DEFAULT_TARGETS[i], (unsigned long)st.st_size);
            }
        } else {
            printf("  %-35s " ANSI_DIM "[NON ARME]" ANSI_RESET "  0 octet\n", DEFAULT_TARGETS[i]);
        }
    }
    printf("\n");
    return 0;
}

/* ============================================================================
 * 119. cmd_sentry_run
 * Démarre le sous-système de surveillance passive noyau fanotify.
 * Si le drapeau -d est présent, détache le processus en démon d'arrière-plan,
 * active le camouflage prctl, mlockall et l'immunité OOM.
 * ============================================================================
 */
static int cmd_sentry_run(int argc, char **argv)
{
    SentryConfig config;
    memset(&config, 0, sizeof(config));
    config.auto_freeze = true;

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0) {
            config.daemon_mode = true;
        } else if (config.target_count < SENTRY_MAX_TARGETS) {
            strncpy(config.target_dirs[config.target_count], argv[i], SENTRY_PATH_MAX - 1);
            config.target_count++;
        }
    }

    /* Remplissage par défaut si aucun chemin explicite */
    if (config.target_count == 0) {
        for (size_t i = 0; i < DEFAULT_TARGETS_COUNT && config.target_count < SENTRY_MAX_TARGETS; ++i) {
            if (access(DEFAULT_TARGETS[i], F_OK) == 0) {
                strncpy(config.target_dirs[config.target_count], DEFAULT_TARGETS[i], SENTRY_PATH_MAX - 1);
                config.target_count++;
            }
        }
    }

    /* Initialisation du descripteur fanotify avant détachement */
    int fan_fd = sentry_init_fanotify(&config);
    if (fan_fd < 0) {
        fprintf(stderr, ANSI_RED "[ERREUR]" ANSI_RESET " Impossible d'armer fanotify. Privileges root (CAP_SYS_ADMIN) requis.\n");
        return 2;
    }

    /* Bascule en arrière-plan (Démon furtif) */
    if (config.daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) {
            close(fan_fd);
            return 2;
        }
        if (pid > 0) {
            /* Processus père : affiche la confirmation et quitte */
            printf(ANSI_GREEN "[SENTINELLE DISGUEE]" ANSI_RESET " Guet actif en arriere-plan (PID Sentinelle : %d).\n", (int)pid);
            return 0;
        }

        /* Processus fils : émancipation de session */
        if (setsid() < 0) {
            close(fan_fd);
            return 2;
        }

        /* Camouflage dans la table des processus et blindage mémoire */
        sentry_enter_stealth("[kworker/u16:3]");

        /* Redirection des descripteurs standards pour libérer le terminal */
        int dev_null = open("/dev/null", O_RDWR);
        if (dev_null >= 0) {
            dup2(dev_null, STDIN_FILENO);
            dup2(dev_null, STDOUT_FILENO);
            close(dev_null);
        }
    } else {
        printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Guet passif en direct active (0%% CPU). Interception sur FAN_OPEN_PERM...\n");
        printf(ANSI_DIM "Appuyez sur Ctrl+C pour interrompre le guet.\n\n" ANSI_RESET);
    }

    /* Boucle de veille infinie */
    int ret = sentry_listen_loop(fan_fd, &config);
    close(fan_fd);
    return ret;
}

/* ============================================================================
 * 120. cmd_sentry
 * Point d'entrée de la commande 'rap sentry'.
 * ============================================================================
 */
int cmd_sentry(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage : rap sentry <arm | disarm | status | run [-d]> [chemins...]\n");
        printf("\nCommandes de la sentinelle anti-rancongiciel :\n");
        printf("  arm     [chemins...] : Depose les leurres denses (32 Ko, signature KeePass).\n");
        printf("  disarm  [chemins...] : Supprime proprement les fichiers canaris.\n");
        printf("  status               : Verifie la presence et l'integrite physique des leurres.\n");
        printf("  run     [-d] [dirs...] : Enclenche le guet passif (option -d en demon furtif).\n");
        return 2;
    }

    const char *sub = argv[1];

    if (strcmp(sub, "arm") == 0) {
        return cmd_sentry_arm(argc, argv);
    } else if (strcmp(sub, "disarm") == 0) {
        return cmd_sentry_disarm(argc, argv);
    } else if (strcmp(sub, "status") == 0) {
        return cmd_sentry_status(argc, argv);
    } else if (strcmp(sub, "run") == 0) {
        return cmd_sentry_run(argc, argv);
    }

    fprintf(stderr, ANSI_RED "[ERREUR]" ANSI_RESET " Sous-commande sentry inconnue : '%s'\n", sub);
    return 2;
}
