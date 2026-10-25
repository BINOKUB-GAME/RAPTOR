/* ==============================================================================
 * NOM DU SCRIPT : src/core/rap_oom.h
 * RÉVISION      : Rev 1.0 (2026-10-25)
 * PROJET        : RAPTOR DEFENSE v2.0
 * DESCRIPTION   : Interfaces de sécurisation mémoire et immunité système.
 *                 Fournit les verrous matériels interdisant la pagination sur disque
 *                 et l'élimination par le mécanisme OOM Killer du noyau Linux.
 * FONCTIONS NUMÉROTÉES :
 *   1. rap_oom_protect()     - Injection de la priorité de survie absolue (-1000).
 *   2. rap_oom_lock_memory() - Verrouillage des pages en RAM physique via mlockall.
 * ============================================================================== */

#ifndef RAP_OOM_H
#define RAP_OOM_H

#include <stdbool.h>

/* --- Fonctions du module (1 à 2) --- */

/**
 * 1. Configure /proc/self/oom_score_adj à -1000 afin d'exclure le processus
 *    de la politique de sacrifice d'urgence du noyau Linux.
 * @return 0 en cas de succès, -1 en cas d'erreur ou privilèges insuffisants.
 */
int rap_oom_protect(void);

/**
 * 2. Bloque la totalité des segments mémoire actuels et futurs en RAM physique.
 *    Interdit tout déchargement vers la mémoire d'échange (swap).
 * @return 0 en cas de succès, -1 en cas d'échec.
 */
int rap_oom_lock_memory(void);

#endif /* RAP_OOM_H */
