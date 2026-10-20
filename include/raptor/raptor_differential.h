/**
 * ============================================================================
 * NOM DU SCRIPT   : include/raptor/raptor_differential.h
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Déclare les structures et prototypes du moteur d'analyse
 *                   différentielle temporelle. Assure la capture d'instantanés
 *                   (snapshots), le calcul de dérive (Mo/min) entre deux audits
 *                   consécutifs et la détection d'anomalies de consommation.
 *
 * LISTE DES FONCTIONS DÉCLARÉES :
 *   1. raptor_diff_init
 *   2. raptor_diff_compute
 *   3. raptor_diff_snapshot_save
 *   4. raptor_diff_cleanup
 * ============================================================================
 */

#ifndef RAPTOR_DIFFERENTIAL_H
#define RAPTOR_DIFFERENTIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "raptor_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* FONCTIONS DU MOTEUR DIFFÉRENTIEL (API 3.3)                                */
/* ------------------------------------------------------------------------- */

/**
 * 1. Initialise les structures de mise en cache différentielle.
 */
void raptor_diff_init(void);

/**
 * 2. Compare Tn et Tn-1, calcule les dérives (Mo/min) et peuple la section différentielle.
 */
bool raptor_diff_compute(const RaptorReportContext* current, RaptorSection* out_diff_sec);

/**
 * 3. Archive l'état actuel comme point de repère pour le prochain audit.
 */
void raptor_diff_snapshot_save(const RaptorReportContext* current);

/**
 * 4. Libère les instantanés d'historique conservés en mémoire vive.
 */
void raptor_diff_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_DIFFERENTIAL_H */
