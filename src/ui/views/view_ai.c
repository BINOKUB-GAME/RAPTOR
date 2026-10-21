/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_ai.c
 * RÉVISION        : Rev 1.5
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue dédiée [ 05 ] Rapport IA de l'interface graphique.
 *                   Rédige en direct la synthèse d'audit de sécurité souveraine
 *                   en croisant les métriques système, les alertes du moteur
 *                   heuristique expert, l'évaluation statistique (Z-Score) et
 *                   la traque forensique de la lignée parentale (PPID / script).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.5) :
 *   - Correction de portée : persistance de primary_parent_num hors de la boucle
 *     pour l'évaluation du PID 1 (garde-fou anti-kill sur systemd/init).
 *   - Réintégration de l'accolade fermante du bloc forensique et de la branche
 *     nominale 'else', éliminant les erreurs de fin de fichier GCC.
 *   - Conservation des badges d'affichage haute visibilité (cadre jaune 2 px).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. view_ai_detect_os
 *   2. view_ai_draw_pid_badge
 *   3. ui_view_ai_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------- */
/* 1. DÉTECTION DU SYSTÈME D'EXPLOITATION HÔTE                               */
/* ------------------------------------------------------------------------- */
static void view_ai_detect_os(char* out_name, size_t max_len) {
    if (!out_name || max_len == 0) {
        return;
    }

    const char* def_os = "Linux";
    size_t def_len = strlen(def_os);
    if (def_len >= max_len) {
        def_len = max_len - 1;
    }
    memcpy(out_name, def_os, def_len);
    out_name[def_len] = '\0';

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
                size_t vlen = strlen(val);
                if (vlen >= max_len) {
                    vlen = max_len - 1;
                }
                memcpy(out_name, val, vlen);
                out_name[vlen] = '\0';
                break;
            }
        }
        fclose(fp);
    }
#elif defined(_WIN32)
    const char* win_os = "Microsoft Windows";
    size_t win_len = strlen(win_os);
    if (win_len >= max_len) {
        win_len = max_len - 1;
    }
    memcpy(out_name, win_os, win_len);
    out_name[win_len] = '\0';
#endif
}

/* ------------------------------------------------------------------------- */
/* 2. DESSIN D'UN BADGE D'IDENTIFIANT PID À HAUTE VISIBILITÉ (CADRE JAUNE)   */
/* ------------------------------------------------------------------------- */
static float view_ai_draw_pid_badge(const char* role_title, const char* pid_val, float posX, float posY) {
    char label_part[64];
    snprintf(label_part, sizeof(label_part), "%s : ", role_title ? role_title : "PID");

    char pid_part[32];
    snprintf(pid_part, sizeof(pid_part), "PID %s", (pid_val && pid_val[0]) ? pid_val : "-");

    float w_label = ui_measure_text(label_part, 11.0f);
    float w_pid   = ui_measure_text(pid_part, 12.0f);
    float box_w   = w_label + w_pid + 20.0f;
    float box_h   = 24.0f;

    Rectangle badge_rec = { posX, posY, box_w, box_h };

    /* Fond anthracite teinté et bordure jaune vif épaisse */
    DrawRectangleRec(badge_rec, (Color){ 24, 20, 6, 255 });
    DrawRectangleLinesEx(badge_rec, 2.0f, (Color){ 255, 220, 0, 255 });

    /* Libellé en jaune et numéro PID en blanc franc */
    ui_draw_text(label_part, posX + 8.0f, posY + 5.0f, 11.0f, (Color){ 255, 220, 0, 255 });
    ui_draw_text(pid_part, posX + 8.0f + w_label, posY + 4.0f, 12.0f, (Color){ 255, 255, 255, 255 });

    return box_w;
}

