/* ==============================================================================
 * NOM DU SCRIPT : src/cli/netop_cmd_inspect.c
 * RÉVISION      : Rev 1.0 (2026-10-26)
 * PROJET        : RAPTOR DEFENSE v2.0 // Console Tactique Temps Réel
 * RESPONSABILITÉ: Greffon d'inspection médico-légale 'i' (Forensic Overlay).
 *                 - Extraction /proc : PPID, binaire parent, cmdline complète, exe réel.
 *                 - Détection de Reverse Shell (analyse STDIN/STDOUT redirigés vers socket).
 *                 - Corrélation OSINT et réputation IP avec état de bannissement Netfilter.
 *                 - Actions directes contextuelles : 'k' (Kill) et 'b' (Ban IP) intégrées.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Autopsie système complète sans allocation dynamique (0-heap).
 *   - Heuristique de détection de Reverse Shell par correspondance Inode sur fd/0 et fd/1.
 *   - Rendu modal plein écran avec effacement absolu des résidus graphiques.
 *   - Clavier contextuel dans la modale : 'k' frappe le PID, 'b' isole l'IP distante.
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 20 (Fonctions 153 à 157)
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   153. netop_cmd_inspect_exec            - Sélection du slot, extraction et ouverture modale.
 *   154. netop_cmd_inspect_collect         - Extraction chirurgicale des métadonnées sous /proc.
 *   155. netop_cmd_inspect_detect_revshell - Corrélation Inode/Socket sur les descripteurs ouverts.
 *   156. netop_cmd_inspect_render_modal    - Rendu plein écran de la fiche médico-légale.
 *   157. netop_cmd_inspect_handle_key      - Routage des actions contextuelles 'k' et 'b' en modale.
 * ============================================================================== */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "netop_cmd_inspect.h"
#include "netop_router.h"
#include "netop_cmd_kill.h"
#include "netop_cmd_ban.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

typedef struct {
    uint32_t      pid;
    uint32_t      ppid;
    char          comm[64];
    char          parent_comm[64];
    char          exe_path[256];
    char          cmdline[512];
    char          local_endpoint[64];
    char          remote_endpoint[64];
    char          remote_ip[48];
    uint16_t      remote_port;
    char          state[16];
    char          verdict[48];
    unsigned long inode;
    int           total_fds;
    char          fd0_target[128];
    char          fd1_target[128];
    char          fd2_target[128];
    bool          is_reverse_shell;
    uint8_t       threat_score;
    uint8_t       threat_category;
    bool          threat_matched;
    bool          is_banned;
    int           original_slot;
    bool          active;
} NetopForensicRecord;

static NetopForensicRecord s_forensic;

/* ==============================================================================
 * FONCTION 155 : netop_cmd_inspect_detect_revshell
 * ============================================================================== */
static void netop_cmd_inspect_detect_revshell(NetopForensicRecord *f)
{
    f->is_reverse_shell = false;
    if (f->pid == 0) return;

    char fd_path[64];
    snprintf(fd_path, sizeof(fd_path), "/proc/%u/fd", f->pid);
    DIR *d = opendir(fd_path);
    if (!d) return;

    f->total_fds = 0;
    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        if (de->d_name[0] != '.') f->total_fds++;
    }
    closedir(d);

    char link_target[128];
    char p[64];

    /* fd 0 (STDIN) */
    snprintf(p, sizeof(p), "/proc/%u/fd/0", f->pid);
    ssize_t len = readlink(p, link_target, sizeof(link_target) - 1);
    if (len > 0) {
        link_target[len] = '\0';
        snprintf(f->fd0_target, sizeof(f->fd0_target), "%s", link_target);
    } else {
        snprintf(f->fd0_target, sizeof(f->fd0_target), "[non-accessible]");
    }

    /* fd 1 (STDOUT) */
    snprintf(p, sizeof(p), "/proc/%u/fd/1", f->pid);
    len = readlink(p, link_target, sizeof(link_target) - 1);
    if (len > 0) {
        link_target[len] = '\0';
        snprintf(f->fd1_target, sizeof(f->fd1_target), "%s", link_target);
    } else {
        snprintf(f->fd1_target, sizeof(f->fd1_target), "[non-accessible]");
    }

    /* fd 2 (STDERR) */
    snprintf(p, sizeof(p), "/proc/%u/fd/2", f->pid);
    len = readlink(p, link_target, sizeof(link_target) - 1);
    if (len > 0) {
        link_target[len] = '\0';
        snprintf(f->fd2_target, sizeof(f->fd2_target), "%s", link_target);
    } else {
        snprintf(f->fd2_target, sizeof(f->fd2_target), "[non-accessible]");
    }

    /* Corrélation formelle : si stdin et stdout pointent vers un socket */
    bool in_is_sock  = (strncmp(f->fd0_target, "socket:[", 8) == 0);
    bool out_is_sock = (strncmp(f->fd1_target, "socket:[", 8) == 0);

    if (in_is_sock && out_is_sock) {
        f->is_reverse_shell = true;
    }
}

