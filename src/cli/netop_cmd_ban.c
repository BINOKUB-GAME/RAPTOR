/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_ban.c
 * RÉVISION      : Rev 1.5 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Greffon d'isolation réseau 'b' (Ban/Déban multi-sélection).
 *                 - Multi-bannissement groupé (ex: 4,6,9 ou 4 6 9).
 *                 - Débannissement multi-cibles (ex: -4,-6 ou dans Ctrl+B: 1, 2 ou -1 -2).
 *                 - Résolution par instantané (Snapshot) éliminant le bug de décalage d'index.
 *                 - Tolérance syntaxique (virgules, espaces, tirets et préfixes B).
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.5) :
 *   - Snapshot statique 0-heap des index du registre (reg_map) pour multi-déban fiable.
 *   - Support universel des séparateurs d'arguments (", \t").
 *   - Traitement par lots (Batch) avec cumul des succès d'embargo et de réhabilitation.
 *   - Nettoyage d'écran sans résidu dans la modale Ctrl+B.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 147 à 150)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   147. netop_cmd_ban_exec        - Parseur multi-tokens, dispatch ban/déban et synchronisation.
 *   148. netop_cmd_ban_is_banned   - Vérification de présence dans le registre des IP bannies.
 *   149. netop_cmd_ban_toggle_list - Bascule d'affichage de la modale Ctrl+B avec purge écran.
 *   150. netop_cmd_ban_render_list - Rendu graphique du registre des embargos sans résidus.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "netop_cmd_ban.h"
#include "netop_router.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define NETOP_MAX_BANS 32

typedef struct {
    char     ip[48];
    uint64_t timestamp_ms;
    char     method[24];
    int      socket_count;
    bool     active;
} NetopBanRecord;

static NetopBanRecord s_ban_registry[NETOP_MAX_BANS];

static uint64_t netop_ban_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

/* ==============================================================================
 * FONCTION 148 : netop_cmd_ban_is_banned
 * ============================================================================== */
bool netop_cmd_ban_is_banned(const char *remote_ip)
{
    if (!remote_ip || remote_ip[0] == '\0') return false;

    for (size_t i = 0; i < NETOP_MAX_BANS; i++) {
        if (s_ban_registry[i].active && strcmp(s_ban_registry[i].ip, remote_ip) == 0) {
            return true;
        }
    }
    return false;
}

static int run_kernel_cmd(const char *bin1, char *const argv1[], const char *bin2, char *const argv2[])
{
    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        if (bin1) execv(bin1, argv1);
        if (bin2) execv(bin2, argv2);
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) return -1;
    return (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 0 : -1;
}

static int netop_execute_kernel_block(const char *ip_str, char *out_method, size_t method_sz)
{
    char *args_iptables[] = { "iptables", "-I", "INPUT", "-s", (char *)ip_str, "-j", "DROP", NULL };
    int ret = run_kernel_cmd("/usr/sbin/iptables", args_iptables, "/sbin/iptables", args_iptables);

    if (ret == 0) {
        snprintf(out_method, method_sz, "Netfilter (DROP)");
        return 0;
    }

    char *args_ip[] = { "ip", "route", "add", "blackhole", (char *)ip_str, NULL };
    ret = run_kernel_cmd("/usr/sbin/ip", args_ip, "/sbin/ip", args_ip);
    if (ret == 0) {
        snprintf(out_method, method_sz, "Route (Blackhole)");
        return 0;
    }

    return -1;
}

static int netop_execute_kernel_unblock(const char *ip_str)
{
    char *args_iptables_del[] = { "iptables", "-D", "INPUT", "-s", (char *)ip_str, "-j", "DROP", NULL };
    int unblocked_any = -1;

    while (run_kernel_cmd("/usr/sbin/iptables", args_iptables_del, "/sbin/iptables", args_iptables_del) == 0) {
        unblocked_any = 0;
    }

    char *args_ip_del[] = { "ip", "route", "del", "blackhole", (char *)ip_str, NULL };
    if (run_kernel_cmd("/usr/sbin/ip", args_ip_del, "/sbin/ip", args_ip_del) == 0) {
        unblocked_any = 0;
    }

    return unblocked_any;
}

/* ==============================================================================
 * FONCTION 150 : netop_cmd_ban_render_list (Modale Ctrl+B)
 * ============================================================================== */
