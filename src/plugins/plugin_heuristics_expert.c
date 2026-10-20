/**
 * ============================================================================
 * NOM DU SCRIPT   : src/plugins/plugin_heuristics_expert.c
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Greffon d'analyse heuristique croisée et d'intégrité binaire.
 *                   Traque les techniques d'évasion avancées sans dépendance :
 *                   usurpation d'identité système (faux systemd/sshd/kworker),
 *                   exécution clandestine depuis des répertoires volatils
 *                   (/tmp, /dev/shm, /var/tmp) et processus s'exécutant sur
 *                   des binaires préalablement supprimés du système de fichiers.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Dimensionnement des tampons de chemins path_comm et path_exe à 320 octets
 *     pour neutraliser l'avertissement GCC -Wformat-truncation (pire cas NAME_MAX).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. heuristics_is_suspicious_path
 *   2. heuristics_is_system_masquerade
 *   3. heuristics_check_process
 *   4. plugin_heuristics_init
 *   5. plugin_heuristics_run_audit
 *   6. plugin_heuristics_cleanup
 *   7. plugin_heuristics_register_constructor
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

#if defined(__linux__) || defined(__unix__)
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

/* ------------------------------------------------------------------------- */
/* 1. DÉTECTION DE CHEMINS VOLATILS ET ÉPHÉMÈRES SUSPECTS                    */
/* ------------------------------------------------------------------------- */
static bool heuristics_is_suspicious_path(const char* path) {
    if (!path || path[0] == '\0') {
        return false;
    }

    if (strncmp(path, "/tmp/", 5) == 0 ||
        strncmp(path, "/var/tmp/", 9) == 0 ||
        strncmp(path, "/dev/shm/", 9) == 0 ||
        strncmp(path, "/run/user/", 10) == 0) {
        return true;
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* 2. DÉTECTION D'USURPATION D'IDENTITÉ SYSTÈME (MASQUERADING)               */
/* ------------------------------------------------------------------------- */
static bool heuristics_is_system_masquerade(const char* comm, const char* exe_target) {
    if (!comm || !exe_target || exe_target[0] == '\0') {
        return false;
    }

    const char* critical_system_names[] = {
        "systemd", "init", "sshd", "cron", "crond", "rsyslogd",
        "udevd", "dbus-daemon", "login", "agetty", NULL
    };

    for (int i = 0; critical_system_names[i] != NULL; ++i) {
        if (strcmp(comm, critical_system_names[i]) == 0) {
            if (strncmp(exe_target, "/usr/", 5) != 0 &&
                strncmp(exe_target, "/bin/", 5) != 0 &&
                strncmp(exe_target, "/sbin/", 6) != 0 &&
                strncmp(exe_target, "/lib/", 5) != 0) {
                return true;
            }
        }
    }

    if (strncmp(comm, "kworker/", 8) == 0 || strncmp(comm, "ksoftirqd/", 10) == 0) {
        if (exe_target[0] != '\0') {
            return true;
        }
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* 3. CONTRÔLE D'INTÉGRITÉ D'UN PROCESSUS UNIQUE                             */
/* ------------------------------------------------------------------------- */
static void heuristics_check_process(const char* pid_str,
                                     size_t* out_masquerades,
                                     size_t* out_deleted,
                                     size_t* out_suspicious_paths,
                                     RaptorTable* tbl) {
#if defined(__linux__) || defined(__unix__)
    char path_comm[320];
    snprintf(path_comm, sizeof(path_comm), "/proc/%s/comm", pid_str);

    FILE* fp = fopen(path_comm, "r");
    if (!fp) {
        return;
    }

    char comm[64] = {0};
    if (fgets(comm, sizeof(comm), fp)) {
        size_t len = strlen(comm);
        while (len > 0 && (comm[len - 1] == '\n' || comm[len - 1] == '\r')) {
            comm[len - 1] = '\0';
            len--;
        }
    }
    fclose(fp);

    char path_exe[320];
    snprintf(path_exe, sizeof(path_exe), "/proc/%s/exe", pid_str);

    char exe_target[512] = {0};
    ssize_t ret = readlink(path_exe, exe_target, sizeof(exe_target) - 1);
    if (ret > 0) {
        exe_target[ret] = '\0';
    }

    bool is_masq = heuristics_is_system_masquerade(comm, exe_target);
    bool is_deleted = (strstr(exe_target, "(deleted)") != NULL);
    bool is_sus_path = heuristics_is_suspicious_path(exe_target);

    if (is_masq) (*out_masquerades)++;
    if (is_deleted) (*out_deleted)++;
    if (is_sus_path) (*out_suspicious_paths)++;

    if ((is_masq || is_deleted || is_sus_path) && tbl) {
        char qualif[128] = {0};
        if (is_masq) {
            strncat(qualif, "[USURPATION] ", sizeof(qualif) - strlen(qualif) - 1);
        }
        if (is_deleted) {
            strncat(qualif, "[DÉLINKÉ/SUPPRIMÉ] ", sizeof(qualif) - strlen(qualif) - 1);
        }
        if (is_sus_path) {
            strncat(qualif, "[ZONE VOLATILE] ", sizeof(qualif) - strlen(qualif) - 1);
        }

        const char* row_cells[4];
        row_cells[0] = pid_str;
        row_cells[1] = comm[0] != '\0' ? comm : "inconnu";
        row_cells[2] = exe_target[0] != '\0' ? exe_target : "[aucun lien]";
        row_cells[3] = qualif;

        RaptorSeverity row_sevs[4] = {
            RAPTOR_SEV_WARNING,
            RAPTOR_SEV_CRITICAL,
            RAPTOR_SEV_CRITICAL,
            RAPTOR_SEV_CRITICAL
        };

        raptor_data_table_row_add(tbl, row_cells, row_sevs);
    }
#else
    (void)pid_str; (void)out_masquerades; (void)out_deleted;
    (void)out_suspicious_paths; (void)tbl;
#endif
}

/* ------------------------------------------------------------------------- */
/* 4. INITIALISATION DU GREFFON D'HEURISTIQUE                                */
/* ------------------------------------------------------------------------- */
static bool plugin_heuristics_init(void) {
    return true;
}

/* ------------------------------------------------------------------------- */
/* 5. EXÉCUTION DE L'AUDIT HEURISTIQUE APPROFONDI                            */
/* ------------------------------------------------------------------------- */
static bool plugin_heuristics_run_audit(RaptorReportContext* ctx) {
    if (!ctx) {
        return false;
    }

    RaptorSection* sec = raptor_data_section_add(
        ctx,
        "sec_heuristics",
        "MOTEUR HEURISTIQUE & DÉFENSE AVANCÉE"
    );

    if (!sec) {
        return false;
    }

    size_t count_masq = 0;
    size_t count_deleted = 0;
    size_t count_sus_paths = 0;

    const char* col_names[] = {
        "PID",
        "Nom Affiché",
        "Binaire Exécutable Résolu",
        "Classification Heuristique"
    };

    RaptorTable* tbl = raptor_data_table_create(
        sec,
        "ANOMALIES COMPORTEMENTALES & HEURISTIQUES",
        4,
        col_names
    );

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
                    heuristics_check_process(
                        entry->d_name,
                        &count_masq,
                        &count_deleted,
                        &count_sus_paths,
                        tbl
                    );
                }
            }
        }
        closedir(dir);
    }
#endif

    size_t total_threats = count_masq + count_deleted + count_sus_paths;

    char str_total[32];
    snprintf(str_total, sizeof(str_total), "%zu", total_threats);

    char str_masq[32];
    snprintf(str_masq, sizeof(str_masq), "%zu", count_masq);

    char str_del[32];
    snprintf(str_del, sizeof(str_del), "%zu", count_deleted);

    char str_sus[32];
    snprintf(str_sus, sizeof(str_sus), "%zu", count_sus_paths);

    RaptorSeverity sev_global = (total_threats > 0) ? RAPTOR_SEV_CRITICAL : RAPTOR_SEV_OK;

    raptor_data_card_add(sec, "HEURISTIQUES CRITIQUES", str_total, (float)total_threats, sev_global);
    raptor_data_card_add(sec, "USURPATIONS NOM", str_masq, 0.0f, (count_masq > 0) ? RAPTOR_SEV_CRITICAL : RAPTOR_SEV_OK);
    raptor_data_card_add(sec, "BINAIRES DÉLINKÉS", str_del, 0.0f, (count_deleted > 0) ? RAPTOR_SEV_WARNING : RAPTOR_SEV_OK);
    raptor_data_card_add(sec, "ZONES VOLATILES", str_sus, 0.0f, (count_sus_paths > 0) ? RAPTOR_SEV_CRITICAL : RAPTOR_SEV_OK);

    if (total_threats == 0) {
        raptor_data_alert_add(
            sec,
            RAPTOR_SEV_OK,
            "Contrôle d'intégrité nominal : aucune usurpation de binaire système ni exécution clandestine détectée."
        );
    } else {
        char alert_msg[256];
        snprintf(
            alert_msg,
            sizeof(alert_msg),
            "Alerte heuristique : %zu comportement(s) suspect(s) identifié(s) (usurpation, zone volatile ou binaire supprimé).",
            total_threats
        );
        raptor_data_alert_add(sec, RAPTOR_SEV_CRITICAL, alert_msg);
    }

    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. LIBÉRATION ET NETTOYAGE DU GREFFON                                     */
/* ------------------------------------------------------------------------- */
static void plugin_heuristics_cleanup(void) {
    /* Aucune allocation persistante globale */
}

/* ------------------------------------------------------------------------- */
/* STRUCTURE DU GREFFON RAPTOR                                               */
/* ------------------------------------------------------------------------- */
static const RaptorPlugin s_plugin_heuristics = {
    .id = "plugin_heuristics_expert",
    .name = "Moteur Heuristique Expert & Usurpation Binaire",
    .init = plugin_heuristics_init,
    .run_audit = plugin_heuristics_run_audit,
    .cleanup = plugin_heuristics_cleanup
};

/* ------------------------------------------------------------------------- */
/* 7. CONSTRUCTEUR D'AUTO-ENREGISTREMENT AU CHARGEMENT                       */
/* ------------------------------------------------------------------------- */
__attribute__((constructor))
static void plugin_heuristics_register_constructor(void) {
    raptor_registry_register(&s_plugin_heuristics);
}
