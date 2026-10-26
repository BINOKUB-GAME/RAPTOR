/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_router.c
 * RÉVISION      : Rev 1.6 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Aiguilleur central des commandes clavier et gestionnaire des vues modales.
 *                 Gère la table statique des greffons, la saisie non-bloquante
 *                 des sélections multiples (virgules, tirets) et le basculement d'écrans.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.6) :
 *   - Enregistrement du greffon 's' (Bascule de Tri / netop_cmd_sort_exec).
 *   - Enregistrement du greffon 'f' (Filtre dynamique / netop_cmd_filter_exec).
 *   - Clavier tactique complet : k, b, s, f, i, Ctrl+B, p, q.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 144 à 146, 151, 152)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   144. netop_router_init           - Initialisation de la table de dispatch statique.
 *   145. netop_router_handle_key     - Routage de la touche ou saisie d'invite multi-tokens.
 *   146. netop_router_get_footer     - Restitution du bandeau de commande ou de prompt.
 *   151. netop_router_render_overlay - Délégation du rendu à la modale active si présente.
 *   152. netop_router_set_overlay    - Activation ou fermeture d'une vue modale avec purge écran.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "netop_router.h"
#include "netop_cmd_kill.h"
#include "netop_cmd_ban.h"
#include "netop_cmd_inspect.h"
#include "netop_cmd_sort.h"
#include "netop_cmd_filter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define ROUTER_MAX_PLUGINS 12

static NetopPlugin           s_plugins[ROUTER_MAX_PLUGINS];
static size_t                s_plugin_count = 0;

static bool                  s_prompt_active = false;
static const NetopPlugin    *s_active_plugin = NULL;
static char                  s_input_buf[64];
static size_t                s_input_len = 0;
static char                  s_footer_buf[256];

static NetopOverlayRenderer  s_active_overlay = NULL;

static int netop_cmd_pause_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg)
{
    (void)entries;
    (void)count;
    (void)arg;
    if (ctx) {
        ctx->paused = !ctx->paused;
    }
    return 0;
}

static uint64_t router_get_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

/* ==============================================================================
 * FONCTION 152 : netop_router_set_overlay
 * ============================================================================== */
void netop_router_set_overlay(NetopOverlayRenderer renderer)
{
    s_active_overlay = renderer;
    printf("\033[2J\033[H");
    fflush(stdout);
}

/* ==============================================================================
 * FONCTION 151 : netop_router_render_overlay
 * ============================================================================== */
bool netop_router_render_overlay(NetopContext *ctx)
{
    if (s_active_overlay != NULL) {
        return s_active_overlay(ctx);
    }
    return false;
}

NetopOverlayRenderer netop_router_get_overlay(void)
{
    return s_active_overlay;
}

/* ==============================================================================
 * FONCTION 144 : netop_router_init
 * ============================================================================== */
void netop_router_init(void)
{
    s_plugin_count   = 0;
    s_prompt_active  = false;
    s_active_plugin  = NULL;
    s_active_overlay = NULL;
    s_input_len      = 0;
    s_input_buf[0]   = '\0';

    /* [k] Neutraliser */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 'k',
        .name         = "Neutraliser",
        .type         = NETOP_CMD_TARGETED,
        .prompt_label = "FRAPPE REFLEXE",
        .handler      = netop_cmd_kill_exec
    };

    /* [b] Ban/Déban */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 'b',
        .name         = "Ban/Déban",
        .type         = NETOP_CMD_TARGETED,
        .prompt_label = "BAN IP / DÉBAN",
        .handler      = netop_cmd_ban_exec
    };

    /* [s] Bascule de Tri */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 's',
        .name         = "Tri",
        .type         = NETOP_CMD_INSTANT,
        .prompt_label = NULL,
        .handler      = netop_cmd_sort_exec
    };

    /* [f] Filtre Dynamique */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 'f',
        .name         = "Filtre",
        .type         = NETOP_CMD_INSTANT,
        .prompt_label = NULL,
        .handler      = netop_cmd_filter_exec
    };

    /* [i] Inspecter */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 'i',
        .name         = "Inspecter",
        .type         = NETOP_CMD_TARGETED,
        .prompt_label = "INSPECTION MÉDICO-LÉGALE",
        .handler      = netop_cmd_inspect_exec
    };

    /* [Ctrl+B] Registre */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 2,
        .name         = "Liste Bans",
        .type         = NETOP_CMD_INSTANT,
        .prompt_label = NULL,
        .handler      = netop_cmd_ban_toggle_list
    };

    /* [p] Pause */
    s_plugins[s_plugin_count++] = (NetopPlugin){
        .key          = 'p',
        .name         = "Pause",
        .type         = NETOP_CMD_INSTANT,
        .prompt_label = NULL,
        .handler      = netop_cmd_pause_exec
    };
}

