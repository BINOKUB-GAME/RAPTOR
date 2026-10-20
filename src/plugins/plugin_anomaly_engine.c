/**
 * ============================================================================
 * NOM DU SCRIPT   : src/plugins/plugin_anomaly_engine.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Greffon d'inférence statistique et de scoring d'anomalies en C pur.
 *                   Analyse vectorielle multivariée des processus actifs du système
 *                   (mémoire physique RSS, volume virtuel VSZ, descripteurs de fichiers
 *                   et sockets ouverts, densité de threads) par calcul de déviation
 *                   standardisée (Z-Score / règle 3-Sigma) et arbre de décision compact.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Règle d'exemption contextuelle dans l'arbre de décision pour les concentrateurs
 *     IPC légitimes (dbus-daemon, systemd, pipewire) maintenant des descripteurs
 *     élevés pour le bureau mais une RAM faible (< 50 Mo).
 *   - Remplacement de la notation brute UTF-8 par "sigma" pour assurer une restitution
 *     uniforme et sans artefact entre le rapport HTML et l'interface Raylib.
 *   - Préservation des tampons 320 octets neutralisant les avertissements GCC.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. anomaly_parse_proc_status
 *   2. anomaly_count_open_fds
 *   3. anomaly_compute_zscore
 *   4. anomaly_classify_decision_tree
 *   5. plugin_anomaly_init
 *   6. plugin_anomaly_run_audit
 *   7. plugin_anomaly_cleanup
 *   8. plugin_anomaly_register_constructor
 * ============================================================================
 */

#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#if defined(__linux__) || defined(__unix__)
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

typedef struct {
    int pid;
    char name[64];
    float rss_mb;
    float vsz_mb;
    int thread_count;
    int fd_count;
    float z_score_rss;
    float z_score_fds;
    float combined_anomaly_score;
    const char* classification;
    RaptorSeverity severity;
} ProcessMetricSample;

/* ------------------------------------------------------------------------- */
/* 1. LECTURE DES MÉTRIQUES STATISTIQUES VIA /proc/[PID]/status              */
/* ------------------------------------------------------------------------- */
static bool anomaly_parse_proc_status(const char* pid_str, ProcessMetricSample* sample) {
#if defined(__linux__) || defined(__unix__)
    char path_status[320];
    snprintf(path_status, sizeof(path_status), "/proc/%s/status", pid_str);

    FILE* fp = fopen(path_status, "r");
    if (!fp) {
        return false;
    }

    sample->pid = (int)strtol(pid_str, NULL, 10);
    sample->rss_mb = 0.0f;
    sample->vsz_mb = 0.0f;
    sample->thread_count = 1;
    sample->fd_count = 0;
    sample->name[0] = '\0';

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Name:", 5) == 0) {
            char* val = line + 5;
            while (*val == ' ' || *val == '\t') val++;
            size_t l = strlen(val);
            while (l > 0 && (val[l - 1] == '\n' || val[l - 1] == '\r')) {
                val[l - 1] = '\0';
                l--;
            }
            strncpy(sample->name, val, sizeof(sample->name) - 1);
            sample->name[sizeof(sample->name) - 1] = '\0';
        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            long kb = 0;
            if (sscanf(line + 6, "%ld", &kb) == 1) {
                sample->rss_mb = (float)kb / 1024.0f;
            }
        } else if (strncmp(line, "VmSize:", 7) == 0) {
            long kb = 0;
            if (sscanf(line + 7, "%ld", &kb) == 1) {
                sample->vsz_mb = (float)kb / 1024.0f;
            }
        } else if (strncmp(line, "Threads:", 8) == 0) {
            int th = 1;
            if (sscanf(line + 8, "%d", &th) == 1) {
                sample->thread_count = th;
            }
        }
    }
    fclose(fp);

    return (sample->name[0] != '\0');
#else
    (void)pid_str; (void)sample;
    return false;
#endif
}

