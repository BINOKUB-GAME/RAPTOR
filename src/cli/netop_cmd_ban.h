/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_ban.h
 * RÉVISION      : Rev 1.1 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête du greffon de bannissement et d'isolation réseau 'b'.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 147 à 150)
 * ============================================================================== */

#ifndef NETOP_CMD_BAN_H
#define NETOP_CMD_BAN_H

#include "rap_netop.h"

int  netop_cmd_ban_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);
int  netop_cmd_ban_toggle_list(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);
bool netop_cmd_ban_render_list(NetopContext *ctx);
bool netop_cmd_ban_is_banned(const char *remote_ip);

#endif /* NETOP_CMD_BAN_H */
