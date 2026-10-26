/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_inspect.h
 * RÉVISION      : Rev 1.0 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête du greffon d'inspection médico-légale 'i' (Forensic Overlay).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 153 à 157)
 * ============================================================================== */

#ifndef NETOP_CMD_INSPECT_H
#define NETOP_CMD_INSPECT_H

#include "rap_netop.h"

int  netop_cmd_inspect_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);
bool netop_cmd_inspect_render_modal(NetopContext *ctx);
bool netop_cmd_inspect_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);

#endif /* NETOP_CMD_INSPECT_H */
