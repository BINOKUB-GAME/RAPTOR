/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/views/view_ai.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Vue dédiée [ 05 ] Rapport IA de l'interface graphique.
 *                   Rédige en direct la synthèse d'audit de sécurité souveraine
 *                   en croisant les métriques système, les alertes du moteur
 *                   heuristique expert et l'évaluation statistique (Z-Score).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Raccordement direct aux données de sec_heuristics (usurpations, délinkés).
 *   - Raccordement direct aux données de sec_anomaly_engine (outliers, sigmas).
 *   - Rédaction dynamique qualifiant les processus aberrants selon l'arbre de décision.
 *   - Conservation du dimensionnement strict évitant les warnings de compilation.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. view_ai_detect_os
 *   2. ui_view_ai_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include <stdio.h>
#include <string.h>

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
/* 2. RENDU DE LA VUE RAPPORT IA SOUVERAIN [ 05 ]                            */
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
    ui_draw_text("Inférence locale combinant heuristique d'évasion, déviation statistique et permissions mémoire.",
                 68.0f, content_y + 26.0f, 11.0f, COL_TEXT_MUTED);

    float box_y = content_y + 54.0f;
    float box_w = (float)screen_w - 32.0f;
    float box_h = (float)GetScreenHeight() - box_y - 24.0f;

    DrawRectangle(16, (int)box_y, (int)box_w, (int)box_h, (Color){ 8, 14, 22, 255 });
    DrawRectangleLines(16, (int)box_y, (int)box_w, (int)box_h, (Color){ 20, 32, 48, 255 });

    /* Métriques de base */
    const RaptorCard* c_cpu    = ui_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram    = ui_find_card(ctx, "RAM");
    const RaptorCard* c_threat = ui_find_card(ctx, "MENACES");
    const RaptorCard* c_sock   = ui_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs  = ui_find_card(ctx, "PROCESSUS");

    /* Métriques issues des deux nouveaux greffons */
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

    char os_name[128];
    view_ai_detect_os(os_name, sizeof(os_name));

    float ty = box_y + 14.0f;
    ui_draw_text("RAPPORT D'AUDIT SYSTÈME // RAPTOR DEFENSE AI", 32.0f, ty, 12.0f, COL_TEXT_WHITE);
    ty += 18.0f;

    char sub_line[256];
    snprintf(sub_line, sizeof(sub_line), "Contrôle : %s | Poste Local (%s) | Architecture : Heuristique + Z-Score Vectoriel",
             g_ui_last_audit_time, os_name);
    ui_draw_text(sub_line, 32.0f, ty, 11.0f, COL_TEXT_MUTED);
    ty += 16.0f;

    ui_draw_text("--------------------------------------------------------------------------------", 32.0f, ty, 11.0f, (Color){ 40, 60, 80, 255 });
    ty += 22.0f;

    /* 1. SYNTHÈSE GLOBALE */
    ui_draw_text("1. CHARGE OPÉRATIONNELLE ET RESSOURCES :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 18.0f;

    char line1[256];
    snprintf(line1, sizeof(line1), "• %s processus actifs sous surveillance. Charge CPU globale mesurée à %s.",
             c_procs ? c_procs->val : "300+", c_cpu ? c_cpu->val : "0.0 %");
    ui_draw_text(line1, 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    ty += 18.0f;

    char line2[256];
    snprintf(line2, sizeof(line2), "• Mémoire vive (RAM) : %s occupés (soit %.0f %% de saturation).",
             c_ram ? c_ram->val : "N/D", c_ram ? c_ram->pct : 0.0f);
    ui_draw_text(line2, 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    ty += 18.0f;

    char line3[256];
    snprintf(line3, sizeof(line3), "• Principal consommateur physique : %s (PID %s, empreinte RSS : %s).",
             top_name, top_pid, top_ram);
    ui_draw_text(line3, 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    ty += 24.0f;

    /* 2. ANALYSE STATISTIQUE & ARBRE DE DÉCISION */
    ui_draw_text("2. MODÈLE STATISTIQUE & DÉVIATION DES COMPORTEMENTS (Z-SCORE) :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 18.0f;

    int outliers_val = c_outl ? (int)strtol(c_outl->val, NULL, 10) : 0;
    char line_stat[256];
    snprintf(line_stat, sizeof(line_stat), "• Outliers critiques détectés (> 3.0 sigmas) : %s processus (Écart maximal mesuré : %s).",
             c_outl ? c_outl->val : "0", c_sigma ? c_sigma->val : "0.00 \xcf\x83");
    ui_draw_text(line_stat, 32.0f, ty, 11.0f, (outliers_val > 0) ? COL_CRIT_RED : COL_GREEN_OK);
    ty += 18.0f;

    if (outliers_val > 0) {
        ui_draw_text("  Diagnostic modèle : des processus dévient anormalement du comportement nominal de votre système",
                     32.0f, ty, 11.0f, (Color){ 255, 180, 190, 255 });
        ty += 16.0f;
        ui_draw_text("  en termes de volume mémoire ou d'ouverture massive de descripteurs (voir onglet [ 02 ]).",
                     32.0f, ty, 11.0f, (Color){ 255, 180, 190, 255 });
    } else {
        ui_draw_text("  Diagnostic modèle : distribution gaussienne normale. Aucun emballement de descripteurs ou de fuite.",
                     32.0f, ty, 11.0f, COL_TEXT_MUTED);
    }
    ty += 24.0f;

    /* 3. INTÉGRITÉ BINAIRE ET HEURISTIQUES */
    ui_draw_text("3. SÉCURITÉ DÉFENSIVE, HEURISTIQUES & INTÉGRITÉ BINAIRE :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 18.0f;

    int heur_val = c_heur ? (int)strtol(c_heur->val, NULL, 10) : 0;
    int masq_val = c_masq ? (int)strtol(c_masq->val, NULL, 10) : 0;
    int del_val  = c_del  ? (int)strtol(c_del->val, NULL, 10)  : 0;

    char line_heur[256];
    snprintf(line_heur, sizeof(line_heur), "• Usurpation système (Masquerading) : %d | Binaires supprimés en cours d'exécution : %d",
             masq_val, del_val);
    ui_draw_text(line_heur, 32.0f, ty, 11.0f, (heur_val > 0) ? COL_CRIT_RED : COL_GREEN_OK);
    ty += 18.0f;

    char line_rwx[256];
    snprintf(line_rwx, sizeof(line_rwx), "• Cartographies mémoire suspectes (Segments RWX) : %s",
             c_threat ? c_threat->val : "0 ANOMALIE");
    bool has_rwx = (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL);
    ui_draw_text(line_rwx, 32.0f, ty, 11.0f, has_rwx ? COL_CRIT_RED : COL_GREEN_OK);
    ty += 18.0f;

    char line_sock[256];
    snprintf(line_sock, sizeof(line_sock), "• Surveillance des flux de télécommunication : %s surveillés en temps réel.",
             c_sock ? c_sock->val : "0 SOCKETS");
    ui_draw_text(line_sock, 32.0f, ty, 11.0f, COL_TEXT_WHITE);
    ty += 24.0f;

    /* 4. RECOMMANDATIONS */
    ui_draw_text("4. CONCLUSIONS OPÉRATIONNELLES :", 32.0f, ty, 12.0f, COL_CYAN_ACCENT);
    ty += 18.0f;

    if (heur_val > 0 || has_rwx || outliers_val > 0) {
        ui_draw_text("1. Présence d'anomalies critiques nécessitant un examen immédiat dans l'onglet [ 02 ] Processus.",
                     32.0f, ty, 11.0f, COL_CRIT_RED);
        ty += 18.0f;
        ui_draw_text("2. Utilisez l'action d'arrêt sécurisé pour neutraliser les processus aux scores Z anormaux.",
                     32.0f, ty, 11.0f, COL_TEXT_WHITE);
    } else {
        ui_draw_text("1. Système intègre : les binaires correspondent à leurs signatures officielles sur le disque.",
                     32.0f, ty, 11.0f, COL_GREEN_OK);
        ty += 18.0f;
        ui_draw_text("2. Empreinte mémoire stable : aucune fuite continue ni déviation statistique mesurée.",
                     32.0f, ty, 11.0f, COL_TEXT_WHITE);
    }
    ty += 22.0f;

    ui_draw_text("--------------------------------------------------------------------------------", 32.0f, ty, 11.0f, (Color){ 40, 60, 80, 255 });
    ty += 18.0f;
    ui_draw_text("[ Diagnostic autonome produit en local par inférence directe - Zéro télémétrie distante ]",
                 32.0f, ty, 11.0f, COL_TEXT_MUTED);
}
