/* ==============================================================================
 * NOM DU SCRIPT : plugin_net_guard.c
 * NUMÉRO DE RÉVISION : Rev 2.2
 * BLOC DESCRIPTIF DES INTÉGRATIONS :
 *   - Implémentation du greffon défensif autonome RAPTOR NET-GUARD.
 *   - Analyse passive et temps réel des connexions actives (IPv4 / TCP / UDP).
 *   - Moteur lexical de détection de combosquatting et usurpation de marques.
 *   - Filtrage binaire d'adresses IP avec masques CIDR et gestion de ports.
 *   - Parsing tolérant et assainissement automatique des registres .conf.
 *   - Sérialisation canonique et persistance locale dans fw_custom.conf.
 *   - Procédure de mise à jour atomique via curl sans dépendance réseau lourde.
 *   - Enregistrement automatique par constructeur GCC sur le bus RaptorPlugin.
 *   - Correctifs de conformité : garde conditionnelle sur _GNU_SOURCE et
 *     suppression de la variable inutilisée test_reg (-Wall -Wextra propre).
 * 
 * LISTE NUMÉROTÉE DES FONCTIONS :
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
 *   [Utilitaire] net_guard_update_official_rules
 * ============================================================================== */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "raptor/raptor_net_guard.h"
#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_data.h"

/* -----------------------------------------------------------------------------
 * ÉTAT GLOBAL ET INSTANCE DU REGISTRE EN MÉMOIRE VIVE
 * ----------------------------------------------------------------------------- */
static NetGuardRegistry g_registry = {0};

static const char* DEFAULT_DOM_KEYWORDS = 
    "login,connexion,auth,verify,securite,compte,verification,update,client,facture";

/* Prototypes internes */
static void net_guard_str_to_lower(char* str);
static bool net_guard_parse_net_file(const char* filepath, const char* proto, RaptorTable* table, size_t* out_crit, size_t* out_appr);

/* ==============================================================================
 * 58. net_guard_trim
 * ============================================================================== */
char* net_guard_trim(char* str) {
    if (!str) return NULL;

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == '\0') {
        return str;
    }

    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    return str;
}

