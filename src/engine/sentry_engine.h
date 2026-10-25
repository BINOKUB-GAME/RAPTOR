/* ============================================================================
 * NOM DU SCRIPT   : src/engine/sentry_engine.h
 * RÉVISION        : Rev 1.1
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Déclarations et structures de la sentinelle anti-rançongiciel
 *                   basée sur le mécanisme de notification noyau 'fanotify'
 *                   avec interception permissionnelle et fichiers canaris.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Encadrement conditionnel (#ifndef) de _GNU_SOURCE pour éliminer les
 *     avertissements de redéfinition avec CMake / GCC.
 *   - Définition des constantes de calibrage des leurres (32 Ko, préfixes d'appât).
 *   - Structures de configuration statiques pour le mode sentinelle 0 heap.
 *   - Déclaration des fonctions 111 à 115 constituant le module de veille passive.
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *   111. sentry_deploy_canary    : Dépose un fichier leurre dense et valide.
 *   112. sentry_init_fanotify    : Initialise le descripteur noyau permissionnel.
 *   113. sentry_listen_loop      : Boucle bloquante de guet passif (0% CPU).
 *   114. sentry_trigger_reflex   : Riposte synchrone (SIGSTOP + FAN_DENY).
 *   115. sentry_enter_stealth    : Camouflage prctl, immunité OOM et mlockall.
 * ============================================================================
 */

#ifndef RAPTOR_SENTRY_ENGINE_H
#define RAPTOR_SENTRY_ENGINE_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

#define SENTRY_CANARY_SIZE       32768      /* 32 Ko par fichier leurre */
#define SENTRY_CANARY_NAME       "!00_raptor_vault.kdbx"
#define SENTRY_MAX_TARGETS       16         /* Nombre max de répertoires surveillés */
#define SENTRY_PATH_MAX          512
#define SENTRY_EVENT_BUF_SIZE    4096

typedef struct {
    char target_dirs[SENTRY_MAX_TARGETS][SENTRY_PATH_MAX];
    uint32_t target_count;
    bool daemon_mode;
    bool auto_freeze;
} SentryConfig;

typedef struct {
    pid_t suspicious_pid;
    char triggered_path[SENTRY_PATH_MAX];
    uint32_t event_mask;
    bool neutralized;
} SentryAlert;

/* ============================================================================
 * 111. sentry_deploy_canary
 * Crée un fichier appât binaire de 32 Ko dans le répertoire spécifié.
 * Paramètres : dir_path (répertoire cible où déposer le leurre)
 * Retour     : 0 en cas de succès, -1 en cas d'échec d'écriture.
 * ============================================================================
 */
int sentry_deploy_canary(const char *dir_path);

/* ============================================================================
 * 112. sentry_init_fanotify
 * Ouvre le descripteur fanotify avec FAN_CLASS_CONTENT et marque les canaris.
 * Paramètres : config (configuration contenant la liste des cibles)
 * Retour     : Descripteur de fichier fanotify valide, ou -1 en cas d'erreur.
 * ============================================================================
 */
int sentry_init_fanotify(const SentryConfig *config);

/* ============================================================================
 * 113. sentry_listen_loop
 * Attend passivement les événements d'accès via poll() (consommation 0% CPU).
 * Paramètres : fanotify_fd (descripteur ouvert), config (paramètres de veille)
 * Retour     : 0 en cas d'arrêt nominal, code d'erreur POSIX sinon.
 * ============================================================================
 */
int sentry_listen_loop(int fanotify_fd, const SentryConfig *config);

/* ============================================================================
 * 114. sentry_trigger_reflex
 * Applique le gel immédiat du suspect (SIGSTOP) et refuse l'ouverture au noyau.
 * Paramètres : fanotify_fd (descripteur), fd_event (fichier visé), pid (agresseur)
 * Retour     : 0 si le tir réflexe a réussi, -1 si le PID est protégé.
 * ============================================================================
 */
int sentry_trigger_reflex(int fanotify_fd, int fd_event, pid_t pid);

/* ============================================================================
 * 115. sentry_enter_stealth
 * Verrouille la mémoire physique, définit l'immunité OOM et masque le nom du processus.
 * Paramètres : fake_name (nom de substitution anodin pour le processus)
 * Retour     : 0 en cas de verrouillage réussi, -1 si privilèges insuffisants.
 * ============================================================================
 */
int sentry_enter_stealth(const char *fake_name);

#endif /* RAPTOR_SENTRY_ENGINE_H */
