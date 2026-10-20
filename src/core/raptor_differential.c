/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/raptor_differential.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Implémentation du moteur d'analyse différentielle temporelle.
 *                   Compare l'état courant (T0) à l'instantané précédent (T-1),
 *                   calcule les dérivées de consommation mémoire (Mo/min),
 *                   identifie les fuites continues et génère le tableau de bord
 *                   d'évolution pour la restitution (CLI, HTML, GUI).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Élargissement de `proc_label` à 256 octets pour éviter toute troncature.
 *   - Remplacement de l'encodage hexadécimal par de l'UTF-8 direct.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. raptor_diff_init
 *   2. raptor_diff_compute
 *   3. raptor_diff_snapshot_save
 *   4. raptor_diff_cleanup
 * ============================================================================
 */

#include "raptor/raptor_differential.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char pid_str[32];
    char name[128];
    uint64_t rss_bytes;
} DiffProcessEntry;

typedef struct {
    time_t timestamp;
    DiffProcessEntry* entries;
    size_t count;
    size_t capacity;
} DiffSnapshot;

static DiffSnapshot s_prev_snapshot;
static bool s_has_previous_snapshot = false;

static uint64_t diff_parse_memory_to_bytes(const char* mem_str) {
    if (!mem_str || strlen(mem_str) == 0 || strcmp(mem_str, "-") == 0) {
        return 0;
    }

    double value = 0.0;
    char unit[16] = {0};

    if (sscanf(mem_str, "%lf %15s", &value, unit) < 1) {
        return 0;
    }

    if (strstr(unit, "Go") || strstr(unit, "GB") || strstr(unit, "go")) {
        return (uint64_t)(value * 1024.0 * 1024.0 * 1024.0);
    } else if (strstr(unit, "Mo") || strstr(unit, "MB") || strstr(unit, "mo")) {
        return (uint64_t)(value * 1024.0 * 1024.0);
    } else if (strstr(unit, "Ko") || strstr(unit, "KB") || strstr(unit, "ko")) {
        return (uint64_t)(value * 1024.0);
    }

    return (uint64_t)value;
}

static void diff_extract_entries_from_context(const RaptorReportContext* ctx, DiffSnapshot* out_snap) {
    if (!ctx || !out_snap) {
        return;
    }

    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];

        for (size_t t = 0; t < sec->table_count; ++t) {
            const RaptorTable* tbl = &sec->tables[t];
            int pid_col = -1;
            int name_col = -1;
            int rss_col = -1;

            for (size_t c = 0; c < tbl->col_count; ++c) {
                const char* col_name = tbl->col_names[c];
                if (!col_name) continue;

                if (strstr(col_name, "PID")) {
                    pid_col = (int)c;
                } else if (strstr(col_name, "Nom") || strstr(col_name, "Processus")) {
                    if (name_col == -1) name_col = (int)c;
                } else if (strstr(col_name, "RSS") || strstr(col_name, "RAM")) {
                    if (rss_col == -1) rss_col = (int)c;
                }
            }

            if (pid_col >= 0 && rss_col >= 0) {
                for (size_t r = 0; r < tbl->row_count; ++r) {
                    const RaptorTableRow* row = &tbl->rows[r];
                    const char* pid_str = row->cell_texts[pid_col];
                    const char* rss_str = row->cell_texts[rss_col];
                    const char* name_str = (name_col >= 0) ? row->cell_texts[name_col] : "inconnu";

                    if (!pid_str || pid_str[0] == '\0' || strcmp(pid_str, "-") == 0) {
                        continue;
                    }

                    if (out_snap->count >= out_snap->capacity) {
                        size_t new_cap = (out_snap->capacity == 0) ? 64 : out_snap->capacity * 2;
                        DiffProcessEntry* new_entries = (DiffProcessEntry*)realloc(
                            out_snap->entries, new_cap * sizeof(DiffProcessEntry)
                        );
                        if (!new_entries) {
                            return;
                        }
                        out_snap->entries = new_entries;
                        out_snap->capacity = new_cap;
                    }

                    DiffProcessEntry* entry = &out_snap->entries[out_snap->count];
                    strncpy(entry->pid_str, pid_str, sizeof(entry->pid_str) - 1);
                    entry->pid_str[sizeof(entry->pid_str) - 1] = '\0';

                    strncpy(entry->name, name_str ? name_str : "inconnu", sizeof(entry->name) - 1);
                    entry->name[sizeof(entry->name) - 1] = '\0';

                    entry->rss_bytes = diff_parse_memory_to_bytes(rss_str);
                    out_snap->count++;
                }
            }
        }
    }
}

