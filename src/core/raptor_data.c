/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/raptor_data.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Implémentation du conteneur de données universel et agnostique.
 *                   Gère le cycle de vie du contexte d'audit, l'instanciation des
 *                   sections, des cartes de métriques, des alertes de sévérité
 *                   et des tableaux structurés dynamiques consommés par les
 *                   moteurs de rendu (CLI, HTML, GUI).
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
 *   9. raptor_data_context_destroy
 * ============================================================================
 */

#include "raptor/raptor_data.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* 1. INITIALISATION DU CONTEXTE DE RAPPORT                                  */
/* ------------------------------------------------------------------------- */
RaptorReportContext* raptor_data_context_create(void) {
    RaptorReportContext* ctx = (RaptorReportContext*)calloc(1, sizeof(RaptorReportContext));
    if (!ctx) {
        return NULL;
    }

    ctx->section_capacity = 8;
    ctx->section_count = 0;
    ctx->sections = (RaptorSection*)calloc(ctx->section_capacity, sizeof(RaptorSection));
    if (!ctx->sections) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

/* ------------------------------------------------------------------------- */
/* 2. AJOUT D'UNE SECTION AUTONOME                                           */
/* ------------------------------------------------------------------------- */
RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title) {
    if (!ctx || !id || !title) {
        return NULL;
    }

    if (ctx->section_count >= ctx->section_capacity) {
        size_t new_cap = (ctx->section_capacity == 0) ? 8 : ctx->section_capacity * 2;
        RaptorSection* new_sections = (RaptorSection*)realloc(ctx->sections, new_cap * sizeof(RaptorSection));
        if (!new_sections) {
            return NULL;
        }
        ctx->sections = new_sections;
        ctx->section_capacity = new_cap;
    }

    RaptorSection* sec = &ctx->sections[ctx->section_count];
    memset(sec, 0, sizeof(RaptorSection));

    strncpy(sec->id, id, sizeof(sec->id) - 1);
    sec->id[sizeof(sec->id) - 1] = '\0';

    strncpy(sec->title, title, sizeof(sec->title) - 1);
    sec->title[sizeof(sec->title) - 1] = '\0';

    ctx->section_count++;
    return sec;
}

