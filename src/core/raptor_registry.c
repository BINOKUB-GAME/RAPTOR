/**
 * ============================================================================
 * NOM DU SCRIPT   : src/core/raptor_registry.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Implémentation du bus d'enregistrement et du registre central
 *                   des greffons. Permet l'auto-enregistrement dynamique des
 *                   sondes au chargement du binaire, l'exécution séquentielle
 *                   des audits sur le contexte universel et l'orchestration du
 *                   cycle de vie (init, run_audit, cleanup).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. raptor_registry_init
 *   2. raptor_registry_register
 *   3. raptor_registry_run_all
 *   4. raptor_registry_get_count
 *   5. raptor_registry_get_by_idx
 *   6. raptor_registry_cleanup
 * ============================================================================
 */

#include "raptor/raptor_registry.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* ÉTAT INTERNE DU REGISTRE                                                  */
/* ------------------------------------------------------------------------- */
static const RaptorPlugin** s_plugins = NULL;
static size_t s_plugin_count = 0;
static size_t s_plugin_capacity = 0;

/* ------------------------------------------------------------------------- */
/* 1. INITIALISATION DU REGISTRE DE GREFFONS                                 */
/* ------------------------------------------------------------------------- */
void raptor_registry_init(void) {
    /* Préserve les enregistrements déjà effectués par les constructeurs C */
    if (!s_plugins) {
        s_plugin_capacity = 16;
        s_plugin_count = 0;
        s_plugins = (const RaptorPlugin**)calloc(s_plugin_capacity, sizeof(const RaptorPlugin*));
    }
}

/* ------------------------------------------------------------------------- */
/* 2. ENREGISTREMENT D'UN GREFFON DANS LA LISTE GLOBALE                      */
/* ------------------------------------------------------------------------- */
bool raptor_registry_register(const RaptorPlugin* plugin) {
    if (!plugin || !plugin->id || !plugin->run_audit) {
        return false;
    }

    /* Alloue dynamiquement si un constructeur s'exécute avant raptor_registry_init */
    if (!s_plugins) {
        s_plugin_capacity = 16;
        s_plugin_count = 0;
        s_plugins = (const RaptorPlugin**)calloc(s_plugin_capacity, sizeof(const RaptorPlugin*));
        if (!s_plugins) {
            return false;
        }
    }

    /* Contrôle des doublons éventuels par identifiant unique */
    for (size_t i = 0; i < s_plugin_count; ++i) {
        if (s_plugins[i] == plugin) {
            return true;
        }
        if (s_plugins[i]->id && strcmp(s_plugins[i]->id, plugin->id) == 0) {
            return true;
        }
    }

    /* Agrandissement dynamique si saturation */
    if (s_plugin_count >= s_plugin_capacity) {
        size_t new_cap = (s_plugin_capacity == 0) ? 16 : s_plugin_capacity * 2;
        const RaptorPlugin** new_arr = (const RaptorPlugin**)realloc(
            s_plugins, new_cap * sizeof(const RaptorPlugin*)
        );
        if (!new_arr) {
            return false;
        }
        s_plugins = new_arr;
        s_plugin_capacity = new_cap;
    }

    s_plugins[s_plugin_count] = plugin;
    s_plugin_count++;
    return true;
}

/* ------------------------------------------------------------------------- */
/* 3. EXÉCUTION SÉQUENTIELLE DE TOUS LES GREFFONS ACTIFS                     */
/* ------------------------------------------------------------------------- */
bool raptor_registry_run_all(RaptorReportContext* ctx) {
    if (!ctx) {
        return false;
    }

    bool all_success = true;

    for (size_t i = 0; i < s_plugin_count; ++i) {
        const RaptorPlugin* plugin = s_plugins[i];
        if (!plugin) {
            continue;
        }

        /* Initialisation optionnelle du greffon si déclarée */
        if (plugin->init) {
            if (!plugin->init()) {
                all_success = false;
                continue;
            }
        }

        /* Exécution du diagnostic de la sonde */
        if (plugin->run_audit) {
            if (!plugin->run_audit(ctx)) {
                all_success = false;
            }
        }
    }

    return all_success;
}

/* ------------------------------------------------------------------------- */
/* 4. OBTENTION DU NOMBRE TOTAL DE GREFFONS BRANCHÉS                         */
/* ------------------------------------------------------------------------- */
size_t raptor_registry_get_count(void) {
    return s_plugin_count;
}

/* ------------------------------------------------------------------------- */
/* 5. ACCÈS EN LECTURE À UN GREFFON PAR INDEX                                */
/* ------------------------------------------------------------------------- */
const RaptorPlugin* raptor_registry_get_by_idx(size_t index) {
    if (index >= s_plugin_count || !s_plugins) {
        return NULL;
    }
    return s_plugins[index];
}

/* ------------------------------------------------------------------------- */
/* 6. NETTOYAGE DU REGISTRE ET LIBÉRATION                                    */
/* ------------------------------------------------------------------------- */
void raptor_registry_cleanup(void) {
    if (!s_plugins) {
        return;
    }

    for (size_t i = 0; i < s_plugin_count; ++i) {
        const RaptorPlugin* plugin = s_plugins[i];
        if (plugin && plugin->cleanup) {
            plugin->cleanup();
        }
    }

    free(s_plugins);
    s_plugins = NULL;
    s_plugin_count = 0;
    s_plugin_capacity = 0;
}
