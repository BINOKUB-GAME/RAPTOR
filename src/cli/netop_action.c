/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_action.c
 * RÉVISION      : Rev 1.1 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Greffon d'action cinétique et de remédiation réseau.
 *                 Contrôle les verrous d'inviolabilité (PID 1, shell, self),
 *                 exécute la frappe chirurgicale (SIGSTOP + SIGKILL), mesure la
 *                 latence et gère la trace rémanente orange (Tombstones).
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.1) :
 *   - Calibrage du formatage de latence sur tampon élargi à 256 octets.
 *   - Éradication de l'avertissement GCC -Wformat-truncation.
 *   - Préservation des verrous stricts anti-suicide (PID <= 1, self, ppid).
 *   - Registre statique de pierres tombales (0-heap) rémanent 3,5 secondes.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 142 & 143)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   142. netop_action_kill_slot        - Validation de sécurité et tir cinétique cerné.
 *   143. netop_action_apply_tombstones - Application de la trace orange rémanente.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "rap_netop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define NETOP_MAX_TOMBSTONES   16
#define NETOP_TOMBSTONE_TTL_MS 7000 /* 3,5 secondes d'éclat orange */

typedef struct {
    uint32_t pid;
    char     remote_ip[48];
    uint16_t remote_port;
    uint64_t expire_ms;
    bool     active;
} NetopTombstone;

static NetopTombstone s_tombstones[NETOP_MAX_TOMBSTONES];

static uint64_t netop_get_monotonic_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

/* ==============================================================================
 * FONCTION 143 : netop_action_apply_tombstones
 * Maintient la trace orange [ABATTU] sur les sockets encore visibles
 * ============================================================================== */
void netop_action_apply_tombstones(NetopEntry *entries, size_t count)
{
    uint64_t now = netop_get_monotonic_ms();

    for (size_t t = 0; t < NETOP_MAX_TOMBSTONES; t++) {
        if (!s_tombstones[t].active) continue;

        if (now >= s_tombstones[t].expire_ms) {
            s_tombstones[t].active = false;
            continue;
        }

        /* Confrontation avec les flux actifs rapportés par le noyau */
        for (size_t i = 0; i < count; i++) {
            NetopEntry *e = &entries[i];

            bool match_pid = (e->pid != 0 && e->pid == s_tombstones[t].pid);
            bool match_endpoint = (e->remote_port == s_tombstones[t].remote_port &&
                                   strcmp(e->remote_ip, s_tombstones[t].remote_ip) == 0);

            if (match_pid || match_endpoint) {
                e->severity = NETOP_SEV_KILLED;
                snprintf(e->comm, sizeof(e->comm), "[ABATTU]");
                snprintf(e->state, sizeof(e->state), "DISJONCTÉ");
                snprintf(e->verdict, sizeof(e->verdict), "NEUTRALISÉ (SIGKILL)");
            }
        }
    }
}

/* ==============================================================================
 * FONCTION 142 : netop_action_kill_slot
 * ============================================================================== */
int netop_action_kill_slot(NetopContext *ctx, NetopEntry *entries, size_t count, int line_num)
{
    if (!ctx || !entries || count == 0) return -1;

    uint64_t now = netop_get_monotonic_ms();

    /* 1. Validation de la borne de sélection */
    if (line_num < 1 || (size_t)line_num > count) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[ERREUR] Ligne #%02d inexistante (Plage valide : 01-%02zu)\033[0m",
                 line_num, count);
        ctx->status_msg_expire = now + 3000;
        return -1;
    }

    NetopEntry *target = &entries[line_num - 1];
    uint32_t pid = target->pid;

    /* 2. Contrôles stricts d'inviolabilité */
    if (pid == 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;33m[REFUS] Impossible d'abattre un socket orphelin ou en cours de purge.\033[0m");
        ctx->status_msg_expire = now + 3000;
        return -1;
    }

    if (pid <= 1) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[SÉCURITÉ VIOLÉE] PID 1 (systemd/init) est strictement inviolable !\033[0m");
        ctx->status_msg_expire = now + 3500;
        return -1;
    }

    if (pid == (uint32_t)getpid()) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[SÉCURITÉ VIOLÉE] Auto-neutralisation de RAPTOR proscrite !\033[0m");
        ctx->status_msg_expire = now + 3500;
        return -1;
    }

    if (pid == (uint32_t)getppid()) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[SÉCURITÉ VIOLÉE] Neutralisation du shell parent interdite !\033[0m");
        ctx->status_msg_expire = now + 3500;
        return -1;
    }

    /* 3. Déclenchement de la frappe cinétique et mesure de latence */
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    /* Séquence réflexe : Gel immédiat (SIGSTOP) puis destruction physique (SIGKILL) */
    kill((pid_t)pid, SIGSTOP);
    int kill_res = kill((pid_t)pid, SIGKILL);

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    if (kill_res != 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[ÉCHEC DE TIR] PID %u introuvable ou erreur : %s\033[0m",
                 pid, strerror(errno));
        ctx->status_msg_expire = now + 3500;
        return -1;
    }

    long latency_us = (t_end.tv_sec - t_start.tv_sec) * 1000000L +
                      (t_end.tv_nsec - t_start.tv_nsec) / 1000L;
    if (latency_us < 1) latency_us = 1;

    /* 4. Enregistrement de la pierre tombale rémanente */
    for (size_t t = 0; t < NETOP_MAX_TOMBSTONES; t++) {
        if (!s_tombstones[t].active) {
            s_tombstones[t].pid = pid;
            snprintf(s_tombstones[t].remote_ip, sizeof(s_tombstones[t].remote_ip), "%s", target->remote_ip);
            s_tombstones[t].remote_port = target->remote_port;
            s_tombstones[t].expire_ms   = now + NETOP_TOMBSTONE_TTL_MS;
            s_tombstones[t].active      = true;
            break;
        }
    }

    /* Mutation immédiate de la ligne ciblée pour le cycle d'affichage en cours */
    target->severity = NETOP_SEV_KILLED;
    snprintf(target->comm, sizeof(target->comm), "[ABATTU]");
    snprintf(target->state, sizeof(target->state), "DISJONCTÉ");
    snprintf(target->verdict, sizeof(target->verdict), "NEUTRALISÉ (SIGKILL)");

    /* 5. Production du message médico-légal en barre d'état */
    snprintf(ctx->status_msg, sizeof(ctx->status_msg),
             "\033[1;38;5;208m[FRAPPE RÉUSSIE]\033[0m PID \033[1;37m%u\033[0m abattu via \033[1;31mSIGKILL\033[0m | Latence : \033[1;32m%ld µs\033[0m",
             pid, latency_us);
    ctx->status_msg_expire = now + 4000;

    return 0;
}
