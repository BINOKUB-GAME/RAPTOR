/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_hunt.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Moteur d'audit universel combiné ('rap hunt').
 *                   Coordonne l'exécution séquentielle des trois sondes forensiques :
 *                   1. Mémoire anonyme R-X Inode 0 (Section 13).
 *                   2. Chargeur dynamique LD_PRELOAD (Section 14).
 *                   3. Heuristiques, chemins volatils & usurpations (Section 7).
 *                   Prend en charge l'armement actif du moteur réflexe (--arm)
 *                   et la simulation médico-légale (--dry-run).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Prise en charge des drapeaux d'armement réflexe active :
 *       '--arm' ou '-a'       : Neutralisation immédiate (SIGSTOP, coupure socket, SIGKILL).
 *       '--dry-run'           : Simulation de riposte avec trace en boîte noire.
 *       (Par défaut)          : Surveillance passive sans altération des cibles.
 *   - Activation du verrouillage mémoire mlockall et immunité OOM via rap_oom.
 *   - Bilan d'intervention en fin d'audit avec relevé des neutralisations en RAM.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 91)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   91. rap_cmd_hunt
 * ============================================================================
 */

#include "rap_cli.h"
#include "rap_reflex.h"
#include "rap_oom.h"
#include "rap_blackbox.h"
#include <string.h>

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
    reflex_mode_t reflex_mode = REFLEX_MODE_PASSIVE;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--arm") == 0 || strcmp(argv[i], "-a") == 0) {
            reflex_mode = REFLEX_MODE_ARMED;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            reflex_mode = REFLEX_MODE_DRYRUN;
        }
    }

    /* Initialisation du sous-système réflexe et de la boîte noire */
    rap_reflex_init(reflex_mode);

    /* Verrouillage mémoire physique et immunité OOM en mode armé */
    if (reflex_mode == REFLEX_MODE_ARMED) {
        rap_oom_protect();
        rap_oom_lock_memory();
    }

    if (!cfg->quiet && cfg->is_tty) {
        printf("%s[=== RAPTOR DEFENSE // CHASSE ACTIVE FORENSIQUE (MÉMOIRE / PRELOAD / HEURISTIQUE) ===]%s\n",
               C_CYAN, C_RESET);
        if (reflex_mode == REFLEX_MODE_ARMED) {
            printf("  -> Action réflexe    : %sARMÉE (Neutralisation active SIGSTOP/SIGKILL)%s\n", C_RED, C_RESET);
        } else if (reflex_mode == REFLEX_MODE_DRYRUN) {
            printf("  -> Action réflexe    : %sSIMULATION (Dry-Run Boîte Noire)%s\n", C_YELLOW, C_RESET);
        } else {
            printf("  -> Action réflexe    : %sPASSIVE (Audit seul, pas de neutralisation)%s\n", C_GREEN, C_RESET);
        }
        printf("\n");
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

    /* Bilan médico-légal si le mode réflexe était armé ou en simulation */
    if (!cfg->quiet && cfg->is_tty && reflex_mode != REFLEX_MODE_PASSIVE) {
        uint64_t neutralized = 0;
        rap_reflex_get_stats(&neutralized);
        if (neutralized > 0) {
            printf("\n%s[BILAN RÉFLEXE]%s %lu anomalie(s) traitée(s) en mémoire vive.\n",
                   C_RED, C_RESET, (unsigned long)neutralized);
            printf("Consultez les détails forensiques avec : %srap sentry dump%s\n", C_CYAN, C_RESET);
        }
    }

    if (ret_rx == RAP_EXIT_THREAT || ret_pl == RAP_EXIT_THREAT || ret_hr == RAP_EXIT_THREAT) {
        return RAP_EXIT_THREAT;
    }

    if (ret_rx == RAP_EXIT_ERROR || ret_pl == RAP_EXIT_ERROR || ret_hr == RAP_EXIT_ERROR) {
        return RAP_EXIT_ERROR;
    }

    return RAP_EXIT_OK;
}
