/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_router.h
 * RÉVISION      : Rev 1.2 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête du routeur de commandes et contrat des greffons TUI.
 *                 Définit l'architecture de dispatching découplée pour les hotkeys
 *                 et la gestion modulaire des surcouches/écrans modaux (Overlays).
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Argument 'arg' sous forme de chaîne (const char *) pour saisies multi-slots (ex: "4,6,9").
 *   - Gestionnaire d'overlays modaux (netop_router_set_overlay / netop_router_render_overlay).
 *   - Prise en charge de la navigation modale (Échap ferme la modale sans quitter).
 *   - Préservation stricte de l'architecture 0-heap.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 144 à 146, 151, 152)
 * ============================================================================== */

#ifndef NETOP_ROUTER_H
#define NETOP_ROUTER_H

#include "rap_netop.h"

typedef enum {
    NETOP_CMD_INSTANT,   /* Déclenchement direct (ex: pause, liste bans Ctrl+B) */
    NETOP_CMD_TARGETED   /* Requiert la sélection d'une ou plusieurs lignes (ex: kill, ban) */
} NetopCmdType;

/* Prototype standardisé d'exécution d'une commande */
typedef int (*NetopCmdHandler)(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);

/* Prototype standardisé de rendu pour écran modal / overlay */
typedef bool (*NetopOverlayRenderer)(NetopContext *ctx);

typedef struct {
    char            key;           /* Touche déclencheuse ('k', 'b', 'p', 0x02 pour Ctrl+B) */
    const char     *name;          /* Nom court affiché dans l'aide ("Kill", "Ban/Déban", etc.) */
    NetopCmdType    type;          /* Instantané ou avec invite de sélection */
    const char     *prompt_label;  /* Libellé de l'invite (ex: "BAN IP / DÉBAN") */
    NetopCmdHandler handler;       /* Fonction exécutrice dans le fichier de commande */
} NetopPlugin;

/* API du Routeur */
void                 netop_router_init(void);
void                 netop_router_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);
const char          *netop_router_get_footer(NetopContext *ctx);

/* Gestion des modales / overlays */
void                 netop_router_set_overlay(NetopOverlayRenderer renderer);
NetopOverlayRenderer netop_router_get_overlay(void);
bool                 netop_router_render_overlay(NetopContext *ctx);

#endif /* NETOP_ROUTER_H */
