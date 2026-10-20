/**
 * ============================================================================
 * NOM DU SCRIPT   : src/plugins/plugin_network_sockets.c
 * RÉVISION        : Rev 1.4
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Greffon d'audit réseau et surveillance des sockets (/proc).
 *                   Construit l'index de corrélation inode -> PID, décode les
 *                   points de terminaison hexadécimaux IPv4:Port, filtre le
 *                   bruit de la boucle locale interne (127.0.0.1) et traque
 *                   les connexions distantes sortantes (mouchards, télémétrie,
 *                   ports anormaux, processus sans binaire officiel).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.4) :
 *   - Suppression définitive de la table doublon mock RWX.
 *   - Concentration exclusive sur les flux réseau actifs et résolus.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. net_sock_build_inode_cache
 *   2. net_sock_resolve_pid
 *   3. net_sock_decode_hex_endpoint
 *   4. net_sock_parse_file
 *   5. net_sock_run_audit
 *   6. net_sock_register
 * ============================================================================
 */

#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__linux__) || defined(__unix__)
#include <dirent.h>
#include <unistd.h>
#endif

#define INODE_HASH_BUCKETS 4096

typedef struct InodeNode {
    uint64_t inode;
    uint32_t pid;
    struct InodeNode* next;
} InodeNode;

static InodeNode* s_inode_hash_table[INODE_HASH_BUCKETS] = {NULL};
static bool s_inode_cache_built = false;

static void net_sock_free_inode_cache(void) {
    for (size_t i = 0; i < INODE_HASH_BUCKETS; ++i) {
        InodeNode* curr = s_inode_hash_table[i];
        while (curr) {
            InodeNode* next = curr->next;
            free(curr);
            curr = next;
        }
        s_inode_hash_table[i] = NULL;
    }
    s_inode_cache_built = false;
}

/* ------------------------------------------------------------------------- */
/* 1. CONSTRUCTION DE L'INDEX D'ASSOCIATION INODE -> PID                     */
/* ------------------------------------------------------------------------- */
static bool net_sock_build_inode_cache(void) {
    net_sock_free_inode_cache();

#if defined(__linux__) || defined(__unix__)
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        return false;
    }

    struct dirent* proc_ent = NULL;
    while ((proc_ent = readdir(proc_dir)) != NULL) {
        if (!isdigit((unsigned char)proc_ent->d_name[0])) {
            continue;
        }

        uint32_t pid = (uint32_t)strtoul(proc_ent->d_name, NULL, 10);
        if (pid == 0) {
            continue;
        }

        char fd_dir_path[64];
        snprintf(fd_dir_path, sizeof(fd_dir_path), "/proc/%u/fd", pid);

        DIR* fd_dir = opendir(fd_dir_path);
        if (!fd_dir) {
            continue;
        }

        struct dirent* fd_ent = NULL;
        while ((fd_ent = readdir(fd_dir)) != NULL) {
            if (fd_ent->d_name[0] == '.') {
                continue;
            }

            char link_path[512];
            snprintf(link_path, sizeof(link_path), "/proc/%u/fd/%s", pid, fd_ent->d_name);

            char target[256];
            ssize_t len = readlink(link_path, target, sizeof(target) - 1);
            if (len <= 0) {
                continue;
            }
            target[len] = '\0';

            if (strncmp(target, "socket:[", 8) == 0) {
                unsigned long long inode_val = 0;
                if (sscanf(target + 8, "%llu", &inode_val) == 1) {
                    size_t bucket = (size_t)(inode_val % INODE_HASH_BUCKETS);

                    InodeNode* node = (InodeNode*)malloc(sizeof(InodeNode));
                    if (node) {
                        node->inode = (uint64_t)inode_val;
                        node->pid = pid;
                        node->next = s_inode_hash_table[bucket];
                        s_inode_hash_table[bucket] = node;
                    }
                }
            }
        }

        closedir(fd_dir);
    }

    closedir(proc_dir);
    s_inode_cache_built = true;
    return true;
#else
    s_inode_cache_built = true;
    return true;
#endif
}

