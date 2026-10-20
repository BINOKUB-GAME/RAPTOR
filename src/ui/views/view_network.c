/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_network.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue dédiée [ 03 ] Sockets & Réseau de l'interface graphique.
 *                   Affiche les flux réseau actifs résolus (TCP/UDP), les
 *                   points de terminaison locaux et distants, les logiciels
 *                   responsables identifiés et la table d'intégrité mémoire RWX
 *                   sans aucune superposition de texte.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Découpage modulaire : extraction complète de la vue réseau.
 *   - Rendu dynamique relié directement à sec_network et sec_proc_mem.
 *   - Correction structurelle du tableau RWX : affichage propre et lisible
 *     pleine largeur en l'absence d'anomalie, neutralisant toute collision.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_view_network_draw
 * ============================================================================
 */

#include "../ui_common.h"

/* ------------------------------------------------------------------------- */
/* 1. RENDU DE LA VUE SÉCURITÉ RÉSEAU & RWX [ 03 ]                           */
/* ------------------------------------------------------------------------- */
void ui_view_network_draw(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    float content_y = 102.0f;

    /* 1. Icône de section réseau */
    if (g_icon_network.id > 0) {
        DrawTexturePro(g_icon_network,
                       (Rectangle){0, 0, (float)g_icon_network.width, (float)g_icon_network.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("SÉCURITÉ RÉSEAU & DÉTECTION D'INJECTIONS (RWX)", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Analyse en direct des sockets ouverts et des descripteurs mémoire du noyau.", 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

    /* 2. Tableau des flux réseau actifs */
    const RaptorTable* net_tbl = ui_find_table(ctx, "sec_network", "Flux");
    size_t net_count = net_tbl ? net_tbl->row_count : 0;

    char net_header[128];
    snprintf(net_header, sizeof(net_header), "Connexions réseau actives (%zu détectées) :", net_count);

    float net_y = content_y + 54.0f;
    ui_draw_text(net_header, 16.0f, net_y, 12.0f, (Color){ 255, 204, 0, 255 });

    float tbl_y = net_y + 24.0f;
    float tbl_w = (float)screen_w - 32.0f;
    float row_h = 22.0f;

    DrawRectangle(16, (int)tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);
    ui_draw_text("PID Responsable", 24.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Proto", 200.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Adresse Locale", 280.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Adresse Distante", 580.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("État de Connexion", 900.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);

    size_t draw_net_limit = (net_tbl && net_tbl->row_count < 8) ? net_tbl->row_count : 8;
    for (size_t r = 0; r < 8; ++r) {
        float ry = tbl_y + row_h + ((float)r * row_h);
        DrawRectangle(16, (int)ry, (int)tbl_w, (int)row_h, (r % 2 == 0) ? (Color){ 9, 15, 24, 255 } : (Color){ 12, 20, 32, 255 });
        DrawRectangleLines(16, (int)ry, (int)tbl_w, (int)row_h, (Color){ 20, 32, 48, 255 });

        if (r < draw_net_limit && net_tbl && net_tbl->rows[r].cell_texts) {
            ui_draw_text(net_tbl->rows[r].cell_texts[0], 24.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text(net_tbl->rows[r].cell_texts[1], 200.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(net_tbl->rows[r].cell_texts[2], 280.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(net_tbl->rows[r].cell_texts[3], 580.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            const char* st = net_tbl->rows[r].cell_texts[4];
            ui_draw_text(st, 900.0f, ry + 4.0f, 11.0f, strcmp(st, "ESTABLISHED") == 0 ? COL_GREEN_OK : COL_TEXT_WHITE);
        } else {
            ui_draw_text("-", 24.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 200.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 280.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 580.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 900.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);
        }
    }

    /* 3. Tableau des processus RWX / Anomalies mémoires */
    float rwx_y = tbl_y + (9.0f * row_h) + 24.0f;
    ui_draw_text("Processus avec mémoire exécutable non officielle (RWX) :", 16.0f, rwx_y, 12.0f, (Color){ 255, 187, 0, 255 });

    float rwx_tbl_y = rwx_y + 24.0f;
    DrawRectangle(16, (int)rwx_tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)rwx_tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);
    ui_draw_text("PID", 24.0f, rwx_tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Nom du Processus", 90.0f, rwx_tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Statut d'Injection", 320.0f, rwx_tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Chemin de l'éxécutable", 520.0f, rwx_tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);

    const RaptorTable* real_anomalies_tbl = ui_find_table(ctx, "sec_proc_mem", "ANOMALIES");
    float rwx_ry = rwx_tbl_y + row_h;
    DrawRectangle(16, (int)rwx_ry, (int)tbl_w, (int)(row_h * 4.0f), (Color){ 9, 15, 24, 255 });
    DrawRectangleLines(16, (int)rwx_ry, (int)tbl_w, (int)(row_h * 4.0f), (Color){ 20, 32, 48, 255 });

    if (real_anomalies_tbl && real_anomalies_tbl->row_count > 0 && real_anomalies_tbl->rows[0].cell_texts) {
        const char* p0 = real_anomalies_tbl->rows[0].cell_texts[0];
        const char* p1 = real_anomalies_tbl->rows[0].cell_texts[1];
        bool is_clean = (strcmp(p0, "-") == 0);

        if (is_clean) {
            ui_draw_text("• Aucun processus suspect détecté (Tous les binaires sont conformes aux dépôts officiels).",
                         24.0f, rwx_ry + 10.0f, 12.0f, COL_GREEN_OK);
        } else {
            ui_draw_text(p0, 24.0f, rwx_ry + 6.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text(p1, 90.0f, rwx_ry + 6.0f, 11.0f, COL_CRIT_RED);
            ui_draw_text(real_anomalies_tbl->rows[0].cell_texts[4], 320.0f, rwx_ry + 6.0f, 11.0f, COL_CRIT_RED);
            ui_draw_text(real_anomalies_tbl->rows[0].cell_texts[5], 520.0f, rwx_ry + 6.0f, 11.0f, COL_TEXT_MUTED);
        }
    } else {
        ui_draw_text("• Aucune injection détectée.", 24.0f, rwx_ry + 10.0f, 12.0f, COL_GREEN_OK);
    }
}
