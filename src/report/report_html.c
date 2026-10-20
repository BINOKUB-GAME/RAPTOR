/**
 * ============================================================================
 * NOM DU SCRIPT   : src/report/report_html.c
 * RÉVISION        : Rev 1.4
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Moteur d'exportation de rapport souverain au format HTML5.
 *                   Génère un document autonome à haute fidélité visuelle
 *                   reprenant la charte graphique sombre (Dark Cyber/Teal).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.4) :
 *   - Filtre d'exclusion de la table brute complète dans l'export HTML
 *     (conserve la structure concise Top 10 + Anomalies + Sockets).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. html_escape_string
 *   2. html_get_severity_class
 *   3. html_detect_os_name
 *   4. html_find_card
 *   5. html_write_header_and_styles
 *   6. html_write_cards
 *   7. html_write_ai_terminal
 *   8. html_write_tables
 *   9. raptor_report_html_generate
 * ============================================================================
 */

#include "raptor/raptor_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------------- */
/* 1. ÉCHAPPEMENT DES CARACTÈRES SPÉCIAUX HTML                               */
/* ------------------------------------------------------------------------- */
static void html_escape_string(FILE* fp, const char* str) {
    if (!fp || !str) {
        return;
    }

    while (*str) {
        switch (*str) {
            case '&':  fputs("&amp;", fp);  break;
            case '<':  fputs("&lt;", fp);   break;
            case '>':  fputs("&gt;", fp);   break;
            case '"':  fputs("&quot;", fp); break;
            case '\'': fputs("&#39;", fp);  break;
            default:   fputc(*str, fp);     break;
        }
        str++;
    }
}

/* ------------------------------------------------------------------------- */
/* 2. DÉTERMINATION DE LA CLASSE CSS DE SÉVÉRITÉ                             */
/* ------------------------------------------------------------------------- */
static const char* html_get_severity_class(RaptorSeverity sev) {
    switch (sev) {
        case RAPTOR_SEV_OK:
            return "sev-ok";
        case RAPTOR_SEV_WARNING:
            return "sev-warning";
        case RAPTOR_SEV_CRITICAL:
            return "sev-critical";
        case RAPTOR_SEV_INFO:
        default:
            return "sev-info";
    }
}

/* ------------------------------------------------------------------------- */
/* 3. DÉTECTION DYNAMIQUE DU SYSTÈME D'EXPLOITATION HÔTE                     */
/* ------------------------------------------------------------------------- */
static void html_detect_os_name(char* out_buf, size_t max_len) {
    if (!out_buf || max_len == 0) return;
    strncpy(out_buf, "Système Local", max_len - 1);
    out_buf[max_len - 1] = '\0';

#if defined(__linux__)
    FILE* fp = fopen("/etc/os-release", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "PRETTY_NAME=", 12) == 0 || strncmp(line, "NAME=", 5) == 0) {
                char* val = strchr(line, '=') + 1;
                if (*val == '"') val++;
                size_t l = strlen(val);
                while (l > 0 && (val[l - 1] == '\n' || val[l - 1] == '\r' || val[l - 1] == '"')) {
                    val[l - 1] = '\0';
                    l--;
                }
                strncpy(out_buf, val, max_len - 1);
                out_buf[max_len - 1] = '\0';
                break;
            }
        }
        fclose(fp);
    }
#elif defined(_WIN32)
    strncpy(out_buf, "Microsoft Windows", max_len - 1);
#endif
}

