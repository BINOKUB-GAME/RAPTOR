/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rap_main.c
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Point d'entrée principal et micro-dispatcher déclaratif.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Enregistrement déclaratif de la commande 'netop' (alias 'no') dans g_commands.
 *   - Mise à niveau de rap_parse_flags (Fonction 100) pour accepter indifféremment
 *     la syntaxe collée -nXX (ex: -n20) et la syntaxe espacée -n XX.
 *   - Maintien du pont sentinelle (rap_cmd_sentry) et de l'affichage d'urgence.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonctions 88 & 100, Liaison Sentinelle & Netop)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *    88. rap_dispatch_command : Aiguillage déclaratif vers le gestionnaire de commande.
 *   100. rap_parse_flags      : Analyse et épuration des drapeaux de la ligne de commande.
 *   121. rap_cmd_sentry       : Pont d'aiguillage vers la sentinelle anti-rançongiciel.
 * ============================================================================
 */

#include "rap_cli.h"

/* Déclaration externe du point d'entrée sentinelle (src/cli/cmd_sentry.c) */
extern int cmd_sentry(int argc, char **argv);

/* ============================================================================
 * FONCTION 121 : rap_cmd_sentry
 * Pont d'adaptation entre le dispatcher déclaratif et le gestionnaire de sous-commandes.
 * ============================================================================
 */
int rap_cmd_sentry(int argc, char **argv, const RapConfig *cfg)
{
    (void)cfg;
    if (argc < 2) {
        return cmd_sentry(1, argv);
    }
    return cmd_sentry(argc - 1, &argv[1]);
}

static const RapCliCommand g_commands[] = {
    {"status",  "st", "Bilan vital instantané de la machine hôte", rap_cmd_status},
    {"top",     NULL, "Top des consommateurs de mémoire vive résidente (VmRSS)", rap_cmd_top},
    {"hunt",    NULL, "Audit combiné complet (Mémoire, Preload, Heuristiques)", rap_cmd_hunt},
    {"rx",      NULL, "Traque des mutations anonymes R-X Inode 0", rap_cmd_rx},
    {"preload", NULL, "Traque des détournements LD_PRELOAD", rap_cmd_preload},
    {"heur",    "he", "Analyse heuristique (chemins volatils, binaires supprimés, usurpations)", rap_cmd_heur},
    {"net",     NULL, "Audit des communications réseau TCP établies", rap_cmd_net},
    {"netop",   "no", "Console de combat réseau interactive temps réel (0-heap)", rap_cmd_netop},
    {"sentry",  "se", "Sentinelle passive anti-rançongiciel (canaris & fanotify)", rap_cmd_sentry},
    {"kill",    NULL, "Neutralisation chirurgicale immédiate par signal SIGKILL", rap_cmd_kill},
    {"freeze",  NULL, "Gel d'un processus hostile via SIGSTOP pour analyse", rap_cmd_freeze},
    {"man",     NULL, "Manuel de référence autonome embarqué (.rodata)", rap_cmd_man},
    {NULL,      NULL, NULL, NULL}
};

/* ============================================================================
 * FONCTION 100 : rap_parse_flags
 * ============================================================================
 */
int rap_parse_flags(int *argc, char ***argv, RapConfig *cfg)
{
    memset(cfg, 0, sizeof(RapConfig));
    cfg->is_tty = isatty(STDOUT_FILENO);
    cfg->limit  = 0;

    int new_argc = 1;
    char **new_argv = *argv;

    for (int i = 1; i < *argc; i++) {
        char *arg = (*argv)[i];

        if (strcmp(arg, "-q") == 0 || strcmp(arg, "--quiet") == 0) {
            cfg->quiet = 1;
        } else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--raw") == 0) {
            cfg->raw = 1;
        } else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--force") == 0) {
            cfg->force = 1;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            cfg->verbose = 1;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            rap_cmd_man(1, *argv, cfg);
            exit(RAP_EXIT_OK);
        } else if (strncmp(arg, "-n", 2) == 0) {
            if (strlen(arg) > 2) {
                cfg->limit = atoi(arg + 2);
            } else if (i + 1 < *argc) {
                cfg->limit = atoi((*argv)[++i]);
            }
        } else {
            new_argv[new_argc++] = arg;
        }
    }

    new_argv[new_argc] = NULL;
    *argc = new_argc;
    return 0;
}

/* ============================================================================
 * FONCTION 88 : rap_dispatch_command
 * ============================================================================
 */
int rap_dispatch_command(int argc, char **argv, const RapConfig *cfg)
{
    if (argc < 2) {
        return rap_cmd_man(argc, argv, cfg);
    }

    const char *subcmd = argv[1];

    for (int i = 0; g_commands[i].name != NULL; i++) {
        if (strcmp(subcmd, g_commands[i].name) == 0 ||
           (g_commands[i].alias && strcmp(subcmd, g_commands[i].alias) == 0)) {
            return g_commands[i].handler(argc, argv, cfg);
        }
    }

    fprintf(stderr, "%s[ERREUR] Commande inconnue : '%s'. Utilisez 'rap man' pour la liste des outils.%s\n",
            cfg->is_tty ? C_RED : "", subcmd, cfg->is_tty ? C_RESET : "");
    return RAP_EXIT_ERROR;
}

int main(int argc, char **argv)
{
    RapConfig cfg;
    rap_parse_flags(&argc, &argv, &cfg);
    return rap_dispatch_command(argc, argv, &cfg);
}