/* Fonction utilitaire interne : conversion minuscule */
static void net_guard_str_to_lower(char* str) {
    if (!str) return;
    for (size_t i = 0; str[i] != '\0'; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

/* ==============================================================================
 * 59. net_guard_validate_ip
 * ============================================================================== */
bool net_guard_validate_ip(const char* ip_str, uint32_t* out_ip, uint32_t* out_mask, uint16_t* out_port) {
    if (!ip_str || !out_ip || !out_mask || !out_port) {
        return false;
    }

    char buffer[NET_GUARD_STR_SM];
    strncpy(buffer, ip_str, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* trimmed = net_guard_trim(buffer);
    if (strlen(trimmed) == 0) return false;

    *out_port = 0;
    *out_mask = 0xFFFFFFFFU;

    char* slash = strchr(trimmed, '/');
    if (slash) {
        *slash = '\0';
        char* cidr_str = slash + 1;
        int cidr = atoi(cidr_str);
        if (cidr < 1 || cidr > 32) {
            return false;
        }
        if (cidr == 32) {
            *out_mask = 0xFFFFFFFFU;
        } else {
            *out_mask = htonl(~((1U << (32 - cidr)) - 1U));
        }
    }

    char* colon = strchr(trimmed, ':');
    if (colon) {
        *colon = '\0';
        int p = atoi(colon + 1);
        if (p < 1 || p > 65535) {
            return false;
        }
        *out_port = (uint16_t)p;
    }

    struct in_addr addr;
    if (inet_pton(AF_INET, trimmed, &addr) != 1) {
        return false;
    }

    *out_ip = addr.s_addr;
    return true;
}

/* ==============================================================================
 * 60. net_guard_load_file
 * ============================================================================== */
bool net_guard_load_file(const char* filepath, bool is_custom_source) {
    if (!filepath) return false;

    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        return false;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char* p = net_guard_trim(line);
        if (*p == '\0' || *p == '#') {
            continue;
        }

        char* type_tok = strtok(p, "|");
        if (!type_tok) continue;
        type_tok = net_guard_trim(type_tok);

        if (strcasecmp(type_tok, "DOM") == 0) {
            char* brand_tok = strtok(NULL, "|");
            char* official_tok = strtok(NULL, "|");
            char* keywords_tok = strtok(NULL, "|");
            char* desc_tok = strtok(NULL, "|");

            if (!brand_tok || !official_tok) continue;

            brand_tok = net_guard_trim(brand_tok);
            official_tok = net_guard_trim(official_tok);
            keywords_tok = keywords_tok ? net_guard_trim(keywords_tok) : NULL;
            desc_tok = desc_tok ? net_guard_trim(desc_tok) : "";

            if (g_registry.dom_count < NET_GUARD_MAX_RULES) {
                NetGuardDomRule* r = &g_registry.dom_rules[g_registry.dom_count];
                memset(r, 0, sizeof(NetGuardDomRule));

                strncpy(r->brand, brand_tok, sizeof(r->brand) - 1);
                net_guard_str_to_lower(r->brand);

                strncpy(r->official_domains, official_tok, sizeof(r->official_domains) - 1);
                net_guard_str_to_lower(r->official_domains);

                if (keywords_tok && strlen(keywords_tok) > 0) {
                    strncpy(r->trigger_keywords, keywords_tok, sizeof(r->trigger_keywords) - 1);
                } else {
                    strncpy(r->trigger_keywords, DEFAULT_DOM_KEYWORDS, sizeof(r->trigger_keywords) - 1);
                }
                net_guard_str_to_lower(r->trigger_keywords);

                strncpy(r->description, desc_tok, sizeof(r->description) - 1);
                r->is_custom = is_custom_source;

                g_registry.dom_count++;
            }
        } else if (strcasecmp(type_tok, "IP") == 0) {
            char* target_tok = strtok(NULL, "|");
            char* status_tok = strtok(NULL, "|");
            char* desc_tok = strtok(NULL, "|");

            if (!target_tok || !status_tok) continue;

            target_tok = net_guard_trim(target_tok);
            status_tok = net_guard_trim(status_tok);
            desc_tok = desc_tok ? net_guard_trim(desc_tok) : "";

            uint32_t bin_ip = 0;
            uint32_t bin_mask = 0;
            uint16_t port = 0;

            if (!net_guard_validate_ip(target_tok, &bin_ip, &bin_mask, &port)) {
                continue;
            }

            if (g_registry.ip_count < NET_GUARD_MAX_RULES) {
                NetGuardIpRule* r = &g_registry.ip_rules[g_registry.ip_count];
                memset(r, 0, sizeof(NetGuardIpRule));

                strncpy(r->raw_target, target_tok, sizeof(r->raw_target) - 1);
                r->ip_binary = bin_ip;
                r->mask_binary = bin_mask;
                r->port = port;

                if (strcasecmp(status_tok, "HOSTILE") == 0) {
                    r->status = NET_GUARD_IP_HOSTILE;
                } else {
                    r->status = NET_GUARD_IP_APPROVED;
                }

                strncpy(r->description, desc_tok, sizeof(r->description) - 1);
                r->is_custom = is_custom_source;

                g_registry.ip_count++;
            }
        }
    }

    fclose(fp);
    return true;
}

/* ==============================================================================
 * 61. net_guard_init
 * ============================================================================== */
bool net_guard_init(void) {
    memset(&g_registry, 0, sizeof(NetGuardRegistry));

    net_guard_load_file(RAPTOR_FILE_RULES_OFFICIAL, false);
    net_guard_load_file(RAPTOR_FILE_RULES_CUSTOM, true);

    g_registry.is_initialized = true;
    return true;
}

/* ==============================================================================
 * 62. net_guard_add_custom_rule
 * ============================================================================== */
bool net_guard_add_custom_rule(NetGuardRuleType type, const char* target, const char* param, const char* desc) {
    if (!target || !param) return false;

    FILE* fp = fopen(RAPTOR_FILE_RULES_CUSTOM, "a+");
    if (!fp) return false;

    if (type == NET_GUARD_TYPE_DOM) {
        fprintf(fp, "DOM | %-16s | %-32s | %-24s | %s\n",
                target, param, DEFAULT_DOM_KEYWORDS, desc ? desc : "Règle personnalisée");
    } else {
        fprintf(fp, "IP  | %-20s | %-10s | %s\n",
                target, param, desc ? desc : "Règle personnalisée");
    }

    fflush(fp);
    fclose(fp);

    return net_guard_init();
}

/* ==============================================================================
 * 63. net_guard_remove_rule
 * ============================================================================== */
bool net_guard_remove_rule(NetGuardRuleType type, size_t index) {
    if (type == NET_GUARD_TYPE_DOM && index >= g_registry.dom_count) return false;
    if (type == NET_GUARD_TYPE_IP && index >= g_registry.ip_count) return false;

    FILE* fp = fopen(RAPTOR_FILE_RULES_CUSTOM, "w");
    if (!fp) return false;

    fprintf(fp, "# ==============================================================================\n");
    fprintf(fp, "# RAPTOR NET-GUARD // REGISTRE LOCAL PERSONNALISÉ (fw_custom.conf)\n");
    fprintf(fp, "# ==============================================================================\n");

    for (size_t i = 0; i < g_registry.dom_count; i++) {
        NetGuardDomRule* r = &g_registry.dom_rules[i];
        if (!r->is_custom) continue;
        if (type == NET_GUARD_TYPE_DOM && i == index) continue;

        fprintf(fp, "DOM | %-16s | %-32s | %-24s | %s\n",
                r->brand, r->official_domains, r->trigger_keywords, r->description);
    }

    for (size_t i = 0; i < g_registry.ip_count; i++) {
        NetGuardIpRule* r = &g_registry.ip_rules[i];
        if (!r->is_custom) continue;
        if (type == NET_GUARD_TYPE_IP && i == index) continue;

        fprintf(fp, "IP  | %-20s | %-10s | %s\n",
                r->raw_target,
                (r->status == NET_GUARD_IP_HOSTILE) ? "HOSTILE" : "APPROUVÉ",
                r->description);
    }

    fflush(fp);
    fclose(fp);

    return net_guard_init();
}

/* ==============================================================================
 * 64. net_guard_match_domain
 * ============================================================================== */
NetGuardVerdict net_guard_match_domain(const char* hostname, char* out_reason, size_t reason_sz) {
    if (!hostname || !g_registry.is_initialized) return NET_MATCH_NONE;

    char host_clean[NET_GUARD_STR_LG];
    strncpy(host_clean, hostname, sizeof(host_clean) - 1);
    host_clean[sizeof(host_clean) - 1] = '\0';
    net_guard_str_to_lower(host_clean);

    for (size_t i = 0; i < g_registry.dom_count; i++) {
        NetGuardDomRule* rule = &g_registry.dom_rules[i];

        if (strstr(host_clean, rule->brand) != NULL) {
            bool is_official = false;
            char off_buf[NET_GUARD_STR_LG];
            strncpy(off_buf, rule->official_domains, sizeof(off_buf) - 1);
            off_buf[sizeof(off_buf) - 1] = '\0';

            char* off_tok = strtok(off_buf, ",");
            while (off_tok) {
                off_tok = net_guard_trim(off_tok);
                size_t host_len = strlen(host_clean);
                size_t off_len = strlen(off_tok);

                if (strcmp(host_clean, off_tok) == 0) {
                    is_official = true;
                    break;
                }
                if (host_len > off_len && host_clean[host_len - off_len - 1] == '.' &&
                    strcmp(host_clean + (host_len - off_len), off_tok) == 0) {
                    is_official = true;
                    break;
                }
                off_tok = strtok(NULL, ",");
            }

            if (is_official) {
                if (out_reason && reason_sz > 0) {
                    snprintf(out_reason, reason_sz, "Domaine officiel certifié: %s", rule->brand);
                }
                return NET_MATCH_APPROVED;
            }

            char kw_buf[NET_GUARD_STR_LG];
            strncpy(kw_buf, rule->trigger_keywords, sizeof(kw_buf) - 1);
            kw_buf[sizeof(kw_buf) - 1] = '\0';

            char* kw_tok = strtok(kw_buf, ",");
            while (kw_tok) {
                kw_tok = net_guard_trim(kw_tok);
                if (strlen(kw_tok) > 0 && strstr(host_clean, kw_tok) != NULL) {
                    if (out_reason && reason_sz > 0) {
                        snprintf(out_reason, reason_sz, "Combosquatting: Marque '%s' avec '%s'", rule->brand, kw_tok);
                    }
                    return NET_MATCH_CRITICAL;
                }
                kw_tok = strtok(NULL, ",");
            }
        }
    }

    return NET_MATCH_NONE;
}

/* ==============================================================================
 * 65. net_guard_match_ip
 * ============================================================================== */
NetGuardVerdict net_guard_match_ip(const char* remote_ip, uint16_t port, char* out_reason, size_t reason_sz) {
    if (!remote_ip || !g_registry.is_initialized) return NET_MATCH_NONE;

    struct in_addr addr;
    if (inet_pton(AF_INET, remote_ip, &addr) != 1) {
        return NET_MATCH_NONE;
    }

    uint32_t target_ip = addr.s_addr;

    for (size_t i = 0; i < g_registry.ip_count; i++) {
        NetGuardIpRule* rule = &g_registry.ip_rules[i];

        if ((target_ip & rule->mask_binary) == (rule->ip_binary & rule->mask_binary)) {
            if (rule->port != 0 && rule->port != port) {
                continue;
            }

            if (rule->status == NET_GUARD_IP_HOSTILE) {
                if (out_reason && reason_sz > 0) {
                    snprintf(out_reason, reason_sz, "IP HOSTILE: %s", rule->description);
                }
                return NET_MATCH_CRITICAL;
            } else if (rule->status == NET_GUARD_IP_APPROVED) {
                if (out_reason && reason_sz > 0) {
                    snprintf(out_reason, reason_sz, "IP Approuvée: %s", rule->description);
                }
                return NET_MATCH_APPROVED;
            }
        }
    }

    return NET_MATCH_NONE;
}

/* Décodage d'adresse hexadécimale issue de /proc/net/ */
static void net_guard_decode_hex(const char* hex_ip, uint32_t hex_port, char* out_ip, size_t ip_len, uint16_t* out_port) {
    uint32_t ip_val = 0;
    sscanf(hex_ip, "%X", &ip_val);
    struct in_addr in;
    in.s_addr = ip_val;
    strncpy(out_ip, inet_ntoa(in), ip_len - 1);
    out_ip[ip_len - 1] = '\0';
    *out_port = (uint16_t)hex_port;
}

/* Analyse d'un fichier noyau /proc/net/tcp ou /proc/net/udp */
static bool net_guard_parse_net_file(const char* filepath, const char* proto, RaptorTable* table, size_t* out_crit, size_t* out_appr) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return false;

    char line[512];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return false;
    }

    while (fgets(line, sizeof(line), fp)) {
        char rem_addr[64] = {0};
        char loc_addr[64] = {0};
        int state = 0;

        int matched = sscanf(line, "%*d: %63s %63s %X", loc_addr, rem_addr, &state);
        if (matched < 3) continue;

        char* rem_colon = strchr(rem_addr, ':');
        if (!rem_colon) continue;
        *rem_colon = '\0';
        uint32_t rem_port_hex = 0;
        sscanf(rem_colon + 1, "%X", &rem_port_hex);

        char ip_str[32] = {0};
        uint16_t port_val = 0;
        net_guard_decode_hex(rem_addr, rem_port_hex, ip_str, sizeof(ip_str), &port_val);

        if (strcmp(ip_str, "0.0.0.0") == 0 || port_val == 0) {
            continue;
        }

        char reason[NET_GUARD_STR_LG] = {0};
        NetGuardVerdict verdict = net_guard_match_ip(ip_str, port_val, reason, sizeof(reason));

        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%u", port_val);

        const char* status_text = "NEUTRE";
        RaptorSeverity sev = RAPTOR_SEV_OK;

        if (verdict == NET_MATCH_CRITICAL) {
            status_text = "CRITIQUE (HOSTILE)";
            sev = RAPTOR_SEV_CRITICAL;
            (*out_crit)++;
        } else if (verdict == NET_MATCH_APPROVED) {
            status_text = "APPROUVÉ";
            sev = RAPTOR_SEV_OK;
            (*out_appr)++;
        }

        const char* row_cells[] = {
            proto,
            ip_str,
            port_str,
            status_text,
            (strlen(reason) > 0) ? reason : "Trafic réseau standard"
        };
        const RaptorSeverity row_sevs[] = {
            RAPTOR_SEV_OK,
            RAPTOR_SEV_OK,
            RAPTOR_SEV_OK,
            sev,
            sev
        };

        raptor_data_table_row_add(table, row_cells, row_sevs);
    }

    fclose(fp);
    return true;
}