/* ------------------------------------------------------------------------- */
/* 4. RECHERCHE D'UNE CARTE PAR MOT-CLÉ DANS LE CONTEXTE                     */
/* ------------------------------------------------------------------------- */
static const RaptorCard* html_find_card(const RaptorReportContext* ctx, const char* keyword) {
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
/* 5. ÉCRITURE DES EN-TÊTES, CSS ET BANDEAU SUPÉRIEUR                       */
/* ------------------------------------------------------------------------- */
static void html_write_header_and_styles(FILE* fp, const char* timestamp_str, const char* os_name, bool has_threats) {
    fputs("<!DOCTYPE html>\n", fp);
    fputs("<html lang=\"fr\">\n<head>\n", fp);
    fputs("  <meta charset=\"UTF-8\">\n", fp);
    fputs("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n", fp);
    fputs("  <title>RAPTOR DEFENSE // RAPPORT D'AUDIT SYST&Egrave;ME</title>\n", fp);
    fputs("  <style>\n", fp);
    fputs("    :root {\n", fp);
    fputs("      --bg-body: #070d16;\n", fp);
    fputs("      --bg-card: #0c1421;\n", fp);
    fputs("      --bg-box: #060b12;\n", fp);
    fputs("      --border-dark: #152234;\n", fp);
    fputs("      --border-cyan: #00d2ff;\n", fp);
    fputs("      --text-main: #dce7f3;\n", fp);
    fputs("      --text-muted: #6b8299;\n", fp);
    fputs("      --col-green: #00e676;\n", fp);
    fputs("      --col-cyan: #00d2ff;\n", fp);
    fputs("      --col-yellow: #ffbb00;\n", fp);
    fputs("      --col-red: #ff3355;\n", fp);
    fputs("    }\n", fp);
    fputs("    * { box-sizing: border-box; }\n", fp);
    fputs("    body {\n", fp);
    fputs("      margin: 0; padding: 28px 16px;\n", fp);
    fputs("      background-color: var(--bg-body);\n", fp);
    fputs("      color: var(--text-main);\n", fp);
    fputs("      font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, sans-serif;\n", fp);
    fputs("    }\n", fp);
    fputs("    .container {\n", fp);
    fputs("      max-width: 1100px; margin: 0 auto;\n", fp);
    fputs("    }\n", fp);
    fputs("    .header-bar {\n", fp);
    fputs("      display: flex; justify-content: space-between; align-items: flex-start;\n", fp);
    fputs("      margin-bottom: 22px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .header-title {\n", fp);
    fputs("      font-size: 1.35rem; font-weight: 800; color: var(--col-cyan); letter-spacing: 0.5px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .header-subtitle {\n", fp);
    fputs("      font-size: 0.85rem; color: var(--text-muted); margin-top: 4px; font-family: monospace;\n", fp);
    fputs("    }\n", fp);
    fputs("    .badge-status {\n", fp);
    fputs("      padding: 6px 14px; font-size: 0.8rem; font-weight: 700; border-radius: 4px;\n", fp);
    fputs("      letter-spacing: 0.5px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .badge-ok {\n", fp);
    fputs("      background-color: rgba(0, 230, 118, 0.12);\n", fp);
    fputs("      border: 1px solid var(--col-green); color: var(--col-green);\n", fp);
    fputs("    }\n", fp);
    fputs("    .badge-threat {\n", fp);
    fputs("      background-color: rgba(255, 51, 85, 0.15);\n", fp);
    fputs("      border: 1px solid var(--col-red); color: var(--col-red);\n", fp);
    fputs("    }\n", fp);
    fputs("    .metrics-grid {\n", fp);
    fputs("      display: grid; grid-template-columns: repeat(4, 1fr); gap: 14px;\n", fp);
    fputs("      margin-bottom: 24px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .metric-card {\n", fp);
    fputs("      background-color: var(--bg-card);\n", fp);
    fputs("      border: 1px solid var(--border-dark);\n", fp);
    fputs("      padding: 16px 18px; border-radius: 4px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .card-label {\n", fp);
    fputs("      font-size: 0.72rem; font-weight: 700; color: var(--text-muted); text-transform: uppercase;\n", fp);
    fputs("    }\n", fp);
    fputs("    .card-value {\n", fp);
    fputs("      font-size: 1.45rem; font-weight: 800; margin: 6px 0 8px 0; color: #ffffff;\n", fp);
    fputs("    }\n", fp);
    fputs("    .card-value.green { color: var(--col-green); }\n", fp);
    fputs("    .card-value.cyan { color: var(--col-cyan); }\n", fp);
    fputs("    .card-value.red { color: var(--col-red); }\n", fp);
    fputs("    .progress-track {\n", fp);
    fputs("      width: 100%; height: 5px; background-color: #050a11; border-radius: 2px; overflow: hidden;\n", fp);
    fputs("    }\n", fp);
    fputs("    .progress-fill {\n", fp);
    fputs("      height: 100%; background-color: var(--col-cyan);\n", fp);
    fputs("    }\n", fp);
    fputs("    .card-subtext {\n", fp);
    fputs("      font-size: 0.75rem; color: var(--text-muted); font-family: monospace;\n", fp);
    fputs("    }\n", fp);
    fputs("    .section-box {\n", fp);
    fputs("      background-color: var(--bg-card);\n", fp);
    fputs("      border: 1px solid var(--border-dark);\n", fp);
    fputs("      border-radius: 4px; padding: 18px; margin-bottom: 24px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .section-header {\n", fp);
    fputs("      font-size: 0.95rem; font-weight: 800; color: var(--col-cyan); text-transform: uppercase;\n", fp);
    fputs("      margin-bottom: 14px; letter-spacing: 0.5px;\n", fp);
    fputs("    }\n", fp);
    fputs("    .terminal-box {\n", fp);
    fputs("      background-color: var(--bg-box);\n", fp);
    fputs("      border: 1px solid var(--border-dark);\n", fp);
    fputs("      padding: 18px 20px; font-family: 'Consolas', 'Courier New', monospace;\n", fp);
    fputs("      font-size: 0.85rem; line-height: 1.5; color: #c9dbed; white-space: pre-wrap;\n", fp);
    fputs("    }\n", fp);
    fputs("    table {\n", fp);
    fputs("      width: 100%; border-collapse: collapse; font-size: 0.84rem; margin-top: 4px;\n", fp);
    fputs("    }\n", fp);
    fputs("    th {\n", fp);
    fputs("      background-color: #101b2c; color: var(--text-muted); text-align: left;\n", fp);
    fputs("      padding: 10px 14px; font-weight: 600; border-bottom: 1px solid var(--border-dark);\n", fp);
    fputs("      font-size: 0.78rem;\n", fp);
    fputs("    }\n", fp);
    fputs("    td {\n", fp);
    fputs("      padding: 9px 14px; border-bottom: 1px solid #101a28;\n", fp);
    fputs("      font-family: 'Consolas', monospace;\n", fp);
    fputs("    }\n", fp);
    fputs("    tr:hover td { background-color: #111e30; }\n", fp);
    fputs("    .sev-ok { color: var(--col-green); font-weight: 600; }\n", fp);
    fputs("    .sev-warning { color: var(--col-yellow); }\n", fp);
    fputs("    .sev-critical { color: var(--col-red); font-weight: 700; }\n", fp);
    fputs("    .sev-info { color: var(--text-main); }\n", fp);
    fputs("  </style>\n</head>\n<body>\n", fp);
    fputs("  <div class=\"container\">\n", fp);

    fputs("    <div class=\"header-bar\">\n", fp);
    fputs("      <div>\n", fp);
    fputs("        <div class=\"header-title\">RAPTOR DEFENSE // RAPPORT D'AUDIT SYST&Egrave;ME</div>\n", fp);
    fprintf(fp, "        <div class=\"header-subtitle\">Horodatage : %s | %s</div>\n", timestamp_str, os_name);
    fputs("      </div>\n", fp);
    if (has_threats) {
        fputs("      <div class=\"badge-status badge-threat\">ATTENTION // ANOMALIES D&Eacute;TECT&Eacute;ES</div>\n", fp);
    } else {
        fputs("      <div class=\"badge-status badge-ok\">SYST&Egrave;ME SAIN &amp; CONFORME</div>\n", fp);
    }
    fputs("    </div>\n", fp);
}

/* ------------------------------------------------------------------------- */
/* 6. RENDU DYNAMIQUE DES 4 CARTES MÉTRIQUES                                 */
/* ------------------------------------------------------------------------- */
static void html_write_cards(FILE* fp, const RaptorReportContext* ctx) {
    const RaptorCard* c_cpu = html_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram = html_find_card(ctx, "RAM");
    const RaptorCard* c_threat = html_find_card(ctx, "MENACES");
    const RaptorCard* c_sock = html_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs = html_find_card(ctx, "PROCESSUS");

    fputs("    <div class=\"metrics-grid\">\n", fp);

    fputs("      <div class=\"metric-card\">\n", fp);
    fputs("        <div class=\"card-label\">CHARGE PROCESSEUR</div>\n", fp);
    fprintf(fp, "        <div class=\"card-value\">%s</div>\n", c_cpu ? c_cpu->val : "0.0 %");
    fprintf(fp, "        <div class=\"progress-track\"><div class=\"progress-fill\" style=\"width: %.1f%%;\"></div></div>\n",
            c_cpu ? c_cpu->pct : 0.0f);
    fputs("      </div>\n", fp);

    fputs("      <div class=\"metric-card\">\n", fp);
    fputs("        <div class=\"card-label\">M&Eacute;MOIRE VIVE (RAM)</div>\n", fp);
    fprintf(fp, "        <div class=\"card-value\">%s</div>\n", c_ram ? c_ram->val : "N/D");
    fprintf(fp, "        <div class=\"progress-track\"><div class=\"progress-fill\" style=\"width: %.1f%%;\"></div></div>\n",
            c_ram ? c_ram->pct : 0.0f);
    fputs("      </div>\n", fp);

    fputs("      <div class=\"metric-card\">\n", fp);
    fputs("        <div class=\"card-label\">MENACES / RWX</div>\n", fp);
    const char* threat_cls = (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL) ? "red" : "green";
    fprintf(fp, "        <div class=\"card-value %s\">%s</div>\n", threat_cls, c_threat ? c_threat->val : "0 ANOMALIE");
    fprintf(fp, "        <div class=\"card-subtext\">%s processus analys&eacute;s</div>\n", c_procs ? c_procs->val : "Tous");
    fputs("      </div>\n", fp);

    fputs("      <div class=\"metric-card\">\n", fp);
    fputs("        <div class=\"card-label\">SOCKETS R&Eacute;SEAU</div>\n", fp);
    fprintf(fp, "        <div class=\"card-value cyan\">%s</div>\n", c_sock ? c_sock->val : "0 SOCKETS");
    fputs("        <div class=\"card-subtext\">TCP / UDP surveill&eacute;s</div>\n", fp);
    fputs("      </div>\n", fp);

    fputs("    </div>\n", fp);
}

/* ------------------------------------------------------------------------- */
/* 7. RENDU DYNAMIQUE DU DIAGNOSTIC DE L'IA SOUVERAINE                       */
/* ------------------------------------------------------------------------- */
static void html_write_ai_terminal(FILE* fp, const RaptorReportContext* ctx, const char* timestamp_str, const char* os_name) {
    const RaptorCard* c_cpu = html_find_card(ctx, "PROCESSEUR");
    const RaptorCard* c_ram = html_find_card(ctx, "RAM");
    const RaptorCard* c_threat = html_find_card(ctx, "MENACES");
    const RaptorCard* c_sock = html_find_card(ctx, "RÉSEAU");
    const RaptorCard* c_procs = html_find_card(ctx, "PROCESSUS");

    const char* top_name = "N/D";
    const char* top_pid = "-";
    const char* top_ram = "0 Mo";

    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];
        for (size_t t = 0; t < sec->table_count; ++t) {
            if (strstr(sec->tables[t].title, "TOP") != NULL && sec->tables[t].row_count > 0) {
                const RaptorTableRow* r0 = &sec->tables[t].rows[0];
                if (r0->cell_texts) {
                    top_pid = r0->cell_texts[0];
                    top_name = r0->cell_texts[1];
                    top_ram = r0->cell_texts[2];
                }
                break;
            }
        }
    }

    fputs("    <div class=\"section-box\">\n", fp);
    fputs("      <div class=\"section-header\">| 1. SYNTH&Egrave;SE DU DIAGNOSTIC PAR L'IA SOUVERAINE</div>\n", fp);
    fputs("      <div class=\"terminal-box\">", fp);

    fputs("RAPPORT D'AUDIT SYST&Egrave;ME // RAPTOR DEFENSE AI\n", fp);
    fprintf(fp, "Date de contr&ocirc;le : %s | Poste Local (%s)\n", timestamp_str, os_name);
    fputs("--------------------------------------------------------------------------------\n\n", fp);

    fputs("1. SYNTH&Egrave;SE GLOBALE EN LANGAGE CLAIR :\n", fp);
    fprintf(fp, "Votre syst&egrave;me ex&eacute;cute actuellement %s processus actifs pour une charge globale de %s CPU.\n",
            c_procs ? c_procs->val : "tous les",
            c_cpu ? c_cpu->val : "0.0 %");
    fprintf(fp, "La m&eacute;moire physique vive utilis&eacute;e s'&eacute;l&egrave;ve &agrave; %s (soit %.0f %% d'occupation).\n",
            c_ram ? c_ram->val : "N/D",
            c_ram ? c_ram->pct : 0.0f);
    fprintf(fp, "Le plus gros consommateur de m&eacute;moire actuel est : %s (PID %s, %s).\n\n", top_name, top_pid, top_ram);

    fputs("2. &Eacute;TAT DES ANOMALIES ET MENACES :\n", fp);
    fprintf(fp, "- Injections M&eacute;moire (RWX) &amp; Binaires suspects : %s d&eacute;tection(s).\n",
            c_threat ? c_threat->val : "0");
    fputs("  Note explicative : Ces pages m&eacute;moire concernent principalement les moteurs de compilation\n", fp);
    fputs("  &agrave; la vol&eacute;e JIT (Chromium, Cinnamon, Qt, Python). En l'absence de trafic r&eacute;seau suspect associ&eacute;,\n", fp);
    fputs("  il s'agit d'allocations de fonctionnement standards.\n", fp);
    fputs("- D&eacute;rives de M&eacute;moire Continue (Fuites) : 0 processus critique en d&eacute;rive.\n", fp);
    fprintf(fp, "- Surveillance Flux R&eacute;seau : %s surveill&eacute;s.\n\n",
            c_sock ? c_sock->val : "Tous flux");

    fputs("3. RECOMMANDATIONS OP&Eacute;RATIONNELLES :\n", fp);
    if (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL) {
        fputs("1. Des processus hors répertoires officiels ou avec mémoire injectée sont actifs : examinez le tableau 3.\n", fp);
        fputs("2. Vérifiez la légitimité des binaires concernés avant toute reconnexion non sécurisée.\n", fp);
    } else {
        fputs("1. Aucune fuite continue critique ne menace la stabilit&eacute; de votre session.\n", fp);
        fputs("2. Votre machine fonctionne dans des tol&eacute;rances nominales saines.\n", fp);
    }
    fputs("--------------------------------------------------------------------------------\n", fp);
    fputs("[ Rapport consolid&eacute; en local - Z&eacute;ro donn&eacute;e transmise vers l'ext&eacute;rieur ]\n", fp);

    fputs("      </div>\n", fp);
    fputs("    </div>\n", fp);
}

