/**
 * ============================================================================
 * NOM DU SCRIPT   : include/raptor/raptor_plugin.h
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Définit le contrat d'interface universel d'un greffon
 *                   (RaptorPlugin), les prototypes de points d'entrée
 *                   d'audit et la macro portable d'auto-enregistrement
 *                   au démarrage (compatible Linux GCC/Clang et Windows MSVC).
 *
 * ÉLÉMENTS EXPORTÉS :
 *   1. Type RaptorPluginInitFn    : Prototype de fonction d'initialisation
 *   2. Type RaptorPluginAuditFn   : Prototype de fonction d'audit principal
 *   3. Type RaptorPluginCleanupFn : Prototype de fonction de nettoyage
 *   4. Structure RaptorPlugin     : Descripteur standardisé d'un greffon
 *   5. Macro RAPTOR_PLUGIN_INIT   : Constructeur d'auto-enregistrement portable
 * ============================================================================
 */

#ifndef RAPTOR_PLUGIN_H
#define RAPTOR_PLUGIN_H

#include <stdbool.h>
#include <stddef.h>
#include "raptor_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* CONTRAT D'INTERFACE D'UN GREFFON                                          */
/* ------------------------------------------------------------------------- */

typedef bool (*RaptorPluginInitFn)(void);
typedef bool (*RaptorPluginAuditFn)(RaptorReportContext* ctx);
typedef void (*RaptorPluginCleanupFn)(void);

/**
 * Structure universelle caractérisant tout greffon RAPTOR.
 */
typedef struct {
    const char* id;
    const char* name;
    const char* version;
    bool (*init)(void);
    bool (*run_audit)(RaptorReportContext* ctx);
    void (*cleanup)(void);
} RaptorPlugin;

/* ------------------------------------------------------------------------- */
/* MACRO D'AUTO-ENREGISTREMENT PORTABLE (LINUX GCC/CLANG & WINDOWS MSVC)     */
/* ------------------------------------------------------------------------- */

#if defined(_MSC_VER)
    /* Windows / MSVC : Enregistrement via la table d'initialisation C (.CRT$XCU) */
    #pragma section(".CRT$XCU", read)
    #define RAPTOR_PLUGIN_INIT(fn) \
        static void fn(void); \
        __declspec(allocate(".CRT$XCU")) static void (*fn##_ctor_ptr)(void) = fn; \
        static void fn(void)
#elif defined(__GNUC__) || defined(__clang__)
    /* Linux / GCC / Clang : Constructeur ELF invoqué au chargement du binaire */
    #define RAPTOR_PLUGIN_INIT(fn) \
        __attribute__((constructor)) static void fn(void)
#else
    #define RAPTOR_PLUGIN_INIT(fn) \
        static void fn(void)
#endif

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_PLUGIN_H */
