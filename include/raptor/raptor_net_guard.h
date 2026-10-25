/* ===============================================================================
 * NOM DU SCRIPT : raptor_net_guard.h
 * NUMÉRO DE RÉVISION : Rev 2.3
 * BLOC DESCRIPTIF DES INTÉGRATIONS :
 *   - Couche de définitions et contrats d'API pour le greffon RAPTOR NET-GUARD.
 *   - Structures de stockage en mémoire vive pour les règles DOM et IP.
 *   - Constantes de chemins canoniques et URL GitHub de distribution.
 *   - Énumération NetGuardUpdateStatus pour gestion du différentiel (Diff).
 *   - Fonction 70 : Moteur de résolution DNS inverse asynchrone non-bloquante.
 * 
 * FONCTIONS DÉCLARÉES ET EXPORTÉES :
 *   58. net_guard_trim
 *   59. net_guard_validate_ip
 *   60. net_guard_load_file
 *   61. net_guard_init
 *   62. net_guard_add_custom_rule
 *   63. net_guard_remove_rule
 *   64. net_guard_match_domain
 *   65. net_guard_match_ip
 *   66. net_guard_run_audit
 *   67. net_guard_cleanup
 *   68. net_guard_register
 *   69. net_guard_update_official_rules
 *   70. net_guard_resolve_ip_async
 * ============================================================================== */

#ifndef RAPTOR_NET_GUARD_H
#define RAPTOR_NET_GUARD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "raptor/raptor_data.h"

/* -----------------------------------------------------------------------------
 * CONSTANTES DE CONFIGURATION ET CHEMINS DES REGISTRES
 * ----------------------------------------------------------------------------- */
#define NET_GUARD_MAX_RULES          512
#define NET_GUARD_STR_SM             64
#define NET_GUARD_STR_MD             128
#define NET_GUARD_STR_LG             256

#define RAPTOR_FILE_RULES_OFFICIAL   "assets/rules/fw_officiel.conf"
#define RAPTOR_FILE_RULES_CUSTOM     "assets/rules/fw_custom.conf"
#define RAPTOR_FILE_RULES_TMP        "assets/rules/fw_officiel.conf.tmp"

#define RAPTOR_OFFICIAL_RULES_URL \
    "https://raw.githubusercontent.com/BINOKUB-GAME/RAPTOR/main/assets/rules/fw_officiel.conf"

/* -----------------------------------------------------------------------------
 * ÉNUMÉRATIONS ET TYPAGES DU MOTEUR DÉFENSIF
 * ----------------------------------------------------------------------------- */
typedef enum {
    NET_GUARD_TYPE_DOM = 0,
    NET_GUARD_TYPE_IP  = 1
} NetGuardRuleType;

typedef enum {
    NET_GUARD_IP_HOSTILE  = 0,
    NET_GUARD_IP_APPROVED = 1
} NetGuardIpStatus;

typedef enum {
    NET_MATCH_NONE     = 0,
    NET_MATCH_APPROVED = 1,
    NET_MATCH_CRITICAL = 2
} NetGuardVerdict;

typedef enum {
    NET_UPDATE_ERROR     = 0,
    NET_UPDATE_NO_CHANGE = 1,
    NET_UPDATE_SUCCESS   = 2
} NetGuardUpdateStatus;

/* -----------------------------------------------------------------------------
 * STRUCTURES DE RÈGLES EN MÉMOIRE VIVE
 * ----------------------------------------------------------------------------- */
typedef struct {
    char brand[NET_GUARD_STR_SM];
    char official_domains[NET_GUARD_STR_LG];
    char trigger_keywords[NET_GUARD_STR_LG];
    char description[NET_GUARD_STR_MD];
    bool is_custom;
} NetGuardDomRule;

typedef struct {
    char raw_target[NET_GUARD_STR_SM];
    uint32_t ip_binary;
    uint32_t mask_binary;
    uint16_t port;
    NetGuardIpStatus status;
    char description[NET_GUARD_STR_MD];
    bool is_custom;
} NetGuardIpRule;

typedef struct {
    NetGuardDomRule dom_rules[NET_GUARD_MAX_RULES];
    size_t dom_count;

    NetGuardIpRule ip_rules[NET_GUARD_MAX_RULES];
    size_t ip_count;

    bool is_initialized;
} NetGuardRegistry;

/* -----------------------------------------------------------------------------
 * PROTOTYPES DES FONCTIONS DU DICTIONNAIRE RAPTOR (58 à 70)
 * ----------------------------------------------------------------------------- */
char* net_guard_trim(char* str);
bool net_guard_validate_ip(const char* ip_str, uint32_t* out_ip, uint32_t* out_mask, uint16_t* out_port);
bool net_guard_load_file(const char* filepath, bool is_custom_source);
bool net_guard_init(void);
bool net_guard_add_custom_rule(NetGuardRuleType type, const char* target, const char* param, const char* desc);
bool net_guard_remove_rule(NetGuardRuleType type, size_t index);
NetGuardVerdict net_guard_match_domain(const char* hostname, char* out_reason, size_t reason_sz);
NetGuardVerdict net_guard_match_ip(const char* remote_ip, uint16_t port, char* out_reason, size_t reason_sz);
bool net_guard_run_audit(RaptorReportContext* ctx);
void net_guard_cleanup(void);
void net_guard_register(void);
NetGuardUpdateStatus net_guard_update_official_rules(int* out_added_lines);
bool net_guard_resolve_ip_async(const char* ip_str, char* out_host, size_t out_sz);

#endif /* RAPTOR_NET_GUARD_H */