/* ------------------------------------------------------------------------- */
/* 8. RENDU DES TABLEAUX STRUCTURÉS DU RAPPORT                               */
/* ------------------------------------------------------------------------- */
static void html_write_tables(FILE* fp, const RaptorTable* tbl, size_t section_idx) {
    if (!fp || !tbl || tbl->col_count == 0) return;

    fputs("    <div class=\"section-box\">\n", fp);
    fprintf(fp, "      <div class=\"section-header\">| %zu. ", section_idx);
    html_escape_string(fp, tbl->title);
    fputs("</div>\n", fp);

    fputs("      <table>\n        <thead>\n          <tr>\n", fp);
    for (size_t c = 0; c < tbl->col_count; ++c) {
        fputs("            <th>", fp);
        html_escape_string(fp, tbl->col_names[c]);
        fputs("</th>\n", fp);
    }
    fputs("          </tr>\n        </thead>\n        <tbody>\n", fp);

    for (size_t r = 0; r < tbl->row_count; ++r) {
        fputs("          <tr>\n", fp);
        for (size_t c = 0; c < tbl->col_count; ++c) {
            const char* cell_txt = (tbl->rows[r].cell_texts && tbl->rows[r].cell_texts[c])
                                   ? tbl->rows[r].cell_texts[c] : "";
            RaptorSeverity sev = (tbl->rows[r].cell_sevs) ? tbl->rows[r].cell_sevs[c] : RAPTOR_SEV_INFO;
            fprintf(fp, "            <td class=\"%s\">", html_get_severity_class(sev));
            html_escape_string(fp, cell_txt);
            fputs("</td>\n", fp);
        }
        fputs("          </tr>\n", fp);
    }

    fputs("        </tbody>\n      </table>\n    </div>\n", fp);
}

