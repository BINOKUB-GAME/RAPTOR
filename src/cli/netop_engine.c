/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_engine.c
 * RÉVISION      : Rev 1.7 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Moteur d'audit et d'ingestion réseau passif temps réel.
 *                 Lit /proc/net/tcp, résout les propriétaires sans désynchronisation,
 *                 confronte les cibles à la table binaire rap_threat_data.h
 *                 via recherche dichotomique O(log N) et applique les pierres tombales.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.7) :
 *   - Intégration de netop_action_apply_tombstones pour injecter la trace rémanente [ABATTU].
 *   - Prise en compte de NETOP_SEV_KILLED au sommet du tri Float-to-top.
 *   - Maintien du cycle d'état : Hostile -> Orange [ABATTU] -> Vert Purge -> Fin.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 139 à 141)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   139. netop_engine_scan              - Scan complet, confrontation binaire OSINT et tri.
 *   140. netop_engine_build_inode_cache - Indexation ciblée Inode/PID via /proc/[PID]/fd.
 *   141. netop_engine_get_threat_count  - Restitution du volume exact de signatures actives.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "rap_netop.h"
#include "rap_threat_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>

#define RAP_NETOP_PATH_MAX 512

typedef struct {
    unsigned long inode;
    uint32_t      pid;
    char          comm[32];
} InodeMapEntry;

static InodeMapEntry s_inode_cache[NETOP_MAX_ENTRIES];
static size_t        s_inode_count = 0;

static int netop_threat_cmp(const void *key, const void *element)
{
    uint32_t ip_key = *(const uint32_t *)key;
    const RapThreatRecord *rec = (const RapThreatRecord *)element;

    if (ip_key < rec->ip) return -1;
    if (ip_key > rec->ip) return 1;
    return 0;
}

static const RapThreatRecord *netop_lookup_threat(uint32_t ip_host_order)
{
    if (RAP_THREAT_COUNT == 0) return NULL;
    return (const RapThreatRecord *)bsearch(&ip_host_order, g_rap_threat_db,
                                            RAP_THREAT_COUNT, sizeof(RapThreatRecord),
                                            netop_threat_cmp);
}

/* ==============================================================================
 * FONCTION 141 : netop_engine_get_threat_count
 * ============================================================================== */
size_t netop_engine_get_threat_count(void)
{
    (void)RAP_THREAT_DB_BUILD;
    return RAP_THREAT_COUNT;
}

static bool netop_is_target_inode(unsigned long inode, const NetopEntry *entries, size_t count)
{
    if (inode == 0) return false;
    for (size_t i = 0; i < count; i++) {
        if (entries[i].inode == inode) return true;
    }
    return false;
}

/* ==============================================================================
 * FONCTION 140 : netop_engine_build_inode_cache
 * ============================================================================== */
static void netop_engine_build_inode_cache(const NetopEntry *entries, size_t count)
{
    s_inode_count = 0;
    if (count == 0) return;

    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) return;

    struct dirent *proc_entry;
    while ((proc_entry = readdir(proc_dir)) != NULL) {
        if (!isdigit((unsigned char)proc_entry->d_name[0])) {
            continue;
        }

        uint32_t pid = (uint32_t)atoi(proc_entry->d_name);
        if (pid < 1) continue;

        char fd_dir_path[64];
        snprintf(fd_dir_path, sizeof(fd_dir_path), "/proc/%u/fd", pid);
        DIR *fd_dir = opendir(fd_dir_path);
        if (!fd_dir) continue;

        char comm[32] = "-";
        bool comm_loaded = false;

        struct dirent *fd_entry;
        while ((fd_entry = readdir(fd_dir)) != NULL) {
            if (fd_entry->d_name[0] == '.') continue;

            char link_path[RAP_NETOP_PATH_MAX];
            char target[RAP_NETOP_PATH_MAX];
            snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir_path, fd_entry->d_name);

            ssize_t len = readlink(link_path, target, sizeof(target) - 1);
            if (len > 0) {
                target[len] = '\0';
                if (strncmp(target, "socket:[", 8) == 0) {
                    unsigned long inode = strtoul(target + 8, NULL, 10);

                    if (netop_is_target_inode(inode, entries, count)) {
                        if (!comm_loaded) {
                            char comm_path[64];
                            snprintf(comm_path, sizeof(comm_path), "/proc/%u/comm", pid);
                            int fd_comm = open(comm_path, O_RDONLY);
                            if (fd_comm >= 0) {
                                ssize_t n = read(fd_comm, comm, sizeof(comm) - 1);
                                if (n > 0) {
                                    comm[n] = '\0';
                                    for (ssize_t i = 0; i < n; i++) {
                                        if (comm[i] == '\n' || comm[i] == '\r') {
                                            comm[i] = '\0';
                                            break;
                                        }
                                    }
                                }
                                close(fd_comm);
                            }
                            comm_loaded = true;
                        }

                        if (s_inode_count < NETOP_MAX_ENTRIES) {
                            s_inode_cache[s_inode_count].inode = inode;
                            s_inode_cache[s_inode_count].pid   = pid;
                            snprintf(s_inode_cache[s_inode_count].comm,
                                     sizeof(s_inode_cache[s_inode_count].comm),
                                     "%s", comm);
                            s_inode_count++;
                        }
                    }
                }
            }
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
}

