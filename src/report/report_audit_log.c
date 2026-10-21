/**
 * ============================================================================
 * NOM DU SCRIPT   : src/report/report_audit_log.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Moteur de journalisation forensique sur disque (AUDIT.TXT).
 *                   Extrait dynamiquement l'heure de démarrage via /proc/[PID]/stat
 *                   et /proc/stat (btime), résout les répertoires d'exécution
 *                   (/proc/[PID]/cwd), calcule l'heure exacte d'interception
 *                   et consigne un rapport d'audit numéroté en mode incrémental.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Implémentation du suivi dynamique s_detected_pids évitant la saturation
 *     disque lors du rafraîchissement continu toutes les 3 secondes.
 *   - Implémentation de raptor_audit_log_notify_detection() : écriture obligatoire
 *     dans AUDIT.TXT dès la première détection d'une menace active.
 *   - Implémentation de raptor_audit_log_forget_pid() : purge mémoire post-kill.
 *   - Copie mémoire sécurisée via memcpy neutralisant les avertissements GCC.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. audit_log_get_system_btime
 *   2. audit_log_compute_start_time
 *   3. audit_log_get_next_sequence_number
 *   4. raptor_audit_log_capture_target
 *   5. raptor_audit_log_commit
 *   6. raptor_audit_log_notify_detection
 *   7. raptor_audit_log_forget_pid
 * ============================================================================
 */

#include "raptor/raptor_audit_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#endif

#define MAX_TRACKED_DETECTIONS 256
static int s_detected_pids[MAX_TRACKED_DETECTIONS] = {0};
static size_t s_detected_count = 0;

/* ------------------------------------------------------------------------- */
/* 1. EXTRACTION DU TEMPS DE DÉMARRAGE DU NOYAU (btime DANS /proc/stat)      */
/* ------------------------------------------------------------------------- */
static unsigned long long audit_log_get_system_btime(void) {
    unsigned long long btime = 0;
#if defined(__linux__) || defined(__unix__)
    FILE* fp = fopen("/proc/stat", "r");
    if (!fp) {
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "btime ", 6) == 0) {
            sscanf(line + 6, "%llu", &btime);
            break;
        }
    }
    fclose(fp);
#endif
    return btime;
}

/* ------------------------------------------------------------------------- */
/* 2. CALCUL DYNAMIQUE DE L'HEURE DE DÉMARRAGE EXACTE (HH:MM:SS)            */
/* ------------------------------------------------------------------------- */
static void audit_log_compute_start_time(int pid, char* out_buf, size_t buf_sz) {
    if (!out_buf || buf_sz == 0) return;
    snprintf(out_buf, buf_sz, "Indisponible");

#if defined(__linux__) || defined(__unix__)
    char stat_path[320];
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

    FILE* fp = fopen(stat_path, "r");
    if (!fp) return;

    char buf[1024];
    if (fgets(buf, sizeof(buf), fp)) {
        char* last_paren = strrchr(buf, ')');
        if (last_paren) {
            unsigned long long start_ticks = 0;
            char* token = strtok(last_paren + 1, " ");
            int field_index = 3;

            while (token && field_index <= 22) {
                if (field_index == 22) {
                    start_ticks = strtoull(token, NULL, 10);
                    break;
                }
                token = strtok(NULL, " ");
                field_index++;
            }

            unsigned long long btime = audit_log_get_system_btime();
            long clk_tck = sysconf(_SC_CLK_TCK);
            if (clk_tck <= 0) clk_tck = 100;

            if (btime > 0 && start_ticks > 0) {
                time_t start_epoch = (time_t)(btime + (start_ticks / (unsigned long long)clk_tck));
                struct tm* tm_info = localtime(&start_epoch);
                if (tm_info) {
                    strftime(out_buf, buf_sz, "%H:%M:%S", tm_info);
                }
            }
        }
    }
    fclose(fp);
#else
    (void)pid;
#endif
}

/* ------------------------------------------------------------------------- */
/* 3. DÉTERMINATION DU PROCHAIN NUMÉRO SÉQUENTIEL DANS AUDIT.TXT             */
/* ------------------------------------------------------------------------- */
static int audit_log_get_next_sequence_number(void) {
    int max_no = 0;
    FILE* fp = fopen("AUDIT.TXT", "r");
    if (!fp) {
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int current_no = 0;
        if (sscanf(line, "AUDIT NO #%d", &current_no) == 1) {
            if (current_no > max_no) {
                max_no = current_no;
            }
        }
    }
    fclose(fp);
    return max_no + 1;
}

