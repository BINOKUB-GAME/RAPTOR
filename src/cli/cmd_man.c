/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_man.c
 * RÉVISION        : Rev 1.6
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Affichage paginé autonome du manuel de référence (.rodata)
 *                   et navigation hiérarchique dans le dictionnaire des fonctions.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.6) :
 *   - Recherche robuste de 'fn' / 'fonct' / 'functions' sur toute la ligne argv.
 *   - Immunité totale contre les décalages d'index (rap man fn vs ./build/rap man fn).
 *   - Restitution paginée intégrale du dictionnaire brut (rap man fn).
 *   - Filtrage chirurgical : Section complète (fn_id=0) ou Fonction unitaire.
 * ============================================================================
 */

#include "rap_cli.h"
#include "rap_man_data.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * FONCTION 122 : rap_man_render
 * ============================================================================
 */
static void rap_man_render(const char *text, size_t len, const RapConfig *cfg, bool force_direct)
{
    if (!text || len == 0) return;

    /* Sortie directe : mode script/pipe, drapeau --raw, ou affichage court */
    if (force_direct || !cfg || !cfg->is_tty || cfg->raw) {
        write(STDOUT_FILENO, text, len);
        return;
    }

    /* Sortie interactive TTY : aiguillage vers le téléscripteur rapless */
    FILE *pager = popen("rapless 2>/dev/null || ./build/rapless 2>/dev/null || less -R 2>/dev/null", "w");
    if (!pager) {
        write(STDOUT_FILENO, text, len);
        return;
    }

    fwrite(text, 1, len, pager);
    pclose(pager);
}

/* ============================================================================
 * FONCTION 123 : rap_man_handle_fn
 * ============================================================================
 */
static int rap_man_handle_fn(int argc, char **argv, const RapConfig *cfg)
{
    /* Cas 1 : 'rap man fn' sans numéro -> Dictionnaire intégral */
    if (argc < 2) {
        rap_man_render(g_rap_fn_full_data, sizeof(g_rap_fn_full_data) - 1, cfg, false);
        return RAP_EXIT_OK;
    }

    int p1 = atoi(argv[1]);

    /* Cas 2 : 'rap man fn <sec> <fn>' (ex: rap man fn 17 114 ou rap man fn 2 16) */
    if (argc >= 3) {
        int p2 = atoi(argv[2]);
        for (size_t i = 0; i < g_rap_fn_entries_count; ++i) {
            if (g_rap_fn_entries[i].section == p1 && g_rap_fn_entries[i].fn_id == p2) {
                rap_man_render(g_rap_fn_entries[i].content, strlen(g_rap_fn_entries[i].content), cfg, true);
                write(STDOUT_FILENO, "\n", 1);
                return RAP_EXIT_OK;
            }
        }
        fprintf(stderr, "[ERREUR] Fonction %d introuvable dans la Section %d.\n", p2, p1);
        return RAP_EXIT_ERROR;
    }

    /* Cas 3 : 'rap man fn <valeur>' (un seul numéro passé) */
    /* 3a. Est-ce un numéro de section (fn_id == 0) ? */
    for (size_t i = 0; i < g_rap_fn_entries_count; ++i) {
        if (g_rap_fn_entries[i].section == p1 && g_rap_fn_entries[i].fn_id == 0) {
            rap_man_render(g_rap_fn_entries[i].content, strlen(g_rap_fn_entries[i].content), cfg, false);
            return RAP_EXIT_OK;
        }
    }

    /* 3b. Est-ce un numéro de fonction direct ? */
    for (size_t i = 0; i < g_rap_fn_entries_count; ++i) {
        if (g_rap_fn_entries[i].fn_id == p1) {
            rap_man_render(g_rap_fn_entries[i].content, strlen(g_rap_fn_entries[i].content), cfg, true);
            write(STDOUT_FILENO, "\n", 1);
            return RAP_EXIT_OK;
        }
    }

    fprintf(stderr, "[ERREUR] Aucune section ou fonction ne correspond au numéro '%s'.\n", argv[1]);
    return RAP_EXIT_ERROR;
}

/* ============================================================================
 * FONCTION 99 : rap_cmd_man
 * ============================================================================
 */
int rap_cmd_man(int argc, char **argv, const RapConfig *cfg)
{
    /* Recherche dynamique de 'fn', 'fonct' ou 'functions' n'importe où dans argv */
    int fn_idx = -1;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "fn") == 0 ||
            strcmp(argv[i], "fonct") == 0 ||
            strcmp(argv[i], "functions") == 0) {
            fn_idx = i;
            break;
        }
    }

    if (fn_idx >= 0) {
        return rap_man_handle_fn(argc - fn_idx, argv + fn_idx, cfg);
    }

    /* Affichage du manuel CLI standard */
    rap_man_render(g_rap_man_data, sizeof(g_rap_man_data) - 1, cfg, false);
    return RAP_EXIT_OK;
}