/* ==============================================================================
 * 66. net_guard_run_audit
 * ============================================================================== */
bool net_guard_run_audit(RaptorReportContext* ctx) {
    if (!ctx) return false;

    if (!g_registry.is_initialized) {
        net_guard_init();
    }

    RaptorSection* sec = raptor_data_section_add(ctx, "sec_net_guard", "CONTRÔLE RÉSEAU & DÉFENSE ACTIVE");
    if (!sec) return false;

    size_t crit_count = 0;
    size_t appr_count = 0;

    const char* col_names[] = {"Protocole", "Adresse Distante", "Port", "Statut EDR", "Motif de Détection"};
    RaptorTable* table = raptor_data_table_create(sec, "Flux Réseau et Évaluation Souveraine", 5, col_names);

    net_guard_parse_net_file("/proc/net/tcp", "TCP", table, &crit_count, &appr_count);
    net_guard_parse_net_file("/proc/net/udp", "UDP", table, &crit_count, &appr_count);

    char val_rules[32];
    snprintf(val_rules, sizeof(val_rules), "%zu", g_registry.dom_count + g_registry.ip_count);
    raptor_data_card_add(sec, "RÈGLES CHARGÉES", val_rules, 100.0f, RAPTOR_SEV_OK);

    char val_crit[32];
    snprintf(val_crit, sizeof(val_crit), "%zu", crit_count);
    raptor_data_card_add(sec, "ALERTES CRITIQUES", val_crit, (float)crit_count,
                         (crit_count > 0) ? RAPTOR_SEV_CRITICAL : RAPTOR_SEV_OK);

    char val_appr[32];
    snprintf(val_appr, sizeof(val_appr), "%zu", appr_count);
    raptor_data_card_add(sec, "CONNEXIONS APPROUVÉES", val_appr, (float)appr_count, RAPTOR_SEV_OK);

    if (crit_count > 0) {
        raptor_data_alert_add(sec, RAPTOR_SEV_CRITICAL,
            "Connexion active non conforme identifiée : cible hostile ou tentative d'usurpation.");
    } else {
        raptor_data_alert_add(sec, RAPTOR_SEV_OK,
            "Aucun trafic hostile détecté. Tous les flux sont conformes ou en surveillance neutre.");
    }

    return true;
}

