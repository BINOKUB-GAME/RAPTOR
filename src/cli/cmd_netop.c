/* ==============================================================================
 * NOM DU SCRIPT : src/cli/cmd_netop.c
 * RÉVISION      : Rev 1.13 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * DESCRIPTION   : Contrôleur TUI principal pour 'rap netop'. Gère la boucle de
 *                 veille non-bloquante sur STDIN et le rendu graphique.
 *                 Délègue 100% des commandes et des transformations aux greffons.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.13) :
 *   - Intégration de s_raw_entries (snapshot brut pour re-tri et re-filtrage en pause).
 *   - Application de netop_cmd_sort_apply et netop_cmd_filter_apply sur la vue.
 *   - Télémétrie d'en-tête enrichie : affichage des modes [Tri] et [Filtre] actifs.
 *   - Zéro allocation dynamique (0-heap flicker-free).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 136 à 138)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   136. rap_cmd_netop    - Point d'entrée principal de la commande 'netop'.
 *   137. netop_tui_init   - Bascule du terminal en mode brut et masquage curseur.
 *   138. netop_tui_render - Rendu graphique de l'en-tête et de la table matricielle.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "rap_cli.h"
#include "rap_netop.h"
#include "netop_router.h"
#include "netop_cmd_sort.h"
#include "netop_cmd_filter.h"
#include "rap_oom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <signal.h>

static NetopContext *g_active_ctx = NULL;

static void netop_signal_handler(int sig)
{
    (void)sig;
    if (g_active_ctx && g_active_ctx->raw_mode_active) {
        printf("\033[?25h\033[0m\n");
        tcsetattr(STDIN_FILENO, TCSANOW, &g_active_ctx->orig_termios);
        g_active_ctx->raw_mode_active = false;
    }
    _exit(0);
}

/* ==============================================================================
 * FONCTION 137 : netop_tui_init
 * ============================================================================== */
int netop_tui_init(NetopContext *ctx)
{
    if (!ctx) return -1;

    g_active_ctx = ctx;

    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "rap netop: Erreur, exécution requise sur un terminal TTY interactif.\n");
        return -1;
    }

    if (tcgetattr(STDIN_FILENO, &ctx->orig_termios) != 0) {
        perror("tcgetattr");
        return -1;
    }

    struct termios raw = ctx->orig_termios;
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= (tcflag_t)~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_cflag |= (tcflag_t)(CS8);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
        perror("tcsetattr");
        return -1;
    }

    ctx->raw_mode_active = true;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = netop_signal_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    printf("\033[?25l\033[2J");
    fflush(stdout);

    return 0;
}

static void netop_tui_cleanup(NetopContext *ctx)
{
    if (ctx && ctx->raw_mode_active) {
        printf("\033[?25h\033[0m\n");
        tcsetattr(STDIN_FILENO, TCSANOW, &ctx->orig_termios);
        ctx->raw_mode_active = false;
    }
}

/* ==============================================================================
 * FONCTION 138 : netop_tui_render
 * ============================================================================== */
