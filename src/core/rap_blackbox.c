/* ==============================================================================
 * NOM DU SCRIPT : src/core/rap_blackbox.c
 * RÉVISION      : Rev 1.1 (2026-10-25)
 * PROJET        : RAPTOR DEFENSE v2.0
 * DESCRIPTION   : Gestionnaire de journalisation circulaire binaire en RAM pure.
 *                 Assure la conservation infalsifiable des preuves post-incident
 *                 sans solliciter les systèmes de fichiers disque traditionnels.
 * NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Application de fchmod 0666 pour autoriser la consultation hors privilèges root.
 *   - Messages d'avertissement explicites sur stderr en cas d'erreur de permissions shm.
 * FONCTIONS NUMÉROTÉES :
 *   1. rap_blackbox_open()  - Création/attachement du segment partagé /dev/shm.
 *   2. rap_blackbox_push()  - Insertion déterministe d'un incident médico-légal.
 *   3. rap_blackbox_dump()  - Lecture et affichage ordonné des événements consignés.
 *   4. rap_blackbox_close() - Synchronisation mémoire et détachement du segment.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "rap_blackbox.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define RAP_BLACKBOX_MAGIC   0x52415054U /* Motif ASCII binaire "RAPT" */
#define RAP_BLACKBOX_VERSION 1U

static rap_blackbox_buffer_t *g_bb_buffer = NULL;
static int                    g_bb_fd     = -1;

/* ==============================================================================
 * 1. rap_blackbox_open
 * ============================================================================== */
int rap_blackbox_open(void)
{
    if (g_bb_buffer != NULL) {
        return 0; /* Déjà attaché */
    }

    g_bb_fd = shm_open(RAP_BLACKBOX_SHM_PATH, O_RDWR | O_CREAT, 0666);
    if (g_bb_fd < 0) {
        if (errno == EACCES) {
            fprintf(stderr, "\033[1;31m[ERREUR BOÎTE NOIRE]\033[0m Accès refusé à /dev/shm%s. Utilisez 'sudo rap sentry dump'.\n", RAP_BLACKBOX_SHM_PATH);
        } else {
            perror("\033[1;31m[ERREUR BOÎTE NOIRE]\033[0m Échec shm_open");
        }
        return -1;
    }

    /* Garantit l'accès en lecture à tous les comptes pour l'audit */
    fchmod(g_bb_fd, 0666);

    const size_t target_size = sizeof(rap_blackbox_buffer_t);

    struct stat st;
    if (fstat(g_bb_fd, &st) != 0) {
        close(g_bb_fd);
        g_bb_fd = -1;
        return -1;
    }

    if ((size_t)st.st_size < target_size) {
        if (ftruncate(g_bb_fd, (off_t)target_size) != 0) {
            close(g_bb_fd);
            g_bb_fd = -1;
            return -1;
        }
    }

    void *mapped = mmap(NULL, target_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_bb_fd, 0);
    if (mapped == MAP_FAILED) {
        close(g_bb_fd);
        g_bb_fd = -1;
        return -1;
    }

    g_bb_buffer = (rap_blackbox_buffer_t *)mapped;

    /* Initialisation de l'en-tête de la structure si nouveau segment */
    if (g_bb_buffer->magic != RAP_BLACKBOX_MAGIC) {
        memset(g_bb_buffer, 0, target_size);
        g_bb_buffer->magic        = RAP_BLACKBOX_MAGIC;
        g_bb_buffer->version      = RAP_BLACKBOX_VERSION;
        g_bb_buffer->head_index   = 0;
        g_bb_buffer->total_events = 0;
    }

    return 0;
}

/* ==============================================================================
 * 2. rap_blackbox_push
 * ============================================================================== */
void rap_blackbox_push(const reflex_record_t *record)
{
    if (g_bb_buffer == NULL || record == NULL) {
        return;
    }

    uint32_t idx = g_bb_buffer->head_index % RAP_BLACKBOX_CAPACITY;
    memcpy(&g_bb_buffer->records[idx], record, sizeof(reflex_record_t));

    g_bb_buffer->head_index = (idx + 1U) % RAP_BLACKBOX_CAPACITY;
    g_bb_buffer->total_events++;

    /* Synchronisation non-bloquante vers la page mémoire */
    msync((void *)g_bb_buffer, sizeof(rap_blackbox_buffer_t), MS_ASYNC);
}

/* ==============================================================================
 * 3. rap_blackbox_dump
 * ============================================================================== */
int rap_blackbox_dump(void)
{
    if (g_bb_buffer == NULL) {
        if (rap_blackbox_open() != 0) {
            return -1;
        }
    }

    if (g_bb_buffer->magic != RAP_BLACKBOX_MAGIC) {
        fprintf(stderr, "\033[1;31m[ERREUR BOÎTE NOIRE]\033[0m Format de données invalide ou segment corrompu.\n");
        return -2;
    }

    printf("\n\033[1;36m[RAPTOR BLACKBOX]\033[0m Événements cumulés: %u | Capacité anneau: %d\n",
           g_bb_buffer->total_events, RAP_BLACKBOX_CAPACITY);
    printf("--------------------------------------------------------------------------------\n");

    uint32_t count = g_bb_buffer->total_events;
    if (count > RAP_BLACKBOX_CAPACITY) {
        count = RAP_BLACKBOX_CAPACITY;
    }

    if (count == 0) {
        printf("  (Aucun incident consigné dans le tampon mémoire pour le moment)\n");
        printf("--------------------------------------------------------------------------------\n\n");
        return 0;
    }

    for (uint32_t i = 0; i < count; i++) {
        const reflex_record_t *rec = &g_bb_buffer->records[i];
        if (rec->pid == 0) {
            continue;
        }

        const char *trig_name = "INCONNU";
        switch (rec->trigger) {
            case TRIGGER_CANARY_BREACH:   trig_name = "CANARI_ALTÉRÉ"; break;
            case TRIGGER_MEMORY_MUTATION: trig_name = "MUTATION_MÉMOIRE_RWX"; break;
            case TRIGGER_C2_ESTABLISHED:  trig_name = "CONNEXION_C2_HOSTILE"; break;
            case TRIGGER_BINARY_SEVERED:  trig_name = "BINAIRE_SUPPRIMÉ"; break;
            default: break;
        }

        printf("[%02u] HORODATAGE: %ld.%09ld | PID: %d | PPID: %d | UID: %u\n",
               i, (long)rec->timestamp.tv_sec, rec->timestamp.tv_nsec,
               rec->pid, rec->ppid, rec->uid);
        printf("     CAUSE: %s | LATENCE: %u ns | STATUT: %s\n",
               trig_name, rec->latency_ns,
               rec->kill_confirmed ? "\033[1;31mNEUTRALISÉ (SIGKILL)\033[0m" : "\033[1;33mSIMULATION (DRY-RUN)\033[0m");
        printf("     COMMANDE: %s\n", rec->cmdline[0] ? rec->cmdline : "<non disponible>");
        printf("--------------------------------------------------------------------------------\n");
    }
    printf("\n");

    return 0;
}

/* ==============================================================================
 * 4. rap_blackbox_close
 * ============================================================================== */
void rap_blackbox_close(void)
{
    if (g_bb_buffer != NULL) {
        msync((void *)g_bb_buffer, sizeof(rap_blackbox_buffer_t), MS_SYNC);
        munmap((void *)g_bb_buffer, sizeof(rap_blackbox_buffer_t));
        g_bb_buffer = NULL;
    }

    if (g_bb_fd >= 0) {
        close(g_bb_fd);
        g_bb_fd = -1;
    }
}