/* ------------------------------------------------------------------------- */
/* 3. AJOUT D'UNE CARTE DE MÉTRIQUE VISUELLE                                 */
/* ------------------------------------------------------------------------- */
bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev) {
    if (!sec || !label || !val) {
        return false;
    }

    if (sec->card_count >= sec->card_capacity) {
        size_t new_cap = (sec->card_capacity == 0) ? 4 : sec->card_capacity * 2;
        RaptorCard* new_cards = (RaptorCard*)realloc(sec->cards, new_cap * sizeof(RaptorCard));
        if (!new_cards) {
            return false;
        }
        sec->cards = new_cards;
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
/* 4. CRÉATION D'UN TABLEAU STRUCTURÉ DANS UNE SECTION                       */
/* ------------------------------------------------------------------------- */
RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names) {
    if (!sec || !title || col_count == 0 || !col_names) {
        return NULL;
    }

    if (sec->table_count >= sec->table_capacity) {
        size_t new_cap = (sec->table_capacity == 0) ? 4 : sec->table_capacity * 2;
        RaptorTable* new_tables = (RaptorTable*)realloc(sec->tables, new_cap * sizeof(RaptorTable));
        if (!new_tables) {
            return NULL;
        }
        sec->tables = new_tables;
        sec->table_capacity = new_cap;
    }

    RaptorTable* table = &sec->tables[sec->table_count];
    memset(table, 0, sizeof(RaptorTable));

    strncpy(table->title, title, sizeof(table->title) - 1);
    table->title[sizeof(table->title) - 1] = '\0';

    table->col_count = col_count;
    table->col_names = (char**)calloc(col_count, sizeof(char*));
    if (!table->col_names) {
        return NULL;
    }

    for (size_t i = 0; i < col_count; ++i) {
        if (col_names[i]) {
            size_t len = strlen(col_names[i]);
            table->col_names[i] = (char*)malloc(len + 1);
            if (table->col_names[i]) {
                memcpy(table->col_names[i], col_names[i], len + 1);
            }
        } else {
            table->col_names[i] = (char*)calloc(1, 1);
        }
    }

    table->rows = NULL;
    table->row_count = 0;
    table->row_capacity = 0;

    sec->table_count++;
    return table;
}

/* ------------------------------------------------------------------------- */
/* 5. INSERTION D'UNE LIGNE DANS UN TABLEAU                                  */
/* ------------------------------------------------------------------------- */
bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs) {
    if (!table || !cell_texts || !cell_sevs) {
        return false;
    }

    if (table->row_count >= table->row_capacity) {
        size_t new_cap = (table->row_capacity == 0) ? 16 : table->row_capacity * 2;
        RaptorTableRow* new_rows = (RaptorTableRow*)realloc(table->rows, new_cap * sizeof(RaptorTableRow));
        if (!new_rows) {
            return false;
        }
        table->rows = new_rows;
        table->row_capacity = new_cap;
    }

    RaptorTableRow* row = &table->rows[table->row_count];
    row->cell_texts = (char**)calloc(table->col_count, sizeof(char*));
    row->cell_sevs = (RaptorSeverity*)calloc(table->col_count, sizeof(RaptorSeverity));

    if (!row->cell_texts || !row->cell_sevs) {
        if (row->cell_texts) {
            free(row->cell_texts);
            row->cell_texts = NULL;
        }
        if (row->cell_sevs) {
            free(row->cell_sevs);
            row->cell_sevs = NULL;
        }
        return false;
    }

    for (size_t i = 0; i < table->col_count; ++i) {
        row->cell_sevs[i] = cell_sevs[i];
        if (cell_texts[i]) {
            size_t len = strlen(cell_texts[i]);
            row->cell_texts[i] = (char*)malloc(len + 1);
            if (row->cell_texts[i]) {
                memcpy(row->cell_texts[i], cell_texts[i], len + 1);
            }
        } else {
            row->cell_texts[i] = (char*)calloc(1, 1);
        }
    }

    table->row_count++;
    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. ENREGISTREMENT D'UNE ALERTE DANS UNE SECTION                           */
/* ------------------------------------------------------------------------- */
bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message) {
    if (!sec || !message) {
        return false;
    }

    if (sec->alert_count >= sec->alert_capacity) {
        size_t new_cap = (sec->alert_capacity == 0) ? 4 : sec->alert_capacity * 2;
        RaptorAlert* new_alerts = (RaptorAlert*)realloc(sec->alerts, new_cap * sizeof(RaptorAlert));
        if (!new_alerts) {
            return false;
        }
        sec->alerts = new_alerts;
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
/* 7. COMPTAGE DES SECTIONS ENREGISTRÉES                                     */
/* ------------------------------------------------------------------------- */
size_t raptor_data_get_section_count(const RaptorReportContext* ctx) {
    return ctx ? ctx->section_count : 0;
}

/* ------------------------------------------------------------------------- */
/* 8. OBTENTION SÉCURISÉE D'UNE SECTION PAR INDEX                            */
/* ------------------------------------------------------------------------- */
const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index) {
    if (!ctx || index >= ctx->section_count) {
        return NULL;
    }
    return &ctx->sections[index];
}

/* ------------------------------------------------------------------------- */
/* 9. DESTRUCTION RÉCURSIVE DE LA MÉMOIRE DU CONTEXTE                        */
/* ------------------------------------------------------------------------- */
void raptor_data_context_destroy(RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->sections) {
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
                                row->cell_texts = NULL;
                            }
                            if (row->cell_sevs) {
                                free(row->cell_sevs);
                                row->cell_sevs = NULL;
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

        free(ctx->sections);
        ctx->sections = NULL;
    }

    free(ctx);
}
