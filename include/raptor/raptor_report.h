/**
 * ============================================================================
 * NOM DU SCRIPT   : include/raptor/raptor_report.h
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Déclare les moteurs de restitution universels (rapport CLI
 *                   en console, export HTML autonome) ainsi que les points
 *                   d'entrée du moteur d'interface graphique utilisateur
 *                   (UI Raylib / ImGui). Consomme le conteneur universel
 *                   sans couplage avec les sondes sources.
 *
 * LISTE DES FONCTIONS DÉCLARÉES :
 *   1. raptor_report_cli_print
 *   2. raptor_report_html_generate
 *   3. raptor_ui_init
 *   4. raptor_ui_run
 *   5. raptor_ui_render_tabs
 *   6. raptor_ui_shutdown
 * ============================================================================
 */

#ifndef RAPTOR_REPORT_H
#define RAPTOR_REPORT_H

#include <stdbool.h>
#include <stddef.h>
#include "raptor_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* RESTITUTION EN LIGNE DE COMMANDE & RAPPORT HTML (API 3.6.1 & 3.6.2)       */
/* ------------------------------------------------------------------------- */

/**
 * 1. Affiche l'intégralité du rapport textuel formaté sur le terminal.
 */
void raptor_report_cli_print(const RaptorReportContext* ctx);

/**
 * 2. Exporte le rapport HTML souverain avec thème sombre et tableaux interactifs.
 */
bool raptor_report_html_generate(const RaptorReportContext* ctx, const char* filepath);

/* ------------------------------------------------------------------------- */
/* INTERFACE GRAPHIQUE RAYLIB / IMGUI (API 3.6.3 à 3.6.6)                    */
/* ------------------------------------------------------------------------- */

/**
 * 3. Démarre Raylib et prépare le contexte graphique ImGui.
 */
bool raptor_ui_init(int width, int height, const char* title);

/**
 * 4. Boucle principale (while (!WindowShouldClose())) gérant les interactions et audits.
 */
void raptor_ui_run(RaptorReportContext* ctx);

/**
 * 5. Boucle sur le contexte pour instancier un onglet dynamique par section/greffon.
 */
void raptor_ui_render_tabs(const RaptorReportContext* ctx);

/**
 * 6. Libère les contextes graphiques et ferme la fenêtre.
 */
void raptor_ui_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_REPORT_H */