bool netop_cmd_ban_render_list(NetopContext *ctx)
{
    if (!ctx) return false;

    printf("\033[H");
    printf("\033[2K\033[1;37mRAPTOR v2.0 // \033[1;33mREGISTRE DES EMBARGOS NOYAU (NETFILTER / ROUTE)\033[0m\n");
    printf("\033[2K\033[1;30m-----------------------------------------------------------------------------------------------------\033[0m\n");
    printf("\033[2K\033[7m INDEX | ADRESSE IP DISTANTE | FLUX IMPACTÉS | DURÉE D'EMBARGO | MÉCANISME NOYAU   | ÉTAT ACTUEL             \033[0m\n");

    uint64_t now = netop_ban_now_ms();
    size_t active_count = 0;
    int lines_rendered = 0;

    for (size_t i = 0; i < NETOP_MAX_BANS; i++) {
        if (s_ban_registry[i].active) {
            active_count++;
            uint64_t elapsed_s = (now >= s_ban_registry[i].timestamp_ms) ?
                                 (now - s_ban_registry[i].timestamp_ms) / 1000ULL : 0;
            char duration_str[32];
            snprintf(duration_str, sizeof(duration_str), "Depuis %lus", (unsigned long)elapsed_s);

            char socket_str[24];
            snprintf(socket_str, sizeof(socket_str), "%d socket(s)", s_ban_registry[i].socket_count);

            printf("\033[2K\033[1;33m#B%02zu\033[0m  | %-19.19s | %-13.13s | %-15.15s | %-17.17s | \033[1;31mVERROUILLÉ (BLOQUÉ)\033[0m\n",
                   active_count,
                   s_ban_registry[i].ip,
                   socket_str,
                   duration_str,
                   s_ban_registry[i].method);
            lines_rendered++;
        }
    }

    if (active_count == 0) {
        printf("\033[2K\n");
        printf("\033[2K\033[1;32m      [ AUCUNE ADRESSE IP SOUS EMBARGO ACTIF DANS LE NOYAU - SYSTÈME SOUVERAIN ASSAINI ]\033[0m\n");
        printf("\033[2K\n");
        lines_rendered += 3;
    }

    for (int i = lines_rendered; i < ctx->max_display_lines; i++) {
        printf("\033[2K\n");
    }

    printf("\033[2K\033[1;30m-----------------------------------------------------------------------------------------------------\033[0m\n");
    printf("\033[2K%s\n\033[J", netop_router_get_footer(ctx));
    fflush(stdout);

    return true;
}

/* ==============================================================================
 * FONCTION 149 : netop_cmd_ban_toggle_list
 * ============================================================================== */
int netop_cmd_ban_toggle_list(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg)
{
    (void)ctx;
    (void)entries;
    (void)count;
    (void)arg;

    if (netop_router_get_overlay() == netop_cmd_ban_render_list) {
        netop_router_set_overlay(NULL);
    } else {
        netop_router_set_overlay(netop_cmd_ban_render_list);
    }
    return 0;
}

/* ==============================================================================
 * FONCTION 147 : netop_cmd_ban_exec
 * ============================================================================== */