/* ==============================================================================
 * FONCTION 154 : netop_cmd_inspect_collect
 * ============================================================================== */
static void netop_cmd_inspect_collect(NetopForensicRecord *f, const NetopEntry *e, int slot)
{
    memset(f, 0, sizeof(NetopForensicRecord));

    f->pid           = e->pid;
    f->inode         = e->inode;
    f->original_slot = slot;
    f->remote_port   = e->remote_port;
    snprintf(f->comm, sizeof(f->comm), "%s", e->comm);
    snprintf(f->remote_ip, sizeof(f->remote_ip), "%s", e->remote_ip);
    snprintf(f->local_endpoint, sizeof(f->local_endpoint), "%s:%u", e->local_ip, e->local_port);
    snprintf(f->remote_endpoint, sizeof(f->remote_endpoint), "%s:%u", e->remote_ip, e->remote_port);
    snprintf(f->state, sizeof(f->state), "%s", e->state);
    snprintf(f->verdict, sizeof(f->verdict), "%s", e->verdict);
    f->active = true;

    /* 1. Vérification embargo existant */
    f->is_banned = netop_cmd_ban_is_banned(e->remote_ip);

    /* 2. Lookup OSINT unifié */
    unsigned int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    if (sscanf(e->remote_ip, "%u.%u.%u.%u", &r1, &r2, &r3, &r4) == 4) {
        uint32_t eval_ip = (r1 << 24) | (r2 << 16) | (r3 << 8) | r4;
        f->threat_matched = netop_engine_lookup_threat(eval_ip, &f->threat_category, &f->threat_score);
    }

    if (f->pid == 0) {
        snprintf(f->exe_path, sizeof(f->exe_path), "[Contexte noyau / Socket orphelin]");
        snprintf(f->cmdline, sizeof(f->cmdline), "[Processus terminé ou socket géré par le noyau]");
        snprintf(f->parent_comm, sizeof(f->parent_comm), "[N/A]");
        return;
    }

    /* 3. Extraction /proc/[PID]/status (PPID) */
    char status_path[64];
    snprintf(status_path, sizeof(status_path), "/proc/%u/status", f->pid);
    FILE *fs = fopen(status_path, "r");
    if (fs) {
        char line[256];
        while (fgets(line, sizeof(line), fs)) {
            if (strncmp(line, "PPid:", 5) == 0) {
                f->ppid = (uint32_t)atoi(line + 5);
                break;
            }
        }
        fclose(fs);
    }

    /* 4. Résolution du nom du parent */
    if (f->ppid > 0) {
        char pcomm_path[64];
        snprintf(pcomm_path, sizeof(pcomm_path), "/proc/%u/comm", f->ppid);
        int pfd = open(pcomm_path, O_RDONLY);
        if (pfd >= 0) {
            ssize_t n = read(pfd, f->parent_comm, sizeof(f->parent_comm) - 1);
            if (n > 0) {
                f->parent_comm[n] = '\0';
                for (ssize_t i = 0; i < n; i++) {
                    if (f->parent_comm[i] == '\n' || f->parent_comm[i] == '\r') {
                        f->parent_comm[i] = '\0';
                        break;
                    }
                }
            }
            close(pfd);
        }
    }
    if (f->parent_comm[0] == '\0') {
        snprintf(f->parent_comm, sizeof(f->parent_comm), f->ppid == 0 ? "[Init]" : "[Inconnu]");
    }

    /* 5. Résolution du binaire réel sur disque via readlink */
    char exe_link[64];
    snprintf(exe_link, sizeof(exe_link), "/proc/%u/exe", f->pid);
    ssize_t exe_len = readlink(exe_link, f->exe_path, sizeof(f->exe_path) - 1);
    if (exe_len > 0) {
        f->exe_path[exe_len] = '\0';
    } else {
        snprintf(f->exe_path, sizeof(f->exe_path), "[Accès refusé / Démon masqué]");
    }

    /* 6. Extraction de la ligne de commande intégrale (cmdline) */
    char cmd_path[64];
    snprintf(cmd_path, sizeof(cmd_path), "/proc/%u/cmdline", f->pid);
    int cfd = open(cmd_path, O_RDONLY);
    if (cfd >= 0) {
        ssize_t n = read(cfd, f->cmdline, sizeof(f->cmdline) - 1);
        if (n > 0) {
            f->cmdline[n] = '\0';
            for (ssize_t i = 0; i < n - 1; i++) {
                if (f->cmdline[i] == '\0') f->cmdline[i] = ' ';
            }
        }
        close(cfd);
    }
    if (f->cmdline[0] == '\0') {
        snprintf(f->cmdline, sizeof(f->cmdline), "[%s]", f->comm);
    }

    /* 7. Traque des descripteurs de fichiers et analyse Reverse Shell */
    netop_cmd_inspect_detect_revshell(f);
}

