/**
 * ============================================================================
 * NOM DU SCRIPT   : include/raptor/raptor_registry.h
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Déclare le registre central des greffons (bus d'enregistrement).
 *                   Permet l'auto-enregistrement au chargement, l'itération
 *                   ordonnée sur les sondes actives et le cycle de vie
 *                   complet (initialisation, audit séquentiel, nettoyage).
 *
 * LISTE DES FONCTIONS DÉCLARÉES :
 *   1. raptor_registry_init
 *   2. raptor_registry_register
 *   3. raptor_registry_run_all
 *   4. raptor_registry_get_count
 *   5. raptor_registry_get_by_idx
 *   6. raptor_registry_cleanup
 * ============================================================================
 */

#ifndef RAPTOR_REGISTRY_H
#define RAPTOR_REGISTRY_H

#include <stdbool.h>
#include <stddef.h>
#include "raptor_data.h"
#include "raptor_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* FONCTIONS DU REGISTRE DE GREFFONS (API 3.2)                               */
/* ------------------------------------------------------------------------- */

/**
 * 1. Initialise la table interne des pointeurs de greffons.
 */
void raptor_registry_init(void);

/**
 * 2. Enregistre un greffon dans la liste globale.
 */
bool raptor_registry_register(const RaptorPlugin* plugin);

/**
 * 3. Lance séquentiellement la méthode run_audit de chaque greffon actif.
 */
bool raptor_registry_run_all(RaptorReportContext* ctx);

/**
 * 4. Retourne le nombre total de greffons actuellement branchés.
 */
size_t raptor_registry_get_count(void);

/**
 * 5. Fournit l'accès en lecture à un greffon via son index.
 */
const RaptorPlugin* raptor_registry_get_by_idx(size_t index);

/**
 * 6. Appelle cleanup() sur chaque greffon et vide le registre.
 */
void raptor_registry_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_REGISTRY_H */
