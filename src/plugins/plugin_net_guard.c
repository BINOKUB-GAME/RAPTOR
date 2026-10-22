/* ==============================================================================
 * NOM DU SCRIPT : plugin_net_guard.c
 * NUMÉRO DE RÉVISION : Rev 2.7
 * BLOC DESCRIPTIF DES INTÉGRATIONS :
 *   - Implémentation du greffon défensif autonome RAPTOR NET-GUARD.
 *   - Analyse passive et temps réel des connexions actives (IPv4 / TCP / UDP).
 *   - Moteur lexical de détection de combosquatting et usurpation de marques.
 *   - Filtrage binaire d'adresses IP avec masques CIDR et gestion de ports.
 *   - Parsing tolérant et assainissement automatique des registres .conf.
 *   - Résolution dynamique de chemin d'accès (dépôt Git racine vs dossier build).
 *   - Comparateur différentiel binaire (Diff) anti-cache avec GitHub.
 *   - Module de résolution DNS inverse asynchrone multithread (pthread) avec cache.
 *   - Raccordement au greffon Section 12 (plugin_asn_intel) pour combler les PTR absents.
 *   - Enregistrement automatique par constructeur GCC sur le bus RaptorPlugin.
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
 *   69. net_guard_update_official_rules
 *   70. net_guard_resolve_ip_async
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
#include <time.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>

#include "raptor/raptor_net_guard.h"
#include "raptor/raptor_asn_intel.h"
#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_data.h"

static NetGuardRegistry g_registry = {0};

static const char* DEFAULT_DOM_KEYWORDS = 
    "login,connexion,auth,verify,securite,compte,verification,update,client,facture";

/* -----------------------------------------------------------------------------
 * CACHE DNS ASYNCHRONE ET STRUCTURE MULTITHREAD
 * ----------------------------------------------------------------------------- */
#define NET_GUARD_DNS_CACHE_MAX 128

typedef struct {
    char ip[NET_GUARD_STR_SM];
    char hostname[NET_GUARD_STR_LG];
    bool resolved;
    bool in_progress;
} NetGuardDnsEntry;

static NetGuardDnsEntry g_dns_cache[NET_GUARD_DNS_CACHE_MAX];
static size_t g_dns_cache_count = 0;
static pthread_mutex_t g_dns_mutex = PTHREAD_MUTEX_INITIALIZER;

static void net_guard_str_to_lower(char* str);
static bool net_guard_parse_net_file(const char* filepath, const char* proto, RaptorTable* table, size_t* out_crit, size_t* out_appr);

/* -----------------------------------------------------------------------------
 * UTILITAIRES DE RÉSOLUTION DE CHEMIN D'ACCÈS
 * ----------------------------------------------------------------------------- */
static const char* net_guard_get_official_path(void) {
    if (access("assets/rules/fw_officiel.conf", F_OK) == 0) {
        return "assets/rules/fw_officiel.conf";
    }
    if (access("../assets/rules/fw_officiel.conf", F_OK) == 0) {
        return "../assets/rules/fw_officiel.conf";
    }
    return RAPTOR_FILE_RULES_OFFICIAL;
}

static const char* net_guard_get_custom_path(void) {
    if (access("assets/rules/fw_custom.conf", F_OK) == 0) {
        return "assets/rules/fw_custom.conf";
    }
    if (access("../assets/rules/fw_custom.conf", F_OK) == 0) {
        return "../assets/rules/fw_custom.conf";
    }
    return RAPTOR_FILE_RULES_CUSTOM;
}

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

    net_guard_load_file(net_guard_get_official_path(), false);
    net_guard_load_file(net_guard_get_custom_path(), true);

    g_registry.is_initialized = true;
    return true;
}

/* ==============================================================================
 * 62. net_guard_add_custom_rule
 * ============================================================================== */
