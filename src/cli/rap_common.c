/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rap_common.c
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Fonctions d'assistance transversales pour le binaire 'rap'.
 *                   Centralise le formatage de sortie, les gardes-fous PID,
 *                   le filtrage mémoire de liste blanche et l'évaluation
 *                   souveraine Net-Guard des sockets IP (Section 11).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Implémentation de la fonction 108 (rap_net_eval_ip).
 *   - Moteur d'évaluation CIDR autonome (RFC 1918 et masques arbitraires /8 à /32).
 *   - Chargement passif sans tas (0 heap) des règles IP de fw_officiel.conf
 *     et fw_custom.conf.
 *   - Qualification des flux (APPROUVÉ, EXTERNE/STANDARD, SUSPECT, HOSTILE).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonctions 98, 99, 107 et 108)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *    98. rap_output_formatter
 *    99. rap_check_safety_pid
 *   107. rap_is_mem_whitelisted
 *   108. rap_net_eval_ip
 * ============================================================================
 */

#include <ctype.h>
#include "rap_cli.h"

#define RAP_WHITELIST_MAX_ENTRIES 256
#define RAP_WHITELIST_ENTRY_LEN   64

#define RAP_NET_RULES_MAX         128

typedef struct {
    uint32_t network;
    uint32_t netmask;
    int      is_hostile;
    char     description[64];
} RapNetIpRule;

static char         g_mem_whitelist[RAP_WHITELIST_MAX_ENTRIES][RAP_WHITELIST_ENTRY_LEN];
static int          g_mem_whitelist_count = 0;
static int          g_mem_whitelist_loaded = 0;

static RapNetIpRule g_net_rules[RAP_NET_RULES_MAX];
static int          g_net_rules_count = 0;
static int          g_net_rules_loaded = 0;

/* Repli embarqué (.rodata) si le fichier de configuration est inaccessible */
static const char * const g_default_whitelist[] = {
    "chromium", "chrome", "firefox", "cinnamon", "csd-", "xed",
    "blueman", "ibus-", "applet.py", "mintUpdate", "terminator", "flameshot", NULL
};

/* ============================================================================
 * FONCTION 98 : rap_output_formatter
 * ============================================================================
 */
void rap_output_formatter(const RapConfig *cfg, const char *c1, const char *c2, const char *c3, const char *c4)
{
    if (!cfg->is_tty || cfg->raw) {
        printf("%s\t%s\t%s\t%s\n",
               c1 ? c1 : "",
               c2 ? c2 : "",
               c3 ? c3 : "",
               c4 ? c4 : "");
    } else {
        printf(" %-8s %-22s %-24s %s\n",
               c1 ? c1 : "",
               c2 ? c2 : "",
               c3 ? c3 : "",
               c4 ? c4 : "");
    }
}

/* ============================================================================
 * FONCTION 99 : rap_check_safety_pid
 * ============================================================================
 */
int rap_check_safety_pid(const RapConfig *cfg, pid_t pid)
{
    if (pid <= 1) {
        fprintf(stderr, "%s[REFUS SÉCURITÉ] Interdiction d'abattre le PID %d (init/systemd)%s\n",
                cfg->is_tty ? C_RED : "", pid, cfg->is_tty ? C_RESET : "");
        return -1;
    }

    if (pid == getpid() || pid == getppid()) {
        fprintf(stderr, "%s[REFUS SÉCURITÉ] Interdiction d'abattre le processus courant ou le shell parent%s\n",
                cfg->is_tty ? C_RED : "", cfg->is_tty ? C_RESET : "");
        return -1;
    }

    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "[ERREUR] PID %d introuvable ou déjà terminé.\n", pid);
        return -1;
    }
    fclose(f);

    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    FILE *fmaps = fopen(path, "r");
    if (fmaps) {
        int first_char = fgetc(fmaps);
        fclose(fmaps);
        if (first_char == EOF) {
            fprintf(stderr, "%s[REFUS SÉCURITÉ] Le PID %d est un thread noyau Linux protégé.%s\n",
                    cfg->is_tty ? C_RED : "", pid, cfg->is_tty ? C_RESET : "");
            return -1;
        }
    }

    return 0;
}

/* ============================================================================
 * FONCTION 107 : rap_is_mem_whitelisted
 * ============================================================================
 */