/* ------------------------------------------------------------------------- */
/* 2. DÉNOMBREMENT DES DESCRIPTEURS OUVERTS VIA /proc/[PID]/fd               */
/* ------------------------------------------------------------------------- */
static int anomaly_count_open_fds(const char* pid_str) {
#if defined(__linux__) || defined(__unix__)
    char path_fd[320];
    snprintf(path_fd, sizeof(path_fd), "/proc/%s/fd", pid_str);

    DIR* dir = opendir(path_fd);
    if (!dir) {
        return 0;
    }

    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            count++;
        }
    }
    closedir(dir);
    return count;
#else
    (void)pid_str;
    return 0;
#endif
}

/* ------------------------------------------------------------------------- */
/* 3. CALCUL DU Z-SCORE STATISTIQUE (DISTANCE À LA MOYENNE / ÉCART-TYPE)     */
/* ------------------------------------------------------------------------- */
static float anomaly_compute_zscore(float value, float mean, float stddev) {
    if (stddev <= 0.0001f) {
        return 0.0f;
    }
    return (value - mean) / stddev;
}

/* ------------------------------------------------------------------------- */
/* 4. ARBRE DE DÉCISION COMPACT AVEC CONTEXTUALISATION DES BUS SYSTÈME       */
/* ------------------------------------------------------------------------- */
static void anomaly_classify_decision_tree(ProcessMetricSample* sample) {
    /* 
     * Règle contextuelle IPC / Concentrateurs de sockets :
     * dbus-daemon, systemd ou pipewire maintiennent des centaines de descripteurs
     * (un par application connectée sur la session). Si leur empreinte RAM
     * reste contenue (< 50 Mo), cette déviation FD est fonctionnelle et légitime.
     */
    bool is_ipc_broker = (strcmp(sample->name, "dbus-daemon") == 0 ||
                          strcmp(sample->name, "systemd") == 0 ||
                          strcmp(sample->name, "pipewire") == 0);

    if (is_ipc_broker && sample->rss_mb < 50.0f) {
        sample->combined_anomaly_score = sample->z_score_rss;
        sample->classification = "NOMINAL (Concentrateur IPC / Bus Système)";
        sample->severity = RAPTOR_SEV_OK;
        return;
    }

    sample->combined_anomaly_score = (sample->z_score_rss * 0.55f) + (sample->z_score_fds * 0.45f);

    if (sample->combined_anomaly_score >= 3.2f || sample->z_score_rss >= 4.0f) {
        sample->classification = "OUTLIER CRITIQUE (Forte Aberration RAM/FD)";
        sample->severity = RAPTOR_SEV_CRITICAL;
    } else if (sample->combined_anomaly_score >= 2.0f || sample->z_score_rss >= 2.5f) {
        sample->classification = "DÉVIATION STATISTIQUE (Consommation Singulière)";
        sample->severity = RAPTOR_SEV_WARNING;
    } else {
        sample->classification = "NOMINAL (Distribution Standard)";
        sample->severity = RAPTOR_SEV_OK;
    }
}

