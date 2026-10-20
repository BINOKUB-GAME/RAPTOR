/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_layout.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Gestion du gabarit principal de l'interface utilisateur.
 *                   Affiche le bandeau d'en-tête (titre principal, badge de
 *                   surveillance active et déclencheur du Guide Analytique)
 *                   ainsi que la barre interactive des cinq onglets de navigation.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Découpage modulaire : extraction isolée de l'ossature de navigation.
 *   - Prise en charge des interactions de survol et de clic des onglets.
 *   - Synchronisation avec les variables globales g_ui_active_tab et
 *     g_ui_show_guide_modal.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_layout_draw_header
 *   2. ui_layout_draw_tabs
 * ============================================================================
 */

#include "ui_common.h"

/* ------------------------------------------------------------------------- */
/* 1. DESSIN DU BANDEAU D'EN-TÊTE SUPÉRIEUR                                  */
/* ------------------------------------------------------------------------- */
void ui_layout_draw_header(void) {
    int screen_w = GetScreenWidth();
    Vector2 mouse = GetMousePosition();

    const char* main_title = "RAPTOR // Système d'Audit Défensif & Traque Mémoire";
    float tw = ui_measure_text(main_title, 14.0f);
    ui_draw_text(main_title, ((float)screen_w - tw) / 2.0f, 6.0f, 14.0f, COL_TEXT_WHITE);

    float cur_x = 16.0f;
    float cur_y = 30.0f;

    ui_draw_text("RAPTOR DEFENSE", cur_x, cur_y, 13.0f, COL_CYAN_ACCENT);
    cur_x += ui_measure_text("RAPTOR DEFENSE", 13.0f) + 12.0f;

    ui_draw_text("// AUDIT MÉMOIRE & PROCESSUS", cur_x, cur_y, 13.0f, COL_TEXT_MUTED);
    cur_x += ui_measure_text("// AUDIT MÉMOIRE & PROCESSUS", 13.0f) + 16.0f;

    ui_draw_text("[ SURVEILLANCE ACTIVE ]", cur_x, cur_y, 13.0f, COL_GREEN_OK);

    Rectangle btn_guide_rec = { (float)(screen_w - 200), 24.0f, 184.0f, 28.0f };
    bool hover_guide = CheckCollisionPointRec(mouse, btn_guide_rec);

    DrawRectangleRec(btn_guide_rec, hover_guide ? COL_CARD_BG : COL_PANEL_BG);
    DrawRectangleLinesEx(btn_guide_rec, 1.0f, hover_guide ? COL_BORDER_FOCUS : COL_BORDER);

    const char* guide_label = "[ ? Guide Analytique ]";
    float g_tw = ui_measure_text(guide_label, 12.0f);
    ui_draw_text(guide_label, btn_guide_rec.x + (btn_guide_rec.width - g_tw) / 2.0f, btn_guide_rec.y + 6.0f, 12.0f, COL_TEXT_WHITE);

    if (hover_guide && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = !g_ui_show_guide_modal;
    }
}

/* ------------------------------------------------------------------------- */
/* 2. DESSIN DE LA BARRE DES ONGLETS DE NAVIGATION                           */
/* ------------------------------------------------------------------------- */
void ui_layout_draw_tabs(void) {
    int screen_w = GetScreenWidth();
    Vector2 mouse = GetMousePosition();

    const char* tab_labels[5] = {
        "[ 01 ] Vue Globale",
        "[ 02 ] Processus & RAM",
        "[ 03 ] Sockets & Réseau",
        "[ 04 ] Cahier Différentiel",
        "[ 05 ] Rapport IA"
    };

    float start_x = 16.0f;
    float tab_y = 60.0f;
    float tab_w = 180.0f;
    float tab_h = 28.0f;
    float spacing = 10.0f;

    for (int i = 0; i < 5; ++i) {
        Rectangle tab_rec = { start_x + (float)i * (tab_w + spacing), tab_y, tab_w, tab_h };
        bool is_active = (g_ui_active_tab == i);
        bool hover = CheckCollisionPointRec(mouse, tab_rec);

        if (is_active) {
            DrawRectangleRec(tab_rec, COL_BLUE_BTN);
            DrawRectangleLinesEx(tab_rec, 1.0f, COL_BORDER_FOCUS);
        } else {
            DrawRectangleRec(tab_rec, hover ? COL_CARD_BG : COL_PANEL_BG);
            DrawRectangleLinesEx(tab_rec, 1.0f, hover ? COL_BORDER_FOCUS : COL_BORDER);
        }

        float lbl_w = ui_measure_text(tab_labels[i], 12.0f);
        float lbl_x = tab_rec.x + (tab_rec.width - lbl_w) / 2.0f;
        ui_draw_text(tab_labels[i], lbl_x, tab_rec.y + 6.0f, 12.0f, is_active ? COL_TEXT_WHITE : (hover ? COL_TEXT_WHITE : COL_TEXT_MUTED));

        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            g_ui_active_tab = i;
        }
    }

    DrawLine(16, (int)(tab_y + tab_h + 8.0f), screen_w - 16, (int)(tab_y + tab_h + 8.0f), COL_BLUE_BTN);
}
