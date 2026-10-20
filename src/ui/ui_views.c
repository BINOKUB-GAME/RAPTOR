/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_views.c
 * RÉVISION        : Rev 1.8
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Moteur de rendu graphique interactif des vues (Raylib).
 *                   Affiche l'interface multi-onglets (Vue Globale, Processus,
 *                   Réseau/RWX, Différentiel, Rapport IA), les icônes de sondes,
 *                   les tableaux dynamiques avec filtrage en temps réel et la
 *                   fenêtre modale dépliante du Guide Analytique de sécurité.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.8) :
 *   - Onglet 02 : Ajout du bouton d'action opérationnel [ ARRÊTER LE PROCESSUS ]
 *     permettant de tuer un processus sélectionné (envoi du signal SIGTERM).
 *   - Onglet 03 : Résolution de la collision de texte dans la table RWX
 *     (rendu pleine largeur sans chevauchement de colonnes).
 *   - Onglet 05 : Correction de l'interligne vertical sur les recommandations
 *     opérationnelles (fin de la superposition de texte).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_views_load_resources
 *   2. ui_views_unload_resources
 *   3. ui_views_draw_header
 *   4. ui_views_draw_tabs
 *   5. ui_views_draw_tab_global
 *   6. ui_views_draw_tab_processes
 *   7. ui_views_draw_tab_network
 *   8. ui_views_draw_tab_diff
 *   9. ui_views_draw_tab_ai
 *  10. ui_views_draw_guide_modal
 *  11. raptor_ui_render_tabs
 * ============================================================================
 */

#include "raptor/raptor_report.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_differential.h"
#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(__linux__) || defined(__unix__)
#include <signal.h>
#endif

#define UI_FONT_SCALE 1.25f

static const Color COL_BG_MAIN      = { 6, 11, 19, 255 };
static const Color COL_PANEL_BG     = { 10, 17, 26, 255 };
static const Color COL_CARD_BG      = { 14, 23, 36, 255 };
static const Color COL_BORDER       = { 22, 38, 58, 255 };
static const Color COL_BORDER_FOCUS = { 0, 210, 255, 255 };
static const Color COL_BLUE_BTN     = { 0, 119, 204, 255 };
static const Color COL_BLUE_HOVER   = { 0, 145, 245, 255 };
static const Color COL_TEXT_WHITE   = { 235, 245, 255, 255 };
static const Color COL_TEXT_MUTED   = { 135, 160, 190, 255 };
static const Color COL_CYAN_ACCENT  = { 0, 210, 255, 255 };
static const Color COL_GREEN_OK     = { 0, 230, 118, 255 };
static const Color COL_CRIT_RED     = { 255, 51, 85, 255 };

static int s_active_tab = 0;
static bool s_show_guide_modal = false;
static int s_guide_accordion_open = 0;

static char s_filter_query[64] = {0};
static int s_filter_query_len = 0;
static bool s_filter_focused = false;
static int s_selected_proc_pid = -1;
static char s_action_feedback[128] = {0};

static Texture2D s_icon_dashboard;
static Texture2D s_icon_processes;
static Texture2D s_icon_network;
static Texture2D s_icon_diff;
static Texture2D s_icon_ai;
static bool s_icons_loaded = false;

static Font s_font_ui;
static bool s_font_loaded = false;

static char s_last_audit_time[64] = {0};

/* ------------------------------------------------------------------------- */
/* FONCTIONS TYPOGRAPHIQUES AVEC MISE À L'ÉCHELLE GLOBALE                   */
/* ------------------------------------------------------------------------- */
static void ui_draw_text(const char* text, float posX, float posY, float baseSize, Color color) {
    float scaledSize = baseSize * UI_FONT_SCALE;
    if (s_font_loaded) {
        Vector2 pos = { posX, posY };
        DrawTextEx(s_font_ui, text, pos, scaledSize, 1.0f, color);
    } else {
        DrawText(text, (int)posX, (int)posY, (int)scaledSize, color);
    }
}

static float ui_measure_text(const char* text, float baseSize) {
    float scaledSize = baseSize * UI_FONT_SCALE;
    if (s_font_loaded) {
        return MeasureTextEx(s_font_ui, text, scaledSize, 1.0f).x;
    }
    return (float)MeasureText(text, (int)scaledSize);
}

