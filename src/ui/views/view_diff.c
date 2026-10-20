/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_diff.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue dédiée [ 04 ] Cahier Différentiel de l'interface Raylib.
 *                   Affiche l'analyse comparative temporelle (T-1 vs T0),
 *                   le statut d'évolution de la mémoire (STABLE / DÉRIVE),
 *                   le taux de dérive calculé et l'évaluation du risque de fuite.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Découpage modulaire : extraction isolée du cahier différentiel.
 *   - Consommation dynamique de la table sec_differential du contexte.
 *   - Calcul dynamique du nombre de lignes visibles selon la hauteur d'écran.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_view_diff_draw
 * ============================================================================
 */

#include "../ui_common.h"

/* ------------------------------------------------------------------------- */
/* 1. RENDU DE LA VUE CAHIER DIFFÉRENTIEL [ 04 ]                             */
/* ------------------------------------------------------------------------- */
void ui_view_diff_draw(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    float content_y = 102.0f;

    /* 1. Icône de section */
    if (g_icon_diff.id > 0) {
        DrawTexturePro(g_icon_diff,
                       (Rectangle){0, 0, (float)g_icon_diff.width, (float)g_icon_diff.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("CAHIER DE BORD // ANALYSE DIFFÉRENTIELLE TEMPORELLE", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Surveillance des variations entre deux audits consécutifs (T-1 vs T0).",
                 16.0f, content_y + 54.0f, 11.0f, COL_TEXT_MUTED);

    /* 2. En-tête du tableau différentiel */
    float tbl_y = content_y + 80.0f;
    float tbl_w = (float)screen_w - 32.0f;
    float row_h = 22.0f;

    DrawRectangle(16, (int)tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);

    ui_draw_text("Processus (PID)", 24.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Statut Évolution", 340.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("RAM Actuelle", 460.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Taux Dérive", 560.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Évaluation Risque", 680.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);

    /* 3. Extraction de la table différentielle */
    const RaptorTable* diff_tbl = ui_find_table(ctx, "sec_differential", NULL);

    int max_rows = (GetScreenHeight() - (int)tbl_y - 40) / (int)row_h;
    size_t rows_to_draw = (diff_tbl && diff_tbl->row_count > 0)
                          ? (diff_tbl->row_count < (size_t)max_rows ? diff_tbl->row_count : (size_t)max_rows)
                          : 1;

    for (size_t r = 0; r < rows_to_draw; ++r) {
        float ry = tbl_y + row_h + ((float)r * row_h);
        DrawRectangle(16, (int)ry, (int)tbl_w, (int)row_h, (r % 2 == 0) ? (Color){ 9, 15, 24, 255 } : (Color){ 12, 20, 32, 255 });
        DrawRectangleLines(16, (int)ry, (int)tbl_w, (int)row_h, (Color){ 20, 32, 48, 255 });

        if (diff_tbl && diff_tbl->row_count > r && diff_tbl->rows[r].cell_texts) {
            ui_draw_text(diff_tbl->rows[r].cell_texts[0], 24.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            const char* st = diff_tbl->rows[r].cell_texts[1];
            ui_draw_text(st, 340.0f, ry + 4.0f, 11.0f, strcmp(st, "STABLE") == 0 ? COL_GREEN_OK : COL_CRIT_RED);
            ui_draw_text(diff_tbl->rows[r].cell_texts[2], 460.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(diff_tbl->rows[r].cell_texts[3], 560.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(diff_tbl->rows[r].cell_texts[4], 680.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
        } else {
            ui_draw_text("-", 24.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("STABLE", 340.0f, ry + 4.0f, 11.0f, COL_GREEN_OK);
            ui_draw_text("-", 460.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("0.0 Mo/min", 560.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text("Aucune modification notable constatée entre les deux audits", 680.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
        }
    }
}
