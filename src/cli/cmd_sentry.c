/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_sentry.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Interface utilisateur en ligne de commande pour la sentinelle
 *                   anti-rançongiciel ('rap sentry'). Gestion de l'armement,
 *                   du contrôle d'intégrité des canaris, du détachement furtif,
 *                   de l'armement du moteur réflexe et de la boîte noire.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Intégration des drapeaux d'armement réflexe active :
 *       '--arm' ou '-a'       : Armement réel (SIGSTOP, coupure socket, SIGKILL).
 *       '--dry-run'           : Simulation de riposte avec trace en boîte noire.
 *       (Par défaut)          : Surveillance passive (rejet d'accès FAN_DENY seul).
 *   - Initialisation explicite du moteur rap_reflex_init() lors du lancement.
 *   - Enrichissement de 'cmd_sentry_status' avec l'état du moteur réflexe.
 *   - Intégration de la sous-commande 'dump' pour lire la boîte noire en mémoire vive.
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *   116. cmd_sentry_arm    : Déploie les canaris dans les cibles spécifiées ou par défaut.
 *   117. cmd_sentry_disarm : Nettoie et supprime physiquement les fichiers canaris.
 *   118. cmd_sentry_status : Contrôle la présence des leurres et l'état du réflexe.
 *   119. cmd_sentry_run    : Lance le guet fanotify avec le niveau d'armement réflexe choisi.
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
#include "rap_reflex.h"
#include "rap_blackbox.h"

/* Palette ANSI souveraine */
#define ANSI_RED      "\033[1;31m"
#define ANSI_GREEN    "\033[1;32m"
#define ANSI_YELLOW   "\033[1;33m"
#define ANSI_CYAN     "\033[1;36m"
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
 * Déploie les fichiers canaris dans les répertoires fournis en argument.
 * ============================================================================
 */
static int cmd_sentry_arm(int argc, char **argv)
{
    printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Déploiement des fichiers canaris denses...\n");

    int deployed = 0;
    int failed = 0;

    if (argc <= 2) {
        for (size_t i = 0; i < DEFAULT_TARGETS_COUNT; ++i) {
            if (access(DEFAULT_TARGETS[i], F_OK) == 0) {
                if (sentry_deploy_canary(DEFAULT_TARGETS[i]) == 0) {
                    printf("  " ANSI_GREEN "[ARMÉ]" ANSI_RESET " %s/%s\n", DEFAULT_TARGETS[i], SENTRY_CANARY_NAME);
                    deployed++;
                } else {
                    printf("  " ANSI_RED "[ÉCHEC]" ANSI_RESET " %s (droits root requis ?)\n", DEFAULT_TARGETS[i]);
                    failed++;
                }
            }
        }
    } else {
        for (int i = 2; i < argc; ++i) {
            if (sentry_deploy_canary(argv[i]) == 0) {
                printf("  " ANSI_GREEN "[ARMÉ]" ANSI_RESET " %s/%s\n", argv[i], SENTRY_CANARY_NAME);
                deployed++;
            } else {
                printf("  " ANSI_RED "[ÉCHEC]" ANSI_RESET " %s (chemin invalide ou droits insuffisants)\n", argv[i]);
                failed++;
            }
        }
    }

    printf("\nBilan : " ANSI_GREEN "%d canari(s) actif(s)" ANSI_RESET ", " ANSI_YELLOW "%d échec(s)" ANSI_RESET ".\n", deployed, failed);
    return (deployed > 0) ? 0 : 2;
}

/* ============================================================================
 * 117. cmd_sentry_disarm
 * Supprime proprement les fichiers leurres.
 * ============================================================================
 */
static int cmd_sentry_disarm(int argc, char **argv)
{
    printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Désarmement et purge des canaris...\n");

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

    printf("Total canaris neutralisés : %d\n", removed);
    return 0;
}

/* ============================================================================
 * 118. cmd_sentry_status
 * Vérifie la présence des leurres et consulte l'état du moteur réflexe.
 * ============================================================================
 */
static int cmd_sentry_status(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Audit d'intégrité des leurres anti-rançongiciel :\n\n");
    printf("  %-35s %-12s %-10s\n", "RÉPERTOIRE CIBLE", "STATUT", "TAILLE");
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
                printf("  %-35s " ANSI_RED "[ALTÉRÉ]" ANSI_RESET "    %lu octets (suspect !)\n", DEFAULT_TARGETS[i], (unsigned long)st.st_size);
            }
        } else {
            printf("  %-35s " ANSI_DIM "[NON ARMÉ]" ANSI_RESET "  0 octet\n", DEFAULT_TARGETS[i]);
        }
    }

    uint64_t neutralized = 0;
    reflex_mode_t mode = rap_reflex_get_stats(&neutralized);
    printf("\n" ANSI_CYAN "[MOTEUR RÉFLEXE]" ANSI_RESET " État opérationnel :\n");
    printf("  -> Mode d'action     : %s\n", 
           (mode == REFLEX_MODE_ARMED)  ? ANSI_RED "ARMÉ (Riposte immédiate SIGKILL)" ANSI_RESET :
           (mode == REFLEX_MODE_DRYRUN) ? ANSI_YELLOW "SIMULATION (Dry-Run)" ANSI_RESET :
                                          ANSI_GREEN "PASSIF (Alerte et rejet seul)" ANSI_RESET);
    printf("  -> Neutralisations   : %lu processus abattu(s)\n", (unsigned long)neutralized);
    printf("  -> Boîte Noire RAM   : %s\n\n", RAP_BLACKBOX_SHM_PATH);

    return 0;
}

