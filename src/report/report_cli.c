/**
 * ============================================================================
 * NOM DU SCRIPT   : src/report/report_cli.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Moteur de restitution textuelle souverain en ligne de commande (CLI).
 *                   Consomme le modèle de données universel agnostique et affiche
 *                   l'audit dans le terminal : bannière de sécurité, cartes de
 *                   métriques synthétiques, alertes hiérarchisées par sévérité et
 *                   tableaux avec alignement dynamique des largeurs de colonnes.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Remplacement de toutes les séquences d'échappement hexadécimales par UTF-8 direct.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. cli_print_banner
 *   2. cli_get_severity_str
 *   3. cli_print_cards
 *   4. cli_print_alerts
 *   5. cli_print_table
 *   6. raptor_report_cli_print
 * ============================================================================
 */

#include "raptor/raptor_report.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* 1. AFFICHAGE DE LA BANNIÈRE PRINCIPALE DU SYSTÈME                         */
/* ------------------------------------------------------------------------- */
static void cli_print_banner(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("  RAPTOR // SYSTÈME D'AUDIT DÉFENSIF & TRAQUE MÉMOIRE (v2.0)\n");
    printf("  RAPTOR DEFENSE // AUDIT MÉMOIRE & PROCESSUS  [ SURVEILLANCE ACTIVE ]\n");
    printf("================================================================================\n");
}

/* ------------------------------------------------------------------------- */
/* 2. CONVERSION TEXTUELLE DES NIVEAUX DE SÉVÉRITÉ                           */
/* ------------------------------------------------------------------------- */
static const char* cli_get_severity_str(RaptorSeverity sev) {
    switch (sev) {
        case RAPTOR_SEV_OK:
            return "[ SUCCÈS ]";
        case RAPTOR_SEV_WARNING:
            return "[ ATTENTION ]";
        case RAPTOR_SEV_CRITICAL:
            return "[ CRITIQUE ]";
        case RAPTOR_SEV_INFO:
        default:
            return "[ INFO ]";
    }
}

/* ------------------------------------------------------------------------- */
/* 3. AFFICHAGE DES CARTES DE MÉTRIQUES SYNTHÉTIQUES                         */
/* ------------------------------------------------------------------------- */
static void cli_print_cards(const RaptorSection* sec) {
    if (!sec || sec->card_count == 0) {
        return;
    }

    printf("\n  -- MÉTRIQUES CLÉS --\n");
    for (size_t i = 0; i < sec->card_count; ++i) {
        const RaptorCard* card = &sec->cards[i];
        printf("  | %-24s : %-16s (%5.1f%%) %s\n",
               card->label,
               card->val,
               card->pct,
               cli_get_severity_str(card->sev));
    }
}

/* ------------------------------------------------------------------------- */
/* 4. AFFICHAGE DES NOTIFICATIONS ET ALERTES HIÉRARCHISÉES                   */
/* ------------------------------------------------------------------------- */
static void cli_print_alerts(const RaptorSection* sec) {
    if (!sec || sec->alert_count == 0) {
        return;
    }

    printf("\n  -- NOTIFICATIONS & ALERTES --\n");
    for (size_t i = 0; i < sec->alert_count; ++i) {
        const RaptorAlert* alert = &sec->alerts[i];
        printf("  %s %s\n", cli_get_severity_str(alert->sev), alert->message);
    }
}

/* ------------------------------------------------------------------------- */
/* 5. AFFICHAGE TABULAIRE STRUCTURÉ AVEC ALIGNEMENT DYNAMIQUE                */
/* ------------------------------------------------------------------------- */
static void cli_print_table(const RaptorTable* tbl) {
    if (!tbl || tbl->col_count == 0) {
        return;
    }

    printf("\n  >>> %s\n", tbl->title);

    size_t* col_widths = (size_t*)calloc(tbl->col_count, sizeof(size_t));
    if (!col_widths) {
        return;
    }

    for (size_t c = 0; c < tbl->col_count; ++c) {
        size_t min_w = tbl->col_names[c] ? strlen(tbl->col_names[c]) : 0;
        if (min_w < 8) min_w = 8;
        col_widths[c] = min_w;

        for (size_t r = 0; r < tbl->row_count; ++r) {
            if (tbl->rows[r].cell_texts && tbl->rows[r].cell_texts[c]) {
                size_t cell_len = strlen(tbl->rows[r].cell_texts[c]);
                if (cell_len > col_widths[c]) {
                    col_widths[c] = (cell_len > 48) ? 48 : cell_len;
                }
            }
        }
    }

    printf("  +");
    for (size_t c = 0; c < tbl->col_count; ++c) {
        for (size_t w = 0; w < col_widths[c] + 2; ++w) putchar('-');
        putchar('+');
    }
    printf("\n");

    printf("  |");
    for (size_t c = 0; c < tbl->col_count; ++c) {
        printf(" %-*s |", (int)col_widths[c], tbl->col_names[c] ? tbl->col_names[c] : "");
    }
    printf("\n");

    printf("  +");
    for (size_t c = 0; c < tbl->col_count; ++c) {
        for (size_t w = 0; w < col_widths[c] + 2; ++w) putchar('=');
        putchar('+');
    }
    printf("\n");

    for (size_t r = 0; r < tbl->row_count; ++r) {
        printf("  |");
        for (size_t c = 0; c < tbl->col_count; ++c) {
            const char* val = (tbl->rows[r].cell_texts && tbl->rows[r].cell_texts[c])
                              ? tbl->rows[r].cell_texts[c] : "";
            char truncated[64];
            if (strlen(val) > col_widths[c]) {
                strncpy(truncated, val, col_widths[c] - 3);
                truncated[col_widths[c] - 3] = '\0';
                strcat(truncated, "...");
                printf(" %-*s |", (int)col_widths[c], truncated);
            } else {
                printf(" %-*s |", (int)col_widths[c], val);
            }
        }
        printf("\n");
    }

    printf("  +");
    for (size_t c = 0; c < tbl->col_count; ++c) {
        for (size_t w = 0; w < col_widths[c] + 2; ++w) putchar('-');
        putchar('+');
    }
    printf("\n");

    free(col_widths);
}

/* ------------------------------------------------------------------------- */
/* 6. RESTITUTION TEXTUELLE COMPLÈTE SUR LE TERMINAL                         */
/* ------------------------------------------------------------------------- */
void raptor_report_cli_print(const RaptorReportContext* ctx) {
    if (!ctx) {
        printf("[RAPTOR] Aucun contexte de rapport disponible pour affichage.\n");
        return;
    }

    cli_print_banner();

    size_t sec_count = raptor_data_get_section_count(ctx);
    for (size_t s = 0; s < sec_count; ++s) {
        const RaptorSection* sec = raptor_data_get_section(ctx, s);
        if (!sec) {
            continue;
        }

        printf("\n");
        printf("================================================================================\n");
        printf("  SECTION [%02zu] : %s\n", s + 1, sec->title);
        printf("  ID MODULE    : %s\n", sec->id);
        printf("================================================================================\n");

        cli_print_alerts(sec);
        cli_print_cards(sec);

        for (size_t t = 0; t < sec->table_count; ++t) {
            cli_print_table(&sec->tables[t]);
        }
    }

    printf("\n--------------------------------------------------------------------------------\n");
    printf("  [RAPTOR CLI] Audit achevé avec succès. Zéro fuite d'information.\n");
    printf("--------------------------------------------------------------------------------\n\n");
}
