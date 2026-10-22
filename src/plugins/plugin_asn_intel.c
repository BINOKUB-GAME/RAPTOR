/* ==============================================================================
 * NOM DU SCRIPT : src/plugins/plugin_asn_intel.c
 * NUMÉRO DE RÉVISION : Rev 1.1
 * BLOC DESCRIPTIF DES INTÉGRATIONS :
 *   - Implémentation du greffon autonome RAPTOR NET-INTEL / ASN (Section 12).
 *   - Chargement dynamique et à chaud depuis assets/rules/table_static.conf.
 *   - Résolution dynamique de chemin d'accès (dépôt Git racine vs dossier build).
 *   - Calcul de correspondance de masques CIDR en 0 ms.
 *   - Traque BGP de secours par requête DNS TXT pour les réseaux tiers.
 *   - Cache dynamique thread-safe avec exclusion mutuelle (pthread_mutex).
 * 
 * LISTE NUMÉROTÉE DES FONCTIONS :
 *   71. asn_intel_trim
 *   72. asn_intel_parse_cidr
 *   73. asn_intel_load_file
 *   74. asn_intel_match_static
 *   75. asn_intel_query_bgp
 *   76. asn_intel_lookup
 *   77. asn_intel_init
 *   78. asn_intel_cleanup
 *   79. asn_intel_register
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
#include <pthread.h>

#include "raptor/raptor_asn_intel.h"
#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"

/* Tables chargées dynamiquement depuis table_static.conf */
static AsnStaticRange g_static_ranges[ASN_STATIC_MAX_ENTRIES];
static size_t g_static_ranges_count = 0;

/* Cache dynamique de session */
typedef struct {
    char ip[64];
    char label[ASN_INTEL_NAME_MAX];
} AsnCacheEntry;

static AsnCacheEntry g_asn_cache[ASN_INTEL_CACHE_MAX];
static size_t g_asn_cache_count = 0;
static pthread_mutex_t g_asn_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Détection de l'emplacement du fichier de configuration */
static const char* asn_intel_get_conf_path(void) {
    if (access("assets/rules/table_static.conf", F_OK) == 0) {
        return "assets/rules/table_static.conf";
    }
    if (access("../assets/rules/table_static.conf", F_OK) == 0) {
        return "../assets/rules/table_static.conf";
    }
    return RAPTOR_FILE_TABLE_STATIC;
}

/* ==============================================================================
 * 71. asn_intel_trim
 * ============================================================================== */