/* ==============================================================================
 * FONCTION 145 : netop_router_handle_key
 * ============================================================================== */
void netop_router_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch)
{
    if (!ctx) return;

    if (s_prompt_active) {
        if (ch == '\n' || ch == '\r') {
            if (s_input_len > 0 && s_active_plugin && s_active_plugin->handler) {
                s_active_plugin->handler(ctx, entries, count, s_input_buf);
            }
            s_prompt_active = false;
            s_active_plugin = NULL;
            s_input_buf[0]  = '\0';
            s_input_len     = 0;
        } else if (ch == 27) {
            s_prompt_active = false;
            s_active_plugin = NULL;
            s_input_buf[0]  = '\0';
            s_input_len     = 0;
        } else if ((ch == 127 || ch == '\b') && s_input_len > 0) {
            s_input_buf[--s_input_len] = '\0';
        } else if ((isdigit((unsigned char)ch) || ch == ',' || ch == '-' || ch == ' ' ||
                    ch == 'b' || ch == 'B') && s_input_len < sizeof(s_input_buf) - 1) {
            s_input_buf[s_input_len++] = ch;
            s_input_buf[s_input_len]   = '\0';
        }
        return;
    }

    if (s_active_overlay == netop_cmd_inspect_render_modal) {
        if (netop_cmd_inspect_handle_key(ctx, entries, count, ch)) {
            return;
        }
    }

    if (s_active_overlay != NULL) {
        if (ch == 27 || ch == 'q' || ch == 'Q') {
            netop_router_set_overlay(NULL);
            return;
        }
    } else {
        if (ch == 'q' || ch == 'Q' || ch == 27) {
            ctx->running = false;
            return;
        }
    }

    if (ch == ' ') {
        ch = 'p';
    }

    char low_ch = (ch == 2) ? 2 : (char)tolower((unsigned char)ch);

    for (size_t i = 0; i < s_plugin_count; i++) {
        if (s_plugins[i].key == low_ch) {
            if (s_plugins[i].type == NETOP_CMD_INSTANT) {
                if (s_plugins[i].handler) {
                    s_plugins[i].handler(ctx, entries, count, "");
                }
            } else if (s_plugins[i].type == NETOP_CMD_TARGETED) {
                s_prompt_active = true;
                s_active_plugin = &s_plugins[i];
                s_input_len     = 0;
                s_input_buf[0]  = '\0';
            }
            return;
        }
    }
}

/* ==============================================================================
 * FONCTION 146 : netop_router_get_footer
 * ============================================================================== */
const char *netop_router_get_footer(NetopContext *ctx)
{
    uint64_t now = router_get_now_ms();

    if (s_prompt_active && s_active_plugin) {
        snprintf(s_footer_buf, sizeof(s_footer_buf),
                 "\033[1;31m[%s] Entrez le(s) N° : %s_\033[0m",
                 s_active_plugin->prompt_label ? s_active_plugin->prompt_label : "CIBLE",
                 s_input_buf);
        return s_footer_buf;
    }

    if (ctx && ctx->status_msg[0] != '\0' && now < ctx->status_msg_expire) {
        return ctx->status_msg;
    }

    char menu[256] = "";
    size_t offset = 0;

    for (size_t i = 0; i < s_plugin_count; i++) {
        char key_label[16];
        if (s_plugins[i].key == 2) {
            snprintf(key_label, sizeof(key_label), "Ctrl+B");
        } else {
            snprintf(key_label, sizeof(key_label), "%c", s_plugins[i].key);
        }

        int written = snprintf(menu + offset, sizeof(menu) - offset,
                               "\033[36m[%s]\033[0m %s  ",
                               key_label, s_plugins[i].name);
        if (written > 0) {
            offset += (size_t)written;
        }
    }

    const char *quit_label = (s_active_overlay != NULL) ? "Fermer" : "Quitter";
    snprintf(s_footer_buf, sizeof(s_footer_buf),
             "\033[1;37mCOMMANDE : %s\033[36m[q]\033[0m %s  ==> ", menu, quit_label);

    return s_footer_buf;
}