/* ==============================================================================
 * FONCTION 156 : netop_cmd_inspect_render_modal
 * ============================================================================== */
bool netop_cmd_inspect_render_modal(NetopContext *ctx)
{
    if (!ctx || !s_forensic.active) return false;

    printf("\033[H");
    printf("\033[2K\033[1;37mRAPTOR v2.0 // \033[1;36mINSPECTION MÉDICO-LÉGALE DU PROCESSUS (FORENSIC OVERLAY)\033[0m\n");
    printf("\033[2K\033[1;30m-----------------------------------------------------------------------------------------------------\033[0m\n");

    /* 1. Bloc Processus & Généalogie */
    printf("\033[2K\033[1;33m[ IDENTITÉ DU PROCESSUS & GÉNÉALOGIE ]\033[0m\n");
    printf("\033[2K  PID CIBLÉ    : \033[1;37m%-8u\033[0m          PPID (PARENT): \033[1;36m%-8u\033[0m (%s)\n",
           s_forensic.pid, s_forensic.ppid, s_forensic.parent_comm);
    printf("\033[2K  COMMANDE     : \033[1;32m%-20.20s\033[0m BINAIRE RÉEL: \033[0;37m%s\033[0m\n",
           s_forensic.comm, s_forensic.exe_path);
    printf("\033[2K  LIGNE CMD    : \033[0;36m%-80.80s\033[0m\n", s_forensic.cmdline);
    printf("\033[2K\n");

    /* 2. Bloc Réseau & Socket */
    printf("\033[2K\033[1;33m[ ANCRAGE RÉSEAU & SOCKET NOYAU ]\033[0m\n");
    printf("\033[2K  FLUX ACTIF   : \033[1;37m%s\033[0m -> \033[1;31m%s\033[0m\n",
           s_forensic.local_endpoint, s_forensic.remote_endpoint);
    printf("\033[2K  STATUT TCP   : \033[1;32m%-15.15s\033[0m INODE NOYAU : \033[1;37m%lu\033[0m\n",
           s_forensic.state, s_forensic.inode);
    printf("\033[2K  VERDICT VUE  : \033[1;33m%s\033[0m\n", s_forensic.verdict);
    printf("\033[2K\n");

    /* 3. Bloc Descripteurs & Reverse Shell */
    printf("\033[2K\033[1;33m[ TRAQUE DES DESCRIPTEURS & DÉTOURNEMENT ]\033[0m\n");
    printf("\033[2K  STDIN  (fd/0): %-35.35s STDOUT (fd/1): %-35.35s\n",
           s_forensic.fd0_target, s_forensic.fd1_target);
    printf("\033[2K  STDERR (fd/2): %-35.35s TOTAL FDS    : %d descripteurs ouverts\n",
           s_forensic.fd2_target, s_forensic.total_fds);

    if (s_forensic.is_reverse_shell) {
        printf("\033[2K  ALERTE TACT. : \033[1;41;37m [!] ALERTE CRITIQUE : REVERSE SHELL DÉTECTÉ (STDIN/OUT = SOCKET RÉSEAU) \033[0m\n");
    } else {
        printf("\033[2K  CONFORMITÉ   : \033[1;32m [✓] FLUX STANDARD (Aucun détournement d'entrées/sorties détecté) \033[0m\n");
    }
    printf("\033[2K\n");

    /* 4. Bloc Renseignement OSINT & Embargo */
    printf("\033[2K\033[1;33m[ RENSEIGNEMENT OSINT & EMBARGO NOYAU ]\033[0m\n");
    if (s_forensic.threat_matched) {
        const char *cat_label = (s_forensic.threat_category == 1) ? "Botnet / Ransomware C2" :
                                (s_forensic.threat_category == 2) ? "Relais de sortie Tor" : "Scanners / Abus";
        printf("\033[2K  BASE OSINT   : \033[1;31mHOSTILE (%u/100)\033[0m       CATÉGORIE    : \033[1;31m%s\033[0m\n",
               s_forensic.threat_score, cat_label);
    } else {
        printf("\033[2K  BASE OSINT   : \033[1;32mNON RÉPERTORIÉE (FLUX NEUTRE OU LOCAL)\033[0m\n");
    }

    printf("\033[2K  ÉTAT EMBARGO : %s\033[0m\n",
           s_forensic.is_banned ? "\033[1;31mVERROUILLÉ (IP SOUS EMBARGO NOYAU)" : "\033[1;32mLIBRE (Aucune règle Netfilter)");

    /* Effacement des lignes restantes */
    for (int i = 21; i < ctx->max_display_lines; i++) {
        printf("\033[2K\n");
    }

    printf("\033[2K\033[1;30m-----------------------------------------------------------------------------------------------------\033[0m\n");
    printf("\033[2KACTIONS : \033[31m[k]\033[0m Neutraliser (SIGKILL)  \033[33m[b]\033[0m %s  \033[36m[q]/[Échap]\033[0m Fermer la fiche  ==> \n\033[J",
           s_forensic.is_banned ? "Débannir IP" : "Bannir IP");
    fflush(stdout);

    return true;
}

