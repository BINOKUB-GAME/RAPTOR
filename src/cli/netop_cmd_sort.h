/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_sort.h
 * RÉVISION      : Rev 1.0 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête du greffon de tri dynamique 's' (Sort Toggle).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 158 à 160)
 * ============================================================================== */

#ifndef NETOP_CMD_SORT_H
#define NETOP_CMD_SORT_H

#include "rap_netop.h"

typedef enum {
    NETOP_SORT_SEVERITY = 0, /* Dangerosité décroissante (Défaut tactique) */
    NETOP_SORT_PID      = 1, /* Ordre croissant des PIDs système */
    NETOP_SORT_PORT     = 2  /* Regroupement par port distant / activité */
} NetopSortMode;

int           netop_cmd_sort_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);
void          netop_cmd_sort_apply(NetopEntry *entries, size_t count);
const char   *netop_cmd_sort_get_name(void);
NetopSortMode netop_cmd_sort_get_mode(void);

#endif /* NETOP_CMD_SORT_H */
