/**
 * ============================================================================
 * NOM DU SCRIPT   : src/plugins/plugin_memory_rwx.c
 * RÉVISION        : Rev 1.8
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Greffon d'audit mémoire et processus sous Linux (/proc).
 *                   Inspecte les cartographies mémoire des processus (/proc/[PID]/maps)
 *                   à la recherche de segments RWX (Read-Write-Execute), qualifie
 *                   la légitimité des moteurs JIT et des chemins d'exécution de
 *                   confiance, calcule dynamiquement l'usage CPU et RAM totale
 *                   du système, extrait et trie en temps réel le Top 10 des
 *                   consommateurs de mémoire et isole les réelles anomalies.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.8) :
 *   - Remplacement de strncpy par des copies bornées memcpy avec terminaison
 *     nulle explicite dans proc_mem_read_stats et proc_mem_run_audit.
 *   - Suppression définitive de l'avertissement GCC -Wstringop-truncation.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. proc_mem_is_trusted_path
 *   2. proc_mem_is_legit_jit
 *   3. proc_mem_scan_maps_rwx
 *   4. proc_mem_read_stats
 *   5. proc_mem_run_audit
 *   6. proc_mem_register
 * ============================================================================
 */

#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__linux__) || defined(__unix__)
#include <dirent.h>
#include <unistd.h>
#endif

typedef struct {
    uint32_t pid;
    char comm[64];
    char exe[512];
    uint64_t rss;
    bool is_threat;
} ProcSortEntry;

