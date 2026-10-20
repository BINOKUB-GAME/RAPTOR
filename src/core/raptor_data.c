/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/raptor_data.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Implémentation du conteneur de données universel agnostique.
 *                   Gère le cycle de vie du modèle de rapport (allocation,
 *                   sections, cartes de métriques, tableaux dynamiques,
 *                   alertes, réinitialisation à chaud et destruction complète).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Implémentation de raptor_data_context_clear() : libération en profondeur
 *     de toutes les allocations enfants (lignes, cellules, colonnes, tables,
 *     cartes et alertes) tout en conservant l'enveloppe du contexte valide.
 *   - Résolution du problème d'accumulation mémoire lors des ré-audits successifs.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. raptor_data_context_create
 *   2. raptor_data_section_add
 *   3. raptor_data_card_add
 *   4. raptor_data_table_create
 *   5. raptor_data_table_row_add
 *   6. raptor_data_alert_add
 *   7. raptor_data_get_section_count
 *   8. raptor_data_get_section
 *   9. raptor_data_context_clear
 *  10. raptor_data_context_destroy
 * ============================================================================
 */

#include "raptor/raptor_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* 1. ALLOCATION ET INITIALISATION D'UN NOUVEAU CONTEXTE DE RAPPORT          */
/* ------------------------------------------------------------------------- */
RaptorReportContext* raptor_data_context_create(void) {
    RaptorReportContext* ctx = (RaptorReportContext*)calloc(1, sizeof(RaptorReportContext));
    if (!ctx) {
        return NULL;
    }
    ctx->sections = NULL;
    ctx->section_count = 0;
    ctx->section_capacity = 0;
    return ctx;
}

/* ------------------------------------------------------------------------- */
/* 2. AJOUT D'UNE SECTION AUTONOME AU CONTEXTE                               */
/* ------------------------------------------------------------------------- */
RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title) {
    if (!ctx || !id) {
        return NULL;
    }

    if (ctx->section_count >= ctx->section_capacity) {
        size_t new_cap = (ctx->section_capacity == 0) ? 4 : ctx->section_capacity * 2;
        RaptorSection* new_arr = (RaptorSection*)realloc(ctx->sections, new_cap * sizeof(RaptorSection));
        if (!new_arr) {
            return NULL;
        }
        ctx->sections = new_arr;
        ctx->section_capacity = new_cap;
    }

    RaptorSection* sec = &ctx->sections[ctx->section_count];
    memset(sec, 0, sizeof(RaptorSection));

    strncpy(sec->id, id, sizeof(sec->id) - 1);
    sec->id[sizeof(sec->id) - 1] = '\0';

    if (title) {
        strncpy(sec->title, title, sizeof(sec->title) - 1);
        sec->title[sizeof(sec->title) - 1] = '\0';
    }

    ctx->section_count++;
    return sec;
}

/* ------------------------------------------------------------------------- */
/* 3. AJOUT D'UNE CARTE MÉTRIQUE DANS UNE SECTION                            */
/* ------------------------------------------------------------------------- */
bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev) {
    if (!sec || !label || !val) {
        return false;
    }

    if (sec->card_count >= sec->card_capacity) {
        size_t new_cap = (sec->card_capacity == 0) ? 8 : sec->card_capacity * 2;
        RaptorCard* new_arr = (RaptorCard*)realloc(sec->cards, new_cap * sizeof(RaptorCard));
        if (!new_arr) {
            return false;
        }
        sec->cards = new_arr;
        sec->card_capacity = new_cap;
    }

    RaptorCard* card = &sec->cards[sec->card_count];
    strncpy(card->label, label, sizeof(card->label) - 1);
    card->label[sizeof(card->label) - 1] = '\0';

    strncpy(card->val, val, sizeof(card->val) - 1);
    card->val[sizeof(card->val) - 1] = '\0';

    card->pct = pct;
    card->sev = sev;

    sec->card_count++;
    return true;
}

/* ------------------------------------------------------------------------- */
/* 4. CRÉATION D'UNE TABLE STRUCTURÉE DANS UNE SECTION                       */
/* ------------------------------------------------------------------------- */
RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names) {
    if (!sec || !title || col_count == 0) {
        return NULL;
    }

    if (sec->table_count >= sec->table_capacity) {
        size_t new_cap = (sec->table_capacity == 0) ? 4 : sec->table_capacity * 2;
        RaptorTable* new_arr = (RaptorTable*)realloc(sec->tables, new_cap * sizeof(RaptorTable));
        if (!new_arr) {
            return NULL;
        }
        sec->tables = new_arr;
        sec->table_capacity = new_cap;
    }

    RaptorTable* tbl = &sec->tables[sec->table_count];
    memset(tbl, 0, sizeof(RaptorTable));

    strncpy(tbl->title, title, sizeof(tbl->title) - 1);
    tbl->title[sizeof(tbl->title) - 1] = '\0';

    tbl->col_count = col_count;
    tbl->col_names = (char**)calloc(col_count, sizeof(char*));
    if (!tbl->col_names) {
        return NULL;
    }

    for (size_t c = 0; c < col_count; ++c) {
        const char* name = (col_names && col_names[c]) ? col_names[c] : "";
        size_t len = strlen(name);
        tbl->col_names[c] = (char*)malloc(len + 1);
        if (tbl->col_names[c]) {
            memcpy(tbl->col_names[c], name, len + 1);
        }
    }

    sec->table_count++;
    return tbl;
}