/* ------------------------------------------------------------------------- */
/* 4. CAPTURE DYNAMIQUE DE LA CIBLE EN MÉMOIRE                               */
/* ------------------------------------------------------------------------- */
bool raptor_audit_log_capture_target(int pid, RaptorIncidentDetail* out_detail) {
    if (!out_detail || pid <= 0) {
        return false;
    }

    memset(out_detail, 0, sizeof(RaptorIncidentDetail));
    out_detail->pid = pid;

#if defined(__linux__) || defined(__unix__)
    /* 1. Nom du processus */
    char path_comm[320];
    snprintf(path_comm, sizeof(path_comm), "/proc/%d/comm", pid);
    FILE* fp = fopen(path_comm, "r");
    if (fp) {
        if (fgets(out_detail->comm, sizeof(out_detail->comm), fp)) {
            size_t l = strlen(out_detail->comm);
            while (l > 0 && (out_detail->comm[l - 1] == '\n' || out_detail->comm[l - 1] == '\r')) {
                out_detail->comm[l - 1] = '\0';
                l--;
            }
        }
        fclose(fp);
    }

    /* 2. Binaire sur disque */
    char path_exe[320];
    snprintf(path_exe, sizeof(path_exe), "/proc/%d/exe", pid);
    ssize_t ret = readlink(path_exe, out_detail->exe_path, sizeof(out_detail->exe_path) - 1);
    if (ret > 0) {
        out_detail->exe_path[ret] = '\0';
    } else {
        snprintf(out_detail->exe_path, sizeof(out_detail->exe_path), "[non résolu / mémoire volatile]");
    }

    /* 3. Répertoire de travail courant (CWD) */
    char path_cwd[320];
    snprintf(path_cwd, sizeof(path_cwd), "/proc/%d/cwd", pid);
    ret = readlink(path_cwd, out_detail->cwd_path, sizeof(out_detail->cwd_path) - 1);
    if (ret > 0) {
        out_detail->cwd_path[ret] = '\0';
    } else {
        snprintf(out_detail->cwd_path, sizeof(out_detail->cwd_path), "[inconnu]");
    }

    /* 4. Heure exacte de début du code malveillant */
    audit_log_compute_start_time(pid, out_detail->start_time, sizeof(out_detail->start_time));

    /* 5. Identification du parent créateur (PPID, comm, cmdline, cwd) */
    char path_stat[320];
    snprintf(path_stat, sizeof(path_stat), "/proc/%d/stat", pid);
    fp = fopen(path_stat, "r");
    if (fp) {
        char buf[1024];
        if (fgets(buf, sizeof(buf), fp)) {
            char* last_paren = strrchr(buf, ')');
            if (last_paren) {
                int ppid = 0;
                char state = ' ';
                if (sscanf(last_paren + 1, " %c %d", &state, &ppid) == 2) {
                    out_detail->ppid = ppid;

                    /* Commande parent */
                    char path_pcmd[320];
                    snprintf(path_pcmd, sizeof(path_pcmd), "/proc/%d/cmdline", ppid);
                    FILE* fp_cmd = fopen(path_pcmd, "rb");
                    if (fp_cmd) {
                        size_t bytes = fread(buf, 1, sizeof(buf) - 1, fp_cmd);
                        if (bytes > 0) {
                            buf[bytes] = '\0';
                            for (size_t i = 0; i < bytes - 1; ++i) {
                                if (buf[i] == '\0') buf[i] = ' ';
                            }
                            size_t clen = strlen(buf);
                            if (clen >= sizeof(out_detail->pcmdline)) {
                                clen = sizeof(out_detail->pcmdline) - 1;
                            }
                            memcpy(out_detail->pcmdline, buf, clen);
                            out_detail->pcmdline[clen] = '\0';
                        }
                        fclose(fp_cmd);
                    }

                    /* CWD parent */
                    char path_pcwd[320];
                    snprintf(path_pcwd, sizeof(path_pcwd), "/proc/%d/cwd", ppid);
                    ret = readlink(path_pcwd, out_detail->pcwd_path, sizeof(out_detail->pcwd_path) - 1);
                    if (ret > 0) {
                        out_detail->pcwd_path[ret] = '\0';
                    }

                    /* Nom parent */
                    char path_pcomm[320];
                    snprintf(path_pcomm, sizeof(path_pcomm), "/proc/%d/comm", ppid);
                    FILE* fp_pcomm = fopen(path_pcomm, "r");
                    if (fp_pcomm) {
                        if (fgets(out_detail->pcomm, sizeof(out_detail->pcomm), fp_pcomm)) {
                            size_t l = strlen(out_detail->pcomm);
                            while (l > 0 && (out_detail->pcomm[l - 1] == '\n' || out_detail->pcomm[l - 1] == '\r')) {
                                out_detail->pcomm[l - 1] = '\0';
                                l--;
                            }
                        }
                        fclose(fp_pcomm);
                    }
                }
            }
        }
        fclose(fp);
    }
#endif
    return true;
}