bool net_guard_add_custom_rule(NetGuardRuleType type, const char* target, const char* param, const char* desc) {
    if (!target || !param) return false;

    const char* custom_path = net_guard_get_custom_path();
    FILE* fp = fopen(custom_path, "a+");
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

    const char* custom_path = net_guard_get_custom_path();
    FILE* fp = fopen(custom_path, "w");
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

static void net_guard_decode_hex(const char* hex_ip, uint32_t hex_port, char* out_ip, size_t ip_len, uint16_t* out_port) {
    uint32_t ip_val = 0;
    sscanf(hex_ip, "%X", &ip_val);
    struct in_addr in;
    in.s_addr = ip_val;
    strncpy(out_ip, inet_ntoa(in), ip_len - 1);
    out_ip[ip_len - 1] = '\0';
    *out_port = (uint16_t)hex_port;
}

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
    pthread_mutex_lock(&g_dns_mutex);
    g_dns_cache_count = 0;
    pthread_mutex_unlock(&g_dns_mutex);
}

/* Comparateur binaire d'identité de fichier */
static bool net_guard_files_are_identical(const char* file_a, const char* file_b) {
    FILE* fa = fopen(file_a, "rb");
    FILE* fb = fopen(file_b, "rb");
    if (!fa || !fb) {
        if (fa) fclose(fa);
        if (fb) fclose(fb);
        return false;
    }

    bool identical = true;
    char buf_a[512];
    char buf_b[512];

    while (1) {
        size_t ra = fread(buf_a, 1, sizeof(buf_a), fa);
        size_t rb = fread(buf_b, 1, sizeof(buf_b), fb);

        if (ra != rb || memcmp(buf_a, buf_b, ra) != 0) {
            identical = false;
            break;
        }
        if (ra == 0) break;
    }

    fclose(fa);
    fclose(fb);
    return identical;
}

static int net_guard_count_rules_in_file(const char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return 0;

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char* p = net_guard_trim(line);
        if (*p == '#' || *p == '\0') continue;
        if (strncasecmp(p, "DOM", 3) == 0 || strncasecmp(p, "IP", 2) == 0) {
            count++;
        }
    }
    fclose(fp);
    return count;
}

/* ==============================================================================
 * 69. net_guard_update_official_rules
 * ============================================================================== */