/* ------------------------------------------------------------------------- */
/* 1. INITIALISATION DU SOUS-SYSTÈME DIFFÉRENTIEL                            */
/* ------------------------------------------------------------------------- */
void raptor_diff_init(void) {
    if (!s_prev_snapshot.entries) {
        s_prev_snapshot.entries = NULL;
        s_prev_snapshot.count = 0;
        s_prev_snapshot.capacity = 0;
        s_prev_snapshot.timestamp = 0;
        s_has_previous_snapshot = false;
    }
}

/* ------------------------------------------------------------------------- */
/* 2. CALCUL DIFFÉRENTIEL ET PEUPLEMENT DE LA SECTION DÉDIÉE                 */
/* ------------------------------------------------------------------------- */
bool raptor_diff_compute(const RaptorReportContext* current, RaptorSection* out_diff_sec) {
    if (!current || !out_diff_sec) {
        return false;
    }

    const char* col_headers[] = {
        "Processus (PID)",
        "Statut Évolution",
        "RAM Actuelle",
        "Taux Dérive",
        "Évaluation Risque"
    };

    RaptorTable* diff_table = raptor_data_table_create(
        out_diff_sec,
        "CAHIER DE BORD // ANALYSE DIFFÉRENTIELLE TEMPORELLE",
        5,
        col_headers
    );

    if (!diff_table) {
        return false;
    }

    if (!s_has_previous_snapshot || s_prev_snapshot.count == 0) {
        raptor_data_alert_add(
            out_diff_sec,
            RAPTOR_SEV_INFO,
            "Un seul audit a été enregistré. Effectuez un second audit pour afficher les évolutions."
        );

        const char* empty_row[] = {
            "-",
            "STABLE",
            "-",
            "0.0 Mo/min",
            "Aucune modification notable constatée entre les deux audits"
        };
        const RaptorSeverity empty_sevs[] = {
            RAPTOR_SEV_INFO,
            RAPTOR_SEV_OK,
            RAPTOR_SEV_INFO,
            RAPTOR_SEV_OK,
            RAPTOR_SEV_OK
        };

        raptor_data_table_row_add(diff_table, empty_row, empty_sevs);
        return true;
    }

    DiffSnapshot current_snap;
    memset(&current_snap, 0, sizeof(DiffSnapshot));
    current_snap.timestamp = time(NULL);
    diff_extract_entries_from_context(current, &current_snap);

    double elapsed_seconds = difftime(current_snap.timestamp, s_prev_snapshot.timestamp);
    if (elapsed_seconds < 1.0) {
        elapsed_seconds = 1.0;
    }
    double elapsed_minutes = elapsed_seconds / 60.0;

    size_t leaks_detected = 0;
    double max_drift_rate = 0.0;

    for (size_t i = 0; i < current_snap.count; ++i) {
        const DiffProcessEntry* cur_p = &current_snap.entries[i];
        const DiffProcessEntry* prev_p = NULL;

        for (size_t j = 0; j < s_prev_snapshot.count; ++j) {
            if (strcmp(current_snap.entries[i].pid_str, s_prev_snapshot.entries[j].pid_str) == 0) {
                prev_p = &s_prev_snapshot.entries[j];
                break;
            }
        }

        char proc_label[256];
        snprintf(proc_label, sizeof(proc_label), "%s (%s)", cur_p->name, cur_p->pid_str);

        char ram_str[32];
        snprintf(ram_str, sizeof(ram_str), "%llu Mo", (unsigned long long)(cur_p->rss_bytes / (1024 * 1024)));

        char rate_str[32];
        char eval_str[128];
        const char* status_str = "STABLE";
        RaptorSeverity sev = RAPTOR_SEV_OK;

        if (prev_p) {
            int64_t diff_bytes = (int64_t)cur_p->rss_bytes - (int64_t)prev_p->rss_bytes;
            double diff_mb = (double)diff_bytes / (1024.0 * 1024.0);
            double drift_rate_mb_min = diff_mb / elapsed_minutes;

            if (drift_rate_mb_min > max_drift_rate) {
                max_drift_rate = drift_rate_mb_min;
            }

            snprintf(rate_str, sizeof(rate_str), "%+.1f Mo/min", drift_rate_mb_min);

            if (drift_rate_mb_min >= 5.0) {
                status_str = "FUITE CRITIQUE";
                sev = RAPTOR_SEV_CRITICAL;
                snprintf(eval_str, sizeof(eval_str), "Accroissement continu anormal (+%.1f Mo/min)", drift_rate_mb_min);
                leaks_detected++;
            } else if (drift_rate_mb_min >= 1.0) {
                status_str = "DÉRIVE SUSPECTE";
                sev = RAPTOR_SEV_WARNING;
                snprintf(eval_str, sizeof(eval_str), "Légère augmentation observée");
            } else if (drift_rate_mb_min <= -1.0) {
                status_str = "RÉDUCTION";
                sev = RAPTOR_SEV_OK;
                snprintf(eval_str, sizeof(eval_str), "Mémoire libérée par le binaire");
            } else {
                status_str = "STABLE";
                sev = RAPTOR_SEV_OK;
                snprintf(eval_str, sizeof(eval_str), "Consommation nominale stable");
            }
        } else {
            snprintf(rate_str, sizeof(rate_str), "Nouveau");
            status_str = "APPARITION";
            sev = RAPTOR_SEV_INFO;
            snprintf(eval_str, sizeof(eval_str), "Processus initié récemment");
        }

        const char* row_texts[] = { proc_label, status_str, ram_str, rate_str, eval_str };
        const RaptorSeverity row_sevs[] = { RAPTOR_SEV_INFO, sev, RAPTOR_SEV_INFO, sev, sev };

        raptor_data_table_row_add(diff_table, row_texts, row_sevs);
    }

    if (leaks_detected > 0) {
        char alert_msg[256];
        snprintf(alert_msg, sizeof(alert_msg), "Alerte : %zu processus présentent une dérive mémoire continue.", leaks_detected);
        raptor_data_alert_add(out_diff_sec, RAPTOR_SEV_CRITICAL, alert_msg);
    } else {
        raptor_data_alert_add(out_diff_sec, RAPTOR_SEV_OK, "Aucune dérive anormale ou fuite mémoire détectée sur l'ensemble des processus.");
    }

    if (current_snap.entries) {
        free(current_snap.entries);
    }

    return true;
}