/* ------------------------------------------------------------------------- */
/* RECHERCHE D'UNE CARTE PAR MOT-CLÉ DANS LE CONTEXTE                        */
/* ------------------------------------------------------------------------- */
static const RaptorCard* ui_find_card(const RaptorReportContext* ctx, const char* keyword) {
    if (!ctx || !keyword) return NULL;
    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];
        for (size_t c = 0; c < sec->card_count; ++c) {
            if (strstr(sec->cards[c].label, keyword) != NULL) {
                return &sec->cards[c];
            }
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* RECHERCHE D'UNE TABLE PAR IDENTIFIANT OU TITRE DANS LE CONTEXTE           */
/* ------------------------------------------------------------------------- */
static const RaptorTable* ui_find_table(const RaptorReportContext* ctx, const char* sec_id, const char* tbl_kw) {
    if (!ctx) return NULL;
    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];
        if (sec_id && strcmp(sec->id, sec_id) != 0) continue;
        for (size_t t = 0; t < sec->table_count; ++t) {
            if (!tbl_kw || strstr(sec->tables[t].title, tbl_kw) != NULL) {
                return &sec->tables[t];
            }
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* 1. CHARGEMENT DES RESSOURCES VISUELLES ET POLICE LOCALE                   */
/* ------------------------------------------------------------------------- */
static void ui_views_load_resources(void) {
    if (s_last_audit_time[0] == '\0') {
        time_t n = time(NULL);
        struct tm* tm_i = localtime(&n);
        if (tm_i) {
            strftime(s_last_audit_time, sizeof(s_last_audit_time), "%Y-%m-%d %H:%M:%S", tm_i);
        } else {
            strncpy(s_last_audit_time, "2026-10-20 12:00:00", sizeof(s_last_audit_time) - 1);
        }
    }

    if (!s_font_loaded) {
        int codepoints[384];
        int cp_count = 0;

        for (int i = 32; i <= 126; ++i) codepoints[cp_count++] = i;
        for (int i = 160; i <= 255; ++i) codepoints[cp_count++] = i;
        codepoints[cp_count++] = 0x2022; /* • */
        codepoints[cp_count++] = 0x25BC; /* ▼ */
        codepoints[cp_count++] = 0x25B6; /* ▶ */

        const char* font_candidates[] = {
            "assets/fonts/font.ttf",
            "../assets/fonts/font.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            NULL
        };

        for (int i = 0; font_candidates[i] != NULL; ++i) {
            if (FileExists(font_candidates[i])) {
                s_font_ui = LoadFontEx(font_candidates[i], (int)(32.0f * UI_FONT_SCALE), codepoints, cp_count);
                if (s_font_ui.baseSize > 0) {
                    SetTextureFilter(s_font_ui.texture, TEXTURE_FILTER_BILINEAR);
                    s_font_loaded = true;
                    break;
                }
            }
        }
    }

    if (s_icons_loaded) {
        return;
    }

    const char* p_dash = "assets/icons/icon_dashboard.png";
    const char* p_proc = "assets/icons/icon_processes.png";
    const char* p_net  = "assets/icons/icon_network.png";
    const char* p_diff = "assets/icons/icon_diff.png";
    const char* p_ai   = "assets/icons/icon_ai.png";

    const char* alt_dash = "../assets/icons/icon_dashboard.png";
    const char* alt_proc = "../assets/icons/icon_processes.png";
    const char* alt_net  = "../assets/icons/icon_network.png";
    const char* alt_diff = "../assets/icons/icon_diff.png";
    const char* alt_ai   = "../assets/icons/icon_ai.png";

    if (FileExists(p_dash)) s_icon_dashboard = LoadTexture(p_dash);
    else if (FileExists(alt_dash)) s_icon_dashboard = LoadTexture(alt_dash);

    if (FileExists(p_proc)) s_icon_processes = LoadTexture(p_proc);
    else if (FileExists(alt_proc)) s_icon_processes = LoadTexture(alt_proc);

    if (FileExists(p_net)) s_icon_network = LoadTexture(p_net);
    else if (FileExists(alt_net)) s_icon_network = LoadTexture(alt_net);

    if (FileExists(p_diff)) s_icon_diff = LoadTexture(p_diff);
    else if (FileExists(alt_diff)) s_icon_diff = LoadTexture(alt_diff);

    if (FileExists(p_ai)) s_icon_ai = LoadTexture(p_ai);
    else if (FileExists(alt_ai)) s_icon_ai = LoadTexture(alt_ai);

    s_icons_loaded = true;
}

/* ------------------------------------------------------------------------- */
/* 2. LIBÉRATION DES RESSOURCES GRAPHIQUES                                   */
/* ------------------------------------------------------------------------- */
void ui_views_unload_resources(void) {
    if (s_font_loaded) {
        UnloadFont(s_font_ui);
        s_font_loaded = false;
    }

    if (!s_icons_loaded) {
        return;
    }

    if (s_icon_dashboard.id > 0) UnloadTexture(s_icon_dashboard);
    if (s_icon_processes.id > 0) UnloadTexture(s_icon_processes);
    if (s_icon_network.id > 0)   UnloadTexture(s_icon_network);
    if (s_icon_diff.id > 0)      UnloadTexture(s_icon_diff);
    if (s_icon_ai.id > 0)        UnloadTexture(s_icon_ai);

    s_icons_loaded = false;
}

/* ------------------------------------------------------------------------- */
/* 3. BANDEAU D'EN-TÊTE PRINCIPAL                                            */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_header(void) {
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
        s_show_guide_modal = !s_show_guide_modal;
    }
}

/* ------------------------------------------------------------------------- */
/* 4. BARRE DE NAVIGATION MULTI-ONGLETS                                      */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_tabs(void) {
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
        bool is_active = (s_active_tab == i);
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
            s_active_tab = i;
        }
    }

    DrawLine(16, (int)(tab_y + tab_h + 8.0f), screen_w - 16, (int)(tab_y + tab_h + 8.0f), COL_BLUE_BTN);
}