static void netop_resolve_inode(unsigned long inode, uint32_t *out_pid, char *out_comm, size_t comm_sz)
{
    *out_pid = 0;
    out_comm[0] = '-';
    out_comm[1] = '\0';

    if (inode == 0) return;

    for (size_t i = 0; i < s_inode_count; i++) {
        if (s_inode_cache[i].inode == inode) {
            *out_pid = s_inode_cache[i].pid;
            snprintf(out_comm, comm_sz, "%s", s_inode_cache[i].comm);
            return;
        }
    }
}

static const char *netop_decode_tcp_state(unsigned int state_hex)
{
    switch (state_hex) {
        case 0x01: return "ESTABLISHED";
        case 0x02: return "SYN_SENT";
        case 0x03: return "SYN_RECV";
        case 0x04: return "FIN_WAIT1";
        case 0x05: return "FIN_WAIT2";
        case 0x06: return "TIME_WAIT";
        case 0x07: return "CLOSE";
        case 0x08: return "CLOSE_WAIT";
        case 0x09: return "LAST_ACK";
        case 0x0A: return "LISTEN";
        case 0x0B: return "CLOSING";
        default:   return "UNKNOWN";
    }
}

/* ==============================================================================
 * FONCTION 139 : netop_engine_scan
 * ============================================================================== */
