/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_network.c
 * RÉVISION        : Rev 2.7
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue dédiée [ 03 ] Sockets & Réseau de l'interface graphique.
 *                   - Visualisation passive des flux TCP/UDP ouverts.
 *                   - Raccordement direct au sous-système Net-Guard (Défense active).
 *                   - Qualification en temps réel des flux distants (APPROUVÉ / HOSTILE).
 *                   - Abattage chirurgical des processus malveillants via SIGKILL.
 *                   - Boîte modale ergonomique de création de règles (fw_custom.conf).
 *                   - Boîte de dialogue modale de confirmation pour la synchronisation.
 *                   - Appel du composant modulaire ui_tooltip pour les résolutions DNS.
 *
 * NOUVELLES INTÉGRATIONS (Rev 2.7) :
 *   - Réalignement épuré de l'en-tête de colonne : "Adresse Distante".
 *   - Suppression de tout code redondant d'infobulle : délégation à ui_tooltip.c.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_view_network_draw_btn
 *   2. ui_view_network_handle_typing
 *   3. ui_view_network_draw_modal
 *   4. ui_view_network_draw_dialog
 *   5. ui_view_network_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include "raptor/raptor_net_guard.h"
#include <signal.h>
#include <ctype.h>

static bool s_modal_open = false;
static int  s_rule_mode = 0;
static int  s_active_field = 0;

static char s_dom_brand[NET_GUARD_STR_SM] = {0};
static char s_dom_urls[NET_GUARD_STR_LG]  = {0};
static char s_dom_desc[NET_GUARD_STR_MD]  = {0};

static char s_ip_target[NET_GUARD_STR_SM] = {0};
static int  s_ip_status = 0;
static char s_ip_desc[NET_GUARD_STR_MD]   = {0};

/* Dialogue modal de notification */
static bool  s_dialog_open = false;
static char  s_dialog_title[64] = {0};
static char  s_dialog_msg[256] = {0};
static Color s_dialog_color = { 0, 230, 118, 255 };

/* ------------------------------------------------------------------------- */
/* 1. ASSISTANCE AU RENDU DES BOUTONS INTERACTIFS                            */
/* ------------------------------------------------------------------------- */
static bool ui_view_network_draw_btn(Rectangle rec, const char* label, Color base_col, Color hover_col, Color text_col) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rec);

    DrawRectangleRec(rec, hovered ? hover_col : base_col);
    DrawRectangleLinesEx(rec, 1.0f, hovered ? COL_CYAN_ACCENT : COL_BORDER);

    float font_size = 11.0f;
    float text_w = ui_measure_text(label, font_size);
    float text_x = rec.x + (rec.width - text_w) * 0.5f;
    float text_y = rec.y + (rec.height - (font_size * UI_FONT_SCALE)) * 0.5f;

    ui_draw_text(label, text_x, text_y, font_size, text_col);

    return (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
}

/* ------------------------------------------------------------------------- */
/* 2. CAPTURE ET GESTION DE LA SAISIE CLAVIER DANS UN CHAMP TEXTE            */
/* ------------------------------------------------------------------------- */
static void ui_view_network_handle_typing(char* buffer, size_t max_len) {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126 && key != '|') {
            size_t len = strlen(buffer);
            if (len < max_len - 1) {
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        size_t len = strlen(buffer);
        if (len > 0) {
            buffer[len - 1] = '\0';
        }
    }
}

