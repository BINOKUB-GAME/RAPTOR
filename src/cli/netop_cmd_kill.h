/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_kill.h
 * RÉVISION      : Rev 1.1 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête du greffon d'action cinétique 'k' (Kill / Neutralisation).
 * ============================================================================== */

#ifndef NETOP_CMD_KILL_H
#define NETOP_CMD_KILL_H

#include "rap_netop.h"

int  netop_cmd_kill_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);
void netop_cmd_kill_apply_tombstones(NetopEntry *entries, size_t count);

#endif /* NETOP_CMD_KILL_H */