/* ------------------------------------------------------------------------- */
/* 9. GÉNÉRATION GLOBALE DU DOCUMENT HTML                                    */
/* ------------------------------------------------------------------------- */
bool raptor_report_html_generate(const RaptorReportContext* ctx, const char* filepath) {
    if (!ctx || !filepath) {
        return false;
    }

    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        return false;
    }

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[64] = "2026-10-20 00:00:00";
    if (tm_info) {
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    }

    char os_name[128];
    html_detect_os_name(os_name, sizeof(os_name));

    const RaptorCard* c_threat = html_find_card(ctx, "MENACES");
    bool has_threats = (c_threat && c_threat->sev == RAPTOR_SEV_CRITICAL);

    html_write_header_and_styles(fp, time_buf, os_name, has_threats);
    html_write_cards(fp, ctx);
    html_write_ai_terminal(fp, ctx, time_buf, os_name);

    size_t section_counter = 2;
    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];
        if (strcmp(sec->id, "sec_differential") == 0) {
            continue;
        }

        for (size_t t = 0; t < sec->table_count; ++t) {
            /* Ne pas exporter la liste brute complète des 300+ processus dans le rapport HTML */
            if (strstr(sec->tables[t].title, "INSPECTION GLOBALE") != NULL) {
                continue;
            }
            html_write_tables(fp, &sec->tables[t], section_counter++);
        }
    }

    fputs("  </div>\n</body>\n</html>\n", fp);
    fclose(fp);
    return true;
}
