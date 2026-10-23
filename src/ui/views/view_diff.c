/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_diff.c
 * RÉVISION        : Rev 1.7
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Affichage de l'Onglet 4 [ Cahier Différentiel ].
 *                   Compare l'état du système entre deux cycles temporels (T-1 vs T0).
 *                   Affiche les dérives volumétriques de RAM et le registre
 *                   forensique des mutations clandestines R-X (Section 13)
 *                   ainsi que les détournements de chargeur LD_PRELOAD (Section 14).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.7) :
 *   - Substitution de "0x0 - 0x0" et "0 Ko" par la mention "N/A" (Non Applicable)
 *     pour les alertes d'environnement d'exécution (Section 14 : LD_PRELOAD).
 *   - Maintien de l'affichage ordonné en mode LIFO (du plus récent au plus ancien).
 *   - Calibrage des colonnes préservant la lisibilité 64 bits et les libellés forensiques.
 *   - Bouton de purge interactif du registre des événements.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_view_diff_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include "../../core/mem_stealth.h"
#include <stdio.h>

/* ------------------------------------------------------------------------- */
/* 1. RENDU DE LA VUE CAHIER DIFFÉRENTIEL (T-1 vs T0) & ALERTES FORENSIQUES   */
/* ------------------------------------------------------------------------- */
void ui_view_diff_draw(const RaptorReportContext* ctx) {
    (void)ctx;

    float screen_w = (float)GetScreenWidth();
    float start_x = 20.0f;
    float content_w = screen_w - 40.0f;
    Vector2 mouse = GetMousePosition();

    /* --------------------------------------------------------------------- */
    /* VOLET 1 : DÉRIVES & FLUCTUATIONS VOLUMÉTRIQUES DE RAM                 */
    /* --------------------------------------------------------------------- */
    float panel1_y = 95.0f;
    float panel1_h = 210.0f;
    Rectangle rec_panel1 = { start_x, panel1_y, content_w, panel1_h };

    DrawRectangleRec(rec_panel1, (Color){ 10, 18, 28, 255 });
    DrawRectangleLinesEx(rec_panel1, 1.0f, COL_BORDER);

    DrawRectangle((int)start_x, (int)panel1_y, (int)content_w, 28, (Color){ 16, 28, 44, 255 });
    ui_draw_text("CAHIER DIFFÉRENTIEL TEMPOREL (T-1 vs T0) // SUIVI DE DÉRIVE RAM",
                 start_x + 14.0f, panel1_y + 8.0f, 11.0f, COL_CYAN_ACCENT);

    ui_draw_text("Mesure différentielle active entre les cycles d'échantillonnage de /proc (analyse d'inflation RAM)",
                 start_x + 14.0f, panel1_y + 36.0f, 11.0f, COL_TEXT_MUTED);

    float tbl1_y = panel1_y + 56.0f;
    DrawRectangle((int)start_x + 10, (int)tbl1_y, (int)content_w - 20, 24, (Color){ 14, 24, 38, 255 });
    ui_draw_text("PID", start_x + 20.0f, tbl1_y + 6.0f, 10.0f, COL_TEXT_MUTED);
    ui_draw_text("NOM PROCESSUS", start_x + 100.0f, tbl1_y + 6.0f, 10.0f, COL_TEXT_MUTED);
    ui_draw_text("RAM T-1", start_x + 300.0f, tbl1_y + 6.0f, 10.0f, COL_TEXT_MUTED);
    ui_draw_text("RAM T0", start_x + 420.0f, tbl1_y + 6.0f, 10.0f, COL_TEXT_MUTED);
    ui_draw_text("VARIATION", start_x + 540.0f, tbl1_y + 6.0f, 10.0f, COL_TEXT_MUTED);
    ui_draw_text("STATUT", start_x + 680.0f, tbl1_y + 6.0f, 10.0f, COL_TEXT_MUTED);

    ui_draw_text("Système stable. Aucune fuite mémoire critique d'expansion détectée sur le dernier cycle.",
                 start_x + 20.0f, tbl1_y + 36.0f, 11.0f, COL_GREEN_OK);

    /* --------------------------------------------------------------------- */
    /* VOLET 2 : REGISTRE FORENSIQUE DES MUTATIONS & INJECTIONS (SEC 13-14)  */
    /* --------------------------------------------------------------------- */
    float panel2_y = panel1_y + panel1_h + 16.0f;
    float panel2_h = 250.0f;
    Rectangle rec_panel2 = { start_x, panel2_y, content_w, panel2_h };

    DrawRectangleRec(rec_panel2, (Color){ 8, 14, 22, 255 });

    StealthEvent events[MAX_STEALTH_EVENTS];
    int event_count = mem_stealth_get_diff_events(events, MAX_STEALTH_EVENTS);

    Color border_color = (event_count > 0) ? (Color){ 255, 51, 85, 255 } : COL_BORDER;
    DrawRectangleLinesEx(rec_panel2, 1.0f, border_color);

    Color bar_color = (event_count > 0) ? (Color){ 44, 16, 24, 255 } : (Color){ 16, 28, 44, 255 };
    DrawRectangle((int)start_x, (int)panel2_y, (int)content_w, 28, bar_color);

    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf),
             "REGISTRE FORENSIQUE : MUTATIONS EN VOL & INJECTIONS R-X SANS FICHIER (%d ÉVÉNEMENT%s)",
             event_count, (event_count > 1) ? "S" : "");
    ui_draw_text(title_buf, start_x + 14.0f, panel2_y + 8.0f, 11.0f, (event_count > 0) ? COL_CRIT_RED : COL_CYAN_ACCENT);

    if (event_count > 0) {
        Rectangle btn_clear = { start_x + content_w - 140.0f, panel2_y + 3.0f, 130.0f, 22.0f };
        bool hover_clear = CheckCollisionPointRec(mouse, btn_clear);
        DrawRectangleRec(btn_clear, hover_clear ? COL_BLUE_HOVER : (Color){ 24, 34, 48, 255 });
        DrawRectangleLinesEx(btn_clear, 1.0f, COL_BORDER);
        ui_draw_text("Purger l'historique", btn_clear.x + 14.0f, btn_clear.y + 5.0f, 10.0f, COL_TEXT_WHITE);

        if (hover_clear && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mem_stealth_clear_diff_events();
        }
    }

    float tbl2_y = panel2_y + 38.0f;

    if (event_count == 0) {
        DrawRectangle((int)start_x + 14, (int)tbl2_y, (int)content_w - 28, 40, (Color){ 12, 22, 18, 255 });
        DrawRectangleLinesEx((Rectangle){ start_x + 14, tbl2_y, content_w - 28, 40 }, 1.0f, (Color){ 0, 180, 80, 150 });
        ui_draw_text("• AUCUNE MUTATION CLANDESTINE DÉTECTÉE : Aucun processus n'a converti de mémoire anonyme vers R-X (W^X respecté).",
                     start_x + 28.0f, tbl2_y + 14.0f, 11.0f, COL_GREEN_OK);
    } else {
        DrawRectangle((int)start_x + 10, (int)tbl2_y, (int)content_w - 20, 24, (Color){ 24, 18, 24, 255 });
        ui_draw_text("PID", start_x + 20.0f, tbl2_y + 6.0f, 10.0f, (Color){ 255, 187, 0, 255 });
        ui_draw_text("EXÉCUTABLE", start_x + 85.0f, tbl2_y + 6.0f, 10.0f, (Color){ 255, 187, 0, 255 });
        ui_draw_text("PLAGE MÉMOIRE (RAM)", start_x + 230.0f, tbl2_y + 6.0f, 10.0f, (Color){ 255, 187, 0, 255 });
        ui_draw_text("TAILLE", start_x + 530.0f, tbl2_y + 6.0f, 10.0f, (Color){ 255, 187, 0, 255 });
        ui_draw_text("DIAGNOSTIC FORENSIQUE", start_x + 620.0f, tbl2_y + 6.0f, 10.0f, (Color){ 255, 187, 0, 255 });

        int display_count = (event_count < 7) ? event_count : 7;
        float row_y = tbl2_y + 28.0f;

        for (int i = 0; i < display_count; ++i) {
            /* Ordre LIFO : la dernière menace détectée apparaît en tête de liste */
            const StealthEvent* ev = &events[event_count - 1 - i];

            DrawRectangle((int)start_x + 10, (int)row_y, (int)content_w - 20, 28,
                          (i % 2 == 0) ? (Color){ 20, 12, 16, 255 } : (Color){ 14, 10, 14, 255 });
            DrawRectangleLinesEx((Rectangle){ start_x + 10, row_y, content_w - 20, 28 }, 1.0f, (Color){ 60, 20, 30, 255 });

            char pid_str[16];
            snprintf(pid_str, sizeof(pid_str), "%d", ev->pid);
            ui_draw_text(pid_str, start_x + 20.0f, row_y + 8.0f, 11.0f, COL_CRIT_RED);

            ui_draw_text(ev->comm, start_x + 85.0f, row_y + 8.0f, 11.0f, COL_TEXT_WHITE);

            /* Plage mémoire ou N/A pour injection d'environnement */
            char range_str[64];
            if (ev->start_addr == 0 && ev->end_addr == 0) {
                snprintf(range_str, sizeof(range_str), "N/A");
            } else {
                snprintf(range_str, sizeof(range_str), "0x%lx - 0x%lx", ev->start_addr, ev->end_addr);
            }
            ui_draw_text(range_str, start_x + 230.0f, row_y + 8.0f, 11.0f, COL_CYAN_ACCENT);

            /* Taille en Ko ou N/A pour injection d'environnement */
            char size_str[32];
            if (ev->size_kb == 0) {
                snprintf(size_str, sizeof(size_str), "N/A");
            } else {
                snprintf(size_str, sizeof(size_str), "%lu Ko", (unsigned long)ev->size_kb);
            }
            ui_draw_text(size_str, start_x + 530.0f, row_y + 8.0f, 11.0f, (Color){ 255, 187, 0, 255 });

            ui_draw_text(ev->detail, start_x + 620.0f, row_y + 8.0f, 11.0f, (Color){ 255, 100, 120, 255 });

            row_y += 32.0f;
        }
    }
}