int netop_cmd_ban_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg)
{
    if (!ctx || !entries || count == 0 || !arg || arg[0] == '\0') return -1;

    if (geteuid() != 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[REFUS]\033[0m Privilèges root requis pour manipuler Netfilter (lancez avec sudo).");
        ctx->status_msg_expire = netop_ban_now_ms() + 4000;
        return -1;
    }

    uint64_t now = netop_ban_now_ms();
    char parse_buf[128];
    snprintf(parse_buf, sizeof(parse_buf), "%s", arg);

    /* --- SNAPSHOT STATIQUE DU REGISTRE DES BANS --- */
    /* Empêche le décalage des index quand plusieurs lignes sont libérées d'un coup */
    int reg_map[NETOP_MAX_BANS];
    size_t reg_active_total = 0;
    for (size_t i = 0; i < NETOP_MAX_BANS; i++) {
        if (s_ban_registry[i].active) {
            reg_map[reg_active_total++] = (int)i;
        }
    }

    char processed_ips[NETOP_MAX_BANS][48];
    size_t processed_count = 0;

    int banned_count   = 0;
    int unbanned_count = 0;
    int already_ok     = 0;
    int error_count    = 0;

    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    bool is_in_overlay = (netop_router_get_overlay() == netop_cmd_ban_render_list);

    /* Découpage multi-séparateurs : virgules, espaces et tabulations */
    char *saveptr = NULL;
    char *token = strtok_r(parse_buf, ", \t", &saveptr);

    while (token != NULL) {
        while (*token == ' ') token++;

        bool is_unban = false;
        if (*token == '-') {
            is_unban = true;
            token++;
            while (*token == ' ') token++;
        }

        bool is_reg_ref = false;
        if (*token == 'b' || *token == 'B') {
            is_reg_ref = true;
            token++;
            while (*token == ' ') token++;
        }

        int num = atoi(token);

        /* A. Débannissement ciblé via index registre (#B01, #B02) ou depuis la modale Ctrl+B */
        if (is_reg_ref || (is_in_overlay && num > 0)) {
            if (num >= 1 && (size_t)num <= reg_active_total) {
                int reg_slot = reg_map[num - 1];

                if (s_ban_registry[reg_slot].active) {
                    char target_ip[48];
                    snprintf(target_ip, sizeof(target_ip), "%s", s_ban_registry[reg_slot].ip);

                    bool already_done = false;
                    for (size_t p = 0; p < processed_count; p++) {
                        if (strcmp(processed_ips[p], target_ip) == 0) {
                            already_done = true;
                            break;
                        }
                    }

                    if (!already_done) {
                        netop_execute_kernel_unblock(target_ip);
                        s_ban_registry[reg_slot].active = false;
                        unbanned_count++;

                        if (processed_count < NETOP_MAX_BANS) {
                            snprintf(processed_ips[processed_count++], sizeof(processed_ips[0]), "%s", target_ip);
                        }

                        for (size_t e = 0; e < count; e++) {
                            if (strcmp(entries[e].remote_ip, target_ip) == 0) {
                                snprintf(entries[e].verdict, sizeof(entries[e].verdict), "LEVÉ (DÉBAN)");
                            }
                        }
                    }
                } else {
                    already_ok++;
                }
            } else {
                error_count++;
            }
        }
        /* B. Traitement direct sur la grille réseau principale */
        else if (num >= 1 && (size_t)num <= count) {
            NetopEntry *target = &entries[num - 1];
            const char *remote_ip = target->remote_ip;

            if (strcmp(remote_ip, "0.0.0.0") == 0 || remote_ip[0] == '\0' ||
                strncmp(remote_ip, "127.", 4) == 0) {
                error_count++;
            } else {
                bool already_done = false;
                for (size_t p = 0; p < processed_count; p++) {
                    if (strcmp(processed_ips[p], remote_ip) == 0) {
                        already_done = true;
                        break;
                    }
                }

                if (is_unban) {
                    if (!netop_cmd_ban_is_banned(remote_ip)) {
                        already_ok++;
                    } else if (!already_done) {
                        netop_execute_kernel_unblock(remote_ip);

                        for (size_t i = 0; i < NETOP_MAX_BANS; i++) {
                            if (s_ban_registry[i].active && strcmp(s_ban_registry[i].ip, remote_ip) == 0) {
                                s_ban_registry[i].active = false;
                                break;
                            }
                        }
                        unbanned_count++;

                        if (processed_count < NETOP_MAX_BANS) {
                            snprintf(processed_ips[processed_count++], sizeof(processed_ips[0]), "%s", remote_ip);
                        }

                        for (size_t e = 0; e < count; e++) {
                            if (strcmp(entries[e].remote_ip, remote_ip) == 0) {
                                snprintf(entries[e].verdict, sizeof(entries[e].verdict), "LEVÉ (DÉBAN)");
                            }
                        }
                    }
                } else {
                    if (netop_cmd_ban_is_banned(remote_ip)) {
                        already_ok++;
                    } else if (!already_done) {
                        char method[24] = "Netfilter";
                        if (netop_execute_kernel_block(remote_ip, method, sizeof(method)) == 0) {
                            int sockets = 0;
                            for (size_t e = 0; e < count; e++) {
                                if (strcmp(entries[e].remote_ip, remote_ip) == 0) {
                                    sockets++;
                                    snprintf(entries[e].verdict, sizeof(entries[e].verdict), "ISOLÉ (BAN IP)");
                                }
                            }

                            for (size_t i = 0; i < NETOP_MAX_BANS; i++) {
                                if (!s_ban_registry[i].active) {
                                    snprintf(s_ban_registry[i].ip, sizeof(s_ban_registry[i].ip), "%s", remote_ip);
                                    snprintf(s_ban_registry[i].method, sizeof(s_ban_registry[i].method), "%s", method);
                                    s_ban_registry[i].timestamp_ms = now;
                                    s_ban_registry[i].socket_count = sockets > 0 ? sockets : 1;
                                    s_ban_registry[i].active = true;
                                    break;
                                }
                            }
                            banned_count++;

                            if (processed_count < NETOP_MAX_BANS) {
                                snprintf(processed_ips[processed_count++], sizeof(processed_ips[0]), "%s", remote_ip);
                            }
                        } else {
                            error_count++;
                        }
                    }
                }
            }
        } else {
            error_count++;
        }

        token = strtok_r(NULL, ", \t", &saveptr);
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    long latency_us = (t_end.tv_sec - t_start.tv_sec) * 1000000L +
                      (t_end.tv_nsec - t_start.tv_nsec) / 1000L;
    if (latency_us < 1) latency_us = 1;

    if (banned_count > 0 && unbanned_count == 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;33m[BAN APPLIQUÉ]\033[0m %d cible(s) isolée(s) | Latence : \033[1;32m%ld µs\033[0m",
                 banned_count, latency_us);
    } else if (unbanned_count > 0 && banned_count == 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;32m[DÉBAN APPLIQUÉ]\033[0m %d cible(s) réhabilitée(s) | Latence : \033[1;32m%ld µs\033[0m",
                 unbanned_count, latency_us);
    } else if (banned_count > 0 && unbanned_count > 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;33m[ACTION MIXTE]\033[0m %d isolée(s), %d réhabilitée(s) | Latence : \033[1;32m%ld µs\033[0m",
                 banned_count, unbanned_count, latency_us);
    } else if (already_ok > 0 && error_count == 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;36m[INFO]\033[0m Cible(s) déjà dans l'état demandé (aucun changement requis).");
    } else {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[ERREUR CIBLE]\033[0m Index de ligne inexistant ou service local inviolable (%d échecs)",
                 error_count);
    }
    ctx->status_msg_expire = now + 4000;

    return 0;
}