/* ------------------------------------------------------------------------- */
/* 5. INSERTION D'UNE LIGNE DANS UNE TABLE STRUCTURÉE                        */
/* ------------------------------------------------------------------------- */
bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs) {
    if (!table || !cell_texts || table->col_count == 0) {
        return false;
    }

    if (table->row_count >= table->row_capacity) {
        size_t new_cap = (table->row_capacity == 0) ? 16 : table->row_capacity * 2;
        RaptorTableRow* new_arr = (RaptorTableRow*)realloc(table->rows, new_cap * sizeof(RaptorTableRow));
        if (!new_arr) {
            return false;
        }
        table->rows = new_arr;
        table->row_capacity = new_cap;
    }

    RaptorTableRow* row = &table->rows[table->row_count];
    row->cell_texts = (char**)calloc(table->col_count, sizeof(char*));
    row->cell_sevs = (RaptorSeverity*)calloc(table->col_count, sizeof(RaptorSeverity));

    if (!row->cell_texts || !row->cell_sevs) {
        if (row->cell_texts) free(row->cell_texts);
        if (row->cell_sevs) free(row->cell_sevs);
        return false;
    }

    for (size_t c = 0; c < table->col_count; ++c) {
        const char* src_text = cell_texts[c] ? cell_texts[c] : "";
        size_t len = strlen(src_text);
        row->cell_texts[c] = (char*)malloc(len + 1);
        if (row->cell_texts[c]) {
            memcpy(row->cell_texts[c], src_text, len + 1);
        }

        row->cell_sevs[c] = cell_sevs ? cell_sevs[c] : RAPTOR_SEV_INFO;
    }

    table->row_count++;
    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. AJOUT D'UNE NOTIFICATION OU ALERTE DANS UNE SECTION                    */
/* ------------------------------------------------------------------------- */
bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message) {
    if (!sec || !message) {
        return false;
    }

    if (sec->alert_count >= sec->alert_capacity) {
        size_t new_cap = (sec->alert_capacity == 0) ? 4 : sec->alert_capacity * 2;
        RaptorAlert* new_arr = (RaptorAlert*)realloc(sec->alerts, new_cap * sizeof(RaptorAlert));
        if (!new_arr) {
            return false;
        }
        sec->alerts = new_arr;
        sec->alert_capacity = new_cap;
    }

    RaptorAlert* alert = &sec->alerts[sec->alert_count];
    alert->sev = sev;
    strncpy(alert->message, message, sizeof(alert->message) - 1);
    alert->message[sizeof(alert->message) - 1] = '\0';

    sec->alert_count++;
    return true;
}

/* ------------------------------------------------------------------------- */
/* 7. OBTENTION DU NOMBRE TOTAL DE SECTIONS DU RAPPORT                       */
/* ------------------------------------------------------------------------- */
size_t raptor_data_get_section_count(const RaptorReportContext* ctx) {
    return ctx ? ctx->section_count : 0;
}

/* ------------------------------------------------------------------------- */
/* 8. ACCÈS EN LECTURE À UNE SECTION DU RAPPORT                              */
/* ------------------------------------------------------------------------- */
const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index) {
    if (!ctx || index >= ctx->section_count) {
        return NULL;
    }
    return &ctx->sections[index];
}

/* ------------------------------------------------------------------------- */
/* 9. RÉINITIALISATION EN PROFONDEUR DU CONTEXTE (SANS DÉTRUIRE L'ENVELOPPE) */
/* ------------------------------------------------------------------------- */
void raptor_data_context_clear(RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }

    for (size_t s = 0; s < ctx->section_count; ++s) {
        RaptorSection* sec = &ctx->sections[s];

        if (sec->cards) {
            free(sec->cards);
            sec->cards = NULL;
        }

        if (sec->alerts) {
            free(sec->alerts);
            sec->alerts = NULL;
        }

        if (sec->tables) {
            for (size_t t = 0; t < sec->table_count; ++t) {
                RaptorTable* tbl = &sec->tables[t];

                if (tbl->col_names) {
                    for (size_t c = 0; c < tbl->col_count; ++c) {
                        if (tbl->col_names[c]) {
                            free(tbl->col_names[c]);
                        }
                    }
                    free(tbl->col_names);
                    tbl->col_names = NULL;
                }

                if (tbl->rows) {
                    for (size_t r = 0; r < tbl->row_count; ++r) {
                        RaptorTableRow* row = &tbl->rows[r];
                        if (row->cell_texts) {
                            for (size_t c = 0; c < tbl->col_count; ++c) {
                                if (row->cell_texts[c]) {
                                    free(row->cell_texts[c]);
                                }
                            }
                            free(row->cell_texts);
                        }
                        if (row->cell_sevs) {
                            free(row->cell_sevs);
                        }
                    }
                    free(tbl->rows);
                    tbl->rows = NULL;
                }
            }
            free(sec->tables);
            sec->tables = NULL;
        }
    }

    if (ctx->sections) {
        free(ctx->sections);
        ctx->sections = NULL;
    }

    ctx->section_count = 0;
    ctx->section_capacity = 0;
}

/* ------------------------------------------------------------------------- */
/* 10. DESTRUCTION COMPLÈTE ET LIBÉRATION FINALE DU CONTEXTE                 */
/* ------------------------------------------------------------------------- */
void raptor_data_context_destroy(RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }
    raptor_data_context_clear(ctx);
    free(ctx);
}