size_t netop_engine_scan(NetopContext *ctx, NetopEntry *out_entries, size_t max_entries)
{
    if (!ctx || !out_entries || max_entries == 0) return 0;

    FILE *f = fopen("/proc/net/tcp", "r");
    if (!f) return 0;

    char line[256];
    size_t count = 0;

    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return 0;
    }

    while (fgets(line, sizeof(line), f) && count < max_entries) {
        unsigned int local_ip_hex, local_port_hex;
        unsigned int rem_ip_hex, rem_port_hex;
        unsigned int state_hex;
        unsigned long inode = 0;

        int matched = sscanf(line, "%*d: %8X:%4X %8X:%4X %2X %*X:%*X %*X:%*X %*X %*d %*d %lu",
                             &local_ip_hex, &local_port_hex,
                             &rem_ip_hex, &rem_port_hex,
                             &state_hex, &inode);

        if (matched >= 6) {
            NetopEntry *e = &out_entries[count];
            memset(e, 0, sizeof(NetopEntry));

            e->inode = inode;

            snprintf(e->local_ip, sizeof(e->local_ip), "%u.%u.%u.%u",
                     local_ip_hex & 0xFF,
                     (local_ip_hex >> 8) & 0xFF,
                     (local_ip_hex >> 16) & 0xFF,
                     (local_ip_hex >> 24) & 0xFF);
            e->local_port = (uint16_t)local_port_hex;

            snprintf(e->remote_ip, sizeof(e->remote_ip), "%u.%u.%u.%u",
                     rem_ip_hex & 0xFF,
                     (rem_ip_hex >> 8) & 0xFF,
                     (rem_ip_hex >> 16) & 0xFF,
                     (rem_ip_hex >> 24) & 0xFF);
            e->remote_port = (uint16_t)rem_port_hex;

            const char *st_str = netop_decode_tcp_state(state_hex);
            snprintf(e->state, sizeof(e->state), "%s", st_str);

            count++;
        }
    }
    fclose(f);

    netop_engine_build_inode_cache(out_entries, count);

    ctx->total_entries     = count;
    ctx->established_count = 0;
    ctx->listen_count      = 0;
    ctx->hostile_count     = 0;

    for (size_t i = 0; i < count; i++) {
        NetopEntry *e = &out_entries[i];

        netop_resolve_inode(e->inode, &e->pid, e->comm, sizeof(e->comm));

        if (strcmp(e->state, "ESTABLISHED") == 0) ctx->established_count++;
        if (strcmp(e->state, "LISTEN") == 0)      ctx->listen_count++;

        if (e->pid == 0) {
            if (strcmp(e->state, "TIME_WAIT") == 0) {
                snprintf(e->comm, sizeof(e->comm), "[kernel/wait]");
            } else if (strcmp(e->state, "LAST_ACK") == 0 || strcmp(e->state, "CLOSE_WAIT") == 0) {
                snprintf(e->comm, sizeof(e->comm), "[fermeture]");
            } else if (strcmp(e->state, "ESTABLISHED") == 0) {
                snprintf(e->comm, sizeof(e->comm), "[éphémère]");
            }
        }

        unsigned int r_b1 = 0, r_b2 = 0, r_b3 = 0, r_b4 = 0;
        sscanf(e->remote_ip, "%u.%u.%u.%u", &r_b1, &r_b2, &r_b3, &r_b4);
        uint32_t eval_ip = (r_b1 << 24) | (r_b2 << 16) | (r_b3 << 8) | r_b4;

        bool is_loopback = (strncmp(e->local_ip, "127.", 4) == 0) || (strncmp(e->remote_ip, "127.", 4) == 0);
        bool is_listen   = (strcmp(e->state, "LISTEN") == 0);

        if (strcmp(e->state, "TIME_WAIT") == 0) {
            e->severity = NETOP_SEV_OK;
            snprintf(e->verdict, sizeof(e->verdict), "PURGE NOYAU (60s)");
        } else if (strcmp(e->state, "LAST_ACK") == 0 ||
                   strcmp(e->state, "CLOSE_WAIT") == 0 ||
                   strcmp(e->state, "FIN_WAIT1") == 0 ||
                   strcmp(e->state, "FIN_WAIT2") == 0) {
            e->severity = NETOP_SEV_OK;
            snprintf(e->verdict, sizeof(e->verdict), "FERMETURE");
        } else if (is_listen || eval_ip == 0 || is_loopback) {
            e->severity = NETOP_SEV_OK;
            snprintf(e->verdict, sizeof(e->verdict), "%s", is_listen ? "LOCAL ÉCOUTE" : "LOCAL");
        } else {
            const RapThreatRecord *threat = netop_lookup_threat(eval_ip);
            if (threat != NULL) {
                e->severity = NETOP_SEV_HOSTILE;
                if (threat->category == RAP_THREAT_CAT_C2) {
                    snprintf(e->verdict, sizeof(e->verdict), "MATCH HOSTILE (C2)");
                } else if (threat->category == RAP_THREAT_CAT_TOR) {
                    snprintf(e->verdict, sizeof(e->verdict), "RELAIS TOR (HOSTILE)");
                } else {
                    snprintf(e->verdict, sizeof(e->verdict), "MATCH HOSTILE (%u/100)", threat->score);
                }
                ctx->hostile_count++;
            } else {
                char desc_buf[64] = "";
                int verdict_code = rap_net_eval_ip(eval_ip, e->remote_port, desc_buf, sizeof(desc_buf));

                if (verdict_code == RAP_NET_APPROVED) {
                    e->severity = NETOP_SEV_OK;
                    snprintf(e->verdict, sizeof(e->verdict), "%s", desc_buf[0] ? desc_buf : "APPROUVÉ");
                } else if (verdict_code == RAP_NET_SUSPECT) {
                    e->severity = NETOP_SEV_WARN;
                    snprintf(e->verdict, sizeof(e->verdict), "PORT SUSPECT (%u)", e->remote_port);
                } else {
                    if (strcmp(e->state, "ESTABLISHED") == 0) {
                        e->severity = NETOP_SEV_INFO;
                        snprintf(e->verdict, sizeof(e->verdict), "CONNECTÉ");
                    } else {
                        e->severity = NETOP_SEV_OK;
                        snprintf(e->verdict, sizeof(e->verdict), "EN ATTENTE");
                    }
                }
            }
        }
    }

    /* Injection des traces rémanentes orange [ABATTU] */
    netop_action_apply_tombstones(out_entries, count);

    /* Tri Float-to-top : KILLED > HOSTILE > WARN > ESTABLISHED > LISTEN */
    for (size_t i = 1; i < count; i++) {
        NetopEntry key = out_entries[i];
        int j = (int)i - 1;

        while (j >= 0 && out_entries[j].severity < key.severity) {
            out_entries[j + 1] = out_entries[j];
            j--;
        }
        out_entries[j + 1] = key;
    }

    return count;
}