/* ------------------------------------------------------------------------- */
/* 3. RENDU DE LA MODALE D'AJOUT DE RÈGLE DANS FW_CUSTOM.CONF                */
/* ------------------------------------------------------------------------- */
static void ui_view_network_draw_modal(int screen_w, int screen_h) {
    DrawRectangle(0, 0, screen_w, screen_h, (Color){ 4, 8, 14, 215 });

    float modal_w = 640.0f;
    float modal_h = 440.0f;
    float modal_x = ((float)screen_w - modal_w) * 0.5f;
    float modal_y = ((float)screen_h - modal_h) * 0.5f;

    DrawRectangle((int)modal_x, (int)modal_y, (int)modal_w, (int)modal_h, COL_PANEL_BG);
    DrawRectangleLinesEx((Rectangle){ modal_x, modal_y, modal_w, modal_h }, 1.5f, COL_BORDER_FOCUS);

    ui_draw_text("AJOUTER UNE RÈGLE DE PROTECTION SOUVERAINE", modal_x + 24.0f, modal_y + 20.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Les règles enregistrées ici sont persistées dans fw_custom.conf.", modal_x + 24.0f, modal_y + 40.0f, 10.0f, COL_TEXT_MUTED);

    Rectangle close_rec = { modal_x + modal_w - 36.0f, modal_y + 16.0f, 22.0f, 22.0f };
    if (ui_view_network_draw_btn(close_rec, "X", COL_CARD_BG, COL_CRIT_RED, COL_TEXT_WHITE)) {
        s_modal_open = false;
        return;
    }

    Rectangle tab_dom_rec = { modal_x + 24.0f, modal_y + 72.0f, 280.0f, 28.0f };
    Rectangle tab_ip_rec  = { modal_x + 316.0f, modal_y + 72.0f, 280.0f, 28.0f };

    if (ui_view_network_draw_btn(tab_dom_rec, "1. MARQUE / DOMAINE (DOM)", 
        (s_rule_mode == 0) ? COL_BLUE_BTN : COL_CARD_BG, COL_BLUE_HOVER, COL_TEXT_WHITE)) {
        s_rule_mode = 0;
        s_active_field = 0;
    }
    if (ui_view_network_draw_btn(tab_ip_rec, "2. ADRESSE OU PLAGE (IP)", 
        (s_rule_mode == 1) ? COL_BLUE_BTN : COL_CARD_BG, COL_BLUE_HOVER, COL_TEXT_WHITE)) {
        s_rule_mode = 1;
        s_active_field = 0;
    }

    Vector2 mouse = GetMousePosition();
    bool is_click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    /* MODE DOM (COMBOSQUATTING) */
    if (s_rule_mode == 0) {
        ui_draw_text("Nom de l'entité ou marque à protéger :", modal_x + 24.0f, modal_y + 116.0f, 11.0f, COL_TEXT_WHITE);
        Rectangle f1_rec = { modal_x + 24.0f, modal_y + 134.0f, 572.0f, 26.0f };
        if (is_click && CheckCollisionPointRec(mouse, f1_rec)) s_active_field = 0;
        if (s_active_field == 0) ui_view_network_handle_typing(s_dom_brand, sizeof(s_dom_brand));

        DrawRectangleRec(f1_rec, COL_CARD_BG);
        DrawRectangleLinesEx(f1_rec, 1.0f, (s_active_field == 0) ? COL_BORDER_FOCUS : COL_BORDER);
        char disp_brand[80];
        snprintf(disp_brand, sizeof(disp_brand), "%s%s", s_dom_brand, (s_active_field == 0 && ((int)(GetTime() * 2.0f) % 2 == 0)) ? "_" : "");
        ui_draw_text((strlen(s_dom_brand) > 0 || s_active_field == 0) ? disp_brand : "ex: desjardins, hydroquebec, ameli", 
                     f1_rec.x + 8.0f, f1_rec.y + 6.0f, 11.0f, (strlen(s_dom_brand) > 0) ? COL_TEXT_WHITE : COL_TEXT_MUTED);

        ui_draw_text("Domaines officiels légitimes (séparés par virgules) :", modal_x + 24.0f, modal_y + 174.0f, 11.0f, COL_TEXT_WHITE);
        Rectangle f2_rec = { modal_x + 24.0f, modal_y + 192.0f, 572.0f, 26.0f };
        if (is_click && CheckCollisionPointRec(mouse, f2_rec)) s_active_field = 1;
        if (s_active_field == 1) ui_view_network_handle_typing(s_dom_urls, sizeof(s_dom_urls));

        DrawRectangleRec(f2_rec, COL_CARD_BG);
        DrawRectangleLinesEx(f2_rec, 1.0f, (s_active_field == 1) ? COL_BORDER_FOCUS : COL_BORDER);
        char disp_urls[NET_GUARD_STR_LG + 16];
        snprintf(disp_urls, sizeof(disp_urls), "%s%s", s_dom_urls, (s_active_field == 1 && ((int)(GetTime() * 2.0f) % 2 == 0)) ? "_" : "");
        ui_draw_text((strlen(s_dom_urls) > 0 || s_active_field == 1) ? disp_urls : "ex: desjardins.com, desjardins.ca", 
                     f2_rec.x + 8.0f, f2_rec.y + 6.0f, 11.0f, (strlen(s_dom_urls) > 0) ? COL_TEXT_WHITE : COL_TEXT_MUTED);

        ui_draw_text("Description ou note contextuelle :", modal_x + 24.0f, modal_y + 232.0f, 11.0f, COL_TEXT_WHITE);
        Rectangle f3_rec = { modal_x + 24.0f, modal_y + 250.0f, 572.0f, 26.0f };
        if (is_click && CheckCollisionPointRec(mouse, f3_rec)) s_active_field = 2;
        if (s_active_field == 2) ui_view_network_handle_typing(s_dom_desc, sizeof(s_dom_desc));

        DrawRectangleRec(f3_rec, COL_CARD_BG);
        DrawRectangleLinesEx(f3_rec, 1.0f, (s_active_field == 2) ? COL_BORDER_FOCUS : COL_BORDER);
        char disp_desc[140];
        snprintf(disp_desc, sizeof(disp_desc), "%s%s", s_dom_desc, (s_active_field == 2 && ((int)(GetTime() * 2.0f) % 2 == 0)) ? "_" : "");
        ui_draw_text((strlen(s_dom_desc) > 0 || s_active_field == 2) ? disp_desc : "ex: Portail financier officiel", 
                     f3_rec.x + 8.0f, f3_rec.y + 6.0f, 11.0f, (strlen(s_dom_desc) > 0) ? COL_TEXT_WHITE : COL_TEXT_MUTED);

        ui_draw_text("Note : Tout site contrefait utilisant ce radical déclenchera un drapeau CRITIQUE.", 
                     modal_x + 24.0f, modal_y + 296.0f, 10.0f, (Color){ 255, 187, 0, 255 });
    }
    /* MODE IP (FILTRAGE DE FLUX) */
    else {
        ui_draw_text("Adresse IP, Port ou Sous-réseau CIDR :", modal_x + 24.0f, modal_y + 116.0f, 11.0f, COL_TEXT_WHITE);
        Rectangle f1_rec = { modal_x + 24.0f, modal_y + 134.0f, 572.0f, 26.0f };
        if (is_click && CheckCollisionPointRec(mouse, f1_rec)) s_active_field = 0;
        if (s_active_field == 0) ui_view_network_handle_typing(s_ip_target, sizeof(s_ip_target));

        uint32_t val_ip = 0, val_mask = 0;
        uint16_t val_port = 0;
        bool is_valid_ip = net_guard_validate_ip(s_ip_target, &val_ip, &val_mask, &val_port);

        Color border_col = COL_BORDER;
        if (strlen(s_ip_target) > 0) {
            border_col = is_valid_ip ? COL_GREEN_OK : COL_CRIT_RED;
        } else if (s_active_field == 0) {
            border_col = COL_BORDER_FOCUS;
        }

        DrawRectangleRec(f1_rec, COL_CARD_BG);
        DrawRectangleLinesEx(f1_rec, 1.0f, border_col);
        char disp_ip[80];
        snprintf(disp_ip, sizeof(disp_ip), "%s%s", s_ip_target, (s_active_field == 0 && ((int)(GetTime() * 2.0f) % 2 == 0)) ? "_" : "");
        ui_draw_text((strlen(s_ip_target) > 0 || s_active_field == 0) ? disp_ip : "ex: 45.33.32.156 ou 192.168.1.0/24", 
                     f1_rec.x + 8.0f, f1_rec.y + 6.0f, 11.0f, (strlen(s_ip_target) > 0) ? COL_TEXT_WHITE : COL_TEXT_MUTED);

        if (strlen(s_ip_target) > 0) {
            if (is_valid_ip) {
                ui_draw_text("Syntaxe IPv4 / CIDR conforme.", modal_x + 24.0f, modal_y + 164.0f, 10.0f, COL_GREEN_OK);
            } else {
                ui_draw_text("Erreur syntaxique (attendu: X.X.X.X ou X.X.X.X/XX ou X.X.X.X:PORT)", modal_x + 24.0f, modal_y + 164.0f, 10.0f, COL_CRIT_RED);
            }
        }

        ui_draw_text("Qualification EDR de la cible :", modal_x + 24.0f, modal_y + 188.0f, 11.0f, COL_TEXT_WHITE);
        Rectangle btn_hostile = { modal_x + 24.0f, modal_y + 208.0f, 276.0f, 26.0f };
        Rectangle btn_approv  = { modal_x + 320.0f, modal_y + 208.0f, 276.0f, 26.0f };

        if (ui_view_network_draw_btn(btn_hostile, "HOSTILE (Alerte & Neutralisation)", 
            (s_ip_status == 0) ? COL_CRIT_RED : COL_CARD_BG, (Color){ 255, 70, 100, 255 }, COL_TEXT_WHITE)) {
            s_ip_status = 0;
        }
        if (ui_view_network_draw_btn(btn_approv, "APPROUVÉ (Liste Blanche)", 
            (s_ip_status == 1) ? (Color){ 0, 160, 80, 255 } : COL_CARD_BG, COL_GREEN_OK, COL_TEXT_WHITE)) {
            s_ip_status = 1;
        }

        ui_draw_text("Description ou note explicative :", modal_x + 24.0f, modal_y + 246.0f, 11.0f, COL_TEXT_WHITE);
        Rectangle f3_rec = { modal_x + 24.0f, modal_y + 266.0f, 572.0f, 26.0f };
        if (is_click && CheckCollisionPointRec(mouse, f3_rec)) s_active_field = 2;
        if (s_active_field == 2) ui_view_network_handle_typing(s_ip_desc, sizeof(s_ip_desc));

        DrawRectangleRec(f3_rec, COL_CARD_BG);
        DrawRectangleLinesEx(f3_rec, 1.0f, (s_active_field == 2) ? COL_BORDER_FOCUS : COL_BORDER);
        char disp_desc[140];
        snprintf(disp_desc, sizeof(disp_desc), "%s%s", s_ip_desc, (s_active_field == 2 && ((int)(GetTime() * 2.0f) % 2 == 0)) ? "_" : "");
        ui_draw_text((strlen(s_ip_desc) > 0 || s_active_field == 2) ? disp_desc : "ex: Relais C2 non autorisé ou Serveur NAS local", 
                     f3_rec.x + 8.0f, f3_rec.y + 6.0f, 11.0f, (strlen(s_ip_desc) > 0) ? COL_TEXT_WHITE : COL_TEXT_MUTED);
    }

    Rectangle btn_save   = { modal_x + 376.0f, modal_y + modal_h - 48.0f, 220.0f, 30.0f };
    Rectangle btn_cancel = { modal_x + 24.0f, modal_y + modal_h - 48.0f, 140.0f, 30.0f };

    if (ui_view_network_draw_btn(btn_cancel, "ANNULER", COL_CARD_BG, (Color){ 40, 20, 25, 255 }, COL_TEXT_WHITE) ||
        IsKeyPressed(KEY_ESCAPE)) {
        s_modal_open = false;
        return;
    }

    if (ui_view_network_draw_btn(btn_save, "ENREGISTRER LA RÈGLE", COL_BLUE_BTN, COL_BLUE_HOVER, COL_TEXT_WHITE) ||
        IsKeyPressed(KEY_ENTER)) {
        if (s_rule_mode == 0) {
            if (strlen(s_dom_brand) > 0 && strlen(s_dom_urls) > 0) {
                net_guard_add_custom_rule(NET_GUARD_TYPE_DOM, s_dom_brand, s_dom_urls, s_dom_desc);
                s_modal_open = false;
                
                snprintf(s_dialog_title, sizeof(s_dialog_title), "RÈGLE ENREGISTRÉE");
                snprintf(s_dialog_msg, sizeof(s_dialog_msg), "Règle '%s' ajoutée dans fw_custom.conf.", s_dom_brand);
                s_dialog_color = COL_GREEN_OK;
                s_dialog_open = true;

                s_dom_brand[0] = '\0';
                s_dom_urls[0] = '\0';
                s_dom_desc[0] = '\0';
            }
        } else {
            uint32_t val_ip = 0, val_mask = 0;
            uint16_t val_port = 0;
            if (net_guard_validate_ip(s_ip_target, &val_ip, &val_mask, &val_port)) {
                const char* st_text = (s_ip_status == 0) ? "HOSTILE" : "APPROUVÉ";
                net_guard_add_custom_rule(NET_GUARD_TYPE_IP, s_ip_target, st_text, s_ip_desc);
                s_modal_open = false;

                snprintf(s_dialog_title, sizeof(s_dialog_title), "RÈGLE IP ENREGISTRÉE");
                snprintf(s_dialog_msg, sizeof(s_dialog_msg), "Cible '%s' [%s] ajoutée dans fw_custom.conf.", s_ip_target, st_text);
                s_dialog_color = COL_GREEN_OK;
                s_dialog_open = true;

                s_ip_target[0] = '\0';
                s_ip_desc[0] = '\0';
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
/* 4. RENDU DU DIALOGUE MODAL DE STATUT                                      */
/* ------------------------------------------------------------------------- */
static void ui_view_network_draw_dialog(int screen_w, int screen_h) {
    DrawRectangle(0, 0, screen_w, screen_h, (Color){ 3, 6, 10, 220 });

    float dlg_w = 480.0f;
    float dlg_h = 180.0f;
    float dlg_x = ((float)screen_w - dlg_w) * 0.5f;
    float dlg_y = ((float)screen_h - dlg_h) * 0.5f;

    DrawRectangle((int)dlg_x, (int)dlg_y, (int)dlg_w, (int)dlg_h, COL_PANEL_BG);
    DrawRectangleLinesEx((Rectangle){ dlg_x, dlg_y, dlg_w, dlg_h }, 1.5f, s_dialog_color);

    DrawRectangle((int)dlg_x, (int)dlg_y, (int)dlg_w, 4, s_dialog_color);
    ui_draw_text(s_dialog_title, dlg_x + 24.0f, dlg_y + 22.0f, 13.0f, s_dialog_color);

    ui_draw_text(s_dialog_msg, dlg_x + 24.0f, dlg_y + 60.0f, 12.0f, COL_TEXT_WHITE);

    Rectangle btn_ack = { dlg_x + (dlg_w - 180.0f) * 0.5f, dlg_y + dlg_h - 44.0f, 180.0f, 28.0f };
    if (ui_view_network_draw_btn(btn_ack, "COMPRIS", COL_CARD_BG, COL_BLUE_BTN, COL_TEXT_WHITE) ||
        IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_SPACE)) {
        s_dialog_open = false;
    }
}

/* ------------------------------------------------------------------------- */
/* 5. RENDU PRINCIPAL DE LA VUE RÉSEAU & RWX [ 03 ]                          */
/* ------------------------------------------------------------------------- */
void ui_view_network_draw(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    float content_y = 102.0f;

    if (g_icon_network.id > 0) {
        DrawTexturePro(g_icon_network,
                       (Rectangle){0, 0, (float)g_icon_network.width, (float)g_icon_network.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("SÉCURITÉ RÉSEAU & CONTRÔLE SOUVERAIN (NET-GUARD / RWX)", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Analyse en direct des sockets ouverts, détection anti-phishing et traque d'injections mémoire.", 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

    Rectangle btn_update_rec = { (float)screen_w - 410.0f, content_y + 8.0f, 198.0f, 28.0f };
    Rectangle btn_add_rec    = { (float)screen_w - 200.0f, content_y + 8.0f, 184.0f, 28.0f };

    if (!s_modal_open && !s_dialog_open) {
        if (ui_view_network_draw_btn(btn_update_rec, "SYNCHRONISER REGISTRE", COL_CARD_BG, COL_BLUE_BTN, COL_TEXT_WHITE)) {
            int added_lines = 0;
            NetGuardUpdateStatus status = net_guard_update_official_rules(&added_lines);

            if (status == NET_UPDATE_NO_CHANGE) {
                snprintf(s_dialog_title, sizeof(s_dialog_title), "SYNCHRONISATION REGISTRE");
                snprintf(s_dialog_msg, sizeof(s_dialog_msg), "FW_OFFICIEL.CONF A JOUR: RIEN A FAIRE");
                s_dialog_color = (Color){ 0, 180, 230, 255 };
            } else if (status == NET_UPDATE_SUCCESS) {
                snprintf(s_dialog_title, sizeof(s_dialog_title), "SYNCHRONISATION REGISTRE");
                snprintf(s_dialog_msg, sizeof(s_dialog_msg), "%d LIGNE A ETE AJOUTÉ..", added_lines);
                s_dialog_color = COL_GREEN_OK;
            } else {
                snprintf(s_dialog_title, sizeof(s_dialog_title), "ERREUR SYNCHRONISATION");
                snprintf(s_dialog_msg, sizeof(s_dialog_msg), "Échec de récupération du fichier officiel distant.");
                s_dialog_color = COL_CRIT_RED;
            }
            s_dialog_open = true;
        }

        if (ui_view_network_draw_btn(btn_add_rec, "+ NOUVELLE RÈGLE", COL_BLUE_BTN, COL_BLUE_HOVER, COL_TEXT_WHITE)) {
            s_modal_open = true;
            s_active_field = 0;
        }
    }

    const RaptorTable* net_tbl = ui_find_table(ctx, "sec_network", "Flux");
    size_t net_count = net_tbl ? net_tbl->row_count : 0;

    char net_header[128];
    snprintf(net_header, sizeof(net_header), "Connexions actives et évaluation Net-Guard (%zu flux surveillés) :", net_count);

    float net_y = content_y + 72.0f;
    ui_draw_text(net_header, 16.0f, net_y, 12.0f, (Color){ 255, 204, 0, 255 });

    float tbl_y = net_y + 22.0f;
    float tbl_w = (float)screen_w - 32.0f;
    float row_h = 24.0f;

    /* En-tête de tableau propre : aucun débordement de texte */
    DrawRectangle(16, (int)tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);

    ui_draw_text("PID / Processus", 24.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Proto", 240.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Adresse Locale", 310.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Adresse Distante", 485.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("État", 675.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Statut Net-Guard", 785.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Action Défensive", 940.0f, tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);

    Vector2 mouse = GetMousePosition();
    size_t draw_net_limit = (net_tbl && net_tbl->row_count < 8) ? net_tbl->row_count : 8;

    for (size_t r = 0; r < 8; ++r) {
        float ry = tbl_y + row_h + ((float)r * row_h);

        if (r < draw_net_limit && net_tbl && net_tbl->rows[r].cell_texts) {
            const char* pid_raw  = net_tbl->rows[r].cell_texts[0];
            const char* proto    = net_tbl->rows[r].cell_texts[1];
            const char* loc_addr = net_tbl->rows[r].cell_texts[2];
            const char* rem_addr = net_tbl->rows[r].cell_texts[3];
            const char* st       = net_tbl->rows[r].cell_texts[4];

            char pid_disp[32];
            strncpy(pid_disp, pid_raw, sizeof(pid_disp) - 1);
            pid_disp[sizeof(pid_disp) - 1] = '\0';
            if (strlen(pid_disp) > 25) {
                pid_disp[22] = '.';
                pid_disp[23] = '.';
                pid_disp[24] = '.';
                pid_disp[25] = '\0';
            }

            char ip_copy[64] = {0};
            uint16_t port_val = 0;
            strncpy(ip_copy, rem_addr, sizeof(ip_copy) - 1);
            char* colon = strchr(ip_copy, ':');
            if (colon) {
                *colon = '\0';
                port_val = (uint16_t)atoi(colon + 1);
            }

            char reason[NET_GUARD_STR_LG] = {0};
            NetGuardVerdict verdict = net_guard_match_ip(ip_copy, port_val, reason, sizeof(reason));

            Color row_bg = (r % 2 == 0) ? (Color){ 9, 15, 24, 255 } : (Color){ 12, 20, 32, 255 };
            if (verdict == NET_MATCH_CRITICAL) {
                row_bg = (Color){ 55, 12, 20, 255 };
            }
            DrawRectangle(16, (int)ry, (int)tbl_w, (int)row_h, row_bg);
            DrawRectangleLines(16, (int)ry, (int)tbl_w, (int)row_h, 
                               (verdict == NET_MATCH_CRITICAL) ? COL_CRIT_RED : (Color){ 20, 32, 48, 255 });

            ui_draw_text(pid_disp, 24.0f, ry + 5.0f, 11.0f, (verdict == NET_MATCH_CRITICAL) ? COL_CRIT_RED : COL_TEXT_MUTED);
            ui_draw_text(proto, 240.0f, ry + 5.0f, 11.0f, COL_TEXT_WHITE);
            ui_draw_text(loc_addr, 310.0f, ry + 5.0f, 11.0f, COL_TEXT_WHITE);

            /* Survol sur l'adresse distante : déclenchement du composant universel */
            Rectangle rec_ip = { 480.0f, ry, 185.0f, row_h };
            bool ip_hovered = CheckCollisionPointRec(mouse, rec_ip);

            if (ip_hovered && !s_modal_open && !s_dialog_open) {
                DrawRectangleRec(rec_ip, (Color){ 0, 119, 204, 60 });
                DrawRectangleLinesEx(rec_ip, 1.0f, COL_CYAN_ACCENT);

                char ep_buf[128];
                char host_buf[NET_GUARD_STR_LG];
                char full_host[NET_GUARD_STR_LG + 32];
                char line_edr[NET_GUARD_STR_LG + 32];

                snprintf(ep_buf, sizeof(ep_buf), "CONNEXION : %s (%s)", rem_addr, proto);
                net_guard_resolve_ip_async(ip_copy, host_buf, sizeof(host_buf));
                snprintf(full_host, sizeof(full_host), "HÔTE DNS  : %s", host_buf);
                snprintf(line_edr, sizeof(line_edr), "SÉCURITÉ  : %s", (strlen(reason) > 0) ? reason : "Aucune anomalie détectée");

                ui_tooltip_trigger(mouse, ep_buf, full_host, line_edr, (Color){ 255, 204, 0, 255 });
            }

            ui_draw_text(rem_addr, 485.0f, ry + 5.0f, 11.0f, 
                         ip_hovered ? COL_CYAN_ACCENT : ((verdict == NET_MATCH_CRITICAL) ? COL_CRIT_RED : COL_TEXT_WHITE));
            ui_draw_text(st, 675.0f, ry + 5.0f, 11.0f, strcmp(st, "ESTABLISHED") == 0 ? COL_GREEN_OK : COL_TEXT_WHITE);

            if (verdict == NET_MATCH_CRITICAL) {
                ui_draw_text("HOSTILE (ALERTE)", 785.0f, ry + 5.0f, 11.0f, COL_CRIT_RED);

                Rectangle btn_kill = { 930.0f, ry + 2.0f, 115.0f, row_h - 4.0f };
                if (!s_modal_open && !s_dialog_open && ui_view_network_draw_btn(btn_kill, "NEUTRALISER", COL_CRIT_RED, (Color){ 255, 60, 90, 255 }, WHITE)) {
                    int target_pid = 0;
                    if (sscanf(pid_raw, "PID %d", &target_pid) == 1 && target_pid > 1) {
                        kill(target_pid, SIGKILL);
                        snprintf(s_dialog_title, sizeof(s_dialog_title), "PROCESSUS NEUTRALISÉ");
                        snprintf(s_dialog_msg, sizeof(s_dialog_msg), "Le processus PID %d a été abattu (SIGKILL).", target_pid);
                        s_dialog_color = COL_CRIT_RED;
                        s_dialog_open = true;
                    }
                }
            } else if (verdict == NET_MATCH_APPROVED) {
                ui_draw_text("APPROUVÉ", 785.0f, ry + 5.0f, 11.0f, COL_GREEN_OK);
                ui_draw_text("-", 985.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            } else {
                ui_draw_text("SURVEILLÉ", 785.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("-", 985.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            }
        } else {
            DrawRectangle(16, (int)ry, (int)tbl_w, (int)row_h, (r % 2 == 0) ? (Color){ 9, 15, 24, 255 } : (Color){ 12, 20, 32, 255 });
            DrawRectangleLines(16, (int)ry, (int)tbl_w, (int)row_h, (Color){ 20, 32, 48, 255 });
            ui_draw_text("-", 24.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 240.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 310.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 485.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 675.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 785.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
            ui_draw_text("-", 985.0f, ry + 5.0f, 11.0f, COL_TEXT_MUTED);
        }
    }

    float rwx_y = tbl_y + (9.0f * row_h) + 20.0f;
    ui_draw_text("Processus avec mémoire exécutable non officielle (RWX) :", 16.0f, rwx_y, 12.0f, (Color){ 255, 187, 0, 255 });

    float rwx_tbl_y = rwx_y + 22.0f;
    DrawRectangle(16, (int)rwx_tbl_y, (int)tbl_w, (int)row_h, (Color){ 16, 26, 42, 255 });
    DrawRectangleLines(16, (int)rwx_tbl_y, (int)tbl_w, (int)row_h, COL_BORDER);
    ui_draw_text("PID", 24.0f, rwx_tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Nom du Processus", 90.0f, rwx_tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Statut d'Injection", 320.0f, rwx_tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("Chemin de l'exécutable", 520.0f, rwx_tbl_y + 5.0f, 11.0f, COL_TEXT_MUTED);

    const RaptorTable* real_anomalies_tbl = ui_find_table(ctx, "sec_proc_mem", "ANOMALIES");
    float rwx_ry = rwx_tbl_y + row_h;
    DrawRectangle(16, (int)rwx_ry, (int)tbl_w, (int)(row_h * 3.5f), (Color){ 9, 15, 24, 255 });
    DrawRectangleLines(16, (int)rwx_ry, (int)tbl_w, (int)(row_h * 3.5f), (Color){ 20, 32, 48, 255 });

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

    /* Rendu des calques prioritaires : modale, dialogue ou infobulle */
    if (s_modal_open) {
        ui_view_network_draw_modal(screen_w, screen_h);
    } else if (s_dialog_open) {
        ui_view_network_draw_dialog(screen_w, screen_h);
    } else {
        ui_tooltip_draw();
    }
}