void netop_tui_render(NetopContext *ctx, const NetopEntry *entries, size_t count)
{
    if (!ctx) return;

    if (netop_router_render_overlay(ctx)) {
        return;
    }

    printf("\033[H");

    /* 1. Bandeau supérieur de télémétrie dynamique */
    printf("\033[1;37mFlux Réseau : \033[1;32m%zu capturés\033[0m (\033[1;36m%zu visibles\033[0m), \033[36m%zu établis\033[0m, \033[37m%zu écoute\033[0m, \033[1;31m%zu HOSTILE\033[0m\033[K\n",
           ctx->total_entries, count, ctx->established_count, ctx->listen_count, ctx->hostile_count);

    printf("\033[1;37mBase OSINT  : \033[33m%zu IP (Active)\033[0m | \033[1;37mRéflexe : \033[1;31mARMÉ\033[0m | \033[1;37mSurvie OOM : \033[32m-1000\033[0m | \033[1;37mStatut : \033[1;%sm%s\033[0m\033[K\n",
           netop_engine_get_threat_count(), ctx->paused ? "33" : "32", ctx->paused ? "PAUSE" : "TEMPS RÉEL");

    printf("\033[1;37mRAPTOR v2.0 : \033[36m0-Heap\033[0m | \033[37mTri : \033[1;33m[%s]\033[0m | \033[37mFiltre : \033[1;36m[%s]\033[0m | \033[37mLignes : \033[32m%d\033[0m | \033[37mCadence : \033[36m%.1fs\033[0m\033[K\n\033[K\n",
           netop_cmd_sort_get_name(), netop_cmd_filter_get_name(),
           ctx->max_display_lines, (double)ctx->refresh_delay_ms / 1000.0);

    /* 2. En-tête des colonnes */
    printf("\033[7m N°  | PID   | COMMANDE         | IP LOCALE:PORT       | IP DISTANTE:PORT     | STATUT      | VERDICT OSINT            \033[0m\033[K\n");

    /* 3. Lignes de données réelles */
    int lines_to_show = ctx->max_display_lines;
    if ((size_t)lines_to_show > count) {
        lines_to_show = (int)count;
    }

    for (int i = 0; i < lines_to_show; i++) {
        const NetopEntry *e = &entries[i];

        char local_endpoint[64];
        char remote_endpoint[64];
        snprintf(local_endpoint, sizeof(local_endpoint), "%s:%u", e->local_ip, e->local_port);
        snprintf(remote_endpoint, sizeof(remote_endpoint), "%s:%u", e->remote_ip, e->remote_port);

        const char *color_prefix = "\033[0m";
        if (e->severity == NETOP_SEV_KILLED) {
            color_prefix = "\033[1;38;5;208m";
        } else if (e->severity == NETOP_SEV_HOSTILE) {
            color_prefix = "\033[1;41;37m";
        } else if (e->severity == NETOP_SEV_WARN) {
            color_prefix = "\033[1;33m";
        } else if (strcmp(e->state, "TIME_WAIT") == 0 || strcmp(e->verdict, "PURGE NOYAU (60s)") == 0) {
            color_prefix = "\033[0;32m";
        } else if (strcmp(e->state, "LISTEN") == 0 || strcmp(e->verdict, "LOCAL ÉCOUTE") == 0) {
            color_prefix = "\033[0;90m";
        } else if (e->severity == NETOP_SEV_OK && strcmp(e->verdict, "LOCAL") != 0) {
            color_prefix = "\033[1;32m";
        } else if (e->severity == NETOP_SEV_INFO) {
            color_prefix = "\033[0;36m";
        }

        printf("%s#%02d | %-5u | %-16.16s | %-20.20s | %-20.20s | %-11.11s | %-24.24s\033[0m\033[K\n",
               color_prefix,
               i + 1,
               e->pid,
               e->comm,
               local_endpoint,
               remote_endpoint,
               e->state,
               e->verdict);
    }

    for (int i = lines_to_show; i < ctx->max_display_lines; i++) {
        printf("\033[K\n");
    }

    /* 4. Barre d'état ou prompt unifié avec purge basse */
    printf("\033[1;30m-----------------------------------------------------------------------------------------------------\033[0m\033[K\n");
    printf("%s\033[K\n\033[J", netop_router_get_footer(ctx));
    fflush(stdout);
}

/* ==============================================================================
 * FONCTION 136 : rap_cmd_netop
 * ============================================================================== */
int rap_cmd_netop(int argc, char **argv, const RapConfig *cfg)
{
    NetopContext ctx;
    memset(&ctx, 0, sizeof(ctx));

    ctx.max_display_lines = NETOP_DEFAULT_LINES;
    ctx.refresh_delay_ms  = NETOP_DEFAULT_DELAY;
    ctx.running           = true;
    ctx.paused            = false;

    if (cfg && cfg->limit > 0 && cfg->limit <= NETOP_MAX_ENTRIES) {
        ctx.max_display_lines = cfg->limit;
    }

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-n", 2) == 0) {
            int lines = 0;
            if (strlen(argv[i]) > 2) {
                lines = atoi(argv[i] + 2);
            } else if (i + 1 < argc) {
                lines = atoi(argv[++i]);
            }
            if (lines > 0 && lines <= NETOP_MAX_ENTRIES) {
                ctx.max_display_lines = lines;
            }
        }
    }

    rap_oom_protect();

    netop_router_init();

    if (netop_tui_init(&ctx) != 0) {
        return RAP_EXIT_ERROR;
    }

    /* Tampon brut 0-heap préservé pour les opérations en mode pause */
    static NetopEntry s_raw_entries[NETOP_MAX_ENTRIES];
    static NetopEntry s_display_entries[NETOP_MAX_ENTRIES];
    size_t raw_count = 0;
    size_t display_count = 0;

    struct pollfd pfd;
    pfd.fd     = STDIN_FILENO;
    pfd.events = POLLIN;

    while (ctx.running) {
        if (!ctx.paused) {
            raw_count = netop_engine_scan(&ctx, s_raw_entries, NETOP_MAX_ENTRIES);

            memcpy(s_display_entries, s_raw_entries, raw_count * sizeof(NetopEntry));
            display_count = raw_count;

            netop_cmd_sort_apply(s_display_entries, display_count);
            display_count = netop_cmd_filter_apply(s_display_entries, display_count);

            netop_tui_render(&ctx, s_display_entries, display_count);
        }

        int poll_ret = poll(&pfd, 1, ctx.refresh_delay_ms);

        if (poll_ret > 0 && (pfd.revents & POLLIN)) {
            char ch = 0;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                netop_router_handle_key(&ctx, s_display_entries, display_count, ch);

                /* Recalcul immédiat du tri et du filtre sur le snapshot existant */
                memcpy(s_display_entries, s_raw_entries, raw_count * sizeof(NetopEntry));
                display_count = raw_count;

                netop_cmd_sort_apply(s_display_entries, display_count);
                display_count = netop_cmd_filter_apply(s_display_entries, display_count);

                netop_tui_render(&ctx, s_display_entries, display_count);
            }
        }
    }

    netop_tui_cleanup(&ctx);
    return RAP_EXIT_OK;
}
