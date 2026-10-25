/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_net.c
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Greffon d'inspection des communications réseau ('rap net'),
 *                   forensic IP chirurgical et réputation OSINT souveraine.
 *                   - Décodage passif de /proc/net/tcp sans dépendance externe.
 *                   - Recherche dichotomique O(log N) dans la base OSINT (.rodata).
 *                   - Corrélation socket Inode <-> PID et exécutable local (/proc).
 *                   - Enquête unitaire médico-légale (rap net -i <IP>).
 *                   - Consultation et métriques de la base locale (rap net -l).
 *                   - Zéro allocation dynamique (0 heap), silence radio absolu.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Calibrage du tampon link_path (576 octets) dans rap_net_find_proc_by_inode
 *     pour neutraliser l'avertissement de troncature statique (-Wformat-truncation).
 *   - Élargissement préventif du tampon proc_info (80 octets) pour garantir
 *     un affichage net des processus longs sans troncature sous -Wformat=2.
 *
 * SECTION DU CAHIER DES CHARGES : SECTIONS 11 & 16 (Fonctions 94, 123 à 127)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *    94. rap_cmd_net              : Dispatcher principal de la commande 'net'.
 *   123. rap_net_lookup_threat    : Recherche binaire O(log N) dans la base OSINT.
 *   124. rap_net_find_proc_by_inode: Corrélation socket Inode -> PID et binaire local.
 *   125. rap_net_investigate_ip   : Enquête forensique unitaire et fiche d'identité (-i).
 *   126. rap_net_list_threats     : Inventaire sanitaire et dénombrement des flux OSINT (-l).
 *   127. rap_net_list_active      : Surveillance des flux actifs et corrélation temps réel.
 * ============================================================================
 */

#include <dirent.h>
#include <ctype.h>
#include "rap_cli.h"
#include "rap_threat_data.h"

/* Comparateur binaire pour bsearch contre la base .rodata */
static int rap_threat_cmp(const void *a, const void *b)
{
    const uint32_t *key = (const uint32_t *)a;
    const RapThreatRecord *rec = (const RapThreatRecord *)b;
    if (*key < rec->ip) return -1;
    if (*key > rec->ip) return 1;
    return 0;
}

/* ============================================================================
 * FONCTION 123 : rap_net_lookup_threat
 * ----------------------------------------------------------------------------
 * Recherche dichotomique O(log N) dans les 19 927 signatures de la base OSINT.
 * Temps d'exécution : < 50 nanosecondes. Zéro allocation dynamique (0 heap).
 * ============================================================================
 */
static const RapThreatRecord *rap_net_lookup_threat(uint32_t ip)
{
    if (RAP_THREAT_COUNT == 0) return NULL;
    return (const RapThreatRecord *)bsearch(&ip, g_rap_threat_db, RAP_THREAT_COUNT,
                                           sizeof(RapThreatRecord), rap_threat_cmp);
}

/* ============================================================================
 * FONCTION 124 : rap_net_find_proc_by_inode
 * ----------------------------------------------------------------------------
 * Parcourt /proc/[PID]/fd pour retrouver le processus détenteur d'un socket Inode.
 * Récupère le PID et le nom du binaire via /proc/[PID]/comm en mémoire pure.
 * ============================================================================
 */