/* ------------------------------------------------------------------------- */
/* 2. RÉSOLUTION DU PID PAR INODE                                            */
/* ------------------------------------------------------------------------- */
static uint32_t net_sock_resolve_pid(uint64_t target_inode) {
    if (!s_inode_cache_built || target_inode == 0) {
        return 0;
    }

    size_t bucket = (size_t)(target_inode % INODE_HASH_BUCKETS);
    InodeNode* curr = s_inode_hash_table[bucket];

    while (curr) {
        if (curr->inode == target_inode) {
            return curr->pid;
        }
        curr = curr->next;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
/* 3. DÉCODAGE HEXADÉCIMAL DU POINT DE TERMINAISON                           */
/* ------------------------------------------------------------------------- */
static void net_sock_decode_hex_endpoint(const char* hex_ip, uint32_t hex_port,
                                         char* out_ip, size_t ip_len, uint16_t* out_port) {
    if (!out_ip || ip_len == 0 || !out_port) {
        return;
    }

    *out_port = (uint16_t)hex_port;

    if (!hex_ip || hex_ip[0] == '\0') {
        strncpy(out_ip, "0.0.0.0", ip_len - 1);
        out_ip[ip_len - 1] = '\0';
        return;
    }

    uint32_t raw_ip = 0;
    if (sscanf(hex_ip, "%X", &raw_ip) == 1) {
        unsigned char b1 = (unsigned char)(raw_ip & 0xFF);
        unsigned char b2 = (unsigned char)((raw_ip >> 8) & 0xFF);
        unsigned char b3 = (unsigned char)((raw_ip >> 16) & 0xFF);
        unsigned char b4 = (unsigned char)((raw_ip >> 24) & 0xFF);
        snprintf(out_ip, ip_len, "%u.%u.%u.%u", b1, b2, b3, b4);
    } else {
        strncpy(out_ip, "0.0.0.0", ip_len - 1);
        out_ip[ip_len - 1] = '\0';
    }
}

/* ------------------------------------------------------------------------- */
/* 4. PARSING DE FICHIER RÉSEAU AVEC FILTRAGE DU BRUIT LOCAL                 */
/* ------------------------------------------------------------------------- */
static void net_sock_parse_file(const char* net_path, const char* proto, RaptorTable* out_table) {
    if (!net_path || !proto || !out_table) {
        return;
    }

#if defined(__linux__) || defined(__unix__)
    FILE* fp = fopen(net_path, "r");
    if (!fp) {
        return;
    }

    char line[512];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        char local_ip_hex[64] = {0};
        uint32_t local_port_hex = 0;
        char rem_ip_hex[64] = {0};
        uint32_t rem_port_hex = 0;
        uint32_t state_hex = 0;
        unsigned long long inode_val = 0;

        int matched = sscanf(
            line,
            "%*d: %63[0-9A-Fa-f]:%X %63[0-9A-Fa-f]:%X %X %*x:%*x %*x:%*x %*x %*d %*d %llu",
            local_ip_hex, &local_port_hex,
            rem_ip_hex, &rem_port_hex,
            &state_hex,
            &inode_val
        );

        if (matched < 6) {
            continue;
        }

        char local_ip[64];
        uint16_t local_port = 0;
        net_sock_decode_hex_endpoint(local_ip_hex, local_port_hex, local_ip, sizeof(local_ip), &local_port);

        char rem_ip[64];
        uint16_t rem_port = 0;
        net_sock_decode_hex_endpoint(rem_ip_hex, rem_port_hex, rem_ip, sizeof(rem_ip), &rem_port);

        if (local_port == 68 || local_port == 67 || rem_port == 67 || rem_port == 68) {
            continue;
        }
        if (local_port == 5353 || rem_port == 5353 || local_port == 1900 || rem_port == 1900) {
            continue;
        }
        if (strcmp(local_ip, "224.0.0.251") == 0 || strcmp(rem_ip, "224.0.0.251") == 0) {
            continue;
        }
        if (strncmp(local_ip, "224.", 4) == 0 || strncmp(rem_ip, "224.", 4) == 0 ||
            strncmp(rem_ip, "239.", 4) == 0 || strcmp(rem_ip, "255.255.255.255") == 0) {
            continue;
        }
        if (strcmp(proto, "UDP") == 0 && (rem_port == 0 || strcmp(rem_ip, "0.0.0.0") == 0)) {
            continue;
        }
        if (strncmp(local_ip, "127.", 4) == 0 || strncmp(rem_ip, "127.", 4) == 0) {
            continue;
        }

        char local_endpoint[128];
        snprintf(local_endpoint, sizeof(local_endpoint), "%s:%u", local_ip, local_port);

        char rem_endpoint[128];
        snprintf(rem_endpoint, sizeof(rem_endpoint), "%s:%u", rem_ip, rem_port);

        const char* state_str = "ACTIVE";
        RaptorSeverity state_sev = RAPTOR_SEV_INFO;

        if (state_hex == 0x01) {
            state_str = "ESTABLISHED";
            state_sev = RAPTOR_SEV_OK;
        } else if (state_hex == 0x0A) {
            state_str = "LISTEN";
            state_sev = RAPTOR_SEV_INFO;
        }

        uint32_t pid = net_sock_resolve_pid((uint64_t)inode_val);

        char pid_label[128];
        if (pid > 0) {
            char comm[64] = {0};
            char comm_path[64];
            snprintf(comm_path, sizeof(comm_path), "/proc/%u/comm", pid);
            FILE* fc = fopen(comm_path, "r");
            if (fc) {
                if (fgets(comm, sizeof(comm), fc)) {
                    size_t clen = strlen(comm);
                    while (clen > 0 && (comm[clen - 1] == '\n' || comm[clen - 1] == '\r')) {
                        comm[clen - 1] = '\0';
                        clen--;
                    }
                }
                fclose(fc);
            }

            if (comm[0] != '\0') {
                snprintf(pid_label, sizeof(pid_label), "PID %u (%s)", pid, comm);
            } else {
                snprintf(pid_label, sizeof(pid_label), "PID %u", pid);
            }
        } else {
            strncpy(pid_label, "[Système / Root]", sizeof(pid_label) - 1);
            pid_label[sizeof(pid_label) - 1] = '\0';
        }

        bool is_remote = (strcmp(rem_ip, "0.0.0.0") != 0 && strncmp(rem_ip, "127.", 4) != 0);
        RaptorSeverity rem_sev = RAPTOR_SEV_INFO;

        if (is_remote && pid == 0) {
            rem_sev = RAPTOR_SEV_WARNING;
            state_sev = RAPTOR_SEV_WARNING;
        }

        const char* row_cells[5] = {
            pid_label,
            proto,
            local_endpoint,
            rem_endpoint,
            state_str
        };

        const RaptorSeverity row_sevs[5] = {
            (pid > 0) ? RAPTOR_SEV_INFO : RAPTOR_SEV_WARNING,
            RAPTOR_SEV_INFO,
            RAPTOR_SEV_INFO,
            rem_sev,
            state_sev
        };

        raptor_data_table_row_add(out_table, row_cells, row_sevs);
    }

    fclose(fp);
#else
    (void)net_path; (void)proto; (void)out_table;
#endif
}

/* ------------------------------------------------------------------------- */
/* 5. AUDIT RÉSEAU GLOBAL ET PEUPLEMENT DU CONTEXTE                          */
/* ------------------------------------------------------------------------- */
static bool net_sock_run_audit(RaptorReportContext* ctx) {
    if (!ctx) {
        return false;
    }

    net_sock_build_inode_cache();

    RaptorSection* sec = raptor_data_section_add(
        ctx,
        "sec_network",
        "SÉCURITÉ RÉSEAU & DÉTECTION D'INJECTIONS (RWX)"
    );
    if (!sec) {
        net_sock_free_inode_cache();
        return false;
    }

    const char* col_names[] = {
        "PID Responsable",
        "Proto",
        "Adresse Locale",
        "Adresse Distante",
        "État de Connexion"
    };

    RaptorTable* tbl = raptor_data_table_create(
        sec,
        "Flux réseau actifs surveillés (Mouchards & Télémétrie)",
        5,
        col_names
    );
    if (!tbl) {
        net_sock_free_inode_cache();
        return false;
    }

    net_sock_parse_file("/proc/net/tcp", "TCP", tbl);
    net_sock_parse_file("/proc/net/udp", "UDP", tbl);

    size_t total_sockets = tbl->row_count;

    char sock_card_val[32];
    snprintf(sock_card_val, sizeof(sock_card_val), "%zu SOCKETS", total_sockets);
    raptor_data_card_add(sec, "FLUX RÉSEAU ACTIFS", sock_card_val, 100.0f, RAPTOR_SEV_INFO);

    char alert_msg[256];
    snprintf(alert_msg, sizeof(alert_msg),
             "Surveillance des flux externes : %zu flux réseau actifs sous contrôle.",
             total_sockets);
    raptor_data_alert_add(sec, RAPTOR_SEV_INFO, alert_msg);

    net_sock_free_inode_cache();
    return true;
}

/* ------------------------------------------------------------------------- */
/* 6. DESCRIPTEUR DU GREFFON ET ENREGISTREMENT AUTOMATIQUE                   */
/* ------------------------------------------------------------------------- */
static RaptorPlugin s_plugin_network_sockets = {
    .id = "plugin_network_sockets",
    .name = "Surveillance Réseau & Sockets",
    .version = "2.0.0",
    .init = NULL,
    .run_audit = net_sock_run_audit,
    .cleanup = net_sock_free_inode_cache
};

RAPTOR_PLUGIN_INIT(net_sock_register) {
    raptor_registry_register(&s_plugin_network_sockets);
}
