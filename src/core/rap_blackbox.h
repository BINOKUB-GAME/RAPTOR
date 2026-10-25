/* ==============================================================================
 * NOM DU SCRIPT : src/core/rap_blackbox.h
 * RÉVISION      : Rev 1.0
 * PROJET        : RAPTOR DEFENSE v2.0
 * DESCRIPTION   : Journalisation binaire infalsifiable en anneau circulaire 
 *                 dans la mémoire vive partagée (/dev/shm). Zéro allocation dynamique.
 * FONCTIONS NUMÉROTÉES :
 *   1. rap_blackbox_open()  - Création/attachement de la zone mémoire partagée.
 *   2. rap_blackbox_push()  - Insertion atomique d'un enregistrement d'incident.
 *   3. rap_blackbox_dump()  - Lecture et restitution ordonnée des incidents.
 *   4. rap_blackbox_close() - Synchronisation finale et détachement propre.
 * ============================================================================== */

#ifndef RAP_BLACKBOX_H
#define RAP_BLACKBOX_H

#include "rap_reflex.h"

#define RAP_BLACKBOX_SHM_PATH  "/raptor_blackbox"
#define RAP_BLACKBOX_CAPACITY  64

typedef struct {
    uint32_t        magic;         /* Nombre magique de validation : 0x52415054 ("RAPT") */
    uint32_t        version;       /* Version du format de structure */
    uint32_t        head_index;    /* Pointeur d'écriture circulaire */
    uint32_t        total_events;  /* Compteur cumulatif d'incidents enregistrés */
    reflex_record_t records[RAP_BLACKBOX_CAPACITY];
} rap_blackbox_buffer_t;

/* --- Fonctions du module (1 à 4) --- */
int  rap_blackbox_open(void);
void rap_blackbox_push(const reflex_record_t *record);
int  rap_blackbox_dump(void);
void rap_blackbox_close(void);

#endif /* RAP_BLACKBOX_H */
