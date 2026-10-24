/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_hunt.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Moteur d'audit universel combiné ('rap hunt').
 *                   Coordonne l'exécution séquentielle des trois sondes forensiques :
 *                   1. Mémoire anonyme R-X Inode 0 (Section 13).
 *                   2. Chargeur dynamique LD_PRELOAD (Section 14).
 *                   3. Heuristiques, chemins volatils & usurpations (Section 7).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Intégration de rap_cmd_heur dans la séquence d'investigation.
 *   - Consolidation du code de retour POSIX RAP_EXIT_THREAT sur les 3 vecteurs.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 91)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   91. rap_cmd_hunt
 * ============================================================================
 */

#include "rap_cli.h"

/* ============================================================================
 * FONCTION 91 : rap_cmd_hunt
 * ----------------------------------------------------------------------------
 * Exécute la synthèse des 3 sondes d'attaque :
 *  1. Sondage mémoire R-X Inode 0 (rap_cmd_rx).
 *  2. Sondage persistance LD_PRELOAD (rap_cmd_preload).
 *  3. Sondage heuristique binaire supprimé / chemins volatils (rap_cmd_heur).
 * ============================================================================
 */
int rap_cmd_hunt(int argc, char **argv, const RapConfig *cfg)
{
    if (!cfg->quiet && cfg->is_tty) {
        printf("%s[=== RAPTOR DEFENSE // AUDIT DE SÉCURITÉ COMBINÉ (MÉMOIRE / PRELOAD / HEURISTIQUE) ===]%s\n\n",
               C_CYAN, C_RESET);
    }

    int ret_rx = rap_cmd_rx(argc, argv, cfg);

    if (!cfg->quiet && cfg->is_tty) {
        printf("\n");
    }

    int ret_pl = rap_cmd_preload(argc, argv, cfg);

    if (!cfg->quiet && cfg->is_tty) {
        printf("\n");
    }

    int ret_hr = rap_cmd_heur(argc, argv, cfg);

    if (ret_rx == RAP_EXIT_THREAT || ret_pl == RAP_EXIT_THREAT || ret_hr == RAP_EXIT_THREAT) {
        return RAP_EXIT_THREAT;
    }

    if (ret_rx == RAP_EXIT_ERROR || ret_pl == RAP_EXIT_ERROR || ret_hr == RAP_EXIT_ERROR) {
        return RAP_EXIT_ERROR;
    }

    return RAP_EXIT_OK;
}
