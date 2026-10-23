/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/raptor_notify.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Émission de bulles de notification système natives Freedesktop.
 *                   Avertit l'administrateur dans la barre des tâches / bureau
 *                   même lorsque la fenêtre RAPTOR est minimisée ou en arrière-plan.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Implémentation initiale du module de notification (Section 15).
 *   - Fonction 86 : Déclenchement asynchrone non-bloquant via double fork()
 *     et execlp("notify-send") pour garantir 0 ms d'impact sur Raylib et 0 zombie.
 *   - Fonction 87 : Évaluation d'état avec filtre anti-rebond temporel (30s)
 *     et déclenchement instantané lors du passage de 0 à N menaces.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   86. raptor_notify_send_desktop
 *   87. raptor_notify_evaluate_threats
 * ============================================================================
 */

#include "raptor_notify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* État interne du filtre anti-rebond */
static int s_last_threat_count = 0;
static time_t s_last_notify_time = 0;
#define RAPTOR_NOTIFY_COOLDOWN_SEC 30

/* ------------------------------------------------------------------------- */
/* 86. ÉMISSION ASYNCHRONE D'UNE BULLE BUREAU VIA notify-send (FREEDESKTOP)  */
/* ------------------------------------------------------------------------- */
void raptor_notify_send_desktop(const char* title, const char* body, bool is_critical) {
    if (!title || !body) {
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        return;
    }

    if (pid == 0) {
        /* Processus enfant : second fork pour désolidariser l'exécution */
        pid_t grand_child = fork();
        if (grand_child == 0) {
            const char* urgency = is_critical ? "critical" : "normal";
            const char* icon = is_critical ? "dialog-error" : "dialog-information";

            /* Exécution directe du protocole standard de notification */
            execlp("notify-send",
                   "notify-send",
                   "-u", urgency,
                   "-i", icon,
                   "-a", "RAPTOR Defense",
                   title,
                   body,
                   (char*)NULL);

            _exit(127);
        }
        /* L'enfant intermédiaire se termine immédiatement */
        _exit(0);
    }

    /* Le processus parent de RAPTOR absorbe l'enfant intermédiaire sans bloquer */
    waitpid(pid, NULL, 0);
}

/* ------------------------------------------------------------------------- */
/* 87. ÉVALUATION D'ÉTAT & ARBITRAGE DU DÉCLENCHEMENT DES ALERTES           */
/* ------------------------------------------------------------------------- */
void raptor_notify_evaluate_threats(int current_threats, const char* primary_threat_name) {
    time_t now = time(NULL);

    /* Cas 1 : Retour à la normale (toutes les menaces ont été neutralisées) */
    if (current_threats == 0) {
        if (s_last_threat_count > 0) {
            raptor_notify_send_desktop(
                "RAPTOR DEFENSE // SYSTÈME SÉCURISÉ",
                "Toutes les anomalies mémoire ont été résolues. Retour au statut nominal.",
                false
            );
        }
        s_last_threat_count = 0;
        return;
    }

    /* Cas 2 : Apparition d'une nouvelle menace ou dépassement du délai de rappel */
    bool is_new_threat = (current_threats > s_last_threat_count);
    bool cooldown_elapsed = ((now - s_last_notify_time) >= RAPTOR_NOTIFY_COOLDOWN_SEC);

    if (is_new_threat || (current_threats > 0 && cooldown_elapsed)) {
        char body_buf[256];
        snprintf(body_buf, sizeof(body_buf),
                 "Alerte : %d menace%s active%s !\nProcessus suspect : %s",
                 current_threats,
                 (current_threats > 1) ? "s" : "",
                 (current_threats > 1) ? "s" : "",
                 primary_threat_name ? primary_threat_name : "Activité mémoire clandestine");

        raptor_notify_send_desktop(
            "RAPTOR // ALERTE DÉFENSIVE ACTIVE",
            body_buf,
            true
        );

        s_last_notify_time = now;
    }

    s_last_threat_count = current_threats;
}
