/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_sort.c
 * RÉVISION      : Rev 1.0 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Greffon de tri dynamique 's' (Sort Toggle).
 *                 Alterne cycliquement les modes de tri statiques :
 *                 1. DANGEROSITÉ (KILLED > HOSTILE > WARN > ESTABLISHED > LISTEN)
 *                 2. PID (Ordre croissant des PIDs réels, PIDs 0 en fin)
 *                 3. PORT / ACTIVITÉ (Regroupement par port distant puis local)
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Tri stable par insertion in-place (0-heap, aucune allocation mémoire).
 *   - Commutation instantanée à l'appui sur 's' sans invite de saisie.
 *   - Restitution du libellé du tri actif pour l'en-tête de télémétrie.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 158 à 160)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   158. netop_cmd_sort_exec     - Bascule circulaire du mode de tri et notification.
 *   159. netop_cmd_sort_apply    - Application de l'algorithme de tri stable sur les entrées.
 *   160. netop_cmd_sort_get_name - Restitution du libellé textuel du mode actif.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "netop_cmd_sort.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static NetopSortMode s_sort_mode = NETOP_SORT_SEVERITY;

static uint64_t sort_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

/* ==============================================================================
 * FONCTION 160 : netop_cmd_sort_get_name
 * ============================================================================== */
const char *netop_cmd_sort_get_name(void)
{
    switch (s_sort_mode) {
        case NETOP_SORT_SEVERITY: return "DANGEROSITÉ";
        case NETOP_SORT_PID:      return "PID";
        case NETOP_SORT_PORT:     return "PORT/ACTIVITÉ";
        default:                  return "DÉFAUT";
    }
}

NetopSortMode netop_cmd_sort_get_mode(void)
{
    return s_sort_mode;
}

/* Comparateur stable : renvoie < 0 si 'a' doit précéder 'b' */
static int sort_compare(const NetopEntry *a, const NetopEntry *b)
{
    switch (s_sort_mode) {
        case NETOP_SORT_SEVERITY: {
            if (a->severity != b->severity) {
                return (b->severity - a->severity); /* Décroissant */
            }
            bool a_estab = (strcmp(a->state, "ESTABLISHED") == 0);
            bool b_estab = (strcmp(b->state, "ESTABLISHED") == 0);
            if (a_estab != b_estab) {
                return b_estab ? 1 : -1;
            }
            return 0;
        }

        case NETOP_SORT_PID: {
            /* Mettre les sockets orphelins (PID 0) à la fin */
            if (a->pid == 0 && b->pid != 0) return 1;
            if (a->pid != 0 && b->pid == 0) return -1;
            if (a->pid < b->pid) return -1;
            if (a->pid > b->pid) return 1;
            return 0;
        }

        case NETOP_SORT_PORT: {
            if (a->remote_port == 0 && b->remote_port != 0) return 1;
            if (a->remote_port != 0 && b->remote_port == 0) return -1;
            if (a->remote_port < b->remote_port) return -1;
            if (a->remote_port > b->remote_port) return 1;
            if (a->local_port < b->local_port) return -1;
            if (a->local_port > b->local_port) return 1;
            return 0;
        }

        default:
            return 0;
    }
}

/* ==============================================================================
 * FONCTION 159 : netop_cmd_sort_apply
 * ============================================================================== */
void netop_cmd_sort_apply(NetopEntry *entries, size_t count)
{
    if (!entries || count < 2) return;

    for (size_t i = 1; i < count; i++) {
        NetopEntry key = entries[i];
        int j = (int)i - 1;

        while (j >= 0 && sort_compare(&key, &entries[j]) < 0) {
            entries[j + 1] = entries[j];
            j--;
        }
        entries[j + 1] = key;
    }
}

/* ==============================================================================
 * FONCTION 158 : netop_cmd_sort_exec
 * ============================================================================== */
int netop_cmd_sort_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg)
{
    (void)arg;

    /* Bascule cyclique 0 -> 1 -> 2 -> 0 */
    s_sort_mode = (NetopSortMode)((s_sort_mode + 1) % 3);

    if (entries && count > 1) {
        netop_cmd_sort_apply(entries, count);
    }

    if (ctx) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;33m[TRI MODIFIÉ]\033[0m Mode actif : \033[1;37m%s\033[0m",
                 netop_cmd_sort_get_name());
        ctx->status_msg_expire = sort_now_ms() + 2500;
    }

    return 0;
}
