/* ==============================================================================
 * NOM DU SCRIPT : src/core/rap_reflex.h
 * RÉVISION      : Rev 1.0
 * PROJET        : RAPTOR DEFENSE v2.0
 * DESCRIPTION   : Structures et interfaces du sous-système de remédiation active
 *                 chirurgicale. Fonctionnement déterministe en C99, 0-allocation.
 * FONCTIONS NUMÉROTÉES :
 *   1. rap_reflex_init()     - Initialisation du sous-système et allocation statique.
 *   2. rap_reflex_set_mode() - Bascule entre PASSIVE, DRYRUN et ARMED.
 *   3. rap_reflex_dispatch() - Déclenchement ordonnancé de la riposte.
 *   4. rap_reflex_get_stats()- Lecture des compteurs d'intervention.
 * ============================================================================== */

#ifndef RAP_REFLEX_H
#define RAP_REFLEX_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define RAP_REFLEX_MAX_PAYLOAD_SNAP 512
#define RAP_REFLEX_MAX_CMDLINE      256

typedef enum {
    TRIGGER_NONE            = 0,
    TRIGGER_CANARY_BREACH   = 1,
    TRIGGER_MEMORY_MUTATION = 2,
    TRIGGER_C2_ESTABLISHED  = 3,
    TRIGGER_BINARY_SEVERED  = 4
} reflex_trigger_t;

typedef enum {
    REFLEX_MODE_PASSIVE = 0, /* Surveillance uniquement, zéro action physique */
    REFLEX_MODE_DRYRUN  = 1, /* Simulation du protocole sans émission de signaux */
    REFLEX_MODE_ARMED   = 2  /* Remédiation active immédiate (SIGSTOP / SIGKILL) */
} reflex_mode_t;

typedef struct {
    struct timespec  timestamp;
    pid_t            pid;
    pid_t            ppid;
    uid_t            uid;
    reflex_trigger_t trigger;
    char             cmdline[RAP_REFLEX_MAX_CMDLINE];
    uint8_t          payload_sample[RAP_REFLEX_MAX_PAYLOAD_SNAP];
    uint32_t         latency_ns;
    bool             kill_confirmed;
} reflex_record_t;

/* --- Fonctions du module (1 à 4) --- */
int           rap_reflex_init(reflex_mode_t initial_mode);
void          rap_reflex_set_mode(reflex_mode_t mode);
int           rap_reflex_dispatch(pid_t target_pid, reflex_trigger_t trigger);
reflex_mode_t rap_reflex_get_stats(uint64_t *out_neutralized_count);

#endif /* RAP_REFLEX_H */
