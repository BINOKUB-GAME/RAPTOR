/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_engine.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Moteur d'exécution principal de l'interface graphique (Raylib).
 *                   Initialise la fenêtre d'affichage, configure les paramètres
 *                   d'affichage (framerate, redimensionnement), orchestre la
 *                   boucle d'événements, le cycle de rafraîchissement continu
 *                   (Live Monitoring automatique) et le rendu des onglets.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Intégration d'un cycle de veille et ré-audit automatique en continu
 *     (intervalle de 3.0 secondes) entre deux frames sans blocage UI.
 *   - Purge préalable du contexte (raptor_data_context_clear) assurant que les
 *     processus détruits ou respawnés soient actualisés en direct.
 *   - Synchronisation continue de l'horodatage global g_ui_last_audit_time.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. raptor_ui_init
 *   2. raptor_ui_run
 *   3. raptor_ui_shutdown
 * ============================================================================
 */

#include "raptor/raptor_report.h"
#include "raptor/raptor_data.h"
#include "raptor/raptor_registry.h"
#include "raylib.h"
#include <stdbool.h>
#include <time.h>

extern void ui_views_unload_resources(void);
extern char g_ui_last_audit_time[64];

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
/* 2. BOUCLE D'ÉVÉNEMENTS ET DE SURVEILLANCE CONTINUE                        */
/* ------------------------------------------------------------------------- */
void raptor_ui_run(RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }

    float refresh_timer = 0.0f;
    const float REFRESH_INTERVAL = 3.0f; /* Audit automatique toutes les 3 secondes */

    while (!WindowShouldClose()) {
        /* 1. Cycle de rafraîchissement continu (Live Monitoring) */
        refresh_timer += GetFrameTime();
        if (refresh_timer >= REFRESH_INTERVAL) {
            raptor_data_context_clear(ctx);
            raptor_registry_run_all(ctx);

            time_t n = time(NULL);
            struct tm* tm_i = localtime(&n);
            if (tm_i) {
                strftime(g_ui_last_audit_time, sizeof(g_ui_last_audit_time), "%Y-%m-%d %H:%M:%S", tm_i);
            }

            refresh_timer = 0.0f;
        }

        /* 2. Rendu graphique */
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