/* ------------------------------------------------------------------------- */
/* 5. ENREGISTREMENT FORENSIQUE PERMANENT DANS AUDIT.TXT                     */
/* ------------------------------------------------------------------------- */
bool raptor_audit_log_commit(const RaptorIncidentDetail* detail, const char* action_status) {
    if (!detail) {
        return false;
    }

    FILE* fp = fopen("AUDIT.TXT", "a+");
    if (!fp) {
        return false;
    }

    int seq_no = audit_log_get_next_sequence_number();

    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    char date_str[64];
    if (tm_now) {
        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", tm_now);
    } else {
        snprintf(date_str, sizeof(date_str), "2026-10-20 20:00:00");
    }

    fprintf(fp, "================================================================================\n");
    fprintf(fp, "AUDIT NO #%02d : RAPTOR DEFENSE SYSTEM - %s\n", seq_no, date_str);
    fprintf(fp, "ÉVÉNEMENT    : [%s]\n", action_status ? action_status : "INTERVENTION FORENSIQUE");
    fprintf(fp, "--------------------------------------------------------------------------------\n");
    fprintf(fp, "DÉTAILS DU PROCESSUS CIBLÉ :\n");
    fprintf(fp, "  • PID Cible                : %d\n", detail->pid);
    fprintf(fp, "  • Nom du Processus         : %s\n", detail->comm[0] ? detail->comm : "inconnu");
    fprintf(fp, "  • Binaire Exécuté          : %s\n", detail->exe_path);
    fprintf(fp, "  • Répertoire de Lancement  : %s\n", detail->cwd_path[0] ? detail->cwd_path : "[inconnu]");
    fprintf(fp, "  • Heure de Début (Spawn)   : %s\n", detail->start_time[0] ? detail->start_time : "Indisponible");
    fprintf(fp, "  • Heure Interception / Fin : %s\n", detail->kill_time[0] ? detail->kill_time : date_str);
    if (detail->classification[0] != '\0') {
        fprintf(fp, "  • Qualification Heuristique: %s\n", detail->classification);
    }
    fprintf(fp, "LIGNÉE PARENTALE (CRÉATEUR DU PROCESSUS) :\n");
    fprintf(fp, "  • PID Parent (PPID)        : %d\n", detail->ppid);
    fprintf(fp, "  • Nom du Parent            : %s\n", detail->pcomm[0] ? detail->pcomm : "inconnu");
    fprintf(fp, "  • Ligne Commande Parent    : %s\n", detail->pcmdline[0] ? detail->pcmdline : "[aucun argument]");
    fprintf(fp, "  • Dossier Source Parent    : %s\n", detail->pcwd_path[0] ? detail->pcwd_path : "[inconnu]");
    fprintf(fp, "STATUT : CONSIGNATION FORENSIQUE EFFECTUÉE\n");
    fprintf(fp, "================================================================================\n\n");

    fflush(fp);
    fclose(fp);
    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. DÉCLENCHEMENT DE L'ÉCRITURE OBLIGATOIRE À LA DÉTECTION INITIALE        */
/* ------------------------------------------------------------------------- */
bool raptor_audit_log_notify_detection(int pid, const char* classification) {
    if (pid <= 0) {
        return false;
    }

#if defined(__linux__) || defined(__unix__)
    /* Purge des processus morts du cache de détection */
    for (size_t i = 0; i < s_detected_count; ) {
        if (kill((pid_t)s_detected_pids[i], 0) != 0) {
            s_detected_pids[i] = s_detected_pids[s_detected_count - 1];
            s_detected_count--;
        } else {
            i++;
        }
    }
#endif

    /* Vérification de dédoublonnage : si déjà consigné, ne pas réécrire */
    for (size_t i = 0; i < s_detected_count; ++i) {
        if (s_detected_pids[i] == pid) {
            return true;
        }
    }

    /* Nouveau processus malveillant détecté : capture et écriture obligatoire */
    RaptorIncidentDetail detail;
    if (raptor_audit_log_capture_target(pid, &detail)) {
        if (classification) {
            size_t clen = strlen(classification);
            if (clen >= sizeof(detail.classification)) {
                clen = sizeof(detail.classification) - 1;
            }
            memcpy(detail.classification, classification, clen);
            detail.classification[clen] = '\0';
        }
        snprintf(detail.kill_time, sizeof(detail.kill_time), "[ACTIF - DÉTECTÉ EN MÉMOIRE]");

        raptor_audit_log_commit(&detail, "DÉTECTION HEURISTIQUE EN TEMPS RÉEL");

        if (s_detected_count < MAX_TRACKED_DETECTIONS) {
            s_detected_pids[s_detected_count++] = pid;
        }
        return true;
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* 7. OUBLI D'UN PID LORS DE SA NEUTRALISATION OU SA DISPARITION             */
/* ------------------------------------------------------------------------- */
void raptor_audit_log_forget_pid(int pid) {
    for (size_t i = 0; i < s_detected_count; ++i) {
        if (s_detected_pids[i] == pid) {
            s_detected_pids[i] = s_detected_pids[s_detected_count - 1];
            s_detected_count--;
            break;
        }
    }
}