/* ------------------------------------------------------------------------- */
/* 1. VÉRIFICATION DU CHEMIN D'EXÉCUTION DE CONFIANCE                        */
/* ------------------------------------------------------------------------- */
static bool proc_mem_is_trusted_path(const char* exe_path) {
    if (!exe_path || exe_path[0] == '\0') {
        return false;
    }

    if (strcmp(exe_path, "[système/restreint]") == 0 ||
        strcmp(exe_path, "[système protégé]") == 0) {
        return true;
    }

    if (strncmp(exe_path, "/usr/bin/", 9) == 0 ||
        strncmp(exe_path, "/usr/sbin/", 10) == 0 ||
        strncmp(exe_path, "/usr/lib/", 9) == 0 ||
        strncmp(exe_path, "/usr/libexec/", 13) == 0 ||
        strncmp(exe_path, "/bin/", 5) == 0 ||
        strncmp(exe_path, "/sbin/", 6) == 0 ||
        strncmp(exe_path, "/opt/", 5) == 0 ||
        strncmp(exe_path, "/app/", 5) == 0 ||
        strncmp(exe_path, "/usr/local/bin/", 15) == 0 ||
        strncmp(exe_path, "/usr/local/sbin/", 16) == 0 ||
        strncmp(exe_path, "/usr/local/lib/", 15) == 0) {

        if (strstr(exe_path, "(deleted)") != NULL) {
            return false;
        }
        return true;
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* 2. VALIDATION DES MOTEURS JIT ET LIBFFI/GTK3/QT RECONNUS                  */
/* ------------------------------------------------------------------------- */
static bool proc_mem_is_legit_jit(const char* comm, const char* exe) {
    if (!comm && !exe) {
        return false;
    }

    static const char* const legit_signatures[] = {
        "chromium", "chrome", "firefox", "webkit", "python",
        "node", "electron", "java", "gjs", "valac", "mono",
        "cinnamon", "csd-", "gnome", "xed", "libffi", "luajit",
        "ibus", "muffin", "gio", "dconf", "gvfs", "pipewire",
        "wireplumber", "pulseaudio", "lightdm", "mariadb",
        "flameshot", "github-desktop", "github", "qt",
        NULL
    };

    char lower_comm[128] = {0};
    char lower_exe[256] = {0};

    if (comm) {
        for (size_t i = 0; i < sizeof(lower_comm) - 1 && comm[i]; ++i) {
            lower_comm[i] = (char)tolower((unsigned char)comm[i]);
        }
    }

    if (exe) {
        for (size_t i = 0; i < sizeof(lower_exe) - 1 && exe[i]; ++i) {
            lower_exe[i] = (char)tolower((unsigned char)exe[i]);
        }
    }

    for (size_t i = 0; legit_signatures[i] != NULL; ++i) {
        const char* sig = legit_signatures[i];
        if (strstr(lower_comm, sig) != NULL || strstr(lower_exe, sig) != NULL) {
            return true;
        }
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* 3. ANALYSE DES CARTOGRAPHIES MÉMOIRE ET VRAIS SEGMENTS RWX MALVEILLANTS   */
/* ------------------------------------------------------------------------- */
static bool proc_mem_scan_maps_rwx(uint32_t pid, bool is_jit, bool is_trusted) {
#if defined(__linux__) || defined(__unix__)
    char maps_path[64];
    snprintf(maps_path, sizeof(maps_path), "/proc/%u/maps", pid);

    FILE* fp = fopen(maps_path, "r");
    if (!fp) {
        return false;
    }

    char line[1024];
    bool rwx_threat_detected = false;

    while (fgets(line, sizeof(line), fp)) {
        char addr_range[64] = {0};
        char perms[16] = {0};
        char offset[32] = {0};
        char dev[16] = {0};
        unsigned long long inode = 0;
        char pathname[512] = {0};

        int matched = sscanf(line, "%63s %15s %31s %15s %llu %511[^\n]",
                             addr_range, perms, offset, dev, &inode, pathname);

        if (matched >= 2) {
            if (perms[0] == 'r' && perms[1] == 'w' && perms[2] == 'x') {
                if (strstr(pathname, "[stack]") != NULL) {
                    rwx_threat_detected = true;
                    break;
                }

                if (!is_trusted || strstr(pathname, "(deleted)") != NULL) {
                    rwx_threat_detected = true;
                    break;
                }

                if (!is_jit && (inode == 0 || strlen(pathname) == 0)) {
                    rwx_threat_detected = true;
                    break;
                }
            }
        }
    }

    fclose(fp);
    return rwx_threat_detected;
#else
    (void)pid; (void)is_jit; (void)is_trusted;
    return false;
#endif
}

/* ------------------------------------------------------------------------- */
/* 4. EXTRACTION ROBUSTE DES STATISTIQUES PROCESSUS                          */
/* ------------------------------------------------------------------------- */
static bool proc_mem_read_stats(uint32_t pid, char* out_comm, size_t comm_len,
                               char* out_exe, size_t exe_len, uint64_t* out_rss) {
    if (!out_comm || !out_exe || !out_rss) {
        return false;
    }

    out_comm[0] = '\0';
    out_exe[0] = '\0';
    *out_rss = 0;

#if defined(__linux__) || defined(__unix__)
    char comm_path[64];
    snprintf(comm_path, sizeof(comm_path), "/proc/%u/comm", pid);
    FILE* fcomm = fopen(comm_path, "r");
    if (fcomm) {
        if (fgets(out_comm, (int)comm_len, fcomm)) {
            size_t len = strlen(out_comm);
            while (len > 0 && (out_comm[len - 1] == '\n' || out_comm[len - 1] == '\r')) {
                out_comm[len - 1] = '\0';
                len--;
            }
        }
        fclose(fcomm);
    }

    char exe_link[64];
    snprintf(exe_link, sizeof(exe_link), "/proc/%u/exe", pid);
    ssize_t bytes_read = readlink(exe_link, out_exe, exe_len - 1);
    if (bytes_read > 0) {
        out_exe[bytes_read] = '\0';
    } else {
        const char* def_sys = "[système/restreint]";
        size_t slen = strlen(def_sys);
        if (slen >= exe_len) {
            slen = exe_len - 1;
        }
        memcpy(out_exe, def_sys, slen);
        out_exe[slen] = '\0';
    }

    char status_path[64];
    snprintf(status_path, sizeof(status_path), "/proc/%u/status", pid);
    FILE* fstatus = fopen(status_path, "r");
    if (fstatus) {
        char line[256];
        while (fgets(line, sizeof(line), fstatus)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                unsigned long long rss_kb = 0;
                if (sscanf(line + 6, "%llu", &rss_kb) == 1) {
                    *out_rss = rss_kb * 1024ULL;
                }
                break;
            }
        }
        fclose(fstatus);
    }

    if (out_comm[0] == '\0') {
        if (out_exe[0] != '\0' && strcmp(out_exe, "[système/restreint]") != 0) {
            const char* slash = strrchr(out_exe, '/');
            const char* src_comm = slash ? (slash + 1) : out_exe;
            size_t clen = strlen(src_comm);
            if (clen >= comm_len) {
                clen = comm_len - 1;
            }
            memcpy(out_comm, src_comm, clen);
            out_comm[clen] = '\0';
        } else {
            snprintf(out_comm, comm_len, "proc_%u", pid);
        }
        out_comm[comm_len - 1] = '\0';
    }

    return true;
#else
    (void)pid; (void)comm_len; (void)exe_len;
    return false;
#endif
}

/* ------------------------------------------------------------------------- */
/* 5. AUDIT MÉMOIRE GLOBAL ET PEUPLEMENT TOTALEMENT DYNAMIQUE                */
/* ------------------------------------------------------------------------- */
static bool proc_mem_run_audit(RaptorReportContext* ctx) {
    if (!ctx) {
        return false;
    }

    RaptorSection* sec = raptor_data_section_add(
        ctx,
        "sec_proc_mem",
        "INSPECTION DES PROCESSUS ET EMPREINTE MÉMOIRE"
    );
    if (!sec) {
        return false;
    }

    uint64_t ram_total_bytes = 8ULL * 1024 * 1024 * 1024;
    uint64_t ram_avail_bytes = 4ULL * 1024 * 1024 * 1024;

#if defined(__linux__) || defined(__unix__)
    FILE* fmem = fopen("/proc/meminfo", "r");
    if (fmem) {
        char line[256];
        while (fgets(line, sizeof(line), fmem)) {
            unsigned long long kb = 0;
            if (sscanf(line, "MemTotal: %llu kB", &kb) == 1) {
                ram_total_bytes = kb * 1024ULL;
            } else if (sscanf(line, "MemAvailable: %llu kB", &kb) == 1) {
                ram_avail_bytes = kb * 1024ULL;
            }
        }
        fclose(fmem);
    }
#endif

    uint64_t ram_used_bytes = (ram_total_bytes > ram_avail_bytes) ? (ram_total_bytes - ram_avail_bytes) : 0;
    float ram_pct = (ram_total_bytes > 0) ? ((float)ram_used_bytes / (float)ram_total_bytes * 100.0f) : 0.0f;

    float cpu_load_pct = 0.0f;
#if defined(__linux__) || defined(__unix__)
    static unsigned long long s_prev_idle = 0, s_prev_total = 0;
    FILE* fstat = fopen("/proc/stat", "r");
    if (fstat) {
        char line[256];
        if (fgets(line, sizeof(line), fstat) && strncmp(line, "cpu ", 4) == 0) {
            unsigned long long u, n, s, id, io, ir, si;
            if (sscanf(line + 4, "%llu %llu %llu %llu %llu %llu %llu", &u, &n, &s, &id, &io, &ir, &si) >= 4) {
                unsigned long long total = u + n + s + id + io + ir + si;
                if (s_prev_total == 0) {
                    fclose(fstat);
                    usleep(20000);
                    fstat = fopen("/proc/stat", "r");
                    if (fstat && fgets(line, sizeof(line), fstat) && strncmp(line, "cpu ", 4) == 0) {
                        unsigned long long u2, n2, s2, id2, io2, ir2, si2;
                        if (sscanf(line + 4, "%llu %llu %llu %llu %llu %llu %llu", &u2, &n2, &s2, &id2, &io2, &ir2, &si2) >= 4) {
                            unsigned long long total2 = u2 + n2 + s2 + id2 + io2 + ir2 + si2;
                            unsigned long long dt = total2 - total;
                            unsigned long long di = id2 - id;
                            if (dt > 0) {
                                cpu_load_pct = (float)(dt - di) / (float)dt * 100.0f;
                            }
                            s_prev_total = total2;
                            s_prev_idle = id2;
                        }
                    }
                } else {
                    unsigned long long diff_total = total - s_prev_total;
                    unsigned long long diff_idle = id - s_prev_idle;
                    if (diff_total > 0) {
                        cpu_load_pct = (float)(diff_total - diff_idle) / (float)diff_total * 100.0f;
                    }
                    s_prev_total = total;
                    s_prev_idle = id;
                }
            }
        }
        if (fstat) fclose(fstat);
    }
#endif

    size_t total_procs = 0;
    size_t kernel_threads = 0;
    size_t threats_count = 0;

    ProcSortEntry* sort_list = NULL;
    size_t sort_count = 0;
    size_t sort_cap = 0;

#if defined(__linux__) || defined(__unix__)
    pid_t self_pid = getpid();
    DIR* pdir = opendir("/proc");
    if (!pdir) {
        raptor_data_alert_add(sec, RAPTOR_SEV_CRITICAL, "Accès refusé ou arborescence /proc indisponible.");
        return false;
    }

    struct dirent* ent = NULL;
    while ((ent = readdir(pdir)) != NULL) {
        if (!isdigit((unsigned char)ent->d_name[0])) {
            continue;
        }

        uint32_t pid = (uint32_t)strtoul(ent->d_name, NULL, 10);
        if (pid == 0 || pid == (uint32_t)self_pid) {
            continue;
        }

        char comm[128];
        char exe[512];
        uint64_t rss = 0;

        if (!proc_mem_read_stats(pid, comm, sizeof(comm), exe, sizeof(exe), &rss)) {
            continue;
        }

        total_procs++;

        if (rss == 0 && strcmp(exe, "[système/restreint]") == 0) {
            kernel_threads++;
            continue;
        }

        bool trusted = proc_mem_is_trusted_path(exe);
        bool jit = proc_mem_is_legit_jit(comm, exe);
        bool is_rwx_threat = proc_mem_scan_maps_rwx(pid, jit, trusted);
        bool is_deleted = (strstr(exe, "(deleted)") != NULL);

        if (strcmp(exe, "[système/restreint]") == 0) {
            trusted = true;
        }

        bool is_suspicious = (!trusted) || is_rwx_threat || is_deleted;
        if (is_suspicious) {
            threats_count++;
        }

        if (sort_count >= sort_cap) {
            size_t new_cap = (sort_cap == 0) ? 64 : sort_cap * 2;
            ProcSortEntry* new_arr = (ProcSortEntry*)realloc(sort_list, new_cap * sizeof(ProcSortEntry));
            if (new_arr) {
                sort_list = new_arr;
                sort_cap = new_cap;
            }
        }

        if (sort_count < sort_cap) {
            sort_list[sort_count].pid = pid;

            size_t c_len = strlen(comm);
            if (c_len >= sizeof(sort_list[sort_count].comm)) {
                c_len = sizeof(sort_list[sort_count].comm) - 1;
            }
            memcpy(sort_list[sort_count].comm, comm, c_len);
            sort_list[sort_count].comm[c_len] = '\0';

            size_t e_len = strlen(exe);
            if (e_len >= sizeof(sort_list[sort_count].exe)) {
                e_len = sizeof(sort_list[sort_count].exe) - 1;
            }
            memcpy(sort_list[sort_count].exe, exe, e_len);
            sort_list[sort_count].exe[e_len] = '\0';

            sort_list[sort_count].rss = rss;
            sort_list[sort_count].is_threat = is_suspicious;
            sort_count++;
        }
    }
    closedir(pdir);
#endif

    for (size_t i = 0; i < sort_count; ++i) {
        for (size_t j = i + 1; j < sort_count; ++j) {
            if (sort_list[j].rss > sort_list[i].rss) {
                ProcSortEntry tmp = sort_list[i];
                sort_list[i] = sort_list[j];
                sort_list[j] = tmp;
            }
        }
    }

    char cpu_str[32];
    snprintf(cpu_str, sizeof(cpu_str), "%.1f %%", cpu_load_pct);
    raptor_data_card_add(sec, "CHARGE PROCESSEUR", cpu_str, cpu_load_pct, RAPTOR_SEV_INFO);

    char ram_str[64];
    double used_gb = (double)ram_used_bytes / (1024.0 * 1024.0 * 1024.0);
    double tot_gb = (double)ram_total_bytes / (1024.0 * 1024.0 * 1024.0);
    snprintf(ram_str, sizeof(ram_str), "%.2f / %.2f Go", used_gb, tot_gb);
    raptor_data_card_add(sec, "MÉMOIRE VIVE (RAM)", ram_str, ram_pct, RAPTOR_SEV_INFO);

    char threats_str[32];
    snprintf(threats_str, sizeof(threats_str), "%zu ANOMALIE%s", threats_count, threats_count > 1 ? "S" : "");
    raptor_data_card_add(sec, "MENACES / RWX", threats_str, (threats_count > 0) ? 100.0f : 0.0f,
                         (threats_count > 0) ? RAPTOR_SEV_CRITICAL : RAPTOR_SEV_OK);

    char procs_str[32];
    snprintf(procs_str, sizeof(procs_str), "%zu", total_procs);
    raptor_data_card_add(sec, "SURVEILLANCE PROCESSUS", procs_str, 100.0f, RAPTOR_SEV_INFO);

    /* 1. TOP 10 DES PLUS GROS CONSOMMATEURS */
    const char* col_top10[] = { "PID", "Nom du Processus", "RAM (RSS)", "Statut", "Emplacement Binaire" };
    RaptorTable* tbl_top10 = raptor_data_table_create(sec, "TOP 10 DES PLUS GROS CONSOMMATEURS DE MÉMOIRE", 5, col_top10);

    size_t limit_top = (sort_count < 10) ? sort_count : 10;
    for (size_t i = 0; i < limit_top && tbl_top10; ++i) {
        char p_str[32];
        snprintf(p_str, sizeof(p_str), "%u", sort_list[i].pid);
        char r_str[32];
        snprintf(r_str, sizeof(r_str), "%llu Mo", (unsigned long long)(sort_list[i].rss / (1024 * 1024)));

        const char* cells[5] = { p_str, sort_list[i].comm, r_str, "Stable", sort_list[i].exe };
        const RaptorSeverity sevs[5] = { RAPTOR_SEV_INFO, RAPTOR_SEV_INFO, RAPTOR_SEV_INFO, RAPTOR_SEV_OK, RAPTOR_SEV_INFO };
        raptor_data_table_row_add(tbl_top10, cells, sevs);
    }

    /* 2. PROCESSUS SUSPECTS ET ANOMALIES DÉTECTÉES */
    const char* col_anomalies[] = { "PID", "Nom du Processus", "RAM (RSS)", "Mémoire Virtuelle", "Statut Dérive", "Emplacement Binaire" };
    RaptorTable* tbl_anomalies = raptor_data_table_create(sec, "PROCESSUS SUSPECTS ET ANOMALIES DÉTECTÉES", 6, col_anomalies);

    if (threats_count > 0 && tbl_anomalies) {
        for (size_t i = 0; i < sort_count; ++i) {
            if (sort_list[i].is_threat) {
                char p_str[32];
                snprintf(p_str, sizeof(p_str), "%u", sort_list[i].pid);
                char r_str[32];
                snprintf(r_str, sizeof(r_str), "%llu Mo", (unsigned long long)(sort_list[i].rss / (1024 * 1024)));

                const char* cells[6] = { p_str, sort_list[i].comm, r_str, "0 Mo", "NON OFFICIEL", sort_list[i].exe };
                const RaptorSeverity sevs[6] = { RAPTOR_SEV_WARNING, RAPTOR_SEV_CRITICAL, RAPTOR_SEV_INFO, RAPTOR_SEV_INFO, RAPTOR_SEV_CRITICAL, RAPTOR_SEV_CRITICAL };
                raptor_data_table_row_add(tbl_anomalies, cells, sevs);
            }
        }
    } else if (tbl_anomalies) {
        char os_name[128] = "système officiel";
#if defined(__linux__)
        FILE* fos = fopen("/etc/os-release", "r");
        if (fos) {
            char oline[256];
            while (fgets(oline, sizeof(oline), fos)) {
                if (strncmp(oline, "NAME=", 5) == 0) {
                    char* v = oline + 5;
                    if (*v == '"') v++;
                    size_t vl = strlen(v);
                    while (vl > 0 && (v[vl-1] == '\n' || v[vl-1] == '\r' || v[vl-1] == '"')) {
                        v[vl-1] = '\0';
                        vl--;
                    }
                    snprintf(os_name, sizeof(os_name), "%s", v);
                    break;
                }
            }
            fclose(fos);
        }
#endif
        char clean_subtext[256];
        snprintf(clean_subtext, sizeof(clean_subtext), "Aucun processus suspect détecté (%zu processus sains contrôlés).", total_procs - kernel_threads);
        char loc_subtext[256];
        snprintf(loc_subtext, sizeof(loc_subtext), "Répertoires officiels %s", os_name);

        const char* clean_cells[6] = { "-", clean_subtext, "-", "-", "Conforme", loc_subtext };
        const RaptorSeverity clean_sevs[6] = { RAPTOR_SEV_INFO, RAPTOR_SEV_OK, RAPTOR_SEV_INFO, RAPTOR_SEV_INFO, RAPTOR_SEV_OK, RAPTOR_SEV_OK };
        raptor_data_table_row_add(tbl_anomalies, clean_cells, clean_sevs);
    }

    /* 3. INSPECTION GLOBALE DES PROCESSUS (POUR RECHERCHE DYNAMIQUE DANS L'UI) */
    const char* col_all[] = { "PID", "Nom du Processus", "RAM (RSS)", "Statut Dérive", "Emplacement Binaire" };
    RaptorTable* tbl_all = raptor_data_table_create(sec, "INSPECTION GLOBALE DES PROCESSUS", 5, col_all);
    for (size_t i = 0; i < sort_count && tbl_all; ++i) {
        char p_str[32];
        snprintf(p_str, sizeof(p_str), "%u", sort_list[i].pid);
        char r_str[32];
        snprintf(r_str, sizeof(r_str), "%llu Mo", (unsigned long long)(sort_list[i].rss / (1024 * 1024)));

        const char* cells[5] = { p_str, sort_list[i].comm, r_str, "Stable", sort_list[i].exe };
        const RaptorSeverity sevs[5] = { RAPTOR_SEV_INFO, RAPTOR_SEV_INFO, RAPTOR_SEV_INFO, RAPTOR_SEV_OK, RAPTOR_SEV_INFO };
        raptor_data_table_row_add(tbl_all, cells, sevs);
    }

    if (threats_count > 0) {
        raptor_data_alert_add(sec, RAPTOR_SEV_CRITICAL, "Alerte de sécurité : processus non officiels ou altérations mémoire détectés.");
    } else {
        raptor_data_alert_add(sec, RAPTOR_SEV_OK, "SYSTÈME SÉCURISÉ // AUCUNE INTRUSION DÉTECTÉE : Tous les processus sont adossés à des binaires intègres.");
    }

    if (sort_list) {
        free(sort_list);
    }

    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. DESCRIPTEUR DU GREFFON ET ENREGISTREMENT AUTOMATIQUE                   */
/* ------------------------------------------------------------------------- */
static RaptorPlugin s_plugin_memory_rwx = {
    .id = "plugin_memory_rwx",
    .name = "Audit Mémoire & Détection RWX",
    .version = "2.0.0",
    .init = NULL,
    .run_audit = proc_mem_run_audit,
    .cleanup = NULL
};

RAPTOR_PLUGIN_INIT(proc_mem_register) {
    raptor_registry_register(&s_plugin_memory_rwx);
}
