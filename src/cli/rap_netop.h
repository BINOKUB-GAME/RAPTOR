/* ==============================================================================
 * NOM DU SCRIPT : src/cli/rap_netop.h
 * RÉVISION      : Rev 1.6 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: En-tête interne du module 'netop', structures de données
 *                 0-heap, signatures TUI et prototypes du greffon d'action cinétique.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.6) :
 *   - Calibrage de status_msg à 256 octets pour éliminer tout risque de troncature ANSI.
 *   - Intégration de NETOP_SEV_KILLED (priorité d'affichage Orange ambré).
 *   - Déclaration des fonctions 142 (netop_action_kill_slot) et 143 (netop_action_apply_tombstones).
 *   - Préservation stricte de l'architecture 0-heap sans allocation dynamique.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 136 à 143)
 * ============================================================================== */

#ifndef RAP_NETOP_H
#define RAP_NETOP_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <termios.h>
#include "rap_cli.h"

#define NETOP_MAX_ENTRIES    256
#define NETOP_DEFAULT_LINES  20
#define NETOP_DEFAULT_DELAY  1000 /* Millisecondes (1.0s) */

typedef enum {
    NETOP_SEV_OK = 0,
    NETOP_SEV_INFO = 1,
    NETOP_SEV_WARN = 2,
    NETOP_SEV_HOSTILE = 3,
    NETOP_SEV_KILLED = 4
} NetopSeverity;

typedef struct {
    uint32_t      pid;
    char          comm[64];
    char          local_ip[48];
    uint16_t      local_port;
    char          remote_ip[48];
    uint16_t      remote_port;
    char          state[16];
    char          verdict[32];
    NetopSeverity severity;
    unsigned long inode;
} NetopEntry;

typedef struct {
    int            max_display_lines;
    int            refresh_delay_ms;
    bool           paused;
    bool           running;
    size_t         total_entries;
    size_t         established_count;
    size_t         listen_count;
    size_t         hostile_count;
    struct termios orig_termios;
    bool           raw_mode_active;
    char           input_buffer[32];
    size_t         input_len;
    bool           kill_prompt_active;
    char           status_msg[256];
    uint64_t       status_msg_expire;
} NetopContext;

/* Prototypes TUI */
int    rap_cmd_netop(int argc, char **argv, const RapConfig *cfg);
int    netop_tui_init(NetopContext *ctx);
void   netop_tui_render(NetopContext *ctx, const NetopEntry *entries, size_t count);

/* Moteur d'audit et d'ingestion réseau */
size_t netop_engine_scan(NetopContext *ctx, NetopEntry *out_entries, size_t max_entries);
size_t netop_engine_get_threat_count(void);

/* Greffon d'action cinétique (k) & pierres tombales */
int    netop_action_kill_slot(NetopContext *ctx, NetopEntry *entries, size_t count, int line_num);
void   netop_action_apply_tombstones(NetopEntry *entries, size_t count);

#endif /* RAP_NETOP_H */
