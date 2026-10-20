/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_common.h
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : En-tête commun d'architecture pour l'interface Raylib.
 *                   Définit la palette graphique officielle Cyber Dark/Teal,
 *                   le facteur global de mise à l'échelle de la police,
 *                   les variables d'état d'interface partagées et les signatures
 *                   des modules spécialisés (ressources, layout, vues et modale).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Découpage modulaire : point d'ancrage unique pour tous les sous-composants UI.
 *   - Centralisation des couleurs officielles (thème sombre défensif).
 *   - Déclaration des fonctions d'assistance typographique et d'interrogation du contexte.
 *   - Déclaration des prototypes des 5 vues métier, du layout et de la modale.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES PROTOTYPES ET COMPOSANTS :
 *   1. ui_draw_text
 *   2. ui_measure_text
 *   3. ui_find_card
 *   4. ui_find_table
 *   5. ui_resources_load
 *   6. ui_resources_unload
 *   7. ui_layout_draw_header
 *   8. ui_layout_draw_tabs
 *   9. ui_view_global_draw
 *  10. ui_view_processes_draw
 *  11. ui_view_network_draw
 *  12. ui_view_diff_draw
 *  13. ui_view_ai_draw
 *  14. ui_modal_guide_draw
 * ============================================================================
 */

#ifndef RAPTOR_UI_COMMON_H
#define RAPTOR_UI_COMMON_H

#include "raptor/raptor_report.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_differential.h"
#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* RÉGLAGE UNIQUE DE TAILLE DE POLICE                                        */
/* ========================================================================= */
#define UI_FONT_SCALE 1.25f

/* ------------------------------------------------------------------------- */
/* PALETTE DE COULEURS OFFICIELLE THÈME CYBER DÉFENSIF                       */
/* ------------------------------------------------------------------------- */
#define COL_BG_MAIN      (Color){ 6, 11, 19, 255 }
#define COL_PANEL_BG     (Color){ 10, 17, 26, 255 }
#define COL_CARD_BG      (Color){ 14, 23, 36, 255 }
#define COL_BORDER       (Color){ 22, 38, 58, 255 }
#define COL_BORDER_FOCUS (Color){ 0, 210, 255, 255 }
#define COL_BLUE_BTN     (Color){ 0, 119, 204, 255 }
#define COL_BLUE_HOVER   (Color){ 0, 145, 245, 255 }
#define COL_TEXT_WHITE   (Color){ 235, 245, 255, 255 }
#define COL_TEXT_MUTED   (Color){ 135, 160, 190, 255 }
#define COL_CYAN_ACCENT  (Color){ 0, 210, 255, 255 }
#define COL_GREEN_OK     (Color){ 0, 230, 118, 255 }
#define COL_CRIT_RED     (Color){ 255, 51, 85, 255 }

/* ------------------------------------------------------------------------- */
/* ÉTATS GLOBAUX PARTAGÉS ENTRE COMPOSANTS                                   */
/* ------------------------------------------------------------------------- */
extern int g_ui_active_tab;
extern bool g_ui_show_guide_modal;
extern char g_ui_last_audit_time[64];

/* ------------------------------------------------------------------------- */
/* TEXTURES ET POLICE (GÉRÉES PAR ui_resources.c)                            */
/* ------------------------------------------------------------------------- */
extern Font g_ui_font;
extern bool g_ui_font_loaded;

extern Texture2D g_icon_dashboard;
extern Texture2D g_icon_processes;
extern Texture2D g_icon_network;
extern Texture2D g_icon_diff;
extern Texture2D g_icon_ai;
extern bool g_ui_icons_loaded;

/* ------------------------------------------------------------------------- */
/* FONCTIONS D'ASSISTANCE TYPOGRAPHIQUE ET D'EXTRACTION DU MODÈLE             */
/* ------------------------------------------------------------------------- */
void ui_draw_text(const char* text, float posX, float posY, float baseSize, Color color);
float ui_measure_text(const char* text, float baseSize);
const RaptorCard* ui_find_card(const RaptorReportContext* ctx, const char* keyword);
const RaptorTable* ui_find_table(const RaptorReportContext* ctx, const char* sec_id, const char* tbl_kw);

/* ------------------------------------------------------------------------- */
/* GESTION DU CYCLE DE VIE DES RESSOURCES (ui_resources.c)                   */
/* ------------------------------------------------------------------------- */
void ui_resources_load(void);
void ui_resources_unload(void);

/* ------------------------------------------------------------------------- */
/* STRUCTURE D'EN-TÊTE ET ONGLETS (ui_layout.c)                              */
/* ------------------------------------------------------------------------- */
void ui_layout_draw_header(void);
void ui_layout_draw_tabs(void);

/* ------------------------------------------------------------------------- */
/* VUES MÉTIERS DÉDIÉES (src/ui/views/view_*.c)                              */
/* ------------------------------------------------------------------------- */
void ui_view_global_draw(RaptorReportContext* ctx);
void ui_view_processes_draw(RaptorReportContext* ctx);
void ui_view_network_draw(const RaptorReportContext* ctx);
void ui_view_diff_draw(const RaptorReportContext* ctx);
void ui_view_ai_draw(const RaptorReportContext* ctx);

/* ------------------------------------------------------------------------- */
/* BOÎTE MODALE DU GUIDE ANALYTIQUE (src/ui/modals/modal_guide.c)            */
/* ------------------------------------------------------------------------- */
void ui_modal_guide_draw(void);

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_UI_COMMON_H */
