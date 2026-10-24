/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_kill.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffons d'action cinétique d'abattage et de gel de menaces.
 *                   - 'rap kill'   : Émission non-différable de SIGKILL.
 *                   - 'rap freeze' : Suspension d'urgence via SIGSTOP (gel de
 *                                    l'implant pour extraction mémoire/gcore).
 *                   Applique la barrière de sécurité absolue (rap_check_safety_pid)
 *                   pour interdire tout dégât collatéral sur PID 1, le shell parent
 *                   ou les threads internes du noyau Linux.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonctions 95 et 96)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   95. rap_cmd_kill
 *   96. rap_cmd_freeze
 * ============================================================================
 */

#include <signal.h>
#include "rap_cli.h"

/* ============================================================================
 * FONCTION 95 : rap_cmd_kill
 * ----------------------------------------------------------------------------
 * Neutralisation chirurgicale immédiate d'un PID malveillant :
 *  - Validation des arguments et vérification de sécurité.
 *  - Demande de confirmation interactive en terminal direct sauf si -f/--force.
 *  - Transmission de SIGKILL et compte-rendu d'abattage.
 * ============================================================================
 */
int rap_cmd_kill(int argc, char **argv, const RapConfig *cfg)
{
    if (argc < 3) {
        fprintf(stderr, "%sUsage : rap kill [-f] <PID>%s\n",
                cfg->is_tty ? C_RED : "", cfg->is_tty ? C_RESET : "");
        return RAP_EXIT_ERROR;
    }

    pid_t target_pid = (pid_t)atoi(argv[2]);

    if (rap_check_safety_pid(cfg, target_pid) != 0) {
        return RAP_EXIT_ERROR;
    }

    /* Demande de confirmation si terminal interactif et drapeau force absent */
    if (!cfg->force && cfg->is_tty) {
        printf("Confirmer l'abattage immédiat du PID %d ? [o/N] : ", target_pid);
        fflush(stdout);

        char confirm = 0;
        if (scanf(" %c", &confirm) != 1 || (confirm != 'o' && confirm != 'O')) {
            printf("[ANNULATION] Neutralisation interrompue par l'opérateur.\n");
            return RAP_EXIT_OK;
        }
    }

    if (kill(target_pid, SIGKILL) == 0) {
        printf("%s[+] PID %d neutralisé avec succès (SIGKILL appliqué).%s\n",
               cfg->is_tty ? C_GREEN : "", target_pid, cfg->is_tty ? C_RESET : "");
        return RAP_EXIT_OK;
    } else {
        perror("rap kill: échec lors de l'émission de SIGKILL");
        return RAP_EXIT_ERROR;
    }
}

/* ============================================================================
 * FONCTION 96 : rap_cmd_freeze
 * ----------------------------------------------------------------------------
 * Suspension conservatoire d'un processus suspect sans destruction mémoire :
 *  - Transmet SIGSTOP pour stopper l'exécution du code pirate.
 *  - Préserve les allocations virtuelles pour extraction forensique (gcore/dump).
 * ============================================================================
 */
int rap_cmd_freeze(int argc, char **argv, const RapConfig *cfg)
{
    if (argc < 3) {
        fprintf(stderr, "%sUsage : rap freeze <PID>%s\n",
                cfg->is_tty ? C_RED : "", cfg->is_tty ? C_RESET : "");
        return RAP_EXIT_ERROR;
    }

    pid_t target_pid = (pid_t)atoi(argv[2]);

    if (rap_check_safety_pid(cfg, target_pid) != 0) {
        return RAP_EXIT_ERROR;
    }

    if (kill(target_pid, SIGSTOP) == 0) {
        printf("%s[*] PID %d figé avec succès (SIGSTOP). Mémoire préservée pour analyse.%s\n",
               cfg->is_tty ? C_YELLOW : "", target_pid, cfg->is_tty ? C_RESET : "");
        return RAP_EXIT_OK;
    } else {
        perror("rap freeze: échec lors de l'émission de SIGSTOP");
        return RAP_EXIT_ERROR;
    }
}