/* ------------------------------------------------------------------------- */
/* 3. ARCHIVAGE DE L'INSTANTANÉ COURANT                                      */
/* ------------------------------------------------------------------------- */
void raptor_diff_snapshot_save(const RaptorReportContext* current) {
    if (!current) {
        return;
    }

    if (s_prev_snapshot.entries) {
        free(s_prev_snapshot.entries);
        s_prev_snapshot.entries = NULL;
    }

    s_prev_snapshot.count = 0;
    s_prev_snapshot.capacity = 0;
    s_prev_snapshot.timestamp = time(NULL);

    diff_extract_entries_from_context(current, &s_prev_snapshot);
    s_has_previous_snapshot = (s_prev_snapshot.count > 0);
}

/* ------------------------------------------------------------------------- */
/* 4. NETTOYAGE DU CACHE DIFFÉRENTIEL                                        */
/* ------------------------------------------------------------------------- */
void raptor_diff_cleanup(void) {
    if (s_prev_snapshot.entries) {
        free(s_prev_snapshot.entries);
        s_prev_snapshot.entries = NULL;
    }
    s_prev_snapshot.count = 0;
    s_prev_snapshot.capacity = 0;
    s_prev_snapshot.timestamp = 0;
    s_has_previous_snapshot = false;
}
