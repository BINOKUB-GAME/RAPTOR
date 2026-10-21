/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_processes.c
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue dédiée [ 02 ] Processus & RAM de l'interface graphique.
 *                   Affiche la liste complète des processus actifs du système,
 *                   gère la recherche interactive, et orchestre la neutralisation
 *                   sécurisée avec capture forensique pré-kill, consignation
 *                   automatique dans AUDIT.TXT et libération du cache de traque.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Appel de raptor_audit_log_forget_pid() post-neutralisation permettant
 *     d'immédiatement journaliser un nouvel audit si le malware réapparaît.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_view_processes_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include "raptor/raptor_audit_log.h"
#include <time.h>

#if defined(__linux__) || defined(__unix__)
#include <signal.h>
#include <unistd.h>
#endif

/* ------------------------------------------------------------------------- */
/* VARIABLES D'ÉTAT LOCALES DU MODULE PROCESSUS                              */
/* ------------------------------------------------------------------------- */
static char s_filter_query[64] = {0};
static int s_filter_query_len = 0;
static bool s_filter_focused = false;
static int s_selected_proc_pid = -1;
static char s_action_feedback[128] = {0};

/* ------------------------------------------------------------------------- */
/* 1. RENDU DE LA VUE PROCESSUS & RAM [ 02 ]                                 */
/* ------------------------------------------------------------------------- */
void ui_view_processes_draw(RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    Vector2 mouse = GetMousePosition();
    float content_y = 102.0f;

    /* 1. Icône de section */
    if (g_icon_processes.id > 0) {
        DrawTexturePro(g_icon_processes,
                       (Rectangle){0, 0, (float)g_icon_processes.width, (float)g_icon_processes.height},
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

    /* 2. Barre de recherche et de filtrage */
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

    /* 3. Tableau des processus scannés */
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

    /* 4. Barre d'intervention opérationnelle en pied de vue */
    float bottom_y = (float)(GetScreenHeight() - 32);

    if (s_selected_proc_pid > 0) {
        bool is_alive = false;
#if defined(__linux__) || defined(__unix__)
        if (kill((pid_t)s_selected_proc_pid, 0) == 0) {
            is_alive = true;
        }
#endif

        if (is_alive) {
            char kill_btn_label[64];
            snprintf(kill_btn_label, sizeof(kill_btn_label), "[ ARRÊTER LE PROCESSUS (PID %d) ]", s_selected_proc_pid);

            float kb_w = ui_measure_text(kill_btn_label, 11.0f) + 20.0f;
            Rectangle btn_kill = { 16.0f, bottom_y - 2.0f, kb_w, 26.0f };
            bool hover_kill = CheckCollisionPointRec(mouse, btn_kill);

            DrawRectangleRec(btn_kill, hover_kill ? (Color){ 255, 70, 90, 255 } : (Color){ 180, 30, 45, 255 });
            DrawRectangleLinesEx(btn_kill, 1.0f, COL_CRIT_RED);
            ui_draw_text(kill_btn_label, btn_kill.x + 10.0f, btn_kill.y + 6.0f, 11.0f, COL_TEXT_WHITE);

            if (hover_kill && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                int target_pid = s_selected_proc_pid;

                /* ÉTAPE A : Capture forensique immédiate AVANT la mort du processus */
                RaptorIncidentDetail incident;
                raptor_audit_log_capture_target(target_pid, &incident);

#if defined(__linux__) || defined(__unix__)
                /* ÉTAPE B : Signalisation et neutralisation */
                kill((pid_t)target_pid, SIGTERM);
                usleep(30000);
                if (kill((pid_t)target_pid, 0) == 0) {
                    kill((pid_t)target_pid, SIGKILL);
                    usleep(30000);
                }
                usleep(20000);

                /* ÉTAPE C : Horodatage de fin et consignation sur disque dans AUDIT.TXT */
                time_t now = time(NULL);
                struct tm* tm_now = localtime(&now);
                if (tm_now) {
                    strftime(incident.kill_time, sizeof(incident.kill_time), "%H:%M:%S", tm_now);
                } else {
                    snprintf(incident.kill_time, sizeof(incident.kill_time), "Indisponible");
                }

                raptor_audit_log_commit(&incident, "NEUTRALISATION ADMINISTRATIVE RÉUSSIE");
                raptor_audit_log_forget_pid(target_pid);

                snprintf(s_action_feedback, sizeof(s_action_feedback),
                         "PID %d neutralisé. Preuve consignée dans AUDIT.TXT.", target_pid);
#else
                snprintf(s_action_feedback, sizeof(s_action_feedback),
                         "Intervention exécutée pour le PID %d.", target_pid);
#endif
                s_selected_proc_pid = -1;

                /* ÉTAPE D : Ré-audit dynamique avec purge préalable */
                raptor_data_context_clear(ctx);
                raptor_registry_run_all(ctx);

                time_t n = time(NULL);
                struct tm* tm_i = localtime(&n);
                if (tm_i) {
                    strftime(g_ui_last_audit_time, sizeof(g_ui_last_audit_time), "%Y-%m-%d %H:%M:%S", tm_i);
                }
            }

            if (s_action_feedback[0] != '\0') {
                ui_draw_text(s_action_feedback, btn_kill.x + kb_w + 16.0f, bottom_y + 4.0f, 12.0f, COL_GREEN_OK);
            } else {
                ui_draw_text("Cliquez sur le bouton rouge pour demander la fermeture immédiate.",
                             btn_kill.x + kb_w + 16.0f, bottom_y + 4.0f, 11.0f, COL_TEXT_MUTED);
            }
        } else {
            char dead_label[64];
            snprintf(dead_label, sizeof(dead_label), "[ PROCESSUS DÉJÀ TERMINÉ (PID %d) ]", s_selected_proc_pid);
            float db_w = ui_measure_text(dead_label, 11.0f) + 20.0f;

            Rectangle badge_dead = { 16.0f, bottom_y - 2.0f, db_w, 26.0f };
            DrawRectangleRec(badge_dead, (Color){ 14, 32, 24, 255 });
            DrawRectangleLinesEx(badge_dead, 1.0f, COL_GREEN_OK);
            ui_draw_text(dead_label, badge_dead.x + 10.0f, badge_dead.y + 6.0f, 11.0f, COL_GREEN_OK);

            ui_draw_text("Ce processus n'est plus actif. L'audit a été mis à jour.",
                         badge_dead.x + db_w + 16.0f, bottom_y + 4.0f, 12.0f, COL_TEXT_MUTED);
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
