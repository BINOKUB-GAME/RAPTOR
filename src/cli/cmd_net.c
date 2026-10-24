/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_net.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon d'inspection des communications réseau ('rap net').
 *                   Analyse /proc/net/tcp sans dépendance externe, décode
 *                   les adresses binaires IPv4 et qualifie chaque flux
 *                   selon les règles de souveraineté Net-Guard (Section 11).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Intégration de rap_net_eval_ip pour la qualification en temps réel.
 *   - Remplacement de l'état statique 'ESTABLISHED' par le diagnostic Net-Guard.
 *   - Coloration d'alerte [HOSTILE] et remontée de menace (RAP_EXIT_THREAT).
 *
 * SECTION DU CAHIER DES CHARGES : SECTIONS 11 & 16 (Fonction 94)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   94. rap_cmd_net
 * ============================================================================
 */

#include "rap_cli.h"

/* ============================================================================
 * FONCTION 94 : rap_cmd_net
 * ----------------------------------------------------------------------------
 * Inspection directe de /proc/net/tcp couplée à l'intelligence Net-Guard :
 *  - Conversion mathématique des adresses hexadécimales little-endian.
 *  - Détection et qualification automatique de l'IP distante.
 *  - Renvoie RAP_EXIT_THREAT (1) si au moins une socket hostile est active.
 * ============================================================================
 */
int rap_cmd_net(int argc, char **argv, const RapConfig *cfg)
{
    (void)argc;
    (void)argv;

    FILE *ftcp = fopen("/proc/net/tcp", "r");
    if (!ftcp) {
        perror("rap net: lecture /proc/net/tcp impossible");
        return RAP_EXIT_ERROR;
    }

    if (!cfg->quiet) {
        rap_output_formatter(cfg, "PROTO", "ADRESSE LOCALE", "ADRESSE DISTANTE", "QUALIFICATION NET-GUARD");
        if (cfg->is_tty && !cfg->raw) {
            printf(" --------------------------------------------------------------------------------\n");
        }
    }

    char line[256];
    if (!fgets(line, sizeof(line), ftcp)) {
        fclose(ftcp);
        return RAP_EXIT_ERROR;
    }

    int displayed = 0;
    int hostile_found = 0;

    while (fgets(line, sizeof(line), ftcp)) {
        unsigned int local_ip = 0, local_port = 0;
        unsigned int rem_ip = 0, rem_port = 0;
        unsigned int state = 0;

        int matched = sscanf(line, "%*d: %x:%x %x:%x %x",
                             &local_ip, &local_port, &rem_ip, &rem_port, &state);

        if (matched == 5) {
            /* Filtre strict sur l'état TCP_ESTABLISHED (valeur 1) */
            if (state == 1) {
                /* Conversion en notation décimale pointée */
                unsigned int a1 = local_ip & 0xFF;
                unsigned int b1 = (local_ip >> 8) & 0xFF;
                unsigned int c1 = (local_ip >> 16) & 0xFF;
                unsigned int d1 = (local_ip >> 24) & 0xFF;

                unsigned int a2 = rem_ip & 0xFF;
                unsigned int b2 = (rem_ip >> 8) & 0xFF;
                unsigned int c2 = (rem_ip >> 16) & 0xFF;
                unsigned int d2 = (rem_ip >> 24) & 0xFF;

                char slocal[32], srem[32];
                snprintf(slocal, sizeof(slocal), "%u.%u.%u.%u:%u", a1, b1, c1, d1, local_port);
                snprintf(srem, sizeof(srem), "%u.%u.%u.%u:%u", a2, b2, c2, d2, rem_port);

                /* Calcul de la valeur entière pour l'évaluateur CIDR */
                uint32_t rem_ip_num = (a2 << 24) | (b2 << 16) | (c2 << 8) | d2;
                char diag[128];
                int qual = rap_net_eval_ip(rem_ip_num, (uint16_t)rem_port, diag, sizeof(diag));

                if (qual == RAP_NET_HOSTILE) {
                    hostile_found++;
                }

                /* Formatage avec coloration si TTY interactif */
                char formatted_diag[160];
                if (cfg->is_tty && !cfg->raw) {
                    if (qual == RAP_NET_HOSTILE) {
                        snprintf(formatted_diag, sizeof(formatted_diag), "%s%s%s", C_RED, diag, C_RESET);
                    } else if (qual == RAP_NET_SUSPECT) {
                        snprintf(formatted_diag, sizeof(formatted_diag), "%s%s%s", C_YELLOW, diag, C_RESET);
                    } else {
                        snprintf(formatted_diag, sizeof(formatted_diag), "%s", diag);
                    }
                } else {
                    snprintf(formatted_diag, sizeof(formatted_diag), "%s", diag);
                }

                rap_output_formatter(cfg, "TCP", slocal, srem, formatted_diag);
                displayed++;

                if (cfg->limit > 0 && displayed >= cfg->limit) {
                    break;
                }
            }
        }
    }
    fclose(ftcp);

    if (displayed == 0 && !cfg->quiet) {
        printf("%s[+] Aucun flux TCP sortant à l'état ESTABLISHED détecté%s\n",
               cfg->is_tty ? C_GREEN : "", cfg->is_tty ? C_RESET : "");
    }

    return hostile_found ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}
