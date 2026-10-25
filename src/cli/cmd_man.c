/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_man.c
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Affichage paginé autonome du manuel de référence (.rodata).
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Auto-détection de terminal interactif (cfg->is_tty).
 *   - Invocation automatique de 'rapless' (ou repli standard) si TTY actif.
 *   - Écriture directe sur stdout si mode script/pipe ou option --raw.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 99)
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *    99. rap_cmd_man : Restitue la documentation technique paginée.
 * ============================================================================
 */

#include "rap_cli.h"
#include "rap_man_data.h"

/* ============================================================================
 * FONCTION 99 : rap_cmd_man
 * ============================================================================
 */
int rap_cmd_man(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    /* 1. Sortie non-interactive (script, redirection > fichier, ou drapeau --raw) */
    if (!cfg || !cfg->is_tty || cfg->raw) {
        write(STDOUT_FILENO, g_rap_man_data, sizeof(g_rap_man_data) - 1);
        return RAP_EXIT_OK;
    }

    /* 2. Sortie interactive TTY : aiguillage automatique vers le téléscripteur rapless */
    FILE *pager = popen("rapless 2>/dev/null || ./build/rapless 2>/dev/null || less -R 2>/dev/null", "w");
    if (!pager) {
        /* Repli direct si aucun téléscripteur n'est disponible */
        write(STDOUT_FILENO, g_rap_man_data, sizeof(g_rap_man_data) - 1);
        return RAP_EXIT_OK;
    }

    fwrite(g_rap_man_data, 1, sizeof(g_rap_man_data) - 1, pager);
    pclose(pager);

    return RAP_EXIT_OK;
}
