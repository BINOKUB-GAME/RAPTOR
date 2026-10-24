/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rap_cli.h
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Déclarations communes, structures de configuration,
 *                   codes d'erreur POSIX et prototypes d'interface.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Déclaration de rap_cmd_heur (Fonction 109) pour l'audit heuristique.
 *
 * SECTION DU CAHIER DES CHARGES : SECTIONS 11, 16 & 17
 * ============================================================================
 */

#ifndef RAP_CLI_H
#define RAP_CLI_H

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define RAP_EXIT_OK      0
#define RAP_EXIT_THREAT  1
#define RAP_EXIT_ERROR   2

#define C_RESET   "\033[0m"
#define C_RED     "\033[1;31m"
#define C_GREEN   "\033[1;32m"
#define C_YELLOW  "\033[1;33m"
#define C_CYAN    "\033[1;36m"
#define C_BOLD    "\033[1m"

#define RAP_NET_APPROVED   0
#define RAP_NET_STANDARD   1
#define RAP_NET_SUSPECT    2
#define RAP_NET_HOSTILE    3

typedef struct {
    int quiet;
    int raw;
    int force;
    int verbose;
    int limit;
    int is_tty;
} RapConfig;

typedef int (*RapCmdHandler)(int argc, char **argv, const RapConfig *cfg);

typedef struct {
    const char     *name;
    const char     *alias;
    const char     *description;
    RapCmdHandler   handler;
} RapCliCommand;

/* Fonctions transversales */
int  rap_parse_flags(int *argc, char ***argv, RapConfig *cfg);
int  rap_dispatch_command(int argc, char **argv, const RapConfig *cfg);
void rap_output_formatter(const RapConfig *cfg, const char *c1, const char *c2, const char *c3, const char *c4);
int  rap_check_safety_pid(const RapConfig *cfg, pid_t pid);
int  rap_is_mem_whitelisted(const char *comm);
int  rap_net_eval_ip(uint32_t ip, uint16_t port, char *out_desc, size_t desc_size);

/* Greffons de commandes */
int rap_cmd_status(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_top(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_hunt(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_rx(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_preload(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_heur(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_net(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_kill(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_freeze(int argc, char **argv, const RapConfig *cfg);
int rap_cmd_man(int argc, char **argv, const RapConfig *cfg);

#endif /* RAP_CLI_H */
