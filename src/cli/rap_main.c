/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rap_main.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Point d'entrée principal et micro-dispatcher déclaratif.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Enregistrement de la commande 'heur' (alias 'he') dans g_commands.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonctions 88 & 100)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *    88. rap_dispatch_command
 *   100. rap_parse_flags
 * ============================================================================
 */

#include "rap_cli.h"

static const RapCliCommand g_commands[] = {
    {"status",  "st", "Bilan vital instantané de la machine hôte", rap_cmd_status},
    {"top",     NULL, "Top des consommateurs de mémoire vive résidente (VmRSS)", rap_cmd_top},
    {"hunt",    NULL, "Audit combiné complet (Mémoire, Preload, Heuristiques)", rap_cmd_hunt},
    {"rx",      NULL, "Traque des mutations anonymes R-X Inode 0", rap_cmd_rx},
    {"preload", NULL, "Traque des détournements LD_PRELOAD", rap_cmd_preload},
    {"heur",    "he", "Analyse heuristique (chemins volatils, binaires supprimés, usurpations)", rap_cmd_heur},
    {"net",     NULL, "Audit des communications réseau TCP établies", rap_cmd_net},
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
        } else if (strcmp(arg, "-n") == 0) {
            if (i + 1 < *argc) {
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