int rap_is_mem_whitelisted(const char *comm)
{
    if (!comm || comm[0] == '\0') {
        return 0;
    }

    if (!g_mem_whitelist_loaded) {
        const char *paths[] = {
            "assets/rules/mem_whitelist.conf",
            "../assets/rules/mem_whitelist.conf",
            "/etc/raptor/rules/mem_whitelist.conf",
            NULL
        };

        FILE *f = NULL;
        for (int i = 0; paths[i] != NULL; i++) {
            f = fopen(paths[i], "r");
            if (f) break;
        }

        if (f) {
            char line[256];
            while (fgets(line, sizeof(line), f) && g_mem_whitelist_count < RAP_WHITELIST_MAX_ENTRIES) {
                char *ptr = line;
                while (*ptr && isspace((unsigned char)*ptr)) ptr++;
                if (*ptr == '#' || *ptr == '\0') continue;

                char *p1 = strchr(ptr, '|');
                if (!p1) continue;

                char *motif_start = p1 + 1;
                while (*motif_start && isspace((unsigned char)*motif_start)) motif_start++;
                if (*motif_start == '\0' || *motif_start == '|') continue;

                char *p2 = strchr(motif_start, '|');
                char *motif_end = p2 ? (p2 - 1) : (motif_start + strlen(motif_start) - 1);
                while (motif_end >= motif_start && isspace((unsigned char)*motif_end)) {
                    *motif_end = '\0';
                    motif_end--;
                }
                *(motif_end + 1) = '\0';

                if (motif_start[0] != '\0') {
                    snprintf(g_mem_whitelist[g_mem_whitelist_count],
                             RAP_WHITELIST_ENTRY_LEN,
                             "%s", motif_start);
                    g_mem_whitelist_count++;
                }
            }
            fclose(f);
        } else {
            for (int i = 0; g_default_whitelist[i] != NULL && i < RAP_WHITELIST_MAX_ENTRIES; i++) {
                snprintf(g_mem_whitelist[g_mem_whitelist_count],
                         RAP_WHITELIST_ENTRY_LEN,
                         "%s", g_default_whitelist[i]);
                g_mem_whitelist_count++;
            }
        }
        g_mem_whitelist_loaded = 1;
    }

    size_t comm_len = strlen(comm);

    for (int i = 0; i < g_mem_whitelist_count; i++) {
        const char *entry = g_mem_whitelist[i];
        size_t elen = strlen(entry);
        if (elen == 0) continue;

        if (strcmp(comm, entry) == 0) return 1;
        if (entry[elen - 1] == '-' && strncmp(comm, entry, elen) == 0) return 1;
        if (strncmp(comm, entry, elen) == 0 && (comm[elen] == '-' || comm[elen] == '.')) return 1;
        if (comm_len >= 15 && elen >= 15 && strncmp(entry, comm, 15) == 0) return 1;
    }

    return 0;
}

/* ----------------------------------------------------------------------------
 * CHARGEMENT DES FICHIERS DE RÈGLES IP NET-GUARD
 * ----------------------------------------------------------------------------
 */
static void rap_net_load_file(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f) && g_net_rules_count < RAP_NET_RULES_MAX) {
        char *ptr = line;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr == '#' || *ptr == '\0') continue;

        /* Seules les lignes de type IP sont parsées */
        if (strncmp(ptr, "IP", 2) != 0) continue;

        char *p1 = strchr(ptr, '|');
        if (!p1) continue;
        char *p2 = strchr(p1 + 1, '|');
        if (!p2) continue;
        char *p3 = strchr(p2 + 1, '|');

        char cidr_str[64] = {0};
        char status_str[32] = {0};
        char desc_str[64] = {0};

        /* Extraction CIDR */
        char *c_start = p1 + 1;
        while (*c_start && isspace((unsigned char)*c_start)) c_start++;
        char *c_end = p2 - 1;
        while (c_end >= c_start && isspace((unsigned char)*c_end)) { *c_end = '\0'; c_end--; }
        snprintf(cidr_str, sizeof(cidr_str), "%s", c_start);

        /* Extraction Statut */
        char *s_start = p2 + 1;
        while (*s_start && isspace((unsigned char)*s_start)) s_start++;
        char *s_end = p3 ? (p3 - 1) : (s_start + strlen(s_start) - 1);
        while (s_end >= s_start && isspace((unsigned char)*s_end)) { *s_end = '\0'; s_end--; }
        snprintf(status_str, sizeof(status_str), "%s", s_start);

        /* Extraction Description */
        if (p3) {
            char *d_start = p3 + 1;
            while (*d_start && isspace((unsigned char)*d_start)) d_start++;
            char *d_end = d_start + strlen(d_start) - 1;
            while (d_end >= d_start && isspace((unsigned char)*d_end)) { *d_end = '\0'; d_end--; }
            snprintf(desc_str, sizeof(desc_str), "%s", d_start);
        }

        /* Décodage IP / CIDR */
        unsigned int a = 0, b = 0, c = 0, d = 0, mask = 32;
        int parsed = sscanf(cidr_str, "%u.%u.%u.%u/%u", &a, &b, &c, &d, &mask);
        if (parsed < 4) {
            parsed = sscanf(cidr_str, "%u.%u.%u.%u", &a, &b, &c, &d);
            mask = 32;
        }

        if (parsed >= 4 && mask <= 32) {
            uint32_t net = (a << 24) | (b << 16) | (c << 8) | d;
            uint32_t nmask = (mask == 0) ? 0 : (0xFFFFFFFFU << (32 - mask));

            g_net_rules[g_net_rules_count].network = net & nmask;
            g_net_rules[g_net_rules_count].netmask = nmask;
            g_net_rules[g_net_rules_count].is_hostile = (strstr(status_str, "HOSTILE") != NULL);
            snprintf(g_net_rules[g_net_rules_count].description,
                     sizeof(g_net_rules[g_net_rules_count].description),
                     "%s", desc_str[0] ? desc_str : status_str);
            g_net_rules_count++;
        }
    }
    fclose(f);
}