/* ==============================================================================
 * FONCTION 157 : netop_cmd_inspect_handle_key
 * ============================================================================== */
bool netop_cmd_inspect_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch)
{
    if (!s_forensic.active) return false;

    if (ch == 'q' || ch == 'Q' || ch == 27) {
        s_forensic.active = false;
        netop_router_set_overlay(NULL);
        return true;
    }

    /* Action contextuelle directe 'k' : neutralisation du PID inspecté */
    if (ch == 'k' || ch == 'K') {
        if (s_forensic.original_slot >= 1) {
            char slot_str[16];
            snprintf(slot_str, sizeof(slot_str), "%d", s_forensic.original_slot);
            netop_cmd_kill_exec(ctx, entries, count, slot_str);
            snprintf(s_forensic.verdict, sizeof(s_forensic.verdict), "NEUTRALISÉ (SIGKILL)");
        }
        return true;
    }

    /* Action contextuelle directe 'b' : embargo ou déban de l'IP distante */
    if (ch == 'b' || ch == 'B') {
        if (s_forensic.original_slot >= 1) {
            char cmd_str[16];
            if (s_forensic.is_banned) {
                snprintf(cmd_str, sizeof(cmd_str), "-%d", s_forensic.original_slot);
                netop_cmd_ban_exec(ctx, entries, count, cmd_str);
                s_forensic.is_banned = false;
            } else {
                snprintf(cmd_str, sizeof(cmd_str), "%d", s_forensic.original_slot);
                netop_cmd_ban_exec(ctx, entries, count, cmd_str);
                s_forensic.is_banned = true;
            }
        }
        return true;
    }

    return false;
}

/* ==============================================================================
 * FONCTION 153 : netop_cmd_inspect_exec
 * ============================================================================== */
int netop_cmd_inspect_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg)
{
    if (!ctx || !entries || count == 0 || !arg || arg[0] == '\0') return -1;

    int slot = atoi(arg);
    if (slot < 1 || (size_t)slot > count) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg),
                 "\033[1;31m[ERREUR]\033[0m Ligne #%02d inexistante pour inspection médico-légale.", slot);
        ctx->status_msg_expire = 0;
        return -1;
    }

    const NetopEntry *target = &entries[slot - 1];

    /* Extraction médico-légale et activation de l'overlay */
    netop_cmd_inspect_collect(&s_forensic, target, slot);
    netop_router_set_overlay(netop_cmd_inspect_render_modal);

    return 0;
}