static int rap_net_find_proc_by_inode(unsigned long target_inode, pid_t *out_pid,
                                      char *out_name, size_t name_size)
{
    if (target_inode == 0) {
        if (out_pid) *out_pid = 0;
        if (out_name && name_size > 0) snprintf(out_name, name_size, "-");
        return 0;
    }

    DIR *dir = opendir("/proc");
    if (!dir) {
        if (out_pid) *out_pid = 0;
        if (out_name && name_size > 0) snprintf(out_name, name_size, "-");
        return 0;
    }

    char target_sock[64];
    snprintf(target_sock, sizeof(target_sock), "socket:[%lu]", target_inode);

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (!isdigit((unsigned char)ent->d_name[0])) continue;

        pid_t pid = (pid_t)atoi(ent->d_name);
        char fd_dir_path[280];
        snprintf(fd_dir_path, sizeof(fd_dir_path), "/proc/%s/fd", ent->d_name);

        DIR *fddir = opendir(fd_dir_path);
        if (!fddir) continue;

        struct dirent *fdent;
        int found = 0;
        while ((fdent = readdir(fddir)) != NULL) {
            if (fdent->d_name[0] == '.') continue;

            /* Tampon calibré à 576 octets pour absorber fd_dir_path (280) + d_name (256) */
            char link_path[576];
            snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir_path, fdent->d_name);

            char dest[128];
            ssize_t len = readlink(link_path, dest, sizeof(dest) - 1);
            if (len > 0) {
                dest[len] = '\0';
                if (strcmp(dest, target_sock) == 0) {
                    found = 1;
                    break;
                }
            }
        }
        closedir(fddir);

        if (found) {
            closedir(dir);
            if (out_pid) *out_pid = pid;
            if (out_name && name_size > 0) {
                char comm_path[280];
                snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);
                FILE *fc = fopen(comm_path, "r");
                if (fc) {
                    if (fgets(out_name, name_size, fc)) {
                        char *nl = strchr(out_name, '\n');
                        if (nl) *nl = '\0';
                    } else {
                        snprintf(out_name, name_size, "pid:%d", pid);
                    }
                    fclose(fc);
                } else {
                    snprintf(out_name, name_size, "pid:%d", pid);
                }
            }
            return 1;
        }
    }
    closedir(dir);

    if (out_pid) *out_pid = 0;
    if (out_name && name_size > 0) snprintf(out_name, name_size, "-");
    return 0;
}

/* Détection rapide des plages locales / privées (RFC 1918 / Loopback) */
static int rap_net_is_private(uint32_t ip)
{
    if ((ip & 0xFF000000U) == 0x0A000000U) return 1; /* 10.0.0.0/8 */
    if ((ip & 0xFF000000U) == 0x7F000000U) return 1; /* 127.0.0.0/8 */
    if ((ip & 0xFFF00000U) == 0xAC100000U) return 1; /* 172.16.0.0/12 */
    if ((ip & 0xFFFF0000U) == 0xC0A80000U) return 1; /* 192.168.0.0/16 */
    if ((ip & 0xFFFF0000U) == 0xA9FE0000U) return 1; /* 169.254.0.0/16 */
    return 0;
}

/* ============================================================================
 * FONCTION 125 : rap_net_investigate_ip
 * ----------------------------------------------------------------------------
 * Fiche médico-légale détaillée d'une adresse IP sans aucune fuite réseau :
 *  - Interrogation de la base de réputation OSINT locale.
 *  - Corrélation avec les flux actifs dans /proc/net/tcp.
 *  - Identification de l'exécutable réel sur disque (/proc/[PID]/exe).
 *  - Génération de règles de neutralisation iptables / nftables.
 * ============================================================================
 */
