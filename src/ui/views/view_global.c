/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_global.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue principale [ 01 ] Vue Globale du système.
 *                   Affiche le voyant d'état d'intégrité globale, les 4 cartes
 *                   métriques dynamiques (CPU, RAM, Menaces, Sockets),
 *                   le classement des 5 processus les plus consommateurs
 *                   et le bouton de déclenchement d'un audit immédiat.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Nettoyage en profondeur du contexte via raptor_data_context_clear(ctx)
 *     avant ré-audit, empêchant l'empilement invisible des anciennes sections.
 *   - Ajout d'un timer de feedback visuel (s_audit_feedback_timer) animant
 *     le bouton en vert [ ✔ AUDIT EFFECTUÉ ] pendant 2 secondes au clic.
 *   - Notification textuelle dynamique confirmant la fraîcheur des données.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_view_global_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include <time.h>

static float s_audit_feedback_timer = 0.0f;

/* ------------------------------------------------------------------------- */
/* 1. RENDU DE LA VUE GLOBALE [ 01 ]                                         */
/* ------------------------------------------------------------------------- */
void ui_view_global_draw(RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    Vector2 mouse = GetMousePosition();
    float content_y = 102.0f;

    /* Décompte du retour visuel d'audit */
    float dt = GetFrameTime();
    if (s_audit_feedback_timer > 0.0f) {
        s_audit_feedback_timer -= dt;
        if (s_audit_feedback_timer < 0.0f) {
            s_audit_feedback_timer = 0.0f;
        }
    }

    /* 1. Icône de statut du tableau de bord */
    if (g_icon_dashboard.id > 0) {
        DrawTexturePro(g_icon_dashboard,
                       (Rectangle){0, 0, (float)g_icon_dashboard.width, (float)g_icon_dashboard.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    /* 2. Extraction dynamique des métriques */
    const RaptorCard* c_cpu = ui_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram = ui_find_card(ctx, "RAM");
    const RaptorCard* c_threat = ui_find_card(ctx, "MENACES");
    const RaptorCard* c_sock = ui_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs = ui_find_card(ctx, "PROCESSUS");

    bool has_threats = (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL);

    /* 3. Bandeau d'alerte défensive */
    Rectangle alert_rec = { 16.0f, content_y + 54.0f, (float)(screen_w - 32), 54.0f };
    DrawRectangleRec(alert_rec, has_threats ? (Color){ 28, 12, 16, 255 } : (Color){ 8, 24, 18, 255 });
    DrawRectangleLinesEx(alert_rec, 1.0f, has_threats ? COL_CRIT_RED : (Color){ 0, 180, 90, 255 });

    if (has_threats) {
        ui_draw_text("[ ALERTE DÉFENSIVE // ANOMALIES DÉTECTÉES ]", alert_rec.x + 14.0f, alert_rec.y + 8.0f, 13.0f, COL_CRIT_RED);
        ui_draw_text("Des processus suspects non officiels ou avec mémoire injectée sont actifs sur ce poste.",
                     alert_rec.x + 14.0f, alert_rec.y + 28.0f, 12.0f, (Color){ 255, 180, 190, 255 });
    } else {
        ui_draw_text("[ SYSTÈME SÉCURISÉ // AUCUNE INTRUSION DÉTECTÉE ]", alert_rec.x + 14.0f, alert_rec.y + 8.0f, 13.0f, COL_GREEN_OK);
        ui_draw_text("Tous les processus contrôlés sont adossés à des binaires intègres sans mémoire injectée.",
                     alert_rec.x + 14.0f, alert_rec.y + 28.0f, 12.0f, (Color){ 190, 225, 205, 255 });
    }

    /* 4. Grille des 4 cartes métriques */
    float card_y = content_y + 122.0f;
    float card_gap = 16.0f;
    float card_w = ((float)screen_w - 32.0f - (3.0f * card_gap)) / 4.0f;
    float card_h = 86.0f;

    /* Carte 1 : CPU */
    Rectangle c1 = { 16.0f, card_y, card_w, card_h };
    DrawRectangleRec(c1, COL_CARD_BG);
    DrawRectangleLinesEx(c1, 1.0f, COL_BORDER);
    ui_draw_text("CHARGE PROCESSEUR", c1.x + 12.0f, c1.y + 10.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text(c_cpu ? c_cpu->val : "0.0 %", c1.x + 12.0f, c1.y + 28.0f, 15.0f, COL_TEXT_WHITE);
    DrawRectangle((int)c1.x + 12, (int)c1.y + 56, (int)card_w - 24, 12, (Color){ 12, 22, 34, 255 });
    float cpu_w = (c_cpu ? c_cpu->pct : 0.0f);
    if (cpu_w > 100.0f) cpu_w = 100.0f;
    DrawRectangle((int)c1.x + 12, (int)c1.y + 56, (int)(((card_w - 24.0f) * cpu_w) / 100.0f), 12, COL_CYAN_ACCENT);

    /* Carte 2 : RAM */
    Rectangle c2 = { 16.0f + card_w + card_gap, card_y, card_w, card_h };
    DrawRectangleRec(c2, COL_CARD_BG);
    DrawRectangleLinesEx(c2, 1.0f, COL_BORDER);
    ui_draw_text("MÉMOIRE VIVE (RAM)", c2.x + 12.0f, c2.y + 10.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text(c_ram ? c_ram->val : "N/D", c2.x + 12.0f, c2.y + 28.0f, 15.0f, COL_TEXT_WHITE);
    DrawRectangle((int)c2.x + 12, (int)c2.y + 56, (int)card_w - 24, 12, (Color){ 12, 22, 34, 255 });
    float ram_w = (c_ram ? c_ram->pct : 0.0f);
    if (ram_w > 100.0f) ram_w = 100.0f;
    DrawRectangle((int)c2.x + 12, (int)c2.y + 56, (int)(((card_w - 24.0f) * ram_w) / 100.0f), 12, COL_CYAN_ACCENT);

    /* Carte 3 : Menaces */
    Rectangle c3 = { 16.0f + 2.0f * (card_w + card_gap), card_y, card_w, card_h };
    DrawRectangleRec(c3, COL_CARD_BG);
    DrawRectangleLinesEx(c3, 1.0f, COL_BORDER);
    ui_draw_text("MENACES / RWX", c3.x + 12.0f, c3.y + 10.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text(c_threat ? c_threat->val : "0 ANOMALIE", c3.x + 12.0f, c3.y + 28.0f, 15.0f, has_threats ? COL_CRIT_RED : COL_GREEN_OK);
    char p_sub[256];
    snprintf(p_sub, sizeof(p_sub), "%s processus analysés", c_procs ? c_procs->val : "0");
    ui_draw_text(p_sub, c3.x + 12.0f, c3.y + 56.0f, 11.0f, COL_TEXT_MUTED);

    /* Carte 4 : Sockets Réseau */
    Rectangle c4 = { 16.0f + 3.0f * (card_w + card_gap), card_y, card_w, card_h };
    DrawRectangleRec(c4, COL_CARD_BG);
    DrawRectangleLinesEx(c4, 1.0f, COL_BORDER);
    ui_draw_text("FLUX RÉSEAU ACTIFS", c4.x + 12.0f, c4.y + 10.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text(c_sock ? c_sock->val : "0 SOCKETS", c4.x + 12.0f, c4.y + 28.0f, 15.0f, COL_CYAN_ACCENT);
    ui_draw_text("TCP & UDP surveillés", c4.x + 12.0f, c4.y + 56.0f, 11.0f, COL_TEXT_MUTED);

    /* 5. Tableau Top 5 des consommateurs de mémoire */
    float top_y = card_y + card_h + 24.0f;
    ui_draw_text("TOP 5 DES CONSOMMATEURS DE MÉMOIRE :", 16.0f, top_y, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Processus utilisant actuellement le plus de RAM physique sur ce poste.", 16.0f, top_y + 18.0f, 11.0f, COL_TEXT_MUTED);

    float tbl_y = top_y + 42.0f;
    float tbl_w = (float)screen_w - 32.0f;
    float row_h = 24.0f;

    DrawRectangle(16, (int)tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);
    ui_draw_text("PID", 24.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Nom de l'éxécutable", 120.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("RAM (RSS)", 650.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Statut Dérive", 770.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Emplacement Binaire", 890.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);

    const RaptorTable* t_top = ui_find_table(ctx, "sec_proc_mem", "TOP");
    size_t limit_top5 = (t_top && t_top->row_count > 0) ? (t_top->row_count < 5 ? t_top->row_count : 5) : 0;

    for (size_t r = 0; r < 5; ++r) {
        float ry = tbl_y + row_h + ((float)r * row_h);
        DrawRectangle(16, (int)ry, (int)tbl_w, (int)row_h, (r % 2 == 0) ? (Color){ 9, 15, 24, 255 } : (Color){ 12, 20, 32, 255 });
        DrawRectangleLines(16, (int)ry, (int)tbl_w, (int)row_h, (Color){ 20, 32, 48, 255 });

        if (r < limit_top5 && t_top->rows[r].cell_texts) {
            const char* pid = t_top->rows[r].cell_texts[0];
            const char* name = t_top->rows[r].cell_texts[1];
            const char* rss = t_top->rows[r].cell_texts[2];
            const char* stat = t_top->rows[r].cell_texts[3];
            const char* exe = t_top->rows[r].cell_texts[4];

            ui_draw_text(pid ? pid : "-", 24.0f, ry + 5.0f, 12.0f, COL_TEXT_WHITE);
            ui_draw_text(name ? name : "-", 120.0f, ry + 5.0f, 12.0f, COL_TEXT_WHITE);
            ui_draw_text(rss ? rss : "-", 650.0f, ry + 5.0f, 12.0f, COL_TEXT_WHITE);
            ui_draw_text(stat ? stat : "Stable", 770.0f, ry + 5.0f, 12.0f, COL_GREEN_OK);
            ui_draw_text(exe ? exe : "-", 890.0f, ry + 5.0f, 12.0f, COL_TEXT_MUTED);
        } else {
            ui_draw_text("-", 24.0f, ry + 5.0f, 12.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 120.0f, ry + 5.0f, 12.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 650.0f, ry + 5.0f, 12.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 770.0f, ry + 5.0f, 12.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 890.0f, ry + 5.0f, 12.0f, COL_TEXT_MUTED);
        }
    }

    /* 6. Bouton de lancement d'un audit immédiat avec retour visuel */
    float btn_y = tbl_y + (6.0f * row_h) + 36.0f;
    Rectangle audit_btn_rec = { 16.0f, btn_y, 270.0f, 34.0f };
    bool hover_btn = CheckCollisionPointRec(mouse, audit_btn_rec);

    bool in_feedback = (s_audit_feedback_timer > 0.0f);

    if (in_feedback) {
        DrawRectangleRec(audit_btn_rec, (Color){ 14, 48, 28, 255 });
        DrawRectangleLinesEx(audit_btn_rec, 1.0f, COL_GREEN_OK);

        const char* btn_txt = "[ ✔ AUDIT EFFECTUÉ ]";
        float btw = ui_measure_text(btn_txt, 11.0f);
        ui_draw_text(btn_txt, audit_btn_rec.x + (audit_btn_rec.width - btw) / 2.0f, audit_btn_rec.y + 10.0f, 11.0f, COL_GREEN_OK);

        char time_label[256];
        snprintf(time_label, sizeof(time_label), "● Données actualisées en direct (%s)", g_ui_last_audit_time);
        ui_draw_text(time_label, audit_btn_rec.x + 290.0f, audit_btn_rec.y + 10.0f, 11.0f, COL_GREEN_OK);
    } else {
        DrawRectangleRec(audit_btn_rec, hover_btn ? COL_BLUE_HOVER : COL_BLUE_BTN);
        DrawRectangleLinesEx(audit_btn_rec, 1.0f, COL_BORDER_FOCUS);

        const char* btn_txt = "LANCER UN NOUVEL AUDIT IMMÉDIAT";
        float btw = ui_measure_text(btn_txt, 11.0f);
        ui_draw_text(btn_txt, audit_btn_rec.x + (audit_btn_rec.width - btw) / 2.0f, audit_btn_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);

        char time_label[256];
        snprintf(time_label, sizeof(time_label), "Horodatage du contrôle : %s", g_ui_last_audit_time);
        ui_draw_text(time_label, audit_btn_rec.x + 290.0f, audit_btn_rec.y + 10.0f, 11.0f, COL_TEXT_MUTED);
    }

    if (hover_btn && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        /* Nettoyage obligatoire pour écraser les anciennes métriques */
        raptor_data_context_clear(ctx);
        raptor_registry_run_all(ctx);

        time_t n = time(NULL);
        struct tm* tm_i = localtime(&n);
        if (tm_i) {
            strftime(g_ui_last_audit_time, sizeof(g_ui_last_audit_time), "%Y-%m-%d %H:%M:%S", tm_i);
        }

        /* Déclenchement du retour visuel pendant 2 secondes */
        s_audit_feedback_timer = 2.0f;
    }
}
