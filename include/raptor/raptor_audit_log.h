/**
 * ==============================================================================
 * NOM DU SCRIPT   : include/raptor/raptor_audit_log.h
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Déclaration du gestionnaire de journalisation forensique
 *                   persistante (AUDIT.TXT). Enregistre les métriques d'origine
 *                   des processus suspects (CWD, PID parent, start time, kill time)
 *                   avec séquençage incrémental automatique et dédoublonnage.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Déclaration de raptor_audit_log_notify_detection() pour l'écriture
 *     immédiate et automatique dès l'apparition d'un processus malveillant.
 *   - Déclaration de raptor_audit_log_forget_pid() pour purger le cache de suivi
 *     après neutralisation et autoriser la détection d'un respawn immédiat.
 * ============================================================================
 */

#ifndef RAPTOR_AUDIT_LOG_H
#define RAPTOR_AUDIT_LOG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int pid;
    char comm[64];
    char exe_path[512];
    char cwd_path[512];
    char start_time[32];
    char kill_time[32];
    int ppid;
    char pcomm[64];
    char pcmdline[512];
    char pcwd_path[512];
    char classification[128];
} RaptorIncidentDetail;

/* Collecte dynamique des métriques du processus avant sa destruction */
bool raptor_audit_log_capture_target(int pid, RaptorIncidentDetail* out_detail);

/* Enregistre l'incident dans AUDIT.TXT avec numéro séquentiel auto */
bool raptor_audit_log_commit(const RaptorIncidentDetail* detail, const char* action_status);

/* Déclenche l'écriture obligatoire dès la première détection (anti-flood par PID) */
bool raptor_audit_log_notify_detection(int pid, const char* classification);

/* Retire un PID du cache de détection lors de son arrêt */
void raptor_audit_log_forget_pid(int pid);

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_AUDIT_LOG_H */
