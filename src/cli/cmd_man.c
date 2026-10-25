/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_man.c
 * RÉVISION        : Rev 1.7
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Affichage paginé autonome du manuel de référence (.rodata)
 *                   et navigation hiérarchique dans le dictionnaire des fonctions.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.7) :
 *   - Filtrage chirurgical par rubrique : 'rap man sentry', 'rap man rx', etc.
 *   - Recherche tolérante avec ou sans le préfixe 'rap-' et support des alias ('st', 'se', 'he').
 *   - Restitution intégrale du manuel si aucun argument n'est fourni.
 *   - Maintien du routage dédié pour le dictionnaire des fonctions ('fn').
 * ============================================================================
 */

#include "rap_cli.h"
#include "rap_man_data.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

    /* Cas 2 : 'rap man fn <sec> <fn>' (ex: rap man fn 17 114) */
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
 * FONCTION AUXILIAIRE : Extraction du mot-clé recherché
 * ============================================================================
 */
static const char *rap_man_extract_topic(int argc, char **argv)
{
    int man_idx = -1;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "man") == 0 || strcmp(argv[i], "help") == 0) {
            man_idx = i;
            break;
        }
    }

    if (man_idx >= 0) {
        for (int i = man_idx + 1; i < argc; ++i) {
            if (argv[i][0] != '-') {
                return argv[i];
            }
        }
        return NULL;
    }

    /* Si 'man' n'est pas dans argv */
    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] != '-') {
            const char *slash = strrchr(argv[i], '/');
            const char *base = slash ? slash + 1 : argv[i];
            if (strcmp(base, "rap") != 0 && strcmp(base, "raptor") != 0) {
                return argv[i];
            }
        }
    }

    return NULL;
}

/* ============================================================================
 * FONCTION AUXILIAIRE : Recherche dans la table g_rap_man_pages[]
 * ============================================================================
 */
static const RapManPageEntry *rap_man_lookup_page(const char *topic)
{
    if (!topic || topic[0] == '\0') {
        return NULL;
    }

    /* Normalisation des alias courts */
    const char *norm = topic;
    if (strcasecmp(topic, "st") == 0) norm = "status";
    else if (strcasecmp(topic, "se") == 0) norm = "sentry";
    else if (strcasecmp(topic, "he") == 0) norm = "heur";

    /* 1. Correspondance exacte sur le nom de page (ex: "rap", "rap-sentry", "rapless") */
    for (size_t i = 0; i < g_rap_man_pages_count; ++i) {
        if (strcasecmp(g_rap_man_pages[i].name, norm) == 0) {
            return &g_rap_man_pages[i];
        }
    }

    /* 2. Correspondance sans le préfixe 'rap-' (ex: "sentry" -> "rap-sentry", "rx" -> "rap-rx") */
    for (size_t i = 0; i < g_rap_man_pages_count; ++i) {
        if (strncasecmp(g_rap_man_pages[i].name, "rap-", 4) == 0) {
            if (strcasecmp(g_rap_man_pages[i].name + 4, norm) == 0) {
                return &g_rap_man_pages[i];
            }
        }
    }

    return NULL;
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

    /* Extraction de la rubrique demandée */
    const char *topic = rap_man_extract_topic(argc, argv);

    /* Cas 1 : Aucun argument (ou 'all') -> Projection du manuel complet */
    if (!topic || strcasecmp(topic, "all") == 0) {
        rap_man_render(g_rap_man_data, sizeof(g_rap_man_data) - 1, cfg, false);
        return RAP_EXIT_OK;
    }

    /* Cas 2 : Recherche de la rubrique unitaire */
    const RapManPageEntry *page = rap_man_lookup_page(topic);
    if (page) {
        rap_man_render(page->content, strlen(page->content), cfg, false);
        return RAP_EXIT_OK;
    }

    /* Cas 3 : Rubrique inconnue */
    fprintf(stderr, "[ERREUR] Rubrique de manuel introuvable : '%s'\n\n", topic);
    fprintf(stderr, "Rubriques disponibles :\n");
    fprintf(stderr, "  status, top, hunt, rx, preload, heur, net, sentry, kill, freeze, rapless\n");
    fprintf(stderr, "  (ou 'rap man fn' pour le dictionnaire technique des fonctions)\n");
    return RAP_EXIT_ERROR;
}
