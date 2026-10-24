/* ============================================================================
 * NOM DU SCRIPT   : src/cli/cmd_man.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR DEFENSE v2.0 (Mode CLI & Outils de Secours)
 * RESPONSABILITÉ  : Manuel de référence autonome embarqué ('rap man').
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Documentation complète de la commande 'heur' (Section 7 & 16).
 *
 * SECTION DU CAHIER DES CHARGES : SECTION 16 (Fonction 97)
 * ============================================================================
 */

#include "rap_cli.h"

int rap_cmd_man(int argc, char **argv, const RapConfig *cfg)
{
    const char *target = (argc > 2) ? argv[2] : NULL;

    if (target) {
        if (strcmp(target, "heur") == 0) {
            printf("%sRAP MAN // HEUR (SECTION 7 & 16 - ANALYSE HEURISTIQUE)%s\n\n",
                   cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
            printf("Traque les anomalies structurelles d'exécution :\n");
            printf(" - Binaires supprimés du disque pendant l'exécution (deleted).\n");
            printf(" - Exécutions clandestines depuis les zones volatiles (/tmp, /dev/shm).\n");
            printf(" - Usurpation de noms de démons officiels (systemd, sshd, kworker).\n\n");
            printf("SYNTAXE :\n    rap heur [-q] [-r]\n\n");
            printf("EXEMPLE :\n    rap heur -q | awk '{print $1}' | xargs -r rap freeze\n");
            return RAP_EXIT_OK;
        } else if (strcmp(target, "rx") == 0) {
            printf("%sRAP MAN // RX (SECTION 13 - MUTATIONS ANONYMES)%s\n\n",
                   cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
            printf("Traque les pages virtuelles exécutables r-xp sans fichier adossé (Inode 0).\n");
            printf("SYNTAXE :\n    rap rx [-q] [-r] [-v]\n");
            return RAP_EXIT_OK;
        }
    }

    printf("%sNOM%s\n", cfg->is_tty ? C_BOLD : "", cfg->is_tty ? C_RESET : "");
    printf("    rap - Console chirurgicale et trousse de survie défensive RAPTOR\n\n");

    printf("%sSYNOPSIS%s\n", cfg->is_tty ? C_BOLD : "", cfg->is_tty ? C_RESET : "");
    printf("    rap [OPTIONS] COMMANDE [ARGUMENTS...]\n\n");

    printf("%sCOMMANDES OPÉRATIONNELLES%s\n", cfg->is_tty ? C_BOLD : "", cfg->is_tty ? C_RESET : "");
    printf("    %sstatus%s, %sst%s       Bilan de santé instantané (RAM disponible, CPU, alertes)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "", cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %stop%s              Top des consommateurs de mémoire vive résidente (VmRSS)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %shunt%s             Audit combiné universel (Mémoire RX, Preload et Heuristiques)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %srx%s               Traque des mutations anonymes R-X Inode 0 (Section 13)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %spreload%s          Traque des détournements de chargeur LD_PRELOAD (Section 14)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %sheur%s, %she%s         Analyse heuristique (chemins volatils, binaires supprimés)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "", cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %snet%s              Audit des flux TCP établis sous /proc/net/tcp (Section 11)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %skill <PID>%s       Abattage chirurgical immédiat par signal SIGKILL (Section 16)\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %sfreeze <PID>%s     Gel d'un processus hostile via SIGSTOP pour audit forensique\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");
    printf("    %sman [CMD]%s        Affichage de cette documentation ou du détail d'un outil\n\n",
           cfg->is_tty ? C_CYAN : "", cfg->is_tty ? C_RESET : "");

    printf("%sCODES DE RETOUR POSIX%s\n", cfg->is_tty ? C_BOLD : "", cfg->is_tty ? C_RESET : "");
    printf("    0 (RAP_EXIT_OK)      Système sain / Aucune anomalie non autorisée\n");
    printf("    1 (RAP_EXIT_THREAT)  Menace critique active identifiée sur l'hôte\n");
    printf("    2 (RAP_EXIT_ERROR)   Erreur de syntaxe, cible introuvable ou droits insuffisants\n");

    return RAP_EXIT_OK;
}
