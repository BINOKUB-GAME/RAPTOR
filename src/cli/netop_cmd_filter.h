/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_filter.h
 * RÉVISION      : Rev 1.0 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête du greffon de filtre dynamique 'f' (Filter Toggle).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 161 à 163)
 * ============================================================================== */

#ifndef NETOP_CMD_FILTER_H
#define NETOP_CMD_FILTER_H

#include "rap_netop.h"

typedef enum {
    NETOP_FILTER_ALL         = 0, /* Tous les flux (Vue exhaustive) */
    NETOP_FILTER_NO_LOCAL    = 1, /* Exclure 127.0.0.1 et LISTEN (Flux distants réels) */
    NETOP_FILTER_ESTABLISHED = 2, /* Sessions ESTABLISHED uniquement */
    NETOP_FILTER_ALERTS      = 3  /* Cockpit de crise : Alertes seules (HOSTILE/WARN/BAN/KILLED) */
} NetopFilterMode;

int             netop_cmd_filter_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);
size_t          netop_cmd_filter_apply(NetopEntry *entries, size_t count);
const char     *netop_cmd_filter_get_name(void);
NetopFilterMode netop_cmd_filter_get_mode(void);

#endif /* NETOP_CMD_FILTER_H */
