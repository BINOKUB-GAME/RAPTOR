/**
 * ============================================================================
 * NOM DU SCRIPT   : include/raptor/raptor_data.h
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Modèle de données universel et conteneur de rapport
 *                   agnostique. Définit les structures tabulaires, cartes
 *                   de métriques, alertes et sections consommées par les
 *                   moteurs de restitution (CLI, HTML, GUI ImGui/Raylib).
 *
 * LISTE DES FONCTIONS DÉCLARÉES :
 *   1. raptor_data_context_create
 *   2. raptor_data_section_add
 *   3. raptor_data_card_add
 *   4. raptor_data_table_create
 *   5. raptor_data_table_row_add
 *   6. raptor_data_alert_add
 *   7. raptor_data_get_section_count
 *   8. raptor_data_get_section
 *   9. raptor_data_context_destroy
 * ============================================================================
 */

#ifndef RAPTOR_DATA_H
#define RAPTOR_DATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* ÉNUMÉRATIONS ET STRUCTURES DU MODÈLE UNIVERSEL                            */
/* ------------------------------------------------------------------------- */

/**
 * Niveaux de sévérité universels pour cartes, alertes et cellules tabulaires.
 */
typedef enum {
    RAPTOR_SEV_INFO = 0,
    RAPTOR_SEV_OK,
    RAPTOR_SEV_WARNING,
    RAPTOR_SEV_CRITICAL
} RaptorSeverity;

/**
 * Carte de métrique clé affichée dans les tableaux de bord.
 */
typedef struct {
    char label[64];
    char val[64];
    float pct;
    RaptorSeverity sev;
} RaptorCard;

/**
 * Notification ou avertissement textuel qualifié par un niveau de sévérité.
 */
typedef struct {
    RaptorSeverity sev;
    char message[512];
} RaptorAlert;

/**
 * Ligne complète d'un tableau dynamique avec sévérité par cellule.
 */
typedef struct {
    char** cell_texts;
    RaptorSeverity* cell_sevs;
} RaptorTableRow;

/**
 * Tableau structuré dynamique indépendant de la sonde source.
 */
typedef struct {
    char title[128];
    size_t col_count;
    char** col_names;
    RaptorTableRow* rows;
    size_t row_count;
    size_t row_capacity;
} RaptorTable;

/**
 * Section autonome regroupant les métriques, alertes et tables d'un domaine d'audit.
 */
typedef struct {
    char id[64];
    char title[128];

    RaptorCard* cards;
    size_t card_count;
    size_t card_capacity;

    RaptorAlert* alerts;
    size_t alert_count;
    size_t alert_capacity;

    RaptorTable* tables;
    size_t table_count;
    size_t table_capacity;
} RaptorSection;

/**
 * Conteneur racine universel d'un rapport d'audit RAPTOR.
 */
typedef struct {
    RaptorSection* sections;
    size_t section_count;
    size_t section_capacity;
} RaptorReportContext;

/* ------------------------------------------------------------------------- */
/* FONCTIONS DU MODÈLE DE DONNÉES (API 3.1)                                  */
/* ------------------------------------------------------------------------- */

/**
 * 1. Alloue et initialise un nouveau contexte de rapport vide.
 */
RaptorReportContext* raptor_data_context_create(void);

/**
 * 2. Instancie une nouvelle section autonome rattachée au contexte.
 */
RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title);

/**
 * 3. Ajoute une métrique en carte visuelle à une section.
 */
bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev);

/**
 * 4. Crée un tableau tabulaire dans la section avec ses en-têtes.
 */
RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names);

/**
 * 5. Insère une ligne complète de cellules dans le tableau.
 */
bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs);

/**
 * 6. Enregistre une notification ou un avertissement dans la section.
 */
bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message);

/**
 * 7. Retourne le nombre total de sections enregistrées dans le contexte.
 */
size_t raptor_data_get_section_count(const RaptorReportContext* ctx);

/**
 * 8. Fournit un accès sécurisé en lecture à une section via son index.
 */
const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index);

/**
 * 9. Libère récursivement l'ensemble de la mémoire allouée au contexte.
 */
void raptor_data_context_destroy(RaptorReportContext* ctx);

#ifdef __cplusplus
}
#endif

#endif /* RAPTOR_DATA_H */