NetGuardUpdateStatus net_guard_update_official_rules(int* out_added_lines) {
    if (out_added_lines) *out_added_lines = 0;

    const char* target_official = net_guard_get_official_path();

    char tmp_file[512];
    snprintf(tmp_file, sizeof(tmp_file), "%s.tmp", target_official);

    long now = (long)time(NULL);
    char cmd[768];
    snprintf(cmd, sizeof(cmd),
             "curl -s -f -m 10 -H \"Cache-Control: no-cache\" -H \"Pragma: no-cache\" \"%s?nocache=%ld\" -o \"%s\"",
             RAPTOR_OFFICIAL_RULES_URL, now, tmp_file);

    int res = system(cmd);
    if (res != 0) {
        unlink(tmp_file);
        return NET_UPDATE_ERROR;
    }

    FILE* fp = fopen(tmp_file, "r");
    if (!fp) return NET_UPDATE_ERROR;

    char line[256];
    bool header_valid = false;
    int lines_checked = 0;
    while (fgets(line, sizeof(line), fp) && lines_checked < 10) {
        if (strstr(line, "RAPTOR NET-GUARD") != NULL) {
            header_valid = true;
            break;
        }
        lines_checked++;
    }
    fclose(fp);

    if (!header_valid) {
        unlink(tmp_file);
        return NET_UPDATE_ERROR;
    }

    if (net_guard_files_are_identical(tmp_file, target_official)) {
        unlink(tmp_file);
        return NET_UPDATE_NO_CHANGE;
    }

    int old_count = net_guard_count_rules_in_file(target_official);
    int new_count = net_guard_count_rules_in_file(tmp_file);
    int added = new_count - old_count;
    if (added < 0) added = 0;

    if (rename(tmp_file, target_official) != 0) {
        unlink(tmp_file);
        return NET_UPDATE_ERROR;
    }

    if (!net_guard_init()) {
        return NET_UPDATE_ERROR;
    }

    if (out_added_lines) *out_added_lines = added;
    return NET_UPDATE_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * WORKER THREAD DE RÉSOLUTION DNS INVERSE ET ÉVALUATION ASN
 * ----------------------------------------------------------------------------- */
static void* net_guard_dns_worker(void* arg) {
    char* target_ip = (char*)arg;
    if (!target_ip) return NULL;

    char resolved_name[NET_GUARD_STR_LG] = {0};

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;

    if (inet_pton(AF_INET, target_ip, &sa.sin_addr) == 1) {
        char host[NI_MAXHOST] = {0};
        if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {
            snprintf(resolved_name, sizeof(resolved_name), "%s", host);
        } else {
            uint32_t ip_h = ntohl(sa.sin_addr.s_addr);
            if ((ip_h >= 0xC0A80000 && ip_h <= 0xC0A8FFFF) ||  /* 192.168.0.0/16 */
                (ip_h >= 0x0A000000 && ip_h <= 0x0AFFFFFF) ||  /* 10.0.0.0/8 */
                (ip_h >= 0xAC100000 && ip_h <= 0xAC1FFFFF)) {  /* 172.16.0.0/12 */
                snprintf(resolved_name, sizeof(resolved_name), "Hôte Réseau Local (LAN Privé)");
            } else {
                /* Élimination définitive du vide : cascade table_static.conf -> BGP */
                asn_intel_lookup(target_ip, resolved_name, sizeof(resolved_name));
            }
        }
    }

    pthread_mutex_lock(&g_dns_mutex);
    for (size_t i = 0; i < g_dns_cache_count; i++) {
        if (strcmp(g_dns_cache[i].ip, target_ip) == 0) {
            snprintf(g_dns_cache[i].hostname, sizeof(g_dns_cache[i].hostname), "%s", resolved_name);
            g_dns_cache[i].resolved = true;
            g_dns_cache[i].in_progress = false;
            break;
        }
    }
    pthread_mutex_unlock(&g_dns_mutex);

    free(target_ip);
    return NULL;
}

/* ==============================================================================
 * 70. net_guard_resolve_ip_async
 * ============================================================================== */
bool net_guard_resolve_ip_async(const char* ip_str, char* out_host, size_t out_sz) {
    if (!ip_str || !out_host || out_sz == 0) return false;

    pthread_mutex_lock(&g_dns_mutex);

    for (size_t i = 0; i < g_dns_cache_count; i++) {
        if (strcmp(g_dns_cache[i].ip, ip_str) == 0) {
            if (g_dns_cache[i].resolved) {
                snprintf(out_host, out_sz, "%s", g_dns_cache[i].hostname);
                pthread_mutex_unlock(&g_dns_mutex);
                return true;
            } else {
                snprintf(out_host, out_sz, "Résolution DNS en cours...");
                pthread_mutex_unlock(&g_dns_mutex);
                return false;
            }
        }
    }

    if (g_dns_cache_count < NET_GUARD_DNS_CACHE_MAX) {
        size_t idx = g_dns_cache_count++;
        snprintf(g_dns_cache[idx].ip, sizeof(g_dns_cache[idx].ip), "%s", ip_str);
        g_dns_cache[idx].resolved = false;
        g_dns_cache[idx].in_progress = true;

        pthread_t th;
        char* ip_copy = strdup(ip_str);
        if (pthread_create(&th, NULL, net_guard_dns_worker, ip_copy) == 0) {
            pthread_detach(th);
        } else {
            free(ip_copy);
            g_dns_cache[idx].in_progress = false;
        }
    }

    pthread_mutex_unlock(&g_dns_mutex);
    snprintf(out_host, out_sz, "Résolution DNS en cours...");
    return false;
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