char* asn_intel_trim(char* str) {
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

/* ==============================================================================
 * 72. asn_intel_parse_cidr
 * ============================================================================== */
bool asn_intel_parse_cidr(const char* cidr_str, uint32_t* out_ip, uint32_t* out_mask) {
    if (!cidr_str || !out_ip || !out_mask) return false;

    char buf[64];
    strncpy(buf, cidr_str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* trimmed = asn_intel_trim(buf);
    char* slash = strchr(trimmed, '/');

    int prefix = 32;
    if (slash) {
        *slash = '\0';
        prefix = atoi(slash + 1);
        if (prefix < 1 || prefix > 32) return false;
    }

    struct in_addr addr;
    if (inet_pton(AF_INET, trimmed, &addr) != 1) {
        return false;
    }

    *out_ip = ntohl(addr.s_addr);
    *out_mask = (prefix == 0) ? 0 : (~0U << (32 - prefix));
    return true;
}

/* ==============================================================================
 * 73. asn_intel_load_file
 * ============================================================================== */
bool asn_intel_load_file(const char* filepath) {
    if (!filepath) return false;

    FILE* fp = fopen(filepath, "r");
    if (!fp) return false;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char* p = asn_intel_trim(line);
        if (*p == '\0' || *p == '#') continue;

        char* type_tok = strtok(p, "|");
        if (!type_tok) continue;
        type_tok = asn_intel_trim(type_tok);

        if (strcasecmp(type_tok, "NET") == 0) {
            char* cidr_tok = strtok(NULL, "|");
            char* asn_tok  = strtok(NULL, "|");
            char* name_tok = strtok(NULL, "|");

            if (!cidr_tok || !asn_tok || !name_tok) continue;

            cidr_tok = asn_intel_trim(cidr_tok);
            asn_tok  = asn_intel_trim(asn_tok);
            name_tok = asn_intel_trim(name_tok);

            uint32_t net_ip = 0, net_mask = 0;
            if (asn_intel_parse_cidr(cidr_tok, &net_ip, &net_mask)) {
                if (g_static_ranges_count < ASN_STATIC_MAX_ENTRIES) {
                    AsnStaticRange* r = &g_static_ranges[g_static_ranges_count];
                    r->network_ip = net_ip;
                    r->netmask = net_mask;
                    snprintf(r->asn_tag, sizeof(r->asn_tag), "%s", asn_tok);
                    snprintf(r->name, sizeof(r->name), "%s", name_tok);
                    g_static_ranges_count++;
                }
            }
        }
    }

    fclose(fp);
    return true;
}

/* ==============================================================================
 * 74. asn_intel_match_static
 * ============================================================================== */
bool asn_intel_match_static(uint32_t ip_host, char* out_buf, size_t out_sz) {
    if (!out_buf || out_sz == 0) return false;

    for (size_t i = 0; i < g_static_ranges_count; ++i) {
        const AsnStaticRange* r = &g_static_ranges[i];
        if ((ip_host & r->netmask) == (r->network_ip & r->netmask)) {
            snprintf(out_buf, out_sz, "[%s] %s", r->asn_tag, r->name);
            return true;
        }
    }

    return false;
}

/* ==============================================================================
 * 75. asn_intel_query_bgp
 * ============================================================================== */
bool asn_intel_query_bgp(const char* ip_str, char* out_buf, size_t out_sz) {
    if (!ip_str || !out_buf || out_sz == 0) return false;

    int o1 = 0, o2 = 0, o3 = 0, o4 = 0;
    if (sscanf(ip_str, "%d.%d.%d.%d", &o1, &o2, &o3, &o4) != 4) {
        return false;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "host -t txt -W 2 %d.%d.%d.%d.origin.asn.cymru.com 2>/dev/null", o4, o3, o2, o1);

    FILE* fp = popen(cmd, "r");
    if (!fp) return false;

    char line[256] = {0};
    bool found = false;

    while (fgets(line, sizeof(line), fp)) {
        char* quote = strchr(line, '"');
        if (quote) {
            quote++;
            char* end_quote = strchr(quote, '"');
            if (end_quote) *end_quote = '\0';

            char* asn_tok = strtok(quote, "|");
            strtok(NULL, "|"); /* ignore prefix */
            char* cc_tok = strtok(NULL, "|");

            if (asn_tok) {
                while (*asn_tok == ' ') asn_tok++;
                char* p_end = asn_tok + strlen(asn_tok) - 1;
                while (p_end > asn_tok && *p_end == ' ') { *p_end = '\0'; p_end--; }

                char country[16] = {0};
                if (cc_tok) {
                    while (*cc_tok == ' ') cc_tok++;
                    char* c_end = cc_tok + strlen(cc_tok) - 1;
                    while (c_end > cc_tok && *c_end == ' ') { *c_end = '\0'; c_end--; }
                    snprintf(country, sizeof(country), " (%s)", cc_tok);
                }

                snprintf(out_buf, out_sz, "[AS%s] Opérateur Réseau Public%s", asn_tok, country);
                found = true;
                break;
            }
        }
    }

    pclose(fp);
    return found;
}

/* ==============================================================================
 * 76. asn_intel_lookup
 * ============================================================================== */
bool asn_intel_lookup(const char* ip_str, char* out_buf, size_t out_sz) {
    if (!ip_str || !out_buf || out_sz == 0) return false;

    /* 1. Cache dynamique */
    pthread_mutex_lock(&g_asn_mutex);
    for (size_t i = 0; i < g_asn_cache_count; ++i) {
        if (strcmp(g_asn_cache[i].ip, ip_str) == 0) {
            snprintf(out_buf, out_sz, "%s", g_asn_cache[i].label);
            pthread_mutex_unlock(&g_asn_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&g_asn_mutex);

    /* 2. Correspondance 0 ms avec table_static.conf */
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str, &addr) == 1) {
        uint32_t ip_h = ntohl(addr.s_addr);
        if (asn_intel_match_static(ip_h, out_buf, out_sz)) {
            pthread_mutex_lock(&g_asn_mutex);
            if (g_asn_cache_count < ASN_INTEL_CACHE_MAX) {
                size_t idx = g_asn_cache_count++;
                snprintf(g_asn_cache[idx].ip, sizeof(g_asn_cache[idx].ip), "%s", ip_str);
                snprintf(g_asn_cache[idx].label, sizeof(g_asn_cache[idx].label), "%s", out_buf);
            }
            pthread_mutex_unlock(&g_asn_mutex);
            return true;
        }
    }

    /* 3. Traque BGP de secours */
    if (asn_intel_query_bgp(ip_str, out_buf, out_sz)) {
        pthread_mutex_lock(&g_asn_mutex);
        if (g_asn_cache_count < ASN_INTEL_CACHE_MAX) {
            size_t idx = g_asn_cache_count++;
            snprintf(g_asn_cache[idx].ip, sizeof(g_asn_cache[idx].ip), "%s", ip_str);
            snprintf(g_asn_cache[idx].label, sizeof(g_asn_cache[idx].label), "%s", out_buf);
        }
        pthread_mutex_unlock(&g_asn_mutex);
        return true;
    }

    snprintf(out_buf, out_sz, "Hôte Public / Sans Déclaration BGP");
    return false;
}

/* ==============================================================================
 * 77. asn_intel_init
 * ============================================================================== */
bool asn_intel_init(void) {
    pthread_mutex_lock(&g_asn_mutex);
    g_static_ranges_count = 0;
    g_asn_cache_count = 0;

    asn_intel_load_file(asn_intel_get_conf_path());

    pthread_mutex_unlock(&g_asn_mutex);
    return true;
}

/* ==============================================================================
 * 78. asn_intel_cleanup
 * ============================================================================== */
void asn_intel_cleanup(void) {
    pthread_mutex_lock(&g_asn_mutex);
    g_static_ranges_count = 0;
    g_asn_cache_count = 0;
    pthread_mutex_unlock(&g_asn_mutex);
}

static bool asn_intel_run_audit(RaptorReportContext* ctx) {
    (void)ctx;
    return true;
}

/* -----------------------------------------------------------------------------
 * INSTANCE DU GREFFON RAPTOR
 * ----------------------------------------------------------------------------- */
static const RaptorPlugin g_asn_intel_plugin = {
    .id = "plugin_asn_intel",
    .name = "RAPTOR Net-Intel (ASN / BGP)",
    .version = "1.0",
    .init = asn_intel_init,
    .run_audit = asn_intel_run_audit,
    .cleanup = asn_intel_cleanup
};

/* ==============================================================================
 * 79. asn_intel_register
 * ============================================================================== */
__attribute__((constructor))
void asn_intel_register(void) {
    raptor_registry_register(&g_asn_intel_plugin);
}