/* ============================================================================
 * FONCTION 108 : rap_net_eval_ip
 * ----------------------------------------------------------------------------
 * Évalue une adresse IP distante et son port :
 *  - Interroge les tables de règles IP Net-Guard (fw_officiel / fw_custom).
 *  - Reconnaît les réseaux privés RFC 1918 et la boucle locale (APPROUVÉ).
 *  - Signale immédiatement les relais C2 ou exfiltration (HOSTILE).
 *  - Qualifie les flux web/standards (HTTPS:443, HTTP:80, DNS:53, SSH:22).
 * ============================================================================
 */
int rap_net_eval_ip(uint32_t ip, uint16_t port, char *out_desc, size_t desc_size)
{
    if (!g_net_rules_loaded) {
        const char *official_paths[] = {
            "assets/rules/fw_officiel.conf",
            "../assets/rules/fw_officiel.conf",
            "/etc/raptor/rules/fw_officiel.conf",
            NULL
        };
        for (int i = 0; official_paths[i]; i++) {
            if (access(official_paths[i], R_OK) == 0) {
                rap_net_load_file(official_paths[i]);
                break;
            }
        }

        const char *custom_paths[] = {
            "assets/rules/fw_custom.conf",
            "../assets/rules/fw_custom.conf",
            "/etc/raptor/rules/fw_custom.conf",
            NULL
        };
        for (int i = 0; custom_paths[i]; i++) {
            if (access(custom_paths[i], R_OK) == 0) {
                rap_net_load_file(custom_paths[i]);
                break;
            }
        }

        g_net_rules_loaded = 1;
    }

    /* 1. Concordance avec les règles du pare-feu Net-Guard */
    for (int i = 0; i < g_net_rules_count; i++) {
        if ((ip & g_net_rules[i].netmask) == g_net_rules[i].network) {
            if (g_net_rules[i].is_hostile) {
                snprintf(out_desc, desc_size, "[HOSTILE] %s", g_net_rules[i].description);
                return RAP_NET_HOSTILE;
            } else {
                snprintf(out_desc, desc_size, "APPROUVÉ (%s)", g_net_rules[i].description);
                return RAP_NET_APPROVED;
            }
        }
    }

    /* 2. Qualification des flux sortants externes selon le port de service */
    if (port == 443) {
        snprintf(out_desc, desc_size, "EXTERNE (HTTPS standard :443)");
        return RAP_NET_STANDARD;
    } else if (port == 80) {
        snprintf(out_desc, desc_size, "EXTERNE (HTTP clair :80)");
        return RAP_NET_STANDARD;
    } else if (port == 53 || port == 853) {
        snprintf(out_desc, desc_size, "EXTERNE (Service DNS :%u)", port);
        return RAP_NET_STANDARD;
    } else if (port == 22) {
        snprintf(out_desc, desc_size, "EXTERNE (Session SSH :22)");
        return RAP_NET_STANDARD;
    } else if (port == 123) {
        snprintf(out_desc, desc_size, "EXTERNE (Horodateur NTP :123)");
        return RAP_NET_STANDARD;
    }

    snprintf(out_desc, desc_size, "SUSPECT (Port non standard :%u)", port);
    return RAP_NET_SUSPECT;
}
