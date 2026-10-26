/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_filter.c
 * RÉVISION      : Rev 1.0 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Greffon de filtre dynamique 'f' (Filter Toggle).
 *                 Alterne cycliquement les filtres d'affichage :
 *                 1. TOUS (Vue exhaustive de l'ensemble des sockets noyau)
 *                 2. HORS LOCAL (Masque le bruit 127.0.0.1, 0.0.0.0 et LISTEN)
 *                 3. ÉTABLIS (Isole les sessions actives ESTABLISHED)
 *                 4. ALERTES (Vue de crise : HOSTILE, WARN, BAN et KILLED)
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Compaction linéaire in-place 0-heap sans copie intermédiaire.
 *   - Commutation instantanée à l'appui sur 'f'.
 *   - Détection des flux sous embargo (BAN) et des tombstones (KILLED).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 161 à 163)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   161. netop_cmd_filter_exec     - Bascule circulaire du mode de filtre et notification.
 *   162. netop_cmd_filter_apply    - Compaction des entrées selon le filtre actif.
 *   163. netop_cmd_filter_get_name - Restitution du libellé textuel du mode actif.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "netop_cmd_filter.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static NetopFilterMode s_filter_mode = NETOP_FILTER_ALL;

static uint64_t filter_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

/* ==============================================================================
 * FONCTION 163 : netop_cmd_filter_get_name
 * ============================================================================== */
const char *netop_cmd_filter_get_name(void)
{
    switch (s_filter_mode) {
        case NETOP_FILTER_ALL:         return "TOUS";
        case NETOP_FILTER_NO_LOCAL:    return "HORS LOCAL";
        case NETOP_FILTER_ESTABLISHED: return "ÉTABLIS";
        case NETOP_FILTER_ALERTS:      return "ALERTES SEULES";
        default:                       return "TOUS";
    }
}

NetopFilterMode netop_cmd_filter_get_mode(void)
{
    return s_filter_mode;
}

static bool filter_match_entry(const NetopEntry *e)
{
    switch (s_filter_mode) {
        case NETOP_FILTER_ALL:
            return true;

        case NETOP_FILTER_NO_LOCAL: {
            if (strcmp(e->state, "LISTEN") == 0) return false;
            if (strcmp(e->remote_ip, "0.0.0.0") == 0) return false;
            if (strncmp(e->remote_ip, "127.", 4) == 0) return false;
            if (strncmp(e->local_ip, "127.", 4) == 0 && strncmp(e->remote_ip, "127.", 4) == 0) return false;
            return true;
        }

        case NETOP_FILTER_ESTABLISHED:
            return (strcmp(e->state, "ESTABLISHED") == 0);

        case NETOP_FILTER_ALERTS: {
            if (e->severity == NETOP_SEV_HOSTILE) return true;
            if (e->severity == NETOP_SEV_WARN)    return true;
            if (e->severity == NETOP_SEV_KILLED)  return true;
            if (strstr(e->verdict, "ISOLÉ") != NULL) return true;
            if (strstr(e->verdict, "BAN") != NULL)   return true;
            return false;
        }

        default:
            return true;
    }
}

/* ==============================================================================
 * FONCTION 162 : netop_cmd_filter_apply
 * ============================================================================== */
size_t netop_cmd_filter_apply(NetopEntry *entries, size_t count)
{
    if (!entries || count == 0 || s_filter_mode == NETOP_FILTER_ALL) {
        return count;
    }

    size_t out_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (filter_match_entry(&entries[i])) {
            if (out_count != i) {
                entries[out_count] = entries[i];
            }
            out_count++;
        }
    }

    return out_count;
}

/* ==============================================================================
 * FONCTION 161 : netop_cmd_filter_exec
 * ============================================================================== */
int netop_cmd_filter_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg)
{
    (void)entries;
    (void)count;
    (void)arg;

    /* Bascule cyclique 0 -> 1 -> 2 -> 3 -> 0 */
    s_filter_mode = (NetopFilterMode)((s_filter_mode + 1) % 4);

    if (ctx) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;36m[FILTRE DYNAMIQUE]\033[0m Mode actif : \033[1;37m%s\033[0m",
                 netop_cmd_filter_get_name());
        ctx->status_msg_expire = filter_now_ms() + 2500;
    }

    return 0;
}
