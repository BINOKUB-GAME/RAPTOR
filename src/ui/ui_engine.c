/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_engine.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Moteur d'exécution principal de l'interface graphique (Raylib).
 *                   Initialise la fenêtre d'affichage, configure les paramètres
 *                   d'affichage (framerate, redimensionnement), orchestre la
 *                   boucle d'événements et de rendu des onglets, et assure la
 *                   libération finale du sous-système vidéo.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Déchargement explicite des textures d'icônes lors de raptor_ui_shutdown.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. raptor_ui_init
 *   2. raptor_ui_run
 *   3. raptor_ui_shutdown
 * ============================================================================
 */

#include "raptor/raptor_report.h"
#include "raylib.h"
#include <stdbool.h>

extern void ui_views_unload_resources(void);

/* ------------------------------------------------------------------------- */
/* 1. INITIALISATION DU SOUS-SYSTÈME GRAPHIQUE RAYLIB                        */
/* ------------------------------------------------------------------------- */
bool raptor_ui_init(int width, int height, const char* title) {
    if (width <= 0 || height <= 0 || !title) {
        return false;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    InitWindow(width, height, title);

    if (!IsWindowReady()) {
        return false;
    }

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    return true;
}

/* ------------------------------------------------------------------------- */
/* 2. BOUCLE D'ÉVÉNEMENTS ET DE RENDU GRAPHIQUE PRINCIPALE                   */
/* ------------------------------------------------------------------------- */
void raptor_ui_run(RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }

    while (!WindowShouldClose()) {
        BeginDrawing();
        raptor_ui_render_tabs(ctx);
        EndDrawing();
    }
}

/* ------------------------------------------------------------------------- */
/* 3. LIBÉRATION DES RESSOURCES ET FERMETURE DE LA FENÊTRE                   */
/* ------------------------------------------------------------------------- */
void raptor_ui_shutdown(void) {
    ui_views_unload_resources();
    if (IsWindowReady()) {
        CloseWindow();
    }
}