/* ============================================================================
 * 119. cmd_sentry_run
 * Démarre le sous-système de surveillance passive noyau fanotify.
 * Configure le niveau d'intervention de rap_reflex.
 * ============================================================================
 */
static int cmd_sentry_run(int argc, char **argv)
{
    SentryConfig config;
    memset(&config, 0, sizeof(config));
    config.auto_freeze = true;

    reflex_mode_t reflex_mode = REFLEX_MODE_PASSIVE;

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0) {
            config.daemon_mode = true;
        } else if (strcmp(argv[i], "--arm") == 0 || strcmp(argv[i], "-a") == 0) {
            reflex_mode = REFLEX_MODE_ARMED;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            reflex_mode = REFLEX_MODE_DRYRUN;
        } else if (config.target_count < SENTRY_MAX_TARGETS) {
            strncpy(config.target_dirs[config.target_count], argv[i], SENTRY_PATH_MAX - 1);
            config.target_count++;
        }
    }

    /* Initialisation du moteur réflexe chirurgical */
    if (rap_reflex_init(reflex_mode) != 0) {
        fprintf(stderr, ANSI_RED "[AVERTISSEMENT]" ANSI_RESET " Impossible d'attacher la boîte noire en RAM.\n");
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

    /* Initialisation du descripteur fanotify */
    int fan_fd = sentry_init_fanotify(&config);
    if (fan_fd < 0) {
        fprintf(stderr, ANSI_RED "[ERREUR]" ANSI_RESET " Impossible d'armer fanotify. Privilèges root (CAP_SYS_ADMIN) requis.\n");
        return 2;
    }

    /* Bascule en arrière-plan si mode démon demandé */
    if (config.daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) {
            close(fan_fd);
            return 2;
        }
        if (pid > 0) {
            printf(ANSI_GREEN "[SENTINELLE DISGUISÉE]" ANSI_RESET " Guet actif en arrière-plan (PID : %d).\n", (int)pid);
            return 0;
        }

        if (setsid() < 0) {
            close(fan_fd);
            return 2;
        }

        sentry_enter_stealth("[kworker/u16:3]");

        int dev_null = open("/dev/null", O_RDWR);
        if (dev_null >= 0) {
            dup2(dev_null, STDIN_FILENO);
            dup2(dev_null, STDOUT_FILENO);
            close(dev_null);
        }
    } else {
        printf(ANSI_RED "[RAPTOR SENTRY]" ANSI_RESET " Guet passif en direct activé (0%% CPU). Interception sur FAN_OPEN_PERM...\n");
        printf("  -> Action réflexe    : %s\n",
               (reflex_mode == REFLEX_MODE_ARMED)  ? ANSI_RED "ARMÉE (Neutralisation active SIGKILL)" ANSI_RESET :
               (reflex_mode == REFLEX_MODE_DRYRUN) ? ANSI_YELLOW "SIMULATION (Dry-Run)" ANSI_RESET :
                                                     ANSI_GREEN "PASSIVE (Alerte seule)" ANSI_RESET);
        printf(ANSI_DIM "Appuyez sur Ctrl+C pour interrompre le guet.\n\n" ANSI_RESET);
    }

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
        printf("Usage : rap sentry <arm | disarm | status | run | dump> [options] [chemins...]\n");
        printf("\nCommandes de la sentinelle anti-rançongiciel :\n");
        printf("  arm     [chemins...]           : Déploie les leurres denses (32 Ko, signature KeePass).\n");
        printf("  disarm  [chemins...]           : Supprime proprement les fichiers canaris.\n");
        printf("  status                         : Vérifie la présence des leurres et l'état du réflexe.\n");
        printf("  dump                           : Affiche les enregistrements de la boîte noire (/dev/shm).\n");
        printf("  run     [-d] [--arm] [--dry-run] [dirs...] :\n");
        printf("            -d, --daemon         : Détachement en démon furtif d'arrière-plan.\n");
        printf("            -a, --arm            : Armement chirurgical (paralyse et élimine le processus agresseur).\n");
        printf("            --dry-run            : Simule la neutralisation sans altérer le processus suspect.\n");
        return 2;
    }

    const char *sub = argv[1];

    if (strcmp(sub, "arm") == 0) {
        return cmd_sentry_arm(argc, argv);
    } else if (strcmp(sub, "disarm") == 0) {
        return cmd_sentry_disarm(argc, argv);
    } else if (strcmp(sub, "status") == 0) {
        return cmd_sentry_status(argc, argv);
    } else if (strcmp(sub, "dump") == 0 || strcmp(sub, "blackbox") == 0) {
        return rap_blackbox_dump();
    } else if (strcmp(sub, "run") == 0) {
        return cmd_sentry_run(argc, argv);
    }

    fprintf(stderr, ANSI_RED "[ERREUR]" ANSI_RESET " Sous-commande sentry inconnue : '%s'\n", sub);
    return 2;
}