/* ==============================================================================
 * 67. net_guard_cleanup
 * ============================================================================== */
void net_guard_cleanup(void) {
    memset(&g_registry, 0, sizeof(NetGuardRegistry));
}

/* ==============================================================================
 * Utilitaire : net_guard_update_official_rules
 * ============================================================================== */
bool net_guard_update_official_rules(void) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "curl -s -f -m 10 \"%s\" -o \"%s\"",
             RAPTOR_OFFICIAL_RULES_URL, RAPTOR_FILE_RULES_TMP);

    int res = system(cmd);
    if (res != 0) {
        unlink(RAPTOR_FILE_RULES_TMP);
        return false;
    }

    FILE* fp = fopen(RAPTOR_FILE_RULES_TMP, "r");
    if (!fp) return false;

    char line[128];
    bool header_valid = false;
    if (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "RAPTOR NET-GUARD") != NULL) {
            header_valid = true;
        }
    }
    fclose(fp);

    if (!header_valid) {
        unlink(RAPTOR_FILE_RULES_TMP);
        return false;
    }

    if (rename(RAPTOR_FILE_RULES_TMP, RAPTOR_FILE_RULES_OFFICIAL) != 0) {
        unlink(RAPTOR_FILE_RULES_TMP);
        return false;
    }

    return net_guard_init();
}

/* -----------------------------------------------------------------------------
 * INSTANCE DU GREFFON RAPTOR
 * ----------------------------------------------------------------------------- */
static const RaptorPlugin g_net_guard_plugin = {
    .id = "plugin_net_guard",
    .name = "RAPTOR Net-Guard Défensif",
    .version = "2.1",
    .init = net_guard_init,
    .run_audit = net_guard_run_audit,
    .cleanup = net_guard_cleanup
};

/* ==============================================================================
 * 68. net_guard_register
 * ============================================================================== */
__attribute__((constructor))
void net_guard_register(void) {
    raptor_registry_register(&g_net_guard_plugin);
}