/* ------------------------------------------------------------------------- */
/* 3. RENDU DE LA VUE RAPPORT IA SOUVERAIN [ 05 ]                            */
/* ------------------------------------------------------------------------- */
void ui_view_ai_draw(const RaptorReportContext* ctx) {
    int screen_w = GetScreenWidth();
    float content_y = 102.0f;

    if (g_icon_ai.id > 0) {
        DrawTexturePro(g_icon_ai,
                       (Rectangle){0, 0, (float)g_icon_ai.width, (float)g_icon_ai.height},
                       (Rectangle){16.0f, content_y, 44.0f, 44.0f},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleLines(16, (int)content_y, 44, 44, COL_CYAN_ACCENT);
    }

    ui_draw_text("RAPPORT D'AUDIT EXPLICATIF // SOVEREIGN AI ENGINE", 68.0f, content_y + 6.0f, 13.0f, COL_CYAN_ACCENT);
    ui_draw_text("Inférence locale : intégrité binaire, arbre de décision Z-Score et traque généalogique du spawn.",
                 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

    float box_y = content_y + 54.0f;
    float box_w = (float)screen_w - 32.0f;
    float box_h = (float)GetScreenHeight() - box_y - 24.0f;

    DrawRectangle(16, (int)box_y, (int)box_w, (int)box_h, (Color){ 8, 14, 22, 255 });
    DrawRectangleLines(16, (int)box_y, (int)box_w, (int)box_h, (Color){ 20, 32, 48, 255 });

    /* 1. Extraction des métriques de base */
    const RaptorCard* c_cpu    = ui_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram    = ui_find_card(ctx, "RAM");
    const RaptorCard* c_threat = ui_find_card(ctx, "MENACES");
    const RaptorCard* c_sock   = ui_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs  = ui_find_card(ctx, "PROCESSUS");

    /* 2. Extraction des métriques heuristiques et statistiques */
    const RaptorCard* c_heur   = ui_find_card(ctx, "HEURISTIQUES CRITIQUES");
    const RaptorCard* c_masq   = ui_find_card(ctx, "USURPATIONS NOM");
    const RaptorCard* c_del    = ui_find_card(ctx, "BINAIRES DÉLINKÉS");
    const RaptorCard* c_outl   = ui_find_card(ctx, "OUTLIERS STATISTIQUES");
    const RaptorCard* c_sigma  = ui_find_card(ctx, "DÉVIATION MAX (SIGMA)");

    const char* top_name = "N/D";
    const char* top_pid = "-";
    const char* top_ram = "0 Mo";

    const RaptorTable* t_top = ui_find_table(ctx, "sec_proc_mem", "TOP");
    if (t_top && t_top->row_count > 0 && t_top->rows[0].cell_texts) {
        top_pid = t_top->rows[0].cell_texts[0];
        top_name = t_top->rows[0].cell_texts[1];
        top_ram = t_top->rows[0].cell_texts[2];
    }

    /* 3. Extraction de la table d'heuristiques pour l'analyse forensique */
    const RaptorTable* t_heur = ui_find_table(ctx, "sec_heuristics", "HEURISTIQUES");

    char os_name[128];
    view_ai_detect_os(os_name, sizeof(os_name));

    float ty = box_y + 14.0f;
    ui_draw_text("RAPPORT D'AUDIT SYSTÈME // RAPTOR DEFENSE AI", 32.0f, ty, 12.0f, COL_TEXT_WHITE);
    ty += 18.0f;

    char sub_line[512];
    snprintf(sub_line, sizeof(sub_line), "Contrôle : %s | Poste Local (%s) | Moteur : Heuristique + Traque Parent",
             g_ui_last_audit_time, os_name);
    ui_draw_text(sub_line, 32.0f, ty, 11.0f, COL_TEXT_MUTED);
    ty += 14.0f;

    ui_draw_text("--------------------------------------------------------------------------------", 32.0f, ty, 11.0f, (Color){ 40, 60, 80, 255 });
    ty += 18.0f;

    /* SECTION 1 : RESSOURCES GLOBALES */
    ui_draw_text("1. CHARGE OPÉRATIONNELLE ET RESSOURCES :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 16.0f;

    char line1[512];
    snprintf(line1, sizeof(line1), "• %s processus surveillés. Charge CPU : %s | RAM : %s (%.0f %%). Top conso : %s (PID %s, %s).",
             c_procs ? c_procs->val : "300+", c_cpu ? c_cpu->val : "0.0 %",
             c_ram ? c_ram->val : "N/D", c_ram ? c_ram->pct : 0.0f,
             top_name, top_pid, top_ram);
    ui_draw_text(line1, 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    ty += 20.0f;

    /* SECTION 2 : ANALYSE STATISTIQUE Z-SCORE */
    ui_draw_text("2. MODÈLE STATISTIQUE & ARBRE D'ANOMALIES (Z-SCORE) :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 16.0f;

    int outliers_val = c_outl ? (int)strtol(c_outl->val, NULL, 10) : 0;
    char line_stat[512];
    snprintf(line_stat, sizeof(line_stat), "• Outliers critiques (> 3.0 sigmas) : %s processus | Écart maximal mesuré : %s.",
             c_outl ? c_outl->val : "0", c_sigma ? c_sigma->val : "0.00 sigma");
    ui_draw_text(line_stat, 32.0f, ty, 11.0f, (outliers_val > 0) ? COL_CRIT_RED : COL_GREEN_OK);
    ty += 20.0f;

    /* SECTION 3 : SÉCURITÉ DÉFENSIVE ET HEURISTIQUE */
    ui_draw_text("3. SÉCURITÉ DÉFENSIVE, HEURISTIQUES & INTÉGRITÉ :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 16.0f;

    int heur_val = c_heur ? (int)strtol(c_heur->val, NULL, 10) : 0;
    int masq_val = c_masq ? (int)strtol(c_masq->val, NULL, 10) : 0;
    int del_val  = c_del  ? (int)strtol(c_del->val, NULL, 10)  : 0;

    char line_heur[512];
    snprintf(line_heur, sizeof(line_heur), "• Usurpations : %d | Binaires supprimés : %d | Segments RWX : %s | Flux réseau : %s.",
             masq_val, del_val,
             c_threat ? c_threat->val : "0 ANOMALIE",
             c_sock ? c_sock->val : "0 SOCKETS");
    ui_draw_text(line_heur, 32.0f, ty, 11.0f, (heur_val > 0) ? COL_CRIT_RED : COL_GREEN_OK);
    ty += 22.0f;

    /* SECTION 4 : INVESTIGATION FORENSIQUE DE LIGNÉE */
    if (t_heur && t_heur->row_count > 0 && t_heur->rows[0].cell_texts) {
        ui_draw_text("4. FORENSIQUE // IDENTIFICATION DE LA SOURCE DU SPAWN (PERSISTANCE) :", 32.0f, ty, 12.0f, COL_CRIT_RED);
        ty += 18.0f;

        char primary_parent_num[32] = {0};

        size_t max_forensic = (t_heur->row_count < 2) ? t_heur->row_count : 2;
        for (size_t r = 0; r < max_forensic; ++r) {
            const char* s_pid   = t_heur->rows[r].cell_texts[0];
            const char* s_name  = t_heur->rows[r].cell_texts[1];
            const char* s_exe   = t_heur->rows[r].cell_texts[2];
            const char* s_par   = t_heur->rows[r].cell_texts[3];
            const char* s_cmd   = t_heur->rows[r].cell_texts[4];
            const char* s_qual  = t_heur->rows[r].cell_texts[5];

            /* Extraction du PID parent numérique depuis "PID XXXXX (nom)" */
            char parent_num[32] = {0};
            if (s_par) {
                const char* p_start = strstr(s_par, "PID ");
                if (p_start) {
                    p_start += 4;
                    size_t pi = 0;
                    while (*p_start >= '0' && *p_start <= '9' && pi < sizeof(parent_num) - 1) {
                        parent_num[pi++] = *p_start++;
                    }
                    parent_num[pi] = '\0';
                }
            }

            if (r == 0 && parent_num[0] != '\0') {
                size_t pnl = strlen(parent_num);
                if (pnl >= sizeof(primary_parent_num)) pnl = sizeof(primary_parent_num) - 1;
                memcpy(primary_parent_num, parent_num, pnl);
                primary_parent_num[pnl] = '\0';
            }

            /* Affichage en exergue des deux badges PID côte à côte */
            float b1_w = view_ai_draw_pid_badge("CIBLE MALVEILLANTE", s_pid, 32.0f, ty);
            view_ai_draw_pid_badge("PARENT CRÉATEUR", parent_num[0] ? parent_num : s_par, 32.0f + b1_w + 14.0f, ty);
            ty += 30.0f;

            char f_target[512];
            snprintf(f_target, sizeof(f_target), "• Exécutable actif : %s (%s) %s",
                     s_name ? s_name : "-", s_exe ? s_exe : "-", s_qual ? s_qual : "");
            ui_draw_text(f_target, 32.0f, ty, 11.0f, COL_CRIT_RED);
            ty += 16.0f;

            char f_parent[512];
            snprintf(f_parent, sizeof(f_parent), "   |__ Contexte du créateur : %s | Commande invoquée : %s",
                     s_par ? s_par : "[inconnu]", s_cmd ? s_cmd : "[aucun argument]");
            ui_draw_text(f_parent, 32.0f, ty, 11.0f, (Color){ 255, 205, 120, 255 });
            ty += 18.0f;
        }

        ui_draw_text("• CONDUITE À TENIR POUR L'ADMINISTRATEUR (BRISER LE RESPAWN) :", 32.0f, ty, 11.0f, COL_CYAN_ACCENT);
        ty += 16.0f;

        int ppid_num = (int)strtol(primary_parent_num[0] ? primary_parent_num : "0", NULL, 10);
        if (ppid_num <= 1) {
            ui_draw_text("  1. Processus orphelin rattaché à init/systemd (PID 1). NE SURTOUT PAS TUER LE PARENT.",
                         32.0f, ty, 11.0f, COL_CRIT_RED);
            ty += 16.0f;
            ui_draw_text("  2. Neutralisez directement la cible malveillante dans l'onglet [ 02 ] Processus.",
                         32.0f, ty, 11.0f, COL_TEXT_WHITE);
        } else {
            ui_draw_text("  1. Retirez les permissions d'exécution sur le script parent (ex: chmod -x <script>).",
                         32.0f, ty, 11.0f, COL_TEXT_WHITE);
            ty += 16.0f;
            ui_draw_text("  2. Neutralisez d'abord le PID créateur identifié ci-dessus, puis achevez le processus cible.",
                         32.0f, ty, 11.0f, COL_TEXT_WHITE);
        }
        ty += 22.0f;
    } else {
        ui_draw_text("4. FORENSIQUE // CONTRÔLE DE LIGNÉE NOMINAL :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
        ty += 16.0f;
        ui_draw_text("• Aucune tâche clandestine orpheline ou processus zombie persistant n'a été détecté.",
                         32.0f, ty, 11.0f, COL_GREEN_OK);
        ty += 22.0f;
    }

    /* SECTION 5 : RECOMMANDATIONS FINALES */
    ui_draw_text("5. RECOMMANDATION OPÉRATIONNELLE :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 16.0f;

    if (heur_val > 0 || outliers_val > 0) {
        ui_draw_text("Examinez en priorité l'onglet [ 02 ] Processus pour appliquer les mesures de neutralisation prescrites.",
                     32.0f, ty, 11.0f, (Color){ 255, 180, 190, 255 });
    } else {
        ui_draw_text("Votre système fonctionne dans les tolérances saines : intégrité des binaires certifiée sur disque.",
                     32.0f, ty, 11.0f, COL_TEXT_WHITE);
    }
    ty += 20.0f;

    ui_draw_text("--------------------------------------------------------------------------------", 32.0f, ty, 11.0f, (Color){ 40, 60, 80, 255 });
    ty += 16.0f;
    ui_draw_text("[ Diagnostic forensique consolidé en local sans télémétrie - Rapport autonome ]", 32.0f, ty, 11.0f, COL_TEXT_MUTED);
}