/* ------------------------------------------------------------------------- */
/* 5. INITIALISATION DU GREFFON D'ANALYSE D'ANOMALIES                        */
/* ------------------------------------------------------------------------- */
static bool plugin_anomaly_init(void) {
    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. EXÉCUTION DE L'AUDIT PAR ANALYSE VECTORIELLE STATISTIQUE               */
/* ------------------------------------------------------------------------- */
static bool plugin_anomaly_run_audit(RaptorReportContext* ctx) {
    if (!ctx) {
        return false;
    }

    RaptorSection* sec = raptor_data_section_add(
        ctx,
        "sec_anomaly_engine",
        "MOTEUR STATISTIQUE & ARBRE D'ANOMALIES (Z-SCORE)"
    );

    if (!sec) {
        return false;
    }

    size_t sample_capacity = 256;
    size_t sample_count = 0;
    ProcessMetricSample* samples = (ProcessMetricSample*)calloc(sample_capacity, sizeof(ProcessMetricSample));
    if (!samples) {
        return false;
    }

#if defined(__linux__) || defined(__unix__)
    DIR* dir = opendir("/proc");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                bool is_pid = true;
                for (size_t i = 0; entry->d_name[i] != '\0'; ++i) {
                    if (!isdigit((unsigned char)entry->d_name[i])) {
                        is_pid = false;
                        break;
                    }
                }

                if (is_pid && entry->d_name[0] != '\0') {
                    if (sample_count >= sample_capacity) {
                        sample_capacity *= 2;
                        ProcessMetricSample* new_arr = (ProcessMetricSample*)realloc(
                            samples, sample_capacity * sizeof(ProcessMetricSample)
                        );
                        if (!new_arr) {
                            break;
                        }
                        samples = new_arr;
                    }

                    if (anomaly_parse_proc_status(entry->d_name, &samples[sample_count])) {
                        samples[sample_count].fd_count = anomaly_count_open_fds(entry->d_name);
                        sample_count++;
                    }
                }
            }
        }
        closedir(dir);
    }