static int rap_net_investigate_ip(const char *target_ip_str, const RapConfig *cfg)
{
    unsigned int a = 0, b = 0, c = 0, d = 0;
    if (sscanf(target_ip_str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4 ||
        a > 255 || b > 255 || c > 255 || d > 255) {
        fprintf(stderr, "%s[ERREUR] Format IPv4 invalide : '%s'%s\n",
                cfg->is_tty ? C_RED : "", target_ip_str, cfg->is_tty ? C_RESET : "");
        return RAP_EXIT_ERROR;
    }

    uint32_t target_ip = (a << 24) | (b << 16) | (c << 8) | d;
    const RapThreatRecord *rec = rap_net_lookup_threat(target_ip);
    int is_priv = rap_net_is_private(target_ip);

    if (!cfg->quiet) {
        printf("\n%s[RAPTOR NET-INTEL] FICHE FORENSIC // CIBLE : %s%s\n",
               cfg->is_tty ? C_BOLD : "", target_ip_str, cfg->is_tty ? C_RESET : "");
        printf("================================================================================\n");

        if (is_priv) {
            printf("RÉSEAU / STATUT    : %sRÉSEAU LOCAL PRIVÉ (RFC 1918 / Loopback)%s\n",
                   cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
            printf("SCORE DE MENACE    : 0 / 100 (Trafic local interne)\n");
            printf("ÉTIQUETTE OSINT    : Non routable sur Internet public\n");
        } else if (rec) {
            const char *cat_label = "HÔTE HOSTILE (Scanner / Brute-force / Abus)";
            if (rec->category == RAP_THREAT_CAT_C2) {
                cat_label = "SERVEUR C2 / BOTNET & RANÇONGICIEL (abuse.ch)";
            } else if (rec->category == RAP_THREAT_CAT_TOR) {
                cat_label = "RELAIS DE SORTIE TOR (Anonymisation)";
            }

            printf("RÉPUTATION LOCALE  : %sSCORE CRITIQUE [%u / 100]%s\n",
                   cfg->is_tty ? C_RED : "", rec->score, cfg->is_tty ? C_RESET : "");
            printf("CATÉGORIE OSINT    : %s%s%s\n",
                   cfg->is_tty ? C_RED : "", cat_label, cfg->is_tty ? C_RESET : "");
            printf("BASE SOUS-JACENTE  : Consolidée le %s\n", RAP_THREAT_DB_BUILD);
        } else {
            printf("RÉPUTATION LOCALE  : %sNON SIGNALÉE HOSTILE [Score: 0 / 100]%s\n",
                   cfg->is_tty ? C_GREEN : "", cfg->is_tty ? C_RESET : "");
            printf("CATÉGORIE OSINT    : Aucune signature hostile connue dans la base locale\n");
            printf("BASE SOUS-JACENTE  : %zu signatures vérifiées (0 heap)\n", RAP_THREAT_COUNT);
        }

        printf("\nCORRÉLATION LOCALE (FLUX ACTIFS DÉTECTÉS SOUS /PROC) :\n");
    }

    FILE *ftcp = fopen("/proc/net/tcp", "r");
    int active_flows = 0;
    pid_t last_pid = 0;

    if (ftcp) {
        char line[256];
        if (fgets(line, sizeof(line), ftcp)) {
            while (fgets(line, sizeof(line), ftcp)) {
                unsigned int lip = 0, lport = 0, rip = 0, rport = 0, state = 0;
                unsigned long inode = 0;

                int matched = sscanf(line, "%*d: %x:%x %x:%x %x %*s %*s %*s %*s %*s %lu",
                                     &lip, &lport, &rip, &rport, &state, &inode);

                if (matched >= 5) {
                    unsigned int a2 = rip & 0xFF;
                    unsigned int b2 = (rip >> 8) & 0xFF;
                    unsigned int c2 = (rip >> 16) & 0xFF;
                    unsigned int d2 = (rip >> 24) & 0xFF;
                    uint32_t current_rem_ip = (a2 << 24) | (b2 << 16) | (c2 << 8) | d2;

                    if (current_rem_ip == target_ip) {
                        active_flows++;
                        pid_t pid = 0;
                        char comm[64] = {0};
                        rap_net_find_proc_by_inode(inode, &pid, comm, sizeof(comm));
                        if (pid > 0) last_pid = pid;

                        char exe_path[256] = {0};
                        if (pid > 0) {
                            char exe_link[64];
                            snprintf(exe_link, sizeof(exe_link), "/proc/%d/exe", pid);
                            ssize_t elen = readlink(exe_link, exe_path, sizeof(exe_path) - 1);
                            if (elen > 0) exe_path[elen] = '\0';
                            else snprintf(exe_path, sizeof(exe_path), "inaccessible (droits)");
                        }

                        if (!cfg->quiet) {
                            printf("  - Socket #%d        : TCP Port distant :%u (Statut TCP: %u, Inode: %lu)\n",
                                   active_flows, rport, state, inode);
                            printf("    * PID détenteur  : %d (%s)\n", pid, comm);
                            printf("    * Binaire disque : %s\n", exe_path[0] ? exe_path : "inconnu");
                        }
                    }
                }
            }
        }
        fclose(ftcp);
    }

    if (active_flows == 0 && !cfg->quiet) {
        printf("  - Aucun socket actif actuellement connecté vers cette adresse.\n");
    }

    if (!cfg->quiet) {
        printf("\nACTIONS DÉFENSIVES RECOMMANDÉES :\n");
        if (last_pid > 0 && rec) {
            printf("  - Neutraliser le processus : %s./build/rap kill -9 %d%s (ou ./build/rap freeze %d)\n",
                   cfg->is_tty ? C_RED : "", last_pid, cfg->is_tty ? C_RESET : "", last_pid);
        }
        if (rec) {
            printf("  - Bloquer via iptables     : %ssudo iptables -A OUTPUT -d %s -j DROP%s\n",
                   cfg->is_tty ? C_YELLOW : "", target_ip_str, cfg->is_tty ? C_RESET : "");
            printf("  - Bloquer via nftables     : %ssudo nft add rule inet filter output ip daddr %s drop%s\n",
                   cfg->is_tty ? C_YELLOW : "", target_ip_str, cfg->is_tty ? C_RESET : "");
        } else {
            printf("  - Aucune action de blocage requise : hôte non répertorié malveillant.\n");
        }
        printf("================================================================================\n\n");
    }

    if (rec && rec->score >= 70) return RAP_EXIT_THREAT;
    return RAP_EXIT_OK;
}

/* ============================================================================
 * FONCTION 126 : rap_net_list_threats
 * ----------------------------------------------------------------------------
 * Affiche l'état sanitaire de la base OSINT ou déverse les entrées filtrées.
 * Compatible avec le téléscripteur 'rapless' et les filtres Unix standards.
 * ============================================================================
 */
static int rap_net_list_threats(const char *filter, const RapConfig *cfg)
{
    size_t count_c2 = 0;
    size_t count_tor = 0;
    size_t count_abuse = 0;

    for (size_t i = 0; i < RAP_THREAT_COUNT; i++) {
        if (g_rap_threat_db[i].category == RAP_THREAT_CAT_C2) count_c2++;
        else if (g_rap_threat_db[i].category == RAP_THREAT_CAT_TOR) count_tor++;
        else count_abuse++;
    }

    /* Mode récapitulatif sans argument de catégorie */
    if (!filter || strcmp(filter, "") == 0) {
        if (!cfg->quiet) {
            printf("\n%sBASE FORENSIC RAPTOR // REPUTATION DATABASE (OSINT)%s\n",
                   cfg->is_tty ? C_BOLD : "", cfg->is_tty ? C_RESET : "");
            printf("================================================================================\n");
            printf("Fichier compilé      : src/cli/rap_threat_data.h (.rodata intégrée)\n");
            printf("Dernière synchro     : %s\n", RAP_THREAT_DB_BUILD);
            printf("Total d'IP indexées  : %zu signatures (0 heap, O(log N))\n\n", RAP_THREAT_COUNT);
            printf("RÉPARTITION PAR CATÉGORIES :\n");
            printf("  [1] C2 / Botnets (abuse.ch Feodo) : %zu adresses\n", count_c2);
            printf("  [2] Relais de sortie Tor          : %zu adresses\n", count_tor);
            printf("  [3] IPsum Multi-Blacklists        : %zu adresses\n\n", count_abuse);
            printf("COMMANDES DE NAVIGATION DISPONIBLES :\n");
            printf("  ./build/rap net -l c2             : Lister les IP de serveurs C2\n");
            printf("  ./build/rap net -l tor            : Lister les adresses de sortie Tor\n");
            printf("  ./build/rap net -l abuse          : Lister les adresses d'abus/scanners\n");
            printf("  ./build/rap net -l all            : Lister l'intégralité des %zu adresses\n", RAP_THREAT_COUNT);
            printf("================================================================================\n\n");
        }
        return RAP_EXIT_OK;
    }

    /* Filtrage et déversement d'une catégorie */
    int target_cat = -1;
    if (strcmp(filter, "c2") == 0) target_cat = RAP_THREAT_CAT_C2;
    else if (strcmp(filter, "tor") == 0) target_cat = RAP_THREAT_CAT_TOR;
    else if (strcmp(filter, "abuse") == 0) target_cat = RAP_THREAT_CAT_ABUSE;
    else if (strcmp(filter, "all") == 0) target_cat = 0;
    else {
        fprintf(stderr, "%s[ERREUR] Catégorie inconnue : '%s'. Options : c2, tor, abuse, all%s\n",
                cfg->is_tty ? C_RED : "", filter, cfg->is_tty ? C_RESET : "");
        return RAP_EXIT_ERROR;
    }

    if (!cfg->quiet) {
        if (cfg->is_tty && !cfg->raw) {
            printf(" %-18s %-12s %s\n", "ADRESSE IP", "SCORE RISQUE", "CATÉGORIE OSINT");
            printf(" ------------------------------------------------------------\n");
        } else {
            printf("IP\tSCORE\tCATEGORIE\n");
        }
    }

    int displayed = 0;
    for (size_t i = 0; i < RAP_THREAT_COUNT; i++) {
        if (target_cat == 0 || g_rap_threat_db[i].category == target_cat) {
            uint32_t ip = g_rap_threat_db[i].ip;
            char sip[32];
            snprintf(sip, sizeof(sip), "%u.%u.%u.%u",
                     (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

            const char *clabel = "ABUSE";
            if (g_rap_threat_db[i].category == RAP_THREAT_CAT_C2) clabel = "BOTNET_C2";
            else if (g_rap_threat_db[i].category == RAP_THREAT_CAT_TOR) clabel = "TOR_EXIT";

            if (cfg->is_tty && !cfg->raw) {
                printf(" %-18s %-12u %s\n", sip, g_rap_threat_db[i].score, clabel);
            } else {
                printf("%s\t%u\t%s\n", sip, g_rap_threat_db[i].score, clabel);
            }

            displayed++;
            if (cfg->limit > 0 && displayed >= cfg->limit) break;
        }
    }

    return RAP_EXIT_OK;
}

/* ============================================================================
 * FONCTION 127 : rap_net_list_active
 * ----------------------------------------------------------------------------
 * Tableau temps réel des flux TCP établis couplé à la corrélation PID et
 * à la qualification de réputation OSINT / Net-Guard.
 * ============================================================================
 */
static int rap_net_list_active(const RapConfig *cfg)
{
    FILE *ftcp = fopen("/proc/net/tcp", "r");
    if (!ftcp) {
        perror("rap net: lecture /proc/net/tcp impossible");
        return RAP_EXIT_ERROR;
    }

    if (!cfg->quiet) {
        if (cfg->is_tty && !cfg->raw) {
            printf(" %-5s %-21s %-21s %-22s %s\n",
                   "PROTO", "ADRESSE LOCALE", "ADRESSE DISTANTE", "PROCESSUS (PID)", "QUALIFICATION NET-GUARD");
            printf(" ----------------------------------------------------------------------------------------------------\n");
        } else {
            printf("PROTO\tADRESSE LOCALE\tADRESSE DISTANTE\tPROCESSUS (PID)\tQUALIFICATION NET-GUARD\n");
        }
    }

    char line[256];
    if (!fgets(line, sizeof(line), ftcp)) {
        fclose(ftcp);
        return RAP_EXIT_ERROR;
    }

    int displayed = 0;
    int hostile_found = 0;

    while (fgets(line, sizeof(line), ftcp)) {
        unsigned int local_ip = 0, local_port = 0;
        unsigned int rem_ip = 0, rem_port = 0;
        unsigned int state = 0;
        unsigned long inode = 0;

        int matched = sscanf(line, "%*d: %x:%x %x:%x %x %*s %*s %*s %*s %*s %lu",
                             &local_ip, &local_port, &rem_ip, &rem_port, &state, &inode);

        if (matched >= 5) {
            /* Filtrage sur l'état TCP_ESTABLISHED (valeur 1) */
            if (state == 1) {
                unsigned int a1 = local_ip & 0xFF;
                unsigned int b1 = (local_ip >> 8) & 0xFF;
                unsigned int c1 = (local_ip >> 16) & 0xFF;
                unsigned int d1 = (local_ip >> 24) & 0xFF;

                unsigned int a2 = rem_ip & 0xFF;
                unsigned int b2 = (rem_ip >> 8) & 0xFF;
                unsigned int c2 = (rem_ip >> 16) & 0xFF;
                unsigned int d2 = (rem_ip >> 24) & 0xFF;

                char slocal[32], srem[32];
                snprintf(slocal, sizeof(slocal), "%u.%u.%u.%u:%u", a1, b1, c1, d1, local_port);
                snprintf(srem, sizeof(srem), "%u.%u.%u.%u:%u", a2, b2, c2, d2, rem_port);

                uint32_t rem_ip_num = (a2 << 24) | (b2 << 16) | (c2 << 8) | d2;

                /* 1. Résolution locale du processus détenteur via son Inode */
                pid_t pid = 0;
                char proc_comm[64] = {0};
                rap_net_find_proc_by_inode(inode, &pid, proc_comm, sizeof(proc_comm));

                char proc_info[80];
                if (pid > 0) {
                    snprintf(proc_info, sizeof(proc_info), "%s (%d)", proc_comm, pid);
                } else {
                    snprintf(proc_info, sizeof(proc_info), "-");
                }

                /* 2. Qualification : priorité à la base OSINT locale (bsearch) */
                char diag[128];
                int qual = RAP_NET_STANDARD;
                const RapThreatRecord *rec = rap_net_lookup_threat(rem_ip_num);

                if (rec) {
                    qual = RAP_NET_HOSTILE;
                    hostile_found++;
                    if (rec->category == RAP_THREAT_CAT_C2) {
                        snprintf(diag, sizeof(diag), "[DANGER C2] Score %u/100 (Botnet)", rec->score);
                    } else if (rec->category == RAP_THREAT_CAT_TOR) {
                        snprintf(diag, sizeof(diag), "[ALERTE TOR] Score %u/100 (Exit Node)", rec->score);
                    } else {
                        snprintf(diag, sizeof(diag), "[HOSTILE OSINT] Score %u/100", rec->score);
                    }
                } else {
                    /* Repli sur les règles Net-Guard et ports de service */
                    qual = rap_net_eval_ip(rem_ip_num, (uint16_t)rem_port, diag, sizeof(diag));
                    if (qual == RAP_NET_HOSTILE) {
                        hostile_found++;
                    }
                }

                /* Formatage avec coloration TTY */
                char formatted_diag[160];
                if (cfg->is_tty && !cfg->raw) {
                    if (qual == RAP_NET_HOSTILE) {
                        snprintf(formatted_diag, sizeof(formatted_diag), "%s%s%s", C_RED, diag, C_RESET);
                    } else if (qual == RAP_NET_SUSPECT) {
                        snprintf(formatted_diag, sizeof(formatted_diag), "%s%s%s", C_YELLOW, diag, C_RESET);
                    } else {
                        snprintf(formatted_diag, sizeof(formatted_diag), "%s", diag);
                    }
                } else {
                    snprintf(formatted_diag, sizeof(formatted_diag), "%s", diag);
                }

                if (cfg->is_tty && !cfg->raw) {
                    printf(" %-5s %-21s %-21s %-22s %s\n",
                           "TCP", slocal, srem, proc_info, formatted_diag);
                } else {
                    printf("TCP\t%s\t%s\t%s\t%s\n",
                           slocal, srem, proc_info, diag);
                }

                displayed++;
                if (cfg->limit > 0 && displayed >= cfg->limit) break;
            }
        }
    }
    fclose(ftcp);

    if (displayed == 0 && !cfg->quiet) {
        printf("%s[+] Aucun flux TCP sortant à l'état ESTABLISHED détecté%s\n",
               cfg->is_tty ? C_GREEN : "", cfg->is_tty ? C_RESET : "");
    }

    return hostile_found ? RAP_EXIT_THREAT : RAP_EXIT_OK;
}

/* ============================================================================
 * FONCTION 94 : rap_cmd_net
 * ----------------------------------------------------------------------------
 * Aiguillage déclaratif des sous-commandes de diagnostic réseau :
 *  - 'rap net'            : Tableau enrichi des flux avec processus et réputation.
 *  - 'rap net -i <IP>'    : Fiche forensique unitaire (alias direct : 'rap net <IP>').
 *  - 'rap net -l [cat]'   : Consultation sanitaire et inventaire OSINT.
 * ============================================================================
 */
int rap_cmd_net(int argc, char **argv, const RapConfig *cfg)
{
    /* Cas 1 : 'rap net -l [catégorie]' */
    if (argc >= 3 && (strcmp(argv[2], "-l") == 0 || strcmp(argv[2], "--list") == 0)) {
        const char *filter = (argc >= 4) ? argv[3] : "";
        return rap_net_list_threats(filter, cfg);
    }

    /* Cas 2 : 'rap net -i <IP>' */
    if (argc >= 3 && (strcmp(argv[2], "-i") == 0 || strcmp(argv[2], "--inspect") == 0)) {
        if (argc < 4) {
            fprintf(stderr, "%sUsage : rap net -i <ADRESSE_IPV4>%s\n",
                    cfg->is_tty ? C_RED : "", cfg->is_tty ? C_RESET : "");
            return RAP_EXIT_ERROR;
        }
        return rap_net_investigate_ip(argv[3], cfg);
    }

    /* Cas 3 : 'rap net <IP>' (raccourci direct sans drapeau -i) */
    if (argc >= 3 && argv[2][0] != '-') {
        return rap_net_investigate_ip(argv[2], cfg);
    }

    /* Cas 4 : 'rap net' (surveillance temps réel de tous les flux actifs) */
    return rap_net_list_active(cfg);
}