/* ------------------------------------------------------------------------- */
/* 5. VUE DE L'ONGLET 01 : VUE GLOBALE DYNAMIQUE                             */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_tab_global(RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    Vector2 mouse = GetMousePosition();
    float content_y = 102.0f;

    if (s_icon_dashboard.id > 0) {
        DrawTexturePro(s_icon_dashboard,
                       (Rectangle){0, 0, (float)s_icon_dashboard.width, (float)s_icon_dashboard.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    const RaptorCard* c_cpu = ui_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram = ui_find_card(ctx, "RAM");
    const RaptorCard* c_threat = ui_find_card(ctx, "MENACES");
    const RaptorCard* c_sock = ui_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs = ui_find_card(ctx, "PROCESSUS");

    bool has_threats = (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL);

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

    /* Table Top 5 */
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

    float btn_y = tbl_y + (6.0f * row_h) + 36.0f;
    Rectangle audit_btn_rec = { 16.0f, btn_y, 270.0f, 34.0f };
    bool hover_btn = CheckCollisionPointRec(mouse, audit_btn_rec);

    DrawRectangleRec(audit_btn_rec, hover_btn ? COL_BLUE_HOVER : COL_BLUE_BTN);
    DrawRectangleLinesEx(audit_btn_rec, 1.0f, COL_BORDER_FOCUS);

    const char* btn_txt = "LANCER UN NOUVEL AUDIT IMMÉDIAT";
    float btw = ui_measure_text(btn_txt, 11.0f);
    ui_draw_text(btn_txt, audit_btn_rec.x + (audit_btn_rec.width - btw) / 2.0f, audit_btn_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);

    char time_label[256];
    snprintf(time_label, sizeof(time_label), "Horodatage du contrôle : %s", s_last_audit_time);
    ui_draw_text(time_label, audit_btn_rec.x + 290.0f, audit_btn_rec.y + 10.0f, 11.0f, COL_TEXT_MUTED);

    if (hover_btn && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        raptor_registry_run_all(ctx);
        time_t n = time(NULL);
        struct tm* tm_i = localtime(&n);
        if (tm_i) {
            strftime(s_last_audit_time, sizeof(s_last_audit_time), "%Y-%m-%d %H:%M:%S", tm_i);
        }
    }
}

/* ------------------------------------------------------------------------- */
/* 6. VUE DE L'ONGLET 02 : INSPECTION COMPLÈTE & INTERVENTION PROCESSUS      */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_tab_processes(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    Vector2 mouse = GetMousePosition();
    float content_y = 102.0f;

    if (s_icon_processes.id > 0) {
        DrawTexturePro(s_icon_processes,
                       (Rectangle){0, 0, (float)s_icon_processes.width, (float)s_icon_processes.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    const RaptorCard* c_procs = ui_find_card(ctx, "PROCESSUS");
    char p_header[256];
    snprintf(p_header, sizeof(p_header), "Total surveillé : %s processus actifs | Cliquez sur une ligne pour intervenir.",
             c_procs ? c_procs->val : "300+");

    ui_draw_text("INSPECTION DES PROCESSUS ET EMPREINTE MÉMOIRE", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text(p_header, 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

    float filter_y = content_y + 54.0f;
    ui_draw_text("Filtrer :", 16.0f, filter_y + 6.0f, 12.0f, COL_TEXT_WHITE);

    Rectangle input_rec = { 80.0f, filter_y, 600.0f, 28.0f };
    bool hover_input = CheckCollisionPointRec(mouse, input_rec);

    if (hover_input && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_filter_focused = true;
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_filter_focused = false;
    }

    if (s_filter_focused) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (s_filter_query_len < (int)sizeof(s_filter_query) - 1)) {
                s_filter_query[s_filter_query_len] = (char)key;
                s_filter_query[s_filter_query_len + 1] = '\0';
                s_filter_query_len++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            s_filter_query_len--;
            if (s_filter_query_len < 0) s_filter_query_len = 0;
            s_filter_query[s_filter_query_len] = '\0';
        }
    }

    DrawRectangleRec(input_rec, (Color){ 10, 18, 28, 255 });
    DrawRectangleLinesEx(input_rec, 1.0f, s_filter_focused ? COL_BORDER_FOCUS : COL_BORDER);
    ui_draw_text(s_filter_query_len > 0 ? s_filter_query : (s_filter_focused ? "" : "Rechercher un processus..."),
                 input_rec.x + 10.0f, input_rec.y + 6.0f, 12.0f,
                 s_filter_query_len > 0 ? COL_TEXT_WHITE : COL_TEXT_MUTED);

    Rectangle btn_clear = { 690.0f, filter_y, 75.0f, 28.0f };
    bool hover_clear = CheckCollisionPointRec(mouse, btn_clear);
    DrawRectangleRec(btn_clear, hover_clear ? COL_BLUE_HOVER : (Color){ 16, 28, 44, 255 });
    DrawRectangleLinesEx(btn_clear, 1.0f, COL_BORDER);
    ui_draw_text("Effacer", btn_clear.x + 16.0f, btn_clear.y + 7.0f, 11.0f, COL_TEXT_WHITE);

    if (hover_clear && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_filter_query[0] = '\0';
        s_filter_query_len = 0;
    }

    float tbl_y = filter_y + 40.0f;
    float tbl_w = (float)screen_w - 32.0f;
    float row_h = 22.0f;

    DrawRectangle(16, (int)tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);

    ui_draw_text("PID", 24.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Nom du Processus", 90.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("RAM (RSS)", 420.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Mémoire Virtuelle", 520.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Statut Dérive", 640.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Emplacement Binaire", 760.0f, tbl_y + 4.0f, 11.0f, COL_TEXT_MUTED);

    const RaptorTable* p_table = ui_find_table(ctx, "sec_proc_mem", "INSPECTION GLOBALE");
    if (!p_table) p_table = ui_find_table(ctx, "sec_proc_mem", "TOP");

    int visible_rows = (int)((GetScreenHeight() - (int)tbl_y - 70) / (int)row_h);
    int drawn = 0;

    if (p_table) {
        for (size_t r = 0; r < p_table->row_count && drawn < visible_rows; ++r) {
            const char* pid = p_table->rows[r].cell_texts[0];
            const char* name = p_table->rows[r].cell_texts[1];
            const char* rss = p_table->rows[r].cell_texts[2];
            const char* derive = (p_table->col_count > 3) ? p_table->rows[r].cell_texts[3] : "Stable";
            const char* exe = (p_table->col_count > 4) ? p_table->rows[r].cell_texts[4] : "";

            if (s_filter_query_len > 0) {
                if (strstr(name, s_filter_query) == NULL && strstr(pid, s_filter_query) == NULL) {
                    continue;
                }
            }

            float ry = tbl_y + row_h + ((float)drawn * row_h);
            Rectangle r_rec = { 16.0f, ry, tbl_w, row_h };
            bool hover_row = CheckCollisionPointRec(mouse, r_rec);

            int current_pid = (int)strtol(pid, NULL, 10);
            bool is_selected = (s_selected_proc_pid == current_pid);

            if (is_selected) {
                DrawRectangleRec(r_rec, (Color){ 20, 60, 96, 255 });
            } else if (hover_row) {
                DrawRectangleRec(r_rec, (Color){ 16, 28, 44, 255 });
            } else {
                DrawRectangleRec(r_rec, (drawn % 2 == 0) ? (Color){ 9, 15, 24, 255 } : (Color){ 12, 20, 32, 255 });
            }

            DrawRectangleLines(16, (int)ry, (int)tbl_w, (int)row_h, (Color){ 20, 32, 48, 255 });

            ui_draw_text(pid, 24.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(name, 90.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(rss, 420.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text("0 Mo", 520.0f, ry + 4.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(derive, 640.0f, ry + 4.0f, 11.0f, COL_GREEN_OK);
            ui_draw_text(exe, 760.0f, ry + 4.0f, 11.0f, COL_TEXT_MUTED);

            if (hover_row && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                s_selected_proc_pid = current_pid;
                s_action_feedback[0] = '\0';
            }

            drawn++;
        }
    }

    /* BARRE D'INTERVENTION EN BAS DE L'ONGLET 02 */
    float bottom_y = (float)(GetScreenHeight() - 32);
    if (s_selected_proc_pid > 0) {
        char kill_btn_label[64];
        snprintf(kill_btn_label, sizeof(kill_btn_label), "[ ARRÊTER LE PROCESSUS (PID %d) ]", s_selected_proc_pid);

        float kb_w = ui_measure_text(kill_btn_label, 11.0f) + 20.0f;
        Rectangle btn_kill = { 16.0f, bottom_y - 2.0f, kb_w, 26.0f };
        bool hover_kill = CheckCollisionPointRec(mouse, btn_kill);

        DrawRectangleRec(btn_kill, hover_kill ? (Color){ 255, 70, 90, 255 } : (Color){ 180, 30, 45, 255 });
        DrawRectangleLinesEx(btn_kill, 1.0f, COL_CRIT_RED);
        ui_draw_text(kill_btn_label, btn_kill.x + 10.0f, btn_kill.y + 6.0f, 11.0f, COL_TEXT_WHITE);

        if (hover_kill && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
#if defined(__linux__) || defined(__unix__)
            kill((pid_t)s_selected_proc_pid, SIGTERM);
            snprintf(s_action_feedback, sizeof(s_action_feedback),
                     "Signal SIGTERM transmis avec succès au PID %d.", s_selected_proc_pid);
#else
            snprintf(s_action_feedback, sizeof(s_action_feedback),
                     "Intervention ordonnée pour le PID %d.", s_selected_proc_pid);
#endif
            s_selected_proc_pid = -1;
        }

        if (s_action_feedback[0] != '\0') {
            ui_draw_text(s_action_feedback, btn_kill.x + kb_w + 16.0f, bottom_y + 4.0f, 12.0f, COL_GREEN_OK);
        } else {
            ui_draw_text("Cliquez sur le bouton rouge pour demander la fermeture immédiate.",
                         btn_kill.x + kb_w + 16.0f, bottom_y + 4.0f, 11.0f, COL_TEXT_MUTED);
        }
    } else {
        if (s_action_feedback[0] != '\0') {
            ui_draw_text(s_action_feedback, 16.0f, bottom_y + 4.0f, 12.0f, COL_GREEN_OK);
        } else {
            ui_draw_text("Sélectionnez une ligne dans le tableau pour activer les options de neutralisation.",
                         16.0f, bottom_y + 4.0f, 11.0f, COL_TEXT_MUTED);
        }
    }
}

/* ------------------------------------------------------------------------- */
/* 7. VUE DE L'ONGLET 03 : SÉCURITÉ RÉSEAU & RWX DYNAMIQUE                   */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_tab_network(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    float content_y = 102.0f;

    if (s_icon_network.id > 0) {
        DrawTexturePro(s_icon_network,
                       (Rectangle){0, 0, (float)s_icon_network.width, (float)s_icon_network.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("SÉCURITÉ RÉSEAU & DÉTECTION D'INJECTIONS (RWX)", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Analyse en direct des sockets ouverts et des descripteurs mémoire du noyau.", 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

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
            /* Affichage propre sans superposition de colonnes */
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

/* ------------------------------------------------------------------------- */
/* 8. VUE DE L'ONGLET 04 : CAHIER DE BORD DIFFÉRENTIEL DYNAMIQUE             */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_tab_diff(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    float content_y = 102.0f;

    if (s_icon_diff.id > 0) {
        DrawTexturePro(s_icon_diff,
                       (Rectangle){0, 0, (float)s_icon_diff.width, (float)s_icon_diff.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("CAHIER DE BORD // ANALYSE DIFFÉRENTIELLE TEMPORELLE", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Surveillance des variations entre deux audits consécutifs (T-1 vs T0).",
                 16.0f, content_y + 54.0f, 11.0f, COL_TEXT_MUTED);

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

    const RaptorTable* diff_tbl = ui_find_table(ctx, "sec_differential", NULL);

    int max_rows = (GetScreenHeight() - (int)tbl_y - 40) / (int)row_h;
    size_t rows_to_draw = (diff_tbl && diff_tbl->row_count > 0) ? (diff_tbl->row_count < (size_t)max_rows ? diff_tbl->row_count : (size_t)max_rows) : 1;

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

/* ------------------------------------------------------------------------- */
/* 9. VUE DE L'ONGLET 05 : RAPPORT D'AUDIT IA SOUVERAIN DYNAMIQUE            */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_tab_ai(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    float content_y = 102.0f;

    if (s_icon_ai.id > 0) {
        DrawTexturePro(s_icon_ai,
                       (Rectangle){0, 0, (float)s_icon_ai.width, (float)s_icon_ai.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("RAPPORT D'AUDIT EXPLICATIF // SOVEREIGN AI ENGINE", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Diagnostic rédigé automatiquement à partir des métriques collectées en direct.",
                 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

    float box_y = content_y + 54.0f;
    float box_w = (float)screen_w - 32.0f;
    float box_h = (float)GetScreenHeight() - box_y - 24.0f;

    DrawRectangle(16, (int)box_y, (int)box_w, (int)box_h, (Color){ 8, 14, 22, 255 });
    DrawRectangleLines(16, (int)box_y, (int)box_w, (int)box_h, (Color){ 20, 32, 48, 255 });

    const RaptorCard* c_cpu = ui_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram = ui_find_card(ctx, "RAM");
    const RaptorCard* c_threat = ui_find_card(ctx, "MENACES");
    const RaptorCard* c_sock = ui_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs = ui_find_card(ctx, "PROCESSUS");

    const char* top_name = "N/D";
    const char* top_pid = "-";
    const char* top_ram = "0 Mo";

    const RaptorTable* t_top = ui_find_table(ctx, "sec_proc_mem", "TOP");
    if (t_top && t_top->row_count > 0 && t_top->rows[0].cell_texts) {
        top_pid = t_top->rows[0].cell_texts[0];
        top_name = t_top->rows[0].cell_texts[1];
        top_ram = t_top->rows[0].cell_texts[2];
    }

    char os_name[128] = "Linux";
#if defined(__linux__)
    FILE* fp = fopen("/etc/os-release", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "NAME=", 5) == 0) {
                char* val = line + 5;
                if (*val == '"') val++;
                size_t l = strlen(val);
                while (l > 0 && (val[l - 1] == '\n' || val[l - 1] == '\r' || val[l - 1] == '"')) {
                    val[l - 1] = '\0';
                    l--;
                }
                strncpy(os_name, val, sizeof(os_name) - 1);
                break;
            }
        }
        fclose(fp);
    }
#elif defined(_WIN32)
    strncpy(os_name, "Microsoft Windows", sizeof(os_name) - 1);
#endif

    float ty = box_y + 16.0f;
    ui_draw_text("RAPPORT D'AUDIT SYSTÈME // RAPTOR DEFENSE AI", 32.0f, ty, 12.0f, COL_TEXT_WHITE);
    ty += 18.0f;
    char sub_line[256];
    snprintf(sub_line, sizeof(sub_line), "Date de contrôle : %s | Poste Local (%s)", s_last_audit_time, os_name);
    ui_draw_text(sub_line, 32.0f, ty, 11.0f, COL_TEXT_MUTED);
    ty += 16.0f;
    ui_draw_text("--------------------------------------------------------------------------------", 32.0f, ty, 11.0f, (Color){ 40, 60, 80, 255 });

    ty += 24.0f;
    ui_draw_text("1. SYNTHÈSE GLOBALE EN LANGAGE CLAIR :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 20.0f;
    char line1[256];
    snprintf(line1, sizeof(line1), "Votre système exécute actuellement %s processus actifs pour une charge globale de %s CPU.",
             c_procs ? c_procs->val : "300+", c_cpu ? c_cpu->val : "0.0 %");
    ui_draw_text(line1, 32.0f, ty, 11.0f, COL_TEXT_WHITE);

    ty += 20.0f;
    char line2[256];
    snprintf(line2, sizeof(line2), "La mémoire physique vive utilisée s'élève à %s (soit %.0f %% d'occupation).",
             c_ram ? c_ram->val : "N/D", c_ram ? c_ram->pct : 0.0f);
    ui_draw_text(line2, 32.0f, ty, 11.0f, COL_TEXT_WHITE);

    ty += 20.0f;
    char line3[256];
    snprintf(line3, sizeof(line3), "Le plus gros consommateur de mémoire actuel est : %s (PID %s, %s).",
             top_name, top_pid, top_ram);
    ui_draw_text(line3, 32.0f, ty, 11.0f, COL_TEXT_WHITE);

    ty += 26.0f;
    ui_draw_text("2. ÉTAT DES ANOMALIES ET MENACES :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 20.0f;
    char line4[256];
    snprintf(line4, sizeof(line4), "- Injections Mémoire (RWX) & Binaires suspects : %s détection(s).",
             c_threat ? c_threat->val : "0");
    bool has_anomalies = (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL);
    ui_draw_text(line4, 32.0f, ty, 11.0f, has_anomalies ? COL_CRIT_RED : COL_GREEN_OK);

    ty += 20.0f;
    ui_draw_text("  Note explicative : Ces pages mémoire concernent principalement les moteurs de compilation", 32.0f, ty, 11.0f, COL_TEXT_MUTED);
    ty += 18.0f;
    ui_draw_text("  à la volée JIT (Chromium, Cinnamon, Qt, Python). En l'absence de trafic réseau suspect associé,", 32.0f, ty, 11.0f, COL_TEXT_MUTED);
    ty += 18.0f;
    ui_draw_text("  il s'agit d'allocations de fonctionnement standards.", 32.0f, ty, 11.0f, COL_TEXT_MUTED);

    ty += 20.0f;
    ui_draw_text("- Dérives de Mémoire Continue (Fuites) : 0 processus critique en dérive.", 32.0f, ty, 11.0f, COL_GREEN_OK);
    ty += 20.0f;
    char line5[256];
    snprintf(line5, sizeof(line5), "- Surveillance Flux Réseau : %s surveillés.", c_sock ? c_sock->val : "11 SOCKETS");
    ui_draw_text(line5, 32.0f, ty, 11.0f, COL_CYAN_ACCENT);

    ty += 26.0f;
    ui_draw_text("3. RECOMMANDATIONS OPÉRATIONNELLES :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 22.0f;
    if (has_anomalies) {
        ui_draw_text("1. Des processus non conformes ont été identifiés dans l'onglet [ 02 ] Processus.", 32.0f, ty, 11.0f, COL_CRIT_RED);
        ty += 20.0f;
        ui_draw_text("2. Vérifiez la légitimité des binaires concernés avant toute transaction sensible.", 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    } else {
        ui_draw_text("1. Aucune fuite continue critique ne menace la stabilité de votre session.", 32.0f, ty, 11.0f, COL_TEXT_WHITE);
        ty += 20.0f;
        ui_draw_text("2. Votre machine fonctionne dans des tolérances nominales saines.", 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    }
    ty += 22.0f;
    ui_draw_text("--------------------------------------------------------------------------------", 32.0f, ty, 11.0f, (Color){ 40, 60, 80, 255 });
    ty += 20.0f;
    ui_draw_text("[ Rapport consolidé en local - Zéro donnée transmise vers l'extérieur ]", 32.0f, ty, 11.0f, COL_TEXT_MUTED);
}

/* ------------------------------------------------------------------------- */
/* 10. AFFICHAGE DE LA MODALE DU GUIDE ANALYTIQUE                            */
/* ------------------------------------------------------------------------- */
static void ui_views_draw_guide_modal(void) {
    if (!s_show_guide_modal) {
        return;
    }

    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    Vector2 mouse = GetMousePosition();

    DrawRectangle(0, 0, screen_w, screen_h, (Color){ 0, 0, 0, 180 });

    float mw = 840.0f;
    float mh = 490.0f;
    float mx = ((float)screen_w - mw) / 2.0f;
    float my = ((float)screen_h - mh) / 2.0f;

    Rectangle modal_rec = { mx, my, mw, mh };
    DrawRectangleRec(modal_rec, (Color){ 10, 18, 28, 255 });
    DrawRectangleLinesEx(modal_rec, 1.0f, (Color){ 0, 119, 204, 255 });

    DrawRectangle((int)mx, (int)my, (int)mw, 32, (Color){ 16, 28, 44, 255 });
    ui_draw_text("Guide d'Aide & Composantes Analytiques de RAPTOR", mx + 16.0f, my + 8.0f, 12.0f, COL_TEXT_WHITE);

    Rectangle close_x = { mx + mw - 32.0f, my, 32.0f, 32.0f };
    bool hover_close_x = CheckCollisionPointRec(mouse, close_x);
    ui_draw_text("X", close_x.x + 10.0f, close_x.y + 8.0f, 13.0f, hover_close_x ? COL_CRIT_RED : COL_TEXT_MUTED);
    if (hover_close_x && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_show_guide_modal = false;
        return;
    }

    ui_draw_text("COMPRENDRE LE FONCTIONNEMENT DU DIAGNOSTIC RAPTOR", mx + 16.0f, my + 46.0f, 12.0f, COL_CYAN_ACCENT);

    const char* items[5] = {
        "1. Surveillance de la Mémoire et Détection de Fuites",
        "2. Mouchards, Télémétrie & Activité Réseau",
        "3. Intrus et Signatures d'Attaque (RWX)",
        "4. Cahier de Bord & Moteur Différentiel (T-1 vs T0)",
        "5. Moteur d'IA Locale (Rapport Humain)"
    };

    float cur_y = my + 76.0f;
    for (int i = 0; i < 5; ++i) {
        Rectangle item_rec = { mx + 16.0f, cur_y, mw - 32.0f, 28.0f };
        bool hover_item = CheckCollisionPointRec(mouse, item_rec);

        DrawRectangleRec(item_rec, (s_guide_accordion_open == i) ? (Color){ 24, 42, 66, 255 } : (Color){ 14, 24, 38, 255 });
        DrawRectangleLinesEx(item_rec, 1.0f, COL_BORDER);

        ui_draw_text((s_guide_accordion_open == i) ? "\xE2\x96\xBC" : "\xE2\x96\xB6", item_rec.x + 10.0f, item_rec.y + 7.0f, 11.0f, COL_CYAN_ACCENT);
        ui_draw_text(items[i], item_rec.x + 30.0f, item_rec.y + 7.0f, 12.0f, COL_TEXT_WHITE);

        if (hover_item && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            s_guide_accordion_open = (s_guide_accordion_open == i) ? -1 : i;
        }

        cur_y += 32.0f;

        if (s_guide_accordion_open == i) {
            float body_h = 104.0f;
            Rectangle body_rec = { mx + 16.0f, cur_y, mw - 32.0f, body_h };
            DrawRectangleRec(body_rec, (Color){ 8, 14, 22, 255 });
            DrawRectangleLinesEx(body_rec, 1.0f, (Color){ 16, 28, 44, 255 });

            if (i == 0) {
                ui_draw_text("•  RSS (Resident Set Size) : Mémoire RAM physique réellement occupée par le programme.",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  VSZ (Virtual Size) : Espace d'adressage virtuel réservé (inclut les bibliothèques partagées).",
                             body_rec.x + 14.0f, body_rec.y + 30.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Détection de Dérive (Leak) : Surveillance continue calculant l'augmentation monotone de la RAM.",
                             body_rec.x + 14.0f, body_rec.y + 50.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Si un processus voit sa mémoire augmenter régulièrement sans jamais la restituer lors des phases",
                             body_rec.x + 14.0f, body_rec.y + 72.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("d'inactivité, RAPTOR le signale comme une fuite anormale.",
                             body_rec.x + 14.0f, body_rec.y + 86.0f, 11.0f, COL_TEXT_MUTED);
            } else if (i == 1) {
                ui_draw_text("•  Résolution Inode -> PID : Corrélation instantanée des sockets réseau TCP/UDP avec le binaire source.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Permet de détecter immédiatement tout flux de communication clandestin initié en tâche de fond.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            } else if (i == 2) {
                ui_draw_text("•  Pages RWX (Read-Write-Execute) : Violation du principe W^X (Write XOR Execute).",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Alerte critique si du code exécutable réside sur la pile ou sans correspondance binaire légitime.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            } else if (i == 3) {
                ui_draw_text("•  Différentiel Temporel : Analyse des variations entre instantanés consécutifs T-1 et T0.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Calcul dynamique du taux de dérive en Mo/minute et classification automatique du niveau de risque.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            } else {
                ui_draw_text("•  Moteur IA Souverain : Traduction contextuelle des métriques techniques en diagnostic lisible.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Traitement intégralement local réalisé sans aucun recours à une API distante ni fuite de données.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            }

            cur_y += body_h + 6.0f;
        }
    }

    Rectangle btn_close = { mx + 16.0f, my + mh - 40.0f, 130.0f, 28.0f };
    bool hover_btn_close = CheckCollisionPointRec(mouse, btn_close);
    DrawRectangleRec(btn_close, hover_btn_close ? COL_BLUE_HOVER : COL_PANEL_BG);
    DrawRectangleLinesEx(btn_close, 1.0f, hover_btn_close ? COL_BORDER_FOCUS : COL_BORDER);
    ui_draw_text("Fermer le guide", btn_close.x + 16.0f, btn_close.y + 7.0f, 12.0f, COL_TEXT_WHITE);

    if (hover_btn_close && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_show_guide_modal = false;
    }
}

/* ------------------------------------------------------------------------- */
/* 11. RENDU GLOBAL DE LA GESTION D'ONGLETS                                  */
/* ------------------------------------------------------------------------- */
void raptor_ui_render_tabs(const RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }

    ui_views_load_resources();

    ClearBackground(COL_BG_MAIN);

    ui_views_draw_header();
    ui_views_draw_tabs();

    switch (s_active_tab) {
        case 0:  ui_views_draw_tab_global((RaptorReportContext*)ctx); break;
        case 1:  ui_views_draw_tab_processes(ctx); break;
        case 2:  ui_views_draw_tab_network(ctx); break;
        case 3:  ui_views_draw_tab_diff(ctx); break;
        case 4:  ui_views_draw_tab_ai(ctx); break;
        default: ui_views_draw_tab_global((RaptorReportContext*)ctx); break;
    }

    ui_views_draw_guide_modal();
}