#endif

    if (sample_count == 0) {
        free(samples);
        raptor_data_alert_add(sec, RAPTOR_SEV_INFO, "Aucun processus disponible pour l'analyse statistique.");
        return true;
    }

    float sum_rss = 0.0f;
    float sum_fds = 0.0f;
    for (size_t i = 0; i < sample_count; ++i) {
        sum_rss += samples[i].rss_mb;
        sum_fds += (float)samples[i].fd_count;
    }

    float mean_rss = sum_rss / (float)sample_count;
    float mean_fds = sum_fds / (float)sample_count;

    float var_rss = 0.0f;
    float var_fds = 0.0f;
    for (size_t i = 0; i < sample_count; ++i) {
        float d_rss = samples[i].rss_mb - mean_rss;
        float d_fds = (float)samples[i].fd_count - mean_fds;
        var_rss += (d_rss * d_rss);
        var_fds += (d_fds * d_fds);
    }

    float stddev_rss = sqrtf(var_rss / (float)sample_count);
    float stddev_fds = sqrtf(var_fds / (float)sample_count);

    size_t critical_outliers = 0;
    size_t warning_outliers = 0;
    float max_combined_score = 0.0f;

    for (size_t i = 0; i < sample_count; ++i) {
        samples[i].z_score_rss = anomaly_compute_zscore(samples[i].rss_mb, mean_rss, stddev_rss);
        samples[i].z_score_fds = anomaly_compute_zscore((float)samples[i].fd_count, mean_fds, stddev_fds);

        anomaly_classify_decision_tree(&samples[i]);

        if (samples[i].combined_anomaly_score > max_combined_score) {
            max_combined_score = samples[i].combined_anomaly_score;
        }

        if (samples[i].severity == RAPTOR_SEV_CRITICAL) {
            critical_outliers++;
        } else if (samples[i].severity == RAPTOR_SEV_WARNING) {
            warning_outliers++;
        }
    }

    char str_outliers[32];
    snprintf(str_outliers, sizeof(str_outliers), "%zu", critical_outliers);

    char str_warning[32];
    snprintf(str_warning, sizeof(str_warning), "%zu", warning_outliers);

    char str_max_z[32];
    snprintf(str_max_z, sizeof(str_max_z), "%.2f sigma", max_combined_score);

    char str_mean_rss[32];
    snprintf(str_mean_rss, sizeof(str_mean_rss), "%.1f Mo", mean_rss);

    RaptorSeverity sev_out = (critical_outliers > 0) ? RAPTOR_SEV_CRITICAL :
                            ((warning_outliers > 0) ? RAPTOR_SEV_WARNING : RAPTOR_SEV_OK);

    raptor_data_card_add(sec, "OUTLIERS STATISTIQUES", str_outliers, (float)critical_outliers, sev_out);
    raptor_data_card_add(sec, "DÉVIATIONS MODÉRÉES", str_warning, 0.0f, (warning_outliers > 0) ? RAPTOR_SEV_WARNING : RAPTOR_SEV_OK);
    raptor_data_card_add(sec, "DÉVIATION MAX (SIGMA)", str_max_z, 0.0f, sev_out);
    raptor_data_card_add(sec, "MOYENNE RSS GLOBALE", str_mean_rss, 0.0f, RAPTOR_SEV_INFO);

    const char* col_names[] = {
        "PID",
        "Nom du Processus",
        "RAM Réelle",
        "Descripteurs (FD)",
        "Score Z Combiné",
        "Verdict Modèle Déterministe"
    };

    RaptorTable* tbl = raptor_data_table_create(
        sec,
        "ANOMALIES VECTORIELLES CLASSIFIÉES",
        6,
        col_names
    );

    for (size_t i = 0; i < sample_count; ++i) {
        if (samples[i].severity == RAPTOR_SEV_CRITICAL || samples[i].severity == RAPTOR_SEV_WARNING) {
            char str_pid[16];
            snprintf(str_pid, sizeof(str_pid), "%d", samples[i].pid);

            char str_rss[32];
            snprintf(str_rss, sizeof(str_rss), "%.1f Mo", samples[i].rss_mb);

            char str_fds[16];
            snprintf(str_fds, sizeof(str_fds), "%d", samples[i].fd_count);

            char str_score[32];
            snprintf(str_score, sizeof(str_score), "%.2f sigma", samples[i].combined_anomaly_score);

            const char* row_cells[6] = {
                str_pid,
                samples[i].name,
                str_rss,
                str_fds,
                str_score,
                samples[i].classification
            };

            RaptorSeverity row_sevs[6] = {
                RAPTOR_SEV_INFO,
                samples[i].severity,
                RAPTOR_SEV_INFO,
                RAPTOR_SEV_INFO,
                samples[i].severity,
                samples[i].severity
            };

            raptor_data_table_row_add(tbl, row_cells, row_sevs);
        }
    }

    if (critical_outliers == 0 && warning_outliers == 0) {
        raptor_data_alert_add(
            sec,
            RAPTOR_SEV_OK,
            "Modèle statistique nominal : l'ensemble des processus s'inscrit dans la distribution normale du système."
        );
    } else {
        char msg[256];
        snprintf(
            msg,
            sizeof(msg),
            "Inférence statistique : %zu processus présente(nt) une déviation supérieure à 3 sigmas (comportement aberrant).",
            critical_outliers
        );
        raptor_data_alert_add(sec, (critical_outliers > 0) ? RAPTOR_SEV_CRITICAL : RAPTOR_SEV_WARNING, msg);
    }

    free(samples);
    return true;
}

/* ------------------------------------------------------------------------- */
/* 7. LIBÉRATION ET NETTOYAGE DU GREFFON                                     */
/* ------------------------------------------------------------------------- */
static void plugin_anomaly_cleanup(void) {
    /* Aucune allocation globale persistante */
}

/* ------------------------------------------------------------------------- */
/* STRUCTURE DU GREFFON RAPTOR                                               */
/* ------------------------------------------------------------------------- */
static const RaptorPlugin s_plugin_anomaly = {
    .id = "plugin_anomaly_engine",
    .name = "Moteur d'Inférence Statistique & Arbre Compact",
    .init = plugin_anomaly_init,
    .run_audit = plugin_anomaly_run_audit,
    .cleanup = plugin_anomaly_cleanup
};

/* ------------------------------------------------------------------------- */
/* 8. CONSTRUCTEUR D'AUTO-ENREGISTREMENT AU CHARGEMENT                       */
/* ------------------------------------------------------------------------- */
__attribute__((constructor))
static void plugin_anomaly_register_constructor(void) {
    raptor_registry_register(&s_plugin_anomaly);
}
