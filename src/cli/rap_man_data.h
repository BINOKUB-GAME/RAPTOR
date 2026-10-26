/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rap_man_data.h
 * RÉVISION        : Généré automatiquement par tools/gen_man.py (SSOT)
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Constantes .rodata pour le manuel indexé et le dictionnaire.
 * ============================================================================ */

#ifndef RAP_MAN_DATA_H
#define RAP_MAN_DATA_H

#include <stddef.h>

/* Manuel CLI complet concaténé (Section 1 et 5) */
static const char g_rap_man_data[] =
    "\n"
    "RAP(1)                      RAPTOR DEFENSE                      RAP(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap — Console chirurgicale, traque mémoire 0-heap et remédiation active réflexe\n"
    "\n"
    "SYNOPSIS\n"
    "     rap [-q] [-r] [-f] [-v] [-a|--arm] [--dry-run] [-n nombre] commande [arguments...]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap est l'utilitaire autonome en ligne de commande de RAPTOR v2.0.\n"
    "     Conçu selon la norme C99 pour fonctionner sous famine mémoire extrême\n"
    "     (< 2 Mo RAM) ou en cas d'effondrement du serveur d'affichage (X11/Wayland),\n"
    "     il audite passivement les structures du noyau (/proc) et neutralise les menaces.\n"
    "\n"
    "     Dans sa révision réflexe, rap intègre un disjoncteur chirurgical de dernier\n"
    "     recours (Last-Resort Watchdog) capable d'opérer sous contrainte d'épuisement\n"
    "     mémoire (mlockall et score OOM -1000) pour paralyser et détruire les menaces\n"
    "     en moins d'une milliseconde sans dépendre du disque ni du réseau.\n"
    "\n"
    "OPTIONS UNIVERSELLES\n"
    "     -q, --quiet\n"
    "             Mode silencieux. Supprime les en-têtes et les lignes de séparation\n"
    "             pour ne conserver que la sortie brute.\n"
    "\n"
    "     -r, --raw\n"
    "             Format machine TSV pur. Utilise des tabulations sans arrondir les\n"
    "             unités pour faciliter le traitement via awk, grep ou cut.\n"
    "\n"
    "     -f, --force\n"
    "             Désactive les demandes de confirmation interactive lors des actions\n"
    "             cinétiques (kill et freeze).\n"
    "\n"
    "     -v, --verbose\n"
    "             Mode d'audit forensique étendu. Révèle notamment les segments mémoires\n"
    "             autorisés par mem_whitelist.conf avec le tag [LISTE BLANCHE].\n"
    "\n"
    "     -a, --arm\n"
    "             Arme la riposte cinétique immédiate (SIGSTOP -> Révocation réseau ->\n"
    "             Extraction boîte noire -> SIGKILL). Active également l'immunité OOM\n"
    "             (/proc/self/oom_score_adj = -1000) et le verrouillage mémoire physique.\n"
    "\n"
    "     --dry-run\n"
    "             Mode simulation opérationnelle. Exécute l'intégralité du pipeline réflexe\n"
    "             et enregistre l'incident dans la boîte noire RAM (/dev/shm/raptor_blackbox)\n"
    "             sans émettre les signaux de destruction (SIGSTOP/SIGKILL).\n"
    "\n"
    "     -n nombre\n"
    "             Limite le nombre de lignes projetées (applicable à top et net).\n"
    "\n"
    "COMMANDES\n"
    "     status, st   Diagnostic vital instantané de la machine hôte.\n"
    "     top          Top des processus consommateurs de mémoire vive résidente.\n"
    "     hunt         Audit combiné universel (Mémoire, Preload, Heuristiques) avec riposte réflexe.\n"
    "     rx           Traque des pages mémoires anonymes exécutables (Inode 0) et neutralisation.\n"
    "     preload      Audit d'intégrité du chargeur dynamique (/etc/ld.so.preload).\n"
    "     heur, he     Analyse heuristique (binaires supprimés, chemins volatils).\n"
    "     net          Audit TCP, corrélation PID et forensic IP (Net-Guard / OSINT).\n"
    "     netop, no    Console tactique temps réel matricielle (TUI), audit continu et remédiation active.\n"
    "     sentry, se   Sentinelle passive anti-rançongiciel (canaris, fanotify et boîte noire).\n"
    "     kill <PID>   Neutralisation immédiate par émission du signal SIGKILL.\n"
    "     freeze <PID> Gel conservatoire via signal SIGSTOP pour examen forensique.\n"
    "     man [cmd]    Manuel de référence autonome embarqué (SSOT).\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Système nominal. Aucune anomalie non autorisée.\n"
    "     1 (RAP_EXIT_THREAT)  Menace critique active identifiée sur l'hôte.\n"
    "     2 (RAP_EXIT_ERROR)   Erreur de syntaxe, droits insuffisants ou cible invalide.\n"
    "\n"
    "AUTHORS\n"
    "     Conçu par l'équipe d'ingénierie RAPTOR DEFENSE.\n"
    "\n"
    "\n"
    "RAP-STATUS(1)               RAPTOR DEFENSE               RAP-STATUS(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap status — Diagnostic vital instantané de la machine hôte\n"
    "\n"
    "SYNOPSIS\n"
    "     rap status [-q] [-r]\n"
    "     rap st\n"
    "\n"
    "DESCRIPTION\n"
    "     Extrait en moins de 15 ms les métriques fondamentales de santé du système :\n"
    "     RAM totale et disponible (/proc/meminfo), dénombrement des processus\n"
    "     actifs et vérification d'altération du fichier /etc/ld.so.preload.\n"
    "\n"
    "EXIT STATUS\n"
    "     0    Système sain et intègre.\n"
    "     1    Présence suspecte du fichier /etc/ld.so.preload.\n"
    "\n"
    "\n"
    "RAP-TOP(1)                  RAPTOR DEFENSE                  RAP-TOP(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap top — Top des processus consommateurs de mémoire vive résidente (VmRSS)\n"
    "\n"
    "SYNOPSIS\n"
    "     rap top [-q] [-r] [-n nombre]\n"
    "\n"
    "DESCRIPTION\n"
    "     Analyse les structures /proc/[PID]/status pour extraire la mémoire physique\n"
    "     réelle résidente (VmRSS). Élimine les threads noyau (RSS = 0) et classe les\n"
    "     processus par tri statique sur la pile (0 octet sur le tas).\n"
    "\n"
    "\n"
    "RAP-HUNT(1)                 RAPTOR DEFENSE                 RAP-HUNT(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap hunt — Audit de sécurité universel combiné et riposte active\n"
    "\n"
    "SYNOPSIS\n"
    "     rap hunt [-q] [-r] [-v] [-a|--arm] [--dry-run]\n"
    "\n"
    "DESCRIPTION\n"
    "     Coordonne séquentiellement les trois grandes sondes d'investigation :\n"
    "       1. Traque des mutations mémoires anonymes (rap rx).\n"
    "       2. Traque des détournements LD_PRELOAD (rap preload).\n"
    "       3. Analyse heuristique d'exécution (rap heur).\n"
    "\n"
    "     En mode armé (-a, --arm), toute mutation Inode 0 R-X non autorisée ou\n"
    "     détournement LD_PRELOAD déclenche immédiatement le moteur de remédiation\n"
    "     rap_reflex (paralysie, isolation des descripteurs de socket et purge SIGKILL).\n"
    "\n"
    "     L'option --dry-run permet de simuler la riposte et d'alimenter la boîte noire\n"
    "     médico-légale sans impacter les processus audités.\n"
    "\n"
    "EXIT STATUS\n"
    "     0    Aucune anomalie détectée sur les trois vecteurs.\n"
    "     1    Au moins une anomalie hostile détectée sur l'un des contrôles.\n"
    "\n"
    "\n"
    "RAP-RX(1)                   RAPTOR DEFENSE                   RAP-RX(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap rx — Traque des pages mémoires anonymes exécutables (Inode 0)\n"
    "\n"
    "SYNOPSIS\n"
    "     rap rx [-q] [-r] [-v] [-a|--arm] [--dry-run]\n"
    "\n"
    "DESCRIPTION\n"
    "     Parcourt /proc/[PID]/maps pour identifier les régions mémoires marquées r-xp\n"
    "     dont l'Inode est égal à 0 (aucun fichier adossé sur disque).\n"
    "     \n"
    "     Par défaut, filtre automatiquement le bruit des moteurs JIT (Chromium, Node)\n"
    "     et des liaisons libffi (Cinnamon, GTK) selon assets/rules/mem_whitelist.conf.\n"
    "     L'option -v permet de révéler les allocations légitimes approuvées.\n"
    "\n"
    "     OPTIONS DE RÉACTION :\n"
    "       -a, --arm    : Neutralise sur-le-champ tout processus non approuvé via SIGKILL.\n"
    "       --dry-run    : Simule la riposte et consigne l'incident dans /dev/shm/raptor_blackbox.\n"
    "\n"
    "EXAMPLES\n"
    "     Chasser les injections mémoires et abattre immédiatement les processus hostiles :\n"
    "     sudo rap rx --arm\n"
    "\n"
    "     Auditer les mutations sans altérer l'état du système :\n"
    "     sudo rap rx --dry-run\n"
    "\n"
    "\n"
    "RAP-PRELOAD(1)              RAPTOR DEFENSE              RAP-PRELOAD(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap preload — Détection des rootkits userland via LD_PRELOAD\n"
    "\n"
    "SYNOPSIS\n"
    "     rap preload [-q] [-r]\n"
    "\n"
    "DESCRIPTION\n"
    "     Effectue une double vérification défensive :\n"
    "       1. Contrôle d'accès et d'intégrité de /etc/ld.so.preload.\n"
    "       2. Parcours des blocs d'environnement (/proc/[PID]/environ) délimités par\n"
    "          des octets nuls pour intercepter la variable hostile LD_PRELOAD=.\n"
    "\n"
    "\n"
    "RAP-HEUR(1)                 RAPTOR DEFENSE                 RAP-HEUR(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap heur — Analyse heuristique des anomalies de lancement et d'usurpation\n"
    "\n"
    "SYNOPSIS\n"
    "     rap heur [-q] [-r]\n"
    "     rap he\n"
    "\n"
    "DESCRIPTION\n"
    "     Résout les liens /proc/[PID]/exe pour traquer :\n"
    "       1. Les binaires fantômes supprimés du disque en cours d'exécution (deleted).\n"
    "       2. Les processus exécutés depuis des répertoires temporaires (/tmp, /dev/shm).\n"
    "       3. L'usurpation d'identité de démons officiels (systemd, sshd, kworker).\n"
    "\n"
    "\n"
    "RAP-NET(1)                  RAPTOR DEFENSE                  RAP-NET(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap net — Surveillance réseau, corrélation PID et forensic IP souverain\n"
    "\n"
    "SYNOPSIS\n"
    "     rap net [-q] [-r] [-n nombre]\n"
    "     rap net -i <ADRESSE_IPV4>\n"
    "     rap net <ADRESSE_IPV4>\n"
    "     rap net -l [c2 | tor | abuse | all]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap net est le module d'audit passif des communications réseau et de\n"
    "     renseignement tactique sur les menaces (Threat Intelligence).\n"
    "     \n"
    "     Il combine l'inspection locale des descripteurs de sockets du noyau Linux\n"
    "     (/proc/net/tcp et /proc/[PID]/fd/) à une base de réputation OSINT souveraine\n"
    "     embarquée directement en section .rodata (zéro allocation dynamique, 0 heap).\n"
    "     Toutes les vérifications s'exécutent en mémoire pure sans émettre le moindre\n"
    "     paquet sur le réseau extérieur (silence radio et furtivité absolue).\n"
    "\n"
    "MODES OPÉRATOIRES\n"
    "     1. Surveillance active des flux établis :\n"
    "        rap net [-q] [-r] [-n nombre]\n"
    "        Énumère en temps réel les connexions TCP à l'état ESTABLISHED.\n"
    "        Chaque flux est enrichi instantanément par :\n"
    "          - La corrélation du socket Inode avec le PID et le binaire local (/proc/[PID]/comm).\n"
    "          - La confrontation arithmétique O(log N) avec la table de menaces OSINT.\n"
    "          - La qualification Net-Guard (APPROUVÉ, EXTERNE, SUSPECT, DANGER C2, ALERTE TOR).\n"
    "\n"
    "     2. Enquête médico-légale unitaire :\n"
    "        rap net -i <ADRESSE_IPV4>\n"
    "        rap net <ADRESSE_IPV4>\n"
    "        Édite la fiche forensique complète de l'adresse cible sans contact externe :\n"
    "          - Reconnaissance immédiate des plages privées RFC 1918 et Loopback.\n"
    "          - Score de menace consolidé (0 à 100) et qualification de la source hostile.\n"
    "          - Détection de tout socket actif local connecté à cette IP avec chemin du binaire (/proc/[PID]/exe).\n"
    "          - Génération immédiate des règles de neutralisation iptables et nftables.\n"
    "\n"
    "     3. Inventaire et consultation de la base de signatures :\n"
    "        rap net -l [c2 | tor | abuse | all]\n"
    "        - Sans argument : Affiche le bilan vital de la base (date, volume, répartition).\n"
    "        - Avec argument : Déverse les adresses filtrées par catégorie pour pagination\n"
    "          dans le téléscripteur rapless ou exploitation dans des scripts d'administration.\n"
    "\n"
    "CATÉGORIES OSINT INTÉGRÉES\n"
    "     c2       Serveurs de commande et contrôle (C2), rançongiciels et botnets (abuse.ch Feodo).\n"
    "     tor      Relais de sortie officiels du réseau d'anonymisation Tor.\n"
    "     abuse    Adresses d'attaquants récurrents (scanners, brute-force SSH, fraudes IPsum).\n"
    "     all      Ensemble exhaustif des signatures consolidées.\n"
    "\n"
    "EXAMPLES\n"
    "     Surveiller les flux et isoler les alertes :\n"
    "     rap net\n"
    "\n"
    "     Auditer chirurgicalement une IP suspecte repérée dans les journaux :\n"
    "     rap net -i 194.26.29.112\n"
    "\n"
    "     Inspecter les serveurs C2 actifs dans le téléscripteur zéro-heap :\n"
    "     rap net -l c2 | rapless\n"
    "\n"
    "     Extraire les IP malveillantes pour alimentation d'un pare-feu local :\n"
    "     rap net -q -r -l c2 | awk '{print \"iptables -A OUTPUT -d \" $1 \" -j DROP\"}'\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Trafic sain, IP non hostile ou bilan documentaire nominal.\n"
    "     1 (RAP_EXIT_THREAT)  Connexion active vers un hôte hostile ou IP au score >= 70.\n"
    "     2 (RAP_EXIT_ERROR)   Format IPv4 invalide, catégorie inconnue ou droits insuffisants.\n"
    "\n"
    "\n"
    "RAP-SENTRY(1)               RAPTOR DEFENSE               RAP-SENTRY(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap sentry — Sentinelle passive anti-rançongiciel (canaris, fanotify et boîte noire)\n"
    "\n"
    "SYNOPSIS\n"
    "     rap sentry <arm | disarm | status | dump | run [-d] [--arm|--dry-run]> [chemins...]\n"
    "     rap se <arm | disarm | status | dump | run [-d] [--arm|--dry-run]> [chemins...]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap sentry est un bouclier cinétique passif d'interception synchrone\n"
    "     anti-rançongiciel basé sur l'infrastructure noyau fanotify.\n"
    "     \n"
    "     Contrairement aux antivirus conventionnels qui consomment des cycles CPU\n"
    "     en scrutation continue, sentry délègue la veille passive au planificateur\n"
    "     du noyau (0% CPU au repos). L'interception intervient au niveau Ring 0 dès\n"
    "     l'instruction sys_open() sur les leurres denses.\n"
    "\n"
    "SOUS-COMMANDES\n"
    "     arm [chemins...]\n"
    "             Déploie des fichiers leurres denses (!00_raptor_vault.kdbx, 32 Ko,\n"
    "             signature KeePass KDBX4 authentique et entropie LCG). Cibles par\n"
    "             défaut : /home, /var/www, /var/lib, /etc.\n"
    "\n"
    "     disarm [chemins...]\n"
    "             Supprime proprement les fichiers canaris sans déclencher d'alerte\n"
    "             pour préparer des opérations de maintenance.\n"
    "\n"
    "     status\n"
    "             Vérifie la présence physique, la conformité de taille et l'état\n"
    "             d'armement des canaris ainsi que les statistiques du moteur réflexe.\n"
    "\n"
    "     dump, blackbox\n"
    "             Restitue l'ensemble des enregistrements médico-légaux conservés dans\n"
    "             la boîte noire circulaire en mémoire vive (/dev/shm/raptor_blackbox).\n"
    "             Accessible sans privilèges root (permissions 0666).\n"
    "\n"
    "     run [-d] [--arm|--dry-run] [chemins...]\n"
    "             Enclenche le guet passif noyau FAN_OPEN_PERM.\n"
    "             - Sans option : Mode passif direct (rejet FAN_DENY et alerte seule).\n"
    "             - Option -a, --arm : Remédiation active (gel SIGSTOP, coupure socket, SIGKILL).\n"
    "             - Option --dry-run : Simulation de neutralisation avec trace en boîte noire.\n"
    "             - Option -d   : Détachement en démon d'arrière-plan autonome (fork/setsid),\n"
    "                             camouflage prctl ([kworker/u16:3]), verrouillage mémoire\n"
    "                             physique (mlockall) et immunité OOM (/proc/self/oom_score_adj = -1000).\n"
    "\n"
    "MÉCANISME DE RIPOSTE RÉFLEXE\n"
    "     1. Un processus hostile tente d'ouvrir le fichier leurre (!00_raptor_vault.kdbx).\n"
    "     2. Le noyau Linux suspend immédiatement l'agresseur dans son appel open().\n"
    "     3. Contrôle des gardes-fous autonomes (PID <= 1, sentinelle, shell parent).\n"
    "     4. Rejet catégorique de l'ouverture auprès du noyau via FAN_DENY.\n"
    "     5. En mode armé : paralysie immédiate du process group par SIGSTOP.\n"
    "     6. Révocation forcée des descripteurs de socket (/proc/[pid]/fd).\n"
    "     7. Élimination définitive de l'agresseur par émission de SIGKILL.\n"
    "     8. Enregistrement médico-légal atomique dans l'anneau circulaire en RAM (/dev/shm).\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Guet nominal interrompu proprement par signal.\n"
    "     1 (RAP_EXIT_THREAT)  Atteinte à un canari détectée et riposte réflexe appliquée.\n"
    "     2 (RAP_EXIT_ERROR)   Droits insuffisants (CAP_SYS_ADMIN requis) ou cible invalide.\n"
    "\n"
    "DROITS SYSTÈME REQUIS\n"
    "     Requiert les privilèges root ou les capacités :\n"
    "     setcap 'cap_sys_admin,cap_kill,cap_ipc_lock,cap_sys_ptrace=+ep' /usr/local/bin/rap\n"
    "\n"
    "\n"
    "RAP-KILL(1)                 RAPTOR DEFENSE                 RAP-KILL(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap kill — Neutralisation chirurgicale d'un processus hostile\n"
    "\n"
    "SYNOPSIS\n"
    "     rap kill [-f] <PID>\n"
    "\n"
    "DESCRIPTION\n"
    "     Transmet le signal non rattrapable SIGKILL (9) au PID spécifié.\n"
    "     \n"
    "     GARDES-FOUS INVIOLABLES :\n"
    "     Refuse systématiquement le tir si la cible est le PID 1 (init/systemd),\n"
    "     le PID de l'utilitaire, le shell parent (PPID) ou un thread interne du noyau.\n"
    "     Exige une confirmation manuelle [o/N] sauf si l'option -f est présente.\n"
    "\n"
    "\n"
    "RAP-FREEZE(1)               RAPTOR DEFENSE               RAP-FREEZE(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap freeze — Gel instantané d'une menace pour investigation mémoire\n"
    "\n"
    "SYNOPSIS\n"
    "     rap freeze <PID>\n"
    "\n"
    "DESCRIPTION\n"
    "     Émet le signal SIGSTOP sur le processus cible pour le placer dans l'état\n"
    "     noyau T (Stopped). Permet de suspendre toute activité malveillante tout en\n"
    "     conservant l'intégralité de sa mémoire virtuelle intacte pour copie forensique.\n"
    "\n"
    "\n"
    "RAPLESS(1)                  RAPTOR DEFENSE                  RAPLESS(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rapless — Pager de survie autonome sans allocation dynamique\n"
    "\n"
    "SYNOPSIS\n"
    "     rapless [fichier]\n"
    "     commande | rapless\n"
    "\n"
    "DESCRIPTION\n"
    "     rapless est un téléscripteur d'urgence écrit en C99 n'effectuant aucun\n"
    "     appel à malloc/calloc (0 heap). Il indexe par pagination statique les flux\n"
    "     massifs (testé jusqu'à 25 000 lignes sous /proc/kallsyms) sans saturer la\n"
    "     mémoire résidente de la machine hôte.\n"
    "\n"
    "EXIT STATUS\n"
    "     0    Fermeture nominale du visualiseur (touche 'q').\n"
    "     1    Erreur d'accès au fichier source ou flux vide.\n"
    "\n"
    "\n"
    "RAP-NETOP(1)                RAPTOR DEFENSE                RAP-NETOP(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap netop — Console tactique réseau temps réel matricielle, 0-heap et remédiation active\n"
    "\n"
    "SYNOPSIS\n"
    "     rap netop [-n nombre]\n"
    "     rap no [-n nombre]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap netop projette une console d'audit et de défense réseau active opérant\n"
    "     sans aucune allocation dynamique (0-heap) et sans scintillement (flicker-free).\n"
    "     \n"
    "     Il ingère en flux continu les structures /proc/net/tcp du noyau Linux, résout\n"
    "     les descripteurs Inode vers les PIDs et processus propriétaires, et confronte\n"
    "     chaque flux distant à la base souveraine de renseignement OSINT (19 867 signatures\n"
    "     actives). L'architecture repose sur un routeur de commandes découplé permettant\n"
    "     la neutralisation cinétique, l'embargo réseau et l'autopsie médico-légale sans\n"
    "     jamais interrompre la surveillance.\n"
    "\n"
    "OPTIONS\n"
    "     -n nombre\n"
    "             Fixe le nombre maximal de lignes de sockets affichées simultanément\n"
    "             sur la grille matricielle (valeur par défaut : 20, max : 256).\n"
    "\n"
    "COMMANDES INTERACTIVES AU CLAVIER (HOTKEYS)\n"
    "     k       Neutralisation réflexe chirurgicale (Kill).\n"
    "             Ouvre l'invite [FRAPPE REFLEXE]. Neutralise le processus par émission\n"
    "             séquentielle de SIGSTOP puis SIGKILL, mesure la latence à la microseconde\n"
    "             et active une trace rémanente ambrée (tombstone) de 7 secondes.\n"
    "             Gardes-fous stricts : tir proscrit sur PID 1, RAPTOR et shell parent.\n"
    "\n"
    "     b       Embargo et réhabilitation réseau (Ban/Déban IP).\n"
    "             Ouvre l'invite [BAN IP / DÉBAN]. Permet d'isoler une ou plusieurs cibles\n"
    "             séparées par une virgule ou un espace (ex: b 4,6,9).\n"
    "             L'injection applique une règle Netfilter (iptables -I INPUT -s IP -j DROP)\n"
    "             avec repli automatique sur une route trou noir (ip route add blackhole).\n"
    "             Le préfixe moins permet de lever l'embargo (ex: b -4, -6 ou b -B1).\n"
    "\n"
    "     Ctrl+B  Registre interactif des embargos noyau.\n"
    "             Bascule l'affichage vers la surcouche modale recensant l'ensemble des IP\n"
    "             verrouillées (#B01, #B02...), la durée d'activation, le mécanisme actif\n"
    "             et le volume de sockets associés. Le déban peut être exécuté directement\n"
    "             depuis cette vue via b 1, 2 ou b -1 -2.\n"
    "\n"
    "     s       Bascule instantanée du mode de tri (Sort Toggle).\n"
    "             Alterne cycliquement l'ordonnancement des flux sans réallocation mémoire :\n"
    "               1. DANGEROSITÉ : Float-to-top (KILLED > HOSTILE > WARN > ESTABLISHED > LISTEN).\n"
    "               2. PID         : Ordre croissant des identifiants processus système.\n"
    "               3. PORT/ACT.   : Regroupement par port distant et activité.\n"
    "\n"
    "     f       Filtre dynamique de flux (Filter Toggle).\n"
    "             Alterne instantanément le masque d'affichage par compaction in-place :\n"
    "               1. TOUS         : Vue exhaustive de tous les sockets noyau.\n"
    "               2. HORS LOCAL   : Masque le bruit 127.0.0.1, 0.0.0.0 et l'écoute LISTEN.\n"
    "               3. ÉTABLIS      : Isole strictement les connexions connectées ESTABLISHED.\n"
    "               4. ALERTES      : Cockpit de crise (HOSTILE, WARN, KILLED et BAN).\n"
    "\n"
    "     i       Inspection médico-légale approfondie (Forensic Overlay).\n"
    "             Ouvre l'invite [INSPECTION MÉDICO-LÉGALE] et projette l'autopsie du slot :\n"
    "               - Généalogie complète : PID, PPID et nom du processus parent (/proc).\n"
    "               - Intégrité binaire   : Ligne de commande brute et exécutable réel (/proc/[PID]/exe).\n"
    "               - Anti-Reverse Shell  : Analyse des flux STDIN (fd/0), STDOUT (fd/1) et détection\n"
    "                                       de détournement vers un socket réseau.\n"
    "               - Renseignement OSINT : Catégorisation de la menace (C2, Tor, Abus) et score.\n"
    "               - Actions directes    : Frappe 'k' ou embargo 'b' exécutables depuis la fiche.\n"
    "\n"
    "     p, SPC  Pause / Reprise de la capture.\n"
    "             Fige l'affichage du flux réseau. Les opérations de tri ('s') et de filtrage\n"
    "             ('f') restent immédiatement opérationnelles sur le snapshot gelé.\n"
    "\n"
    "     q, ESC  Fermeture de modale ou sortie propre de l'application.\n"
    "             Ferme la vue modale active (Ctrl+B ou inspection), ou restaure la configuration\n"
    "             du terminal (mode canonique, réaffichage du curseur) et quitte l'outil.\n"
    "\n"
    "DROITS SYSTÈME REQUIS\n"
    "     L'affichage passif et l'inspection ne requièrent que des droits utilisateurs standard.\n"
    "     L'application des embargos réseau (b / iptables) et les frappes cinétiques (k / kill)\n"
    "     requièrent les privilèges superutilisateur (sudo) ou les capacités Linux :\n"
    "     setcap 'cap_net_admin,cap_kill,cap_sys_ptrace=+ep' /usr/local/bin/rap\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Fermeture nominale de la console interactive.\n"
    "     2 (RAP_EXIT_ERROR)   Terminal non interactif (absence de TTY) ou erreur d'initialisation.\n";

/* Structure d'indexation unitaire des pages de manuel (0 heap) */
typedef struct {
    const char *name;       /* Clé de commande (ex: "rap", "rap-sentry") */
    const char *section;    /* Numéro de section Unix (ex: "1") */
    const char *title;      /* Titre descriptif */
    const char *content;    /* Contenu unitaire formaté avec bannière */
} RapManPageEntry;

static const RapManPageEntry g_rap_man_pages[] = {
    {
        "rap",
        "1",
        "RAP(1) — Outils Généraux",
    "RAP(1)                      RAPTOR DEFENSE                      RAP(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap — Console chirurgicale, traque mémoire 0-heap et remédiation active réflexe\n"
    "\n"
    "SYNOPSIS\n"
    "     rap [-q] [-r] [-f] [-v] [-a|--arm] [--dry-run] [-n nombre] commande [arguments...]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap est l'utilitaire autonome en ligne de commande de RAPTOR v2.0.\n"
    "     Conçu selon la norme C99 pour fonctionner sous famine mémoire extrême\n"
    "     (< 2 Mo RAM) ou en cas d'effondrement du serveur d'affichage (X11/Wayland),\n"
    "     il audite passivement les structures du noyau (/proc) et neutralise les menaces.\n"
    "\n"
    "     Dans sa révision réflexe, rap intègre un disjoncteur chirurgical de dernier\n"
    "     recours (Last-Resort Watchdog) capable d'opérer sous contrainte d'épuisement\n"
    "     mémoire (mlockall et score OOM -1000) pour paralyser et détruire les menaces\n"
    "     en moins d'une milliseconde sans dépendre du disque ni du réseau.\n"
    "\n"
    "OPTIONS UNIVERSELLES\n"
    "     -q, --quiet\n"
    "             Mode silencieux. Supprime les en-têtes et les lignes de séparation\n"
    "             pour ne conserver que la sortie brute.\n"
    "\n"
    "     -r, --raw\n"
    "             Format machine TSV pur. Utilise des tabulations sans arrondir les\n"
    "             unités pour faciliter le traitement via awk, grep ou cut.\n"
    "\n"
    "     -f, --force\n"
    "             Désactive les demandes de confirmation interactive lors des actions\n"
    "             cinétiques (kill et freeze).\n"
    "\n"
    "     -v, --verbose\n"
    "             Mode d'audit forensique étendu. Révèle notamment les segments mémoires\n"
    "             autorisés par mem_whitelist.conf avec le tag [LISTE BLANCHE].\n"
    "\n"
    "     -a, --arm\n"
    "             Arme la riposte cinétique immédiate (SIGSTOP -> Révocation réseau ->\n"
    "             Extraction boîte noire -> SIGKILL). Active également l'immunité OOM\n"
    "             (/proc/self/oom_score_adj = -1000) et le verrouillage mémoire physique.\n"
    "\n"
    "     --dry-run\n"
    "             Mode simulation opérationnelle. Exécute l'intégralité du pipeline réflexe\n"
    "             et enregistre l'incident dans la boîte noire RAM (/dev/shm/raptor_blackbox)\n"
    "             sans émettre les signaux de destruction (SIGSTOP/SIGKILL).\n"
    "\n"
    "     -n nombre\n"
    "             Limite le nombre de lignes projetées (applicable à top et net).\n"
    "\n"
    "COMMANDES\n"
    "     status, st   Diagnostic vital instantané de la machine hôte.\n"
    "     top          Top des processus consommateurs de mémoire vive résidente.\n"
    "     hunt         Audit combiné universel (Mémoire, Preload, Heuristiques) avec riposte réflexe.\n"
    "     rx           Traque des pages mémoires anonymes exécutables (Inode 0) et neutralisation.\n"
    "     preload      Audit d'intégrité du chargeur dynamique (/etc/ld.so.preload).\n"
    "     heur, he     Analyse heuristique (binaires supprimés, chemins volatils).\n"
    "     net          Audit TCP, corrélation PID et forensic IP (Net-Guard / OSINT).\n"
    "     netop, no    Console tactique temps réel matricielle (TUI), audit continu et remédiation active.\n"
    "     sentry, se   Sentinelle passive anti-rançongiciel (canaris, fanotify et boîte noire).\n"
    "     kill <PID>   Neutralisation immédiate par émission du signal SIGKILL.\n"
    "     freeze <PID> Gel conservatoire via signal SIGSTOP pour examen forensique.\n"
    "     man [cmd]    Manuel de référence autonome embarqué (SSOT).\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Système nominal. Aucune anomalie non autorisée.\n"
    "     1 (RAP_EXIT_THREAT)  Menace critique active identifiée sur l'hôte.\n"
    "     2 (RAP_EXIT_ERROR)   Erreur de syntaxe, droits insuffisants ou cible invalide.\n"
    "\n"
    "AUTHORS\n"
    "     Conçu par l'équipe d'ingénierie RAPTOR DEFENSE.\n"
    },
    {
        "rap-status",
        "1",
        "RAP-STATUS(1) — Diagnostic Vital",
    "RAP-STATUS(1)               RAPTOR DEFENSE               RAP-STATUS(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap status — Diagnostic vital instantané de la machine hôte\n"
    "\n"
    "SYNOPSIS\n"
    "     rap status [-q] [-r]\n"
    "     rap st\n"
    "\n"
    "DESCRIPTION\n"
    "     Extrait en moins de 15 ms les métriques fondamentales de santé du système :\n"
    "     RAM totale et disponible (/proc/meminfo), dénombrement des processus\n"
    "     actifs et vérification d'altération du fichier /etc/ld.so.preload.\n"
    "\n"
    "EXIT STATUS\n"
    "     0    Système sain et intègre.\n"
    "     1    Présence suspecte du fichier /etc/ld.so.preload.\n"
    },
    {
        "rap-top",
        "1",
        "RAP-TOP(1) — Consommation RAM",
    "RAP-TOP(1)                  RAPTOR DEFENSE                  RAP-TOP(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap top — Top des processus consommateurs de mémoire vive résidente (VmRSS)\n"
    "\n"
    "SYNOPSIS\n"
    "     rap top [-q] [-r] [-n nombre]\n"
    "\n"
    "DESCRIPTION\n"
    "     Analyse les structures /proc/[PID]/status pour extraire la mémoire physique\n"
    "     réelle résidente (VmRSS). Élimine les threads noyau (RSS = 0) et classe les\n"
    "     processus par tri statique sur la pile (0 octet sur le tas).\n"
    },
    {
        "rap-hunt",
        "1",
        "RAP-HUNT(1) — Chasseur Universel",
    "RAP-HUNT(1)                 RAPTOR DEFENSE                 RAP-HUNT(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap hunt — Audit de sécurité universel combiné et riposte active\n"
    "\n"
    "SYNOPSIS\n"
    "     rap hunt [-q] [-r] [-v] [-a|--arm] [--dry-run]\n"
    "\n"
    "DESCRIPTION\n"
    "     Coordonne séquentiellement les trois grandes sondes d'investigation :\n"
    "       1. Traque des mutations mémoires anonymes (rap rx).\n"
    "       2. Traque des détournements LD_PRELOAD (rap preload).\n"
    "       3. Analyse heuristique d'exécution (rap heur).\n"
    "\n"
    "     En mode armé (-a, --arm), toute mutation Inode 0 R-X non autorisée ou\n"
    "     détournement LD_PRELOAD déclenche immédiatement le moteur de remédiation\n"
    "     rap_reflex (paralysie, isolation des descripteurs de socket et purge SIGKILL).\n"
    "\n"
    "     L'option --dry-run permet de simuler la riposte et d'alimenter la boîte noire\n"
    "     médico-légale sans impacter les processus audités.\n"
    "\n"
    "EXIT STATUS\n"
    "     0    Aucune anomalie détectée sur les trois vecteurs.\n"
    "     1    Au moins une anomalie hostile détectée sur l'un des contrôles.\n"
    },
    {
        "rap-rx",
        "1",
        "RAP-RX(1) — Traque Mémoire Inode 0",
    "RAP-RX(1)                   RAPTOR DEFENSE                   RAP-RX(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap rx — Traque des pages mémoires anonymes exécutables (Inode 0)\n"
    "\n"
    "SYNOPSIS\n"
    "     rap rx [-q] [-r] [-v] [-a|--arm] [--dry-run]\n"
    "\n"
    "DESCRIPTION\n"
    "     Parcourt /proc/[PID]/maps pour identifier les régions mémoires marquées r-xp\n"
    "     dont l'Inode est égal à 0 (aucun fichier adossé sur disque).\n"
    "     \n"
    "     Par défaut, filtre automatiquement le bruit des moteurs JIT (Chromium, Node)\n"
    "     et des liaisons libffi (Cinnamon, GTK) selon assets/rules/mem_whitelist.conf.\n"
    "     L'option -v permet de révéler les allocations légitimes approuvées.\n"
    "\n"
    "     OPTIONS DE RÉACTION :\n"
    "       -a, --arm    : Neutralise sur-le-champ tout processus non approuvé via SIGKILL.\n"
    "       --dry-run    : Simule la riposte et consigne l'incident dans /dev/shm/raptor_blackbox.\n"
    "\n"
    "EXAMPLES\n"
    "     Chasser les injections mémoires et abattre immédiatement les processus hostiles :\n"
    "     sudo rap rx --arm\n"
    "\n"
    "     Auditer les mutations sans altérer l'état du système :\n"
    "     sudo rap rx --dry-run\n"
    },
    {
        "rap-preload",
        "1",
        "RAP-PRELOAD(1) — Audit Chargeur Dynamique",
    "RAP-PRELOAD(1)              RAPTOR DEFENSE              RAP-PRELOAD(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap preload — Détection des rootkits userland via LD_PRELOAD\n"
    "\n"
    "SYNOPSIS\n"
    "     rap preload [-q] [-r]\n"
    "\n"
    "DESCRIPTION\n"
    "     Effectue une double vérification défensive :\n"
    "       1. Contrôle d'accès et d'intégrité de /etc/ld.so.preload.\n"
    "       2. Parcours des blocs d'environnement (/proc/[PID]/environ) délimités par\n"
    "          des octets nuls pour intercepter la variable hostile LD_PRELOAD=.\n"
    },
    {
        "rap-heur",
        "1",
        "RAP-HEUR(1) — Analyse Heuristique",
    "RAP-HEUR(1)                 RAPTOR DEFENSE                 RAP-HEUR(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap heur — Analyse heuristique des anomalies de lancement et d'usurpation\n"
    "\n"
    "SYNOPSIS\n"
    "     rap heur [-q] [-r]\n"
    "     rap he\n"
    "\n"
    "DESCRIPTION\n"
    "     Résout les liens /proc/[PID]/exe pour traquer :\n"
    "       1. Les binaires fantômes supprimés du disque en cours d'exécution (deleted).\n"
    "       2. Les processus exécutés depuis des répertoires temporaires (/tmp, /dev/shm).\n"
    "       3. L'usurpation d'identité de démons officiels (systemd, sshd, kworker).\n"
    },
    {
        "rap-net",
        "1",
        "RAP-NET(1) — Sonde Réseau & Forensic Net-Intel",
    "RAP-NET(1)                  RAPTOR DEFENSE                  RAP-NET(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap net — Surveillance réseau, corrélation PID et forensic IP souverain\n"
    "\n"
    "SYNOPSIS\n"
    "     rap net [-q] [-r] [-n nombre]\n"
    "     rap net -i <ADRESSE_IPV4>\n"
    "     rap net <ADRESSE_IPV4>\n"
    "     rap net -l [c2 | tor | abuse | all]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap net est le module d'audit passif des communications réseau et de\n"
    "     renseignement tactique sur les menaces (Threat Intelligence).\n"
    "     \n"
    "     Il combine l'inspection locale des descripteurs de sockets du noyau Linux\n"
    "     (/proc/net/tcp et /proc/[PID]/fd/) à une base de réputation OSINT souveraine\n"
    "     embarquée directement en section .rodata (zéro allocation dynamique, 0 heap).\n"
    "     Toutes les vérifications s'exécutent en mémoire pure sans émettre le moindre\n"
    "     paquet sur le réseau extérieur (silence radio et furtivité absolue).\n"
    "\n"
    "MODES OPÉRATOIRES\n"
    "     1. Surveillance active des flux établis :\n"
    "        rap net [-q] [-r] [-n nombre]\n"
    "        Énumère en temps réel les connexions TCP à l'état ESTABLISHED.\n"
    "        Chaque flux est enrichi instantanément par :\n"
    "          - La corrélation du socket Inode avec le PID et le binaire local (/proc/[PID]/comm).\n"
    "          - La confrontation arithmétique O(log N) avec la table de menaces OSINT.\n"
    "          - La qualification Net-Guard (APPROUVÉ, EXTERNE, SUSPECT, DANGER C2, ALERTE TOR).\n"
    "\n"
    "     2. Enquête médico-légale unitaire :\n"
    "        rap net -i <ADRESSE_IPV4>\n"
    "        rap net <ADRESSE_IPV4>\n"
    "        Édite la fiche forensique complète de l'adresse cible sans contact externe :\n"
    "          - Reconnaissance immédiate des plages privées RFC 1918 et Loopback.\n"
    "          - Score de menace consolidé (0 à 100) et qualification de la source hostile.\n"
    "          - Détection de tout socket actif local connecté à cette IP avec chemin du binaire (/proc/[PID]/exe).\n"
    "          - Génération immédiate des règles de neutralisation iptables et nftables.\n"
    "\n"
    "     3. Inventaire et consultation de la base de signatures :\n"
    "        rap net -l [c2 | tor | abuse | all]\n"
    "        - Sans argument : Affiche le bilan vital de la base (date, volume, répartition).\n"
    "        - Avec argument : Déverse les adresses filtrées par catégorie pour pagination\n"
    "          dans le téléscripteur rapless ou exploitation dans des scripts d'administration.\n"
    "\n"
    "CATÉGORIES OSINT INTÉGRÉES\n"
    "     c2       Serveurs de commande et contrôle (C2), rançongiciels et botnets (abuse.ch Feodo).\n"
    "     tor      Relais de sortie officiels du réseau d'anonymisation Tor.\n"
    "     abuse    Adresses d'attaquants récurrents (scanners, brute-force SSH, fraudes IPsum).\n"
    "     all      Ensemble exhaustif des signatures consolidées.\n"
    "\n"
    "EXAMPLES\n"
    "     Surveiller les flux et isoler les alertes :\n"
    "     rap net\n"
    "\n"
    "     Auditer chirurgicalement une IP suspecte repérée dans les journaux :\n"
    "     rap net -i 194.26.29.112\n"
    "\n"
    "     Inspecter les serveurs C2 actifs dans le téléscripteur zéro-heap :\n"
    "     rap net -l c2 | rapless\n"
    "\n"
    "     Extraire les IP malveillantes pour alimentation d'un pare-feu local :\n"
    "     rap net -q -r -l c2 | awk '{print \"iptables -A OUTPUT -d \" $1 \" -j DROP\"}'\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Trafic sain, IP non hostile ou bilan documentaire nominal.\n"
    "     1 (RAP_EXIT_THREAT)  Connexion active vers un hôte hostile ou IP au score >= 70.\n"
    "     2 (RAP_EXIT_ERROR)   Format IPv4 invalide, catégorie inconnue ou droits insuffisants.\n"
    },
    {
        "rap-sentry",
        "1",
        "RAP-SENTRY(1) — Sentinelle Anti-Rançongiciel",
    "RAP-SENTRY(1)               RAPTOR DEFENSE               RAP-SENTRY(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap sentry — Sentinelle passive anti-rançongiciel (canaris, fanotify et boîte noire)\n"
    "\n"
    "SYNOPSIS\n"
    "     rap sentry <arm | disarm | status | dump | run [-d] [--arm|--dry-run]> [chemins...]\n"
    "     rap se <arm | disarm | status | dump | run [-d] [--arm|--dry-run]> [chemins...]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap sentry est un bouclier cinétique passif d'interception synchrone\n"
    "     anti-rançongiciel basé sur l'infrastructure noyau fanotify.\n"
    "     \n"
    "     Contrairement aux antivirus conventionnels qui consomment des cycles CPU\n"
    "     en scrutation continue, sentry délègue la veille passive au planificateur\n"
    "     du noyau (0% CPU au repos). L'interception intervient au niveau Ring 0 dès\n"
    "     l'instruction sys_open() sur les leurres denses.\n"
    "\n"
    "SOUS-COMMANDES\n"
    "     arm [chemins...]\n"
    "             Déploie des fichiers leurres denses (!00_raptor_vault.kdbx, 32 Ko,\n"
    "             signature KeePass KDBX4 authentique et entropie LCG). Cibles par\n"
    "             défaut : /home, /var/www, /var/lib, /etc.\n"
    "\n"
    "     disarm [chemins...]\n"
    "             Supprime proprement les fichiers canaris sans déclencher d'alerte\n"
    "             pour préparer des opérations de maintenance.\n"
    "\n"
    "     status\n"
    "             Vérifie la présence physique, la conformité de taille et l'état\n"
    "             d'armement des canaris ainsi que les statistiques du moteur réflexe.\n"
    "\n"
    "     dump, blackbox\n"
    "             Restitue l'ensemble des enregistrements médico-légaux conservés dans\n"
    "             la boîte noire circulaire en mémoire vive (/dev/shm/raptor_blackbox).\n"
    "             Accessible sans privilèges root (permissions 0666).\n"
    "\n"
    "     run [-d] [--arm|--dry-run] [chemins...]\n"
    "             Enclenche le guet passif noyau FAN_OPEN_PERM.\n"
    "             - Sans option : Mode passif direct (rejet FAN_DENY et alerte seule).\n"
    "             - Option -a, --arm : Remédiation active (gel SIGSTOP, coupure socket, SIGKILL).\n"
    "             - Option --dry-run : Simulation de neutralisation avec trace en boîte noire.\n"
    "             - Option -d   : Détachement en démon d'arrière-plan autonome (fork/setsid),\n"
    "                             camouflage prctl ([kworker/u16:3]), verrouillage mémoire\n"
    "                             physique (mlockall) et immunité OOM (/proc/self/oom_score_adj = -1000).\n"
    "\n"
    "MÉCANISME DE RIPOSTE RÉFLEXE\n"
    "     1. Un processus hostile tente d'ouvrir le fichier leurre (!00_raptor_vault.kdbx).\n"
    "     2. Le noyau Linux suspend immédiatement l'agresseur dans son appel open().\n"
    "     3. Contrôle des gardes-fous autonomes (PID <= 1, sentinelle, shell parent).\n"
    "     4. Rejet catégorique de l'ouverture auprès du noyau via FAN_DENY.\n"
    "     5. En mode armé : paralysie immédiate du process group par SIGSTOP.\n"
    "     6. Révocation forcée des descripteurs de socket (/proc/[pid]/fd).\n"
    "     7. Élimination définitive de l'agresseur par émission de SIGKILL.\n"
    "     8. Enregistrement médico-légal atomique dans l'anneau circulaire en RAM (/dev/shm).\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Guet nominal interrompu proprement par signal.\n"
    "     1 (RAP_EXIT_THREAT)  Atteinte à un canari détectée et riposte réflexe appliquée.\n"
    "     2 (RAP_EXIT_ERROR)   Droits insuffisants (CAP_SYS_ADMIN requis) ou cible invalide.\n"
    "\n"
    "DROITS SYSTÈME REQUIS\n"
    "     Requiert les privilèges root ou les capacités :\n"
    "     setcap 'cap_sys_admin,cap_kill,cap_ipc_lock,cap_sys_ptrace=+ep' /usr/local/bin/rap\n"
    },
    {
        "rap-kill",
        "1",
        "RAP-KILL(1) — Riposte Cinétique",
    "RAP-KILL(1)                 RAPTOR DEFENSE                 RAP-KILL(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap kill — Neutralisation chirurgicale d'un processus hostile\n"
    "\n"
    "SYNOPSIS\n"
    "     rap kill [-f] <PID>\n"
    "\n"
    "DESCRIPTION\n"
    "     Transmet le signal non rattrapable SIGKILL (9) au PID spécifié.\n"
    "     \n"
    "     GARDES-FOUS INVIOLABLES :\n"
    "     Refuse systématiquement le tir si la cible est le PID 1 (init/systemd),\n"
    "     le PID de l'utilitaire, le shell parent (PPID) ou un thread interne du noyau.\n"
    "     Exige une confirmation manuelle [o/N] sauf si l'option -f est présente.\n"
    },
    {
        "rap-freeze",
        "1",
        "RAP-FREEZE(1) — Gel Conservatoire Forensique",
    "RAP-FREEZE(1)               RAPTOR DEFENSE               RAP-FREEZE(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap freeze — Gel instantané d'une menace pour investigation mémoire\n"
    "\n"
    "SYNOPSIS\n"
    "     rap freeze <PID>\n"
    "\n"
    "DESCRIPTION\n"
    "     Émet le signal SIGSTOP sur le processus cible pour le placer dans l'état\n"
    "     noyau T (Stopped). Permet de suspendre toute activité malveillante tout en\n"
    "     conservant l'intégralité de sa mémoire virtuelle intacte pour copie forensique.\n"
    },
    {
        "rapless",
        "1",
        "RAPLESS(1) — Téléscripteur Zéro-Heap",
    "RAPLESS(1)                  RAPTOR DEFENSE                  RAPLESS(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rapless — Pager de survie autonome sans allocation dynamique\n"
    "\n"
    "SYNOPSIS\n"
    "     rapless [fichier]\n"
    "     commande | rapless\n"
    "\n"
    "DESCRIPTION\n"
    "     rapless est un téléscripteur d'urgence écrit en C99 n'effectuant aucun\n"
    "     appel à malloc/calloc (0 heap). Il indexe par pagination statique les flux\n"
    "     massifs (testé jusqu'à 25 000 lignes sous /proc/kallsyms) sans saturer la\n"
    "     mémoire résidente de la machine hôte.\n"
    "\n"
    "EXIT STATUS\n"
    "     0    Fermeture nominale du visualiseur (touche 'q').\n"
    "     1    Erreur d'accès au fichier source ou flux vide.\n"
    },
    {
        "rap-netop",
        "1",
        "RAP-NETOP(1) — Console Tactique Temps Réel",
    "RAP-NETOP(1)                RAPTOR DEFENSE                RAP-NETOP(1)\n"
    "======================================================================\n"
    "\n"
    "NAME\n"
    "     rap netop — Console tactique réseau temps réel matricielle, 0-heap et remédiation active\n"
    "\n"
    "SYNOPSIS\n"
    "     rap netop [-n nombre]\n"
    "     rap no [-n nombre]\n"
    "\n"
    "DESCRIPTION\n"
    "     rap netop projette une console d'audit et de défense réseau active opérant\n"
    "     sans aucune allocation dynamique (0-heap) et sans scintillement (flicker-free).\n"
    "     \n"
    "     Il ingère en flux continu les structures /proc/net/tcp du noyau Linux, résout\n"
    "     les descripteurs Inode vers les PIDs et processus propriétaires, et confronte\n"
    "     chaque flux distant à la base souveraine de renseignement OSINT (19 867 signatures\n"
    "     actives). L'architecture repose sur un routeur de commandes découplé permettant\n"
    "     la neutralisation cinétique, l'embargo réseau et l'autopsie médico-légale sans\n"
    "     jamais interrompre la surveillance.\n"
    "\n"
    "OPTIONS\n"
    "     -n nombre\n"
    "             Fixe le nombre maximal de lignes de sockets affichées simultanément\n"
    "             sur la grille matricielle (valeur par défaut : 20, max : 256).\n"
    "\n"
    "COMMANDES INTERACTIVES AU CLAVIER (HOTKEYS)\n"
    "     k       Neutralisation réflexe chirurgicale (Kill).\n"
    "             Ouvre l'invite [FRAPPE REFLEXE]. Neutralise le processus par émission\n"
    "             séquentielle de SIGSTOP puis SIGKILL, mesure la latence à la microseconde\n"
    "             et active une trace rémanente ambrée (tombstone) de 7 secondes.\n"
    "             Gardes-fous stricts : tir proscrit sur PID 1, RAPTOR et shell parent.\n"
    "\n"
    "     b       Embargo et réhabilitation réseau (Ban/Déban IP).\n"
    "             Ouvre l'invite [BAN IP / DÉBAN]. Permet d'isoler une ou plusieurs cibles\n"
    "             séparées par une virgule ou un espace (ex: b 4,6,9).\n"
    "             L'injection applique une règle Netfilter (iptables -I INPUT -s IP -j DROP)\n"
    "             avec repli automatique sur une route trou noir (ip route add blackhole).\n"
    "             Le préfixe moins permet de lever l'embargo (ex: b -4, -6 ou b -B1).\n"
    "\n"
    "     Ctrl+B  Registre interactif des embargos noyau.\n"
    "             Bascule l'affichage vers la surcouche modale recensant l'ensemble des IP\n"
    "             verrouillées (#B01, #B02...), la durée d'activation, le mécanisme actif\n"
    "             et le volume de sockets associés. Le déban peut être exécuté directement\n"
    "             depuis cette vue via b 1, 2 ou b -1 -2.\n"
    "\n"
    "     s       Bascule instantanée du mode de tri (Sort Toggle).\n"
    "             Alterne cycliquement l'ordonnancement des flux sans réallocation mémoire :\n"
    "               1. DANGEROSITÉ : Float-to-top (KILLED > HOSTILE > WARN > ESTABLISHED > LISTEN).\n"
    "               2. PID         : Ordre croissant des identifiants processus système.\n"
    "               3. PORT/ACT.   : Regroupement par port distant et activité.\n"
    "\n"
    "     f       Filtre dynamique de flux (Filter Toggle).\n"
    "             Alterne instantanément le masque d'affichage par compaction in-place :\n"
    "               1. TOUS         : Vue exhaustive de tous les sockets noyau.\n"
    "               2. HORS LOCAL   : Masque le bruit 127.0.0.1, 0.0.0.0 et l'écoute LISTEN.\n"
    "               3. ÉTABLIS      : Isole strictement les connexions connectées ESTABLISHED.\n"
    "               4. ALERTES      : Cockpit de crise (HOSTILE, WARN, KILLED et BAN).\n"
    "\n"
    "     i       Inspection médico-légale approfondie (Forensic Overlay).\n"
    "             Ouvre l'invite [INSPECTION MÉDICO-LÉGALE] et projette l'autopsie du slot :\n"
    "               - Généalogie complète : PID, PPID et nom du processus parent (/proc).\n"
    "               - Intégrité binaire   : Ligne de commande brute et exécutable réel (/proc/[PID]/exe).\n"
    "               - Anti-Reverse Shell  : Analyse des flux STDIN (fd/0), STDOUT (fd/1) et détection\n"
    "                                       de détournement vers un socket réseau.\n"
    "               - Renseignement OSINT : Catégorisation de la menace (C2, Tor, Abus) et score.\n"
    "               - Actions directes    : Frappe 'k' ou embargo 'b' exécutables depuis la fiche.\n"
    "\n"
    "     p, SPC  Pause / Reprise de la capture.\n"
    "             Fige l'affichage du flux réseau. Les opérations de tri ('s') et de filtrage\n"
    "             ('f') restent immédiatement opérationnelles sur le snapshot gelé.\n"
    "\n"
    "     q, ESC  Fermeture de modale ou sortie propre de l'application.\n"
    "             Ferme la vue modale active (Ctrl+B ou inspection), ou restaure la configuration\n"
    "             du terminal (mode canonique, réaffichage du curseur) et quitte l'outil.\n"
    "\n"
    "DROITS SYSTÈME REQUIS\n"
    "     L'affichage passif et l'inspection ne requièrent que des droits utilisateurs standard.\n"
    "     L'application des embargos réseau (b / iptables) et les frappes cinétiques (k / kill)\n"
    "     requièrent les privilèges superutilisateur (sudo) ou les capacités Linux :\n"
    "     setcap 'cap_net_admin,cap_kill,cap_sys_ptrace=+ep' /usr/local/bin/rap\n"
    "\n"
    "EXIT STATUS\n"
    "     0 (RAP_EXIT_OK)      Fermeture nominale de la console interactive.\n"
    "     2 (RAP_EXIT_ERROR)   Terminal non interactif (absence de TTY) ou erreur d'initialisation.\n"
    },
};

static const size_t g_rap_man_pages_count = sizeof(g_rap_man_pages) / sizeof(g_rap_man_pages[0]);

/* Dictionnaire intégral brut pour consultation paginée (rap man fn) */
static const char g_rap_fn_full_data[] =
    "# DICTIONNAIRE OFFICIEL DES STRUCTURES ET FONCTIONS // RAPTOR v2.0\n"
    "## RÉFÉRENTIEL TECHNIQUE D'INGÉNIERIE ET D'API (CONSOLIDÉ AU 26-10-2026)\n"
    "\n"
    "================================================================================\n"
    "SECTION 1 : MODÈLE DE DONNÉES UNIVERSEL (src/core/raptor_data.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 01 : raptor_data.c -- raptor_data_context_create -- Allocation du contexte\n"
    "  • Signature C : RaptorReportContext* raptor_data_context_create(void);\n"
    "  • Utilité     : Alloue dynamiquement en mémoire vive et initialise la structure\n"
    "                  centrale du rapport destinée à recevoir les données des greffons.\n"
    "\n"
    "Fonction 02 : raptor_data.c -- raptor_data_section_add -- Création de section\n"
    "  • Signature C : RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title);\n"
    "  • Utilité     : Ajoute un conteneur thématique autonome dans le contexte pour\n"
    "                  regrouper les cartes, tables et alertes produites par un greffon.\n"
    "\n"
    "Fonction 03 : raptor_data.c -- raptor_data_card_add -- Insertion de métrique clé\n"
    "  • Signature C : bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev);\n"
    "  • Utilité     : Enregistre un indicateur synthétique (valeur, pourcentage, niveau\n"
    "                  de criticité) affiché sous forme de tuile métrique dans l'interface.\n"
    "\n"
    "Fonction 04 : raptor_data.c -- raptor_data_table_create -- Initialisation de tableau\n"
    "  • Signature C : RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names);\n"
    "  • Utilité     : Instancie une structure de tableau bidimensionnel avec ses en-têtes\n"
    "                  de colonnes au sein d'une section donnée.\n"
    "\n"
    "Fonction 05 : raptor_data.c -- raptor_data_table_row_add -- Insertion de ligne de données\n"
    "  • Signature C : bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs);\n"
    "  • Utilité     : Ajoute une ligne complète de cellules textuelles associées à des\n"
    "                  codes de couleur/sévérité dans le tableau spécifié.\n"
    "\n"
    "Fonction 06 : raptor_data.c -- raptor_data_alert_add -- Notification de sécurité\n"
    "  • Signature C : bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message);\n"
    "  • Utilité     : Consigne un message d'avertissement ou de confirmation dans la section\n"
    "                  avec un niveau d'alerte défini (OK, AVERTISSEMENT, CRITIQUE).\n"
    "\n"
    "Fonction 07 : raptor_data.c -- raptor_data_get_section_count -- Dénombrement des sections\n"
    "  • Signature C : size_t raptor_data_get_section_count(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Renvoie le nombre total de sections actuellement peuplées dans le\n"
    "                  contexte de rapport actif.\n"
    "\n"
    "Fonction 08 : raptor_data.c -- raptor_data_get_section -- Accès sécurisé par index\n"
    "  • Signature C : const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index);\n"
    "  • Utilité     : Fournit un pointeur en lecture seule vers une section précise à partir\n"
    "                  de son rang séquentiel dans la collection.\n"
    "\n"
    "Fonction 09 : raptor_data.c -- raptor_data_context_clear -- Purge dynamique du contexte\n"
    "  • Signature C : void raptor_data_context_clear(RaptorReportContext* ctx);\n"
    "  • Utilité     : Libère récursivement l'ensemble des sections, tables et alertes\n"
    "                  tout en maintenant le conteneur principal alloué pour le Live Monitoring 3s.\n"
    "\n"
    "Fonction 10 : raptor_data.c -- raptor_data_context_destroy -- Destruction totale\n"
    "  • Signature C : void raptor_data_context_destroy(RaptorReportContext* ctx);\n"
    "  • Utilité     : Désalloue complètement l'arbre de mémoire du contexte et libère\n"
    "                  le pointeur racine lors de l'arrêt définitif de l'application.\n"
    "\n"
    "================================================================================\n"
    "SECTION 2 : REGISTRE DE GREFFONS ET BUS SYSTÈME (src/core/raptor_registry.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 11 : raptor_registry.c -- raptor_registry_init -- Initialisation du bus\n"
    "  • Signature C : void raptor_registry_init(void);\n"
    "  • Utilité     : Prépare la table interne d'enregistrement et réinitialise les\n"
    "                  compteurs de modules d'inspection disponibles.\n"
    "\n"
    "Fonction 12 : raptor_registry.c -- raptor_registry_register -- Enregistrement de greffon\n"
    "  • Signature C : bool raptor_registry_register(const RaptorPlugin* plugin);\n"
    "  • Utilité     : Inscrit un greffon d'audit dans la table globale sans nécessiter\n"
    "                  de modification des fichiers sources du noyau central.\n"
    "\n"
    "Fonction 13 : raptor_registry.c -- raptor_registry_run_all -- Déclenchement séquentiel\n"
    "  • Signature C : bool raptor_registry_run_all(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt l'ensemble des greffons actifs et exécute leur méthode\n"
    "                  `run_audit` pour peupler le contexte en une seule passe.\n"
    "\n"
    "Fonction 14 : raptor_registry.c -- raptor_registry_get_count -- Comptage des greffons\n"
    "  • Signature C : size_t raptor_registry_get_count(void);\n"
    "  • Utilité     : Retourne le nombre total de greffons actuellement enregistrés et\n"
    "                  disponibles pour les audits.\n"
    "\n"
    "Fonction 15 : raptor_registry.c -- raptor_registry_get_by_idx -- Accès greffon par index\n"
    "  • Signature C : const RaptorPlugin* raptor_registry_get_by_idx(size_t index);\n"
    "  • Utilité     : Permet d'interroger la configuration, le nom et les métadonnées d'un\n"
    "                  greffon via son index dans le registre.\n"
    "\n"
    "Fonction 16 : raptor_registry.c -- raptor_registry_cleanup -- Déchargement global\n"
    "  • Signature C : void raptor_registry_cleanup(void);\n"
    "  • Utilité     : Déclenche l'appel de nettoyage `cleanup()` sur chaque greffon et\n"
    "                  vide la table interne des pointeurs de modules.\n"
    "\n"
    "================================================================================\n"
    "SECTION 3 : MOTEUR D'ANALYSE DIFFÉRENTIELLE (src/core/raptor_differential.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 17 : raptor_differential.c -- raptor_diff_init -- Initialisation différentielle\n"
    "  • Signature C : void raptor_diff_init(void);\n"
    "  • Utilité     : Configure les structures mémoire destinées à conserver l'empreinte\n"
    "                  historique nécessaire au calcul des dérives temporelles.\n"
    "\n"
    "Fonction 18 : raptor_differential.c -- raptor_diff_compute -- Calcul des dérivées RAM\n"
    "  • Signature C : bool raptor_diff_compute(const RaptorReportContext* current, RaptorSection* out_diff_sec);\n"
    "  • Utilité     : Compare l'état T(n) avec l'état T(n-1), identifie les fuites de mémoire\n"
    "                  (Mo/min) et peuple la section du cahier différentiel.\n"
    "\n"
    "Fonction 19 : raptor_differential.c -- raptor_diff_snapshot_save -- Sauvegarde d'instantané\n"
    "  • Signature C : void raptor_diff_snapshot_save(const RaptorReportContext* current);\n"
    "  • Utilité     : Copie et fige les métriques de processus actuelles en tant que nouvel\n"
    "                  état de référence pour les comparaisons futures.\n"
    "\n"
    "Fonction 20 : raptor_differential.c -- raptor_diff_cleanup -- Nettoyage différentiel\n"
    "  • Signature C : void raptor_diff_cleanup(void);\n"
    "  • Utilité     : Libère l'intégralité des instantanés historiques archivés en RAM.\n"
    "\n"
    "================================================================================\n"
    "SECTION 4 : JOURNALISATION FORENSIQUE PERMANENTE (src/report/report_audit_log.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 21 : report_audit_log.c -- audit_log_get_system_btime -- Extraction du boot time\n"
    "  • Signature C : static unsigned long long audit_log_get_system_btime(void);\n"
    "  • Utilité     : Lit `/proc/stat` et extrait le timestamp Unix `btime` correspondant\n"
    "                  à l'instant de démarrage matériel de la machine hôte.\n"
    "\n"
    "Fonction 22 : report_audit_log.c -- audit_log_compute_start_time -- Calcul de spawn\n"
    "  • Signature C : static void audit_log_compute_start_time(int pid, char* out_buf, size_t buf_sz);\n"
    "  • Utilité     : Parse le 22e champ (`starttime`) de `/proc/[PID]/stat` et convertit\n"
    "                  les ticks processeur en heure locale absolue de démarrage (HH:MM:SS).\n"
    "\n"
    "Fonction 23 : report_audit_log.c -- audit_log_get_next_sequence_number -- Séquençage auto\n"
    "  • Signature C : static int audit_log_get_next_sequence_number(void);\n"
    "  • Utilité     : Parcourt `AUDIT.TXT` à la recherche du motif `AUDIT NO #XX` pour\n"
    "                  déterminer et retourner le prochain numéro incrémental unique.\n"
    "\n"
    "Fonction 24 : report_audit_log.c -- raptor_audit_log_capture_target -- Capture pré-mortem\n"
    "  • Signature C : bool raptor_audit_log_capture_target(int pid, RaptorIncidentDetail* out_detail);\n"
    "  • Utilité     : Extrait dynamiquement toutes les preuves du processus dans `/proc`\n"
    "                  (binaire, CWD, start time, PPID, commande parent, CWD parent) avant arrêt.\n"
    "\n"
    "Fonction 25 : report_audit_log.c -- raptor_audit_log_commit -- Écriture persistante\n"
    "  • Signature C : bool raptor_audit_log_commit(const RaptorIncidentDetail* detail, const char* action_status);\n"
    "  • Utilité     : Formate et écrit le bloc d'audit horodaté dans `AUDIT.TXT` en mode\n"
    "                  ajout permanent (`a+`) avec forçage d'écriture physique sur disque.\n"
    "\n"
    "Fonction 26 : report_audit_log.c -- raptor_audit_log_notify_detection -- Alerte temps réel\n"
    "  • Signature C : bool raptor_audit_log_notify_detection(int pid, const char* classification);\n"
    "  • Utilité     : Déclenche l'écriture immédiate d'un audit dès qu'une anomalie heuristique\n"
    "                  apparaît, avec filtrage dédoublonné pour éviter la saturation du fichier.\n"
    "\n"
    "Fonction 27 : report_audit_log.c -- raptor_audit_log_forget_pid -- Purge du cache anti-flood\n"
    "  • Signature C : void raptor_audit_log_forget_pid(int pid);\n"
    "  • Utilité     : Supprime un identifiant PID de la table des processus déjà logués\n"
    "                  lors de sa destruction, autorisant la traque immédiate en cas de réapparition.\n"
    "\n"
    "================================================================================\n"
    "SECTION 5 : GREFFON PROCESSUS, MÉMOIRE ET RWX (src/plugins/plugin_memory_rwx.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 28 : plugin_memory_rwx.c -- proc_mem_is_trusted_path -- Filtrage des chemins sûrs\n"
    "  • Signature C : static bool proc_mem_is_trusted_path(const char* exe_path);\n"
    "  • Utilité     : Valide si le binaire provient strictement des répertoires systèmes\n"
    "                  légitimes (`/usr/bin/`, `/usr/lib/`, `/bin/`, `/sbin/`, `/app/`).\n"
    "\n"
    "Fonction 29 : plugin_memory_rwx.c -- proc_mem_is_legit_jit -- Détection des moteurs JIT\n"
    "  • Signature C : static bool proc_mem_is_legit_jit(const char* comm, const char* exe);\n"
    "  • Utilité     : Écarte les faux positifs en reconnaissant les interpréteurs légitimes\n"
    "                  allouant de la mémoire exécutable (navigateurs, Python, machines virtuelles).\n"
    "\n"
    "Fonction 30 : plugin_memory_rwx.c -- proc_mem_scan_maps_rwx -- Inspection des pages mémoire\n"
    "  • Signature C : static bool proc_mem_scan_maps_rwx(uint32_t pid, bool is_jit, bool is_trusted);\n"
    "  • Utilité     : Analyse `/proc/[PID]/maps` pour repérer les segments illégitimes\n"
    "                  possédant simultanément les droits Read-Write-Execute (`rwx`).\n"
    "\n"
    "Fonction 31 : plugin_memory_rwx.c -- proc_mem_read_stats -- Empreinte RAM du processus\n"
    "  • Signature C : static bool proc_mem_read_stats(uint32_t pid, char* out_comm, size_t comm_len, char* out_exe, size_len, uint64_t* out_rss);\n"
    "  • Utilité     : Lit le nom d'exécutable, résout le lien `/proc/[PID]/exe` et calcule\n"
    "                  la consommation résidente RSS en mémoire physique.\n"
    "\n"
    "Fonction 32 : plugin_memory_rwx.c -- proc_mem_run_audit -- Exécution de l'audit mémoire\n"
    "  • Signature C : static bool proc_mem_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt l'arborescence `/proc`, audite la mémoire de chaque tâche\n"
    "                  active et consigne les résultats dans la section dédiée du rapport.\n"
    "\n"
    "Fonction 33 : plugin_memory_rwx.c -- proc_mem_register -- Enregistrement automatique\n"
    "  • Signature C : __attribute__((constructor)) static void proc_mem_register(void);\n"
    "  • Utilité     : Enregistre le greffon mémoire auprès du registre central dès le\n"
    "                  chargement du binaire en mémoire par le système d'exploitation.\n"
    "\n"
    "================================================================================\n"
    "SECTION 6 : GREFFON SURVEILLANCE RÉSEAU (src/plugins/plugin_network_sockets.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 34 : plugin_network_sockets.c -- net_sock_build_inode_cache -- Résolution Inode/PID\n"
    "  • Signature C : static bool net_sock_build_inode_cache(void);\n"
    "  • Utilité     : Analyse tous les descripteurs ouverts dans `/proc/[PID]/fd` pour\n"
    "                  construire la table de correspondance rapide entre numéros d'inodes et PID.\n"
    "\n"
    "Fonction 35 : plugin_network_sockets.c -- net_sock_resolve_pid -- Recherche propriétaire O(1)\n"
    "  • Signature C : static uint32_t net_sock_resolve_pid(uint64_t target_inode);\n"
    "  • Utilité     : Retourne instantanément l'identifiant PID associé à un socket réseau\n"
    "                  à partir de son numéro d'inode noyau.\n"
    "\n"
    "Fonction 36 : plugin_network_sockets.c -- net_sock_decode_hex_endpoint -- Décodage IP/Port\n"
    "  • Signature C : static void net_sock_decode_hex_endpoint(const char* hex_ip, uint32_t hex_port, char* out_ip, size_t ip_len, uint16_t* out_port);\n"
    "  • Utilité     : Convertit la représentation hexadécimale brute (format endianess noyau)\n"
    "                  en adresse IP décimale lisible et port réseau standard.\n"
    "\n"
    "Fonction 37 : plugin_network_sockets.c -- net_sock_parse_file -- Analyse des tables réseau\n"
    "  • Signature C : static void net_sock_parse_file(const char* net_path, const char* proto, RaptorTable* out_table);\n"
    "  • Utilité     : Parse `/proc/net/tcp` et `/proc/net/udp` pour répertorier les ports en\n"
    "                  écoute (`LISTEN`) et les canaux établis (`ESTABLISHED`).\n"
    "\n"
    "Fonction 38 : plugin_network_sockets.c -- net_sock_run_audit -- Exécution de l'audit réseau\n"
    "  • Signature C : static bool net_sock_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Orchestre la collecte des flux réseau, résout les processus propriétaires\n"
    "                  et libère le cache des descripteurs après analyse.\n"
    "\n"
    "Fonction 39 : plugin_network_sockets.c -- net_sock_register -- Enregistrement automatique\n"
    "  • Signature C : __attribute__((constructor)) static void net_sock_register(void);\n"
    "  • Utilité     : Inscrit le greffon réseau dans le registre central dès l'initialisation\n"
    "                  du binaire exécutable.\n"
    "\n"
    "================================================================================\n"
    "SECTION 7 : GREFFON HEURISTIQUE & LIGNÉE PARENT (src/plugins/plugin_heuristics_expert.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 40 : plugin_heuristics_expert.c -- heuristics_is_suspicious_path -- Détection chemins volatils\n"
    "  • Signature C : static bool heuristics_is_suspicious_path(const char* path);\n"
    "  • Utilité     : Identifie si un processus s'exécute depuis des zones temporaires ou\n"
    "                  volatiles suspectes (`/tmp`, `/var/tmp`, `/dev/shm`, `/run/user`).\n"
    "\n"
    "Fonction 41 : plugin_heuristics_expert.c -- heuristics_is_system_masquerade -- Détection d'usurpation\n"
    "  • Signature C : static bool heuristics_is_system_masquerade(const char* comm, const char* exe_target);\n"
    "  • Utilité     : Repère les processus non officiels usurpant le nom de démons critiques\n"
    "                  (`systemd`, `sshd`, `cron`, `kworker`) hors des dossiers systèmes.\n"
    "\n"
    "Fonction 42 : plugin_heuristics_expert.c -- heuristics_get_parent_info -- Traque généalogique (PPID)\n"
    "  • Signature C : static void heuristics_get_parent_info(const char* pid_str, char* out_ppid, size_t ppid_sz, char* out_pcomm, size_t pcomm_sz, char* out_pcmd, size_t pcmd_sz);\n"
    "  • Utilité     : Extrait le PID parent via `/proc/[PID]/stat` et reconstitue la ligne\n"
    "                  de commande exacte et l'exécutable créateur depuis `/proc/[PPID]/cmdline`.\n"
    "\n"
    "Fonction 43 : plugin_heuristics_expert.c -- heuristics_check_process -- Évaluation unitaire\n"
    "  • Signature C : static void heuristics_check_process(const char* pid_str, size_t* out_masquerades, size_t* out_deleted, size_t* out_suspicious_paths, RaptorTable* tbl);\n"
    "  • Utilité     : Exécute l'ensemble des filtres heuristiques sur un processus, remplit la table\n"
    "                  d'incident et déclenche la notification persistante dans `AUDIT.TXT`.\n"
    "\n"
    "Fonction 44 : plugin_heuristics_expert.c -- plugin_heuristics_init -- Initialisation greffon\n"
    "  • Signature C : static bool plugin_heuristics_init(void);\n"
    "  • Utilité     : Prépare les états internes et structures du greffon heuristique.\n"
    "\n"
    "Fonction 45 : plugin_heuristics_expert.c -- plugin_heuristics_run_audit -- Exécution globale heuristique\n"
    "  • Signature C : static bool plugin_heuristics_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt `/proc`, identifie les menaces comportementales, compile les cartes\n"
    "                  d'usurpation et qualifie la criticité défensive globale.\n"
    "\n"
    "Fonction 46 : plugin_heuristics_expert.c -- plugin_heuristics_cleanup -- Nettoyage greffon\n"
    "  • Signature C : static void plugin_heuristics_cleanup(void);\n"
    "  • Utilité     : Libère les structures locales allouées lors de l'exécution de l'audit.\n"
    "\n"
    "Fonction 47 : plugin_heuristics_expert.c -- plugin_heuristics_register_constructor -- Auto-enregistrement\n"
    "  • Signature C : __attribute__((constructor)) static void plugin_heuristics_register_constructor(void);\n"
    "  • Utilité     : Enregistre dynamiquement le greffon heuristique au chargement du programme.\n"
    "\n"
    "================================================================================\n"
    "SECTION 8 : MOTEURS DE RESTITUTION UNIVERSELS (src/report/)\n"
    "================================================================================\n"
    "\n"
    "Fonction 48 : report_cli.c -- raptor_report_cli_print -- Sortie console formatée\n"
    "  • Signature C : void raptor_report_cli_print(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Rendu textuel intégral du rapport dans le terminal avec codes\n"
    "                  ANSI pour les couleurs de sévérité et colonnage structuré.\n"
    "\n"
    "Fonction 49 : report_html.c -- raptor_report_html_generate -- Export de rapport web\n"
    "  • Signature C : bool raptor_report_html_generate(const RaptorReportContext* ctx, const char* filepath);\n"
    "  • Utilité     : Génère un rapport d'audit au format HTML souverain autonome avec\n"
    "                  thème sombre, visualisations interactives et tableaux filtrables.\n"
    "\n"
    "================================================================================\n"
    "SECTION 9 : MOTEUR GRAPHIQUE ET CONTRÔLEURS DE VUES (src/ui/)\n"
    "================================================================================\n"
    "\n"
    "Fonction 50 : ui_engine.c -- raptor_ui_init -- Initialisation graphique Raylib\n"
    "  • Signature C : bool raptor_ui_init(int width, int height, const char* title);\n"
    "  • Utilité     : Configure la fenêtre Raylib, applique les dimensions, le titre,\n"
    "                  la synchronisation verticale (VSync) et la cible de 60 FPS.\n"
    "\n"
    "Fonction 51 : ui_engine.c -- raptor_ui_run -- Boucle de surveillance active\n"
    "  • Signature C : void raptor_ui_run(RaptorReportContext* ctx);\n"
    "  • Utilité     : Pilote la boucle de rendu principale, orchestre l'affichage des onglets\n"
    "                  et intègre le timer cyclique de 3s pour le rafraîchissement continu.\n"
    "\n"
    "Fonction 52 : ui_engine.c -- raptor_ui_shutdown -- Libération graphique\n"
    "  • Signature C : void raptor_ui_shutdown(void);\n"
    "  • Utilité     : Décharge les textures vectorielles (icônes) et détruit proprement\n"
    "                  la fenêtre graphique Raylib à la fermeture.\n"
    "\n"
    "Fonction 53 : view_ai.c -- view_ai_detect_os -- Identification du système hôte\n"
    "  • Signature C : static void view_ai_detect_os(char* out_name, size_t max_len);\n"
    "  • Utilité     : Interroge `/etc/os-release` pour afficher le nom exact de la distribution\n"
    "                  (ex: Linux Mint Debian Edition) dans le bandeau du rapport IA.\n"
    "\n"
    "Fonction 54 : view_ai.c -- view_ai_draw_pid_badge -- Affichage badges d'alerte PID\n"
    "  • Signature C : static float view_ai_draw_pid_badge(const char* role_title, const char* pid_val, float posX, float posY);\n"
    "  • Utilité     : Dessine un conteneur visuel haute visibilité (bordure jaune vif 2 px,\n"
    "                  fond sombre, texte blanc) mettant en évidence les identifiants cibles et parents.\n"
    "\n"
    "Fonction 55 : view_ai.c -- ui_view_ai_draw -- Contrôleur du rapport IA [ 05 ]\n"
    "  • Signature C : void ui_view_ai_draw(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Consolide l'audit explicatif souverain, affiche le modèle Z-Score, trace\n"
    "                  la lignée du spawn et intègre le garde-fou strict interdisant le kill du PID 1.\n"
    "\n"
    "Fonction 56 : view_processes.c -- ui_view_processes_draw -- Contrôleur Processus & RAM [ 02 ]\n"
    "  • Signature C : void ui_view_processes_draw(RaptorReportContext* ctx);\n"
    "  • Utilité     : Affiche la table filtrable des processus, gère la sélection interactive,\n"
    "                  exécute la capture forensique, ordonne le signal de kill et consigne l'événement.\n"
    "\n"
    "================================================================================\n"
    "SECTION 10 : POINT D'ENTRÉE DU SYSTÈME (src/main.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 57 : main.c -- main -- Orchestrateur d'amorçage\n"
    "  • Signature C : int main(int argc, char** argv);\n"
    "  • Utilité     : Parse les arguments de ligne de commande (`--cli` ou interface GUI),\n"
    "                  initialise le registre, lance le premier audit et passe le relais à l'UI.\n"
    "\n"
    "================================================================================\n"
    "SECTION 11 : GREFFON RAPTOR NET-GUARD DÉFENSIF (src/plugins/plugin_net_guard.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 58 : plugin_net_guard.c -- net_guard_trim -- Assainissement de chaîne\n"
    "  • Signature C : char* net_guard_trim(char* str);\n"
    "  • Utilité     : Élimine les espaces blancs et sauts de ligne superflus en début\n"
    "                  et fin de chaîne pour fiabiliser le parsing des fichiers de règles.\n"
    "\n"
    "Fonction 59 : plugin_net_guard.c -- net_guard_validate_ip -- Validation arithmétique IP & CIDR\n"
    "  • Signature C : bool net_guard_validate_ip(const char* ip_str, uint32_t* out_ip, uint32_t* out_mask, uint16_t* out_port);\n"
    "  • Utilité     : Contrôle rigoureusement les plages IPv4 (0-255), extrait le port\n"
    "                  éventuel (1-65535) et calcule le masque binaire réseau à partir de la notation CIDR (/1 à /32).\n"
    "\n"
    "Fonction 60 : plugin_net_guard.c -- net_guard_load_file -- Parsing tolérant de registre .conf\n"
    "  • Signature C : bool net_guard_load_file(const char* filepath, bool is_custom_source);\n"
    "  • Utilité     : Lit ligne par ligne les fichiers de configuration, ignore les commentaires,\n"
    "                  segmente les champs par séparateur pipe (|) et peuple les tables mémoires DOM et IP.\n"
    "\n"
    "Fonction 61 : plugin_net_guard.c -- net_guard_init -- Initialisation et fusion en RAM\n"
    "  • Signature C : bool net_guard_init(void);\n"
    "  • Utilité     : Réinitialise les tables du registre et charge successivement le socle\n"
    "                  officiel (fw_officiel.conf) puis les ajouts locaux (fw_custom.conf) sans conflit.\n"
    "\n"
    "Fonction 62 : plugin_net_guard.c -- net_guard_add_custom_rule -- Sérialisation et persistance IHM\n"
    "  • Signature C : bool net_guard_add_custom_rule(NetGuardRuleType type, const char* target, const char* param, const char* desc);\n"
    "  • Utilité     : Formate avec espacements canoniques une nouvelle règle saisie dans l'IHM,\n"
    "                  l'écrit en mode ajout permanent dans fw_custom.conf et actualise la table active.\n"
    "\n"
    "Fonction 63 : plugin_net_guard.c -- net_guard_remove_rule -- Suppression atomique de règle\n"
    "  • Signature C : bool net_guard_remove_rule(NetGuardRuleType type, size_t index);\n"
    "  • Utilité     : Supprime une règle personnalisée selon son index mémoire et réécrit\n"
    "                  proprement l'intégralité du fichier fw_custom.conf sans toucher au registre officiel.\n"
    "\n"
    "Fonction 64 : plugin_net_guard.c -- net_guard_match_domain -- Détection anti-combosquatting\n"
    "  • Signature C : NetGuardVerdict net_guard_match_domain(const char* hostname, char* out_reason, size_t reason_sz);\n"
    "  • Utilité     : Analyse le nom de domaine demandé, vérifie la présence d'une marque protégée,\n"
    "                  valide les domaines officiels autorisés et lève l'alerte CRITIQUE si un mot-clé suspect y est accolé.\n"
    "\n"
    "Fonction 65 : plugin_net_guard.c -- net_guard_match_ip -- Qualification binaire des endpoints IP\n"
    "  • Signature C : NetGuardVerdict net_guard_match_ip(const char* remote_ip, uint16_t port, char* out_reason, size_t reason_sz);\n"
    "  • Utilité     : Applique le masque de sous-réseau sur l'adresse distante pour déterminer\n"
    "                  instantanément si le socket pointe vers une cible APPROUVÉ (vert) ou HOSTILE (rouge critique).\n"
    "\n"
    "Fonction 66 : plugin_net_guard.c -- net_guard_run_audit -- Moteur d'audit périodique réseau\n"
    "  • Signature C : bool net_guard_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Orchestre la lecture de /proc/net/tcp et /proc/net/udp, confronte chaque\n"
    "                  connexion active aux tables de règles et peuple la section générique du rapport de sécurité.\n"
    "\n"
    "Fonction 67 : plugin_net_guard.c -- net_guard_cleanup -- Déchargement et purge mémoire\n"
    "  • Signature C : void net_guard_cleanup(void);\n"
    "  • Utilité     : Remet à zéro l'ensemble des structures de données du registre Net-Guard\n"
    "                  lors de la fermeture propre de l'application.\n"
    "\n"
    "Fonction 68 : plugin_net_guard.c -- net_guard_register -- Auto-enregistrement constructeur GCC\n"
    "  • Signature C : __attribute__((constructor)) void net_guard_register(void);\n"
    "  • Utilité     : Enregistre automatiquement la structure RaptorPlugin de Net-Guard auprès\n"
    "                  du bus central raptor_registry dès le chargement du binaire en mémoire.\n"
    "\n"
    "Fonction 69 : plugin_net_guard.c -- net_guard_update_official_rules -- Rapatriement atomique GitHub\n"
    "  • Signature C : bool net_guard_update_official_rules(void);\n"
    "  • Utilité     : Télécharge la dernière mouture de fw_officiel.conf via curl vers un fichier\n"
    "                  temporaire (.tmp), valide l'en-tête de conformité et applique un remplacement atomique POSIX (rename).\n"
    "\n"
    "================================================================================\n"
    "SECTION 12 : RENSEIGNEMENT RÉSEAU, BGP & CDN INTEL (src/plugins/plugin_asn_intel.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 71 : plugin_asn_intel.c -- asn_intel_trim -- Nettoyage canonique\n"
    "  • Signature C : static char* asn_intel_trim(char* str);\n"
    "  • Utilité     : Nettoyage canonique des espaces blancs en tête et queue de chaîne.\n"
    "\n"
    "Fonction 72 : plugin_asn_intel.c -- asn_intel_parse_cidr -- Décomposition CIDR\n"
    "  • Signature C : static bool asn_intel_parse_cidr(const char* cidr, uint32_t* out_net, uint32_t* out_mask);\n"
    "  • Utilité     : Décomposition arithmétique d'une notation CIDR (X.X.X.X/YY), extraction de\n"
    "                  l'adresse réseau hôte et génération du masque binaire 32 bits.\n"
    "\n"
    "Fonction 73 : plugin_asn_intel.c -- asn_intel_load_file -- Parsing à chaud table_static.conf\n"
    "  • Signature C : static bool asn_intel_load_file(const char* filepath);\n"
    "  • Utilité     : Chargement dynamique de `assets/rules/table_static.conf` pour peupler la table\n"
    "                  des plages IP de confiance sans recompilation du binaire.\n"
    "\n"
    "Fonction 74 : plugin_asn_intel.c -- asn_intel_match_static -- Évaluation arithmétique locale\n"
    "  • Signature C : static bool asn_intel_match_static(uint32_t ip, char* out_name, size_t max_len);\n"
    "  • Utilité     : Évaluation instantanée (0 ms) contre les infrastructures répertoriées via\n"
    "                  l'opération binaire `(ip & mask) == (net & mask)`.\n"
    "\n"
    "Fonction 75 : plugin_asn_intel.c -- asn_intel_query_bgp -- Sonde DNS TXT BGP\n"
    "  • Signature C : static bool asn_intel_query_bgp(const char* ip_str, uint32_t* out_asn, char* out_cc, size_t cc_sz);\n"
    "  • Utilité     : Résolution DNS de secours vers le registre Team Cymru (`origin.asn.cymru.com`)\n"
    "                  pour identifier le numéro d'ASN et le code pays ISO des adresses inconnues.\n"
    "\n"
    "Fonction 76 : plugin_asn_intel.c -- asn_intel_lookup -- Moteur central en cascade\n"
    "  • Signature C : bool raptor_asn_intel_lookup(const char* ip_str, char* out_name, size_t max_len);\n"
    "  • Utilité     : Cascade d'investigation : 1. Cache dynamique -> 2. `table_static.conf` (0 ms) -> 3. Sonde DNS BGP.\n"
    "\n"
    "Fonction 77 : plugin_asn_intel.c -- asn_intel_init -- Initialisation greffon ASN\n"
    "  • Signature C : static bool asn_intel_init(void);\n"
    "  • Utilité     : Prépare le mutex d'exclusion mutuelle, configure les caches et charge les règles réseau.\n"
    "\n"
    "Fonction 78 : plugin_asn_intel.c -- asn_intel_cleanup -- Vidange et arrêt\n"
    "  • Signature C : static void asn_intel_cleanup(void);\n"
    "  • Utilité     : Purge l'intégralité des structures mémoires et détruit le verrou d'exclusion mutuelle.\n"
    "\n"
    "Fonction 79 : plugin_asn_intel.c -- asn_intel_register -- Auto-enregistrement\n"
    "  • Signature C : __attribute__((constructor)) static void asn_intel_register(void);\n"
    "  • Utilité     : Enregistrement constructeur GCC du greffon sur le bus RaptorPlugin.\n"
    "\n"
    "================================================================================\n"
    "SECTION 13 : TRAQUE MÉMOIRE ANONYME & MUTATIONS R-X (src/core/mem_stealth.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 80 : mem_stealth.c -- mem_stealth_whitelist_load -- Chargement liste blanche\n"
    "  • Signature C : bool mem_stealth_whitelist_load(const char* filepath);\n"
    "  • Utilité     : Charge et analyse `assets/rules/mem_whitelist.conf` (format `TYPE | MOTIF | DESC`),\n"
    "                  éliminant les faux positifs structurels sur le code dynamique.\n"
    "\n"
    "Fonction 81 : mem_stealth.c -- mem_stealth_is_whitelisted -- Filtrage exceptions légitimes\n"
    "  • Signature C : bool mem_stealth_is_whitelisted(const char* comm, const char* exe);\n"
    "  • Utilité     : Vérifie si un processus correspond à une signature autorisée (navigateurs,\n"
    "                  Flatpak, serveurs web, serveurs d'affichage, bureaux Cinnamon/GNOME/XFCE/KDE).\n"
    "\n"
    "Fonction 82 : mem_stealth.c -- mem_stealth_scan_maps_anon_rx -- Inspection Inode 0 & riposte\n"
    "  • Signature C : bool mem_stealth_scan_maps_anon_rx(pid_t pid, const char* comm, const char* exe, StealthEvent* ev_out);\n"
    "  • Utilité     : Analyse `/proc/[PID]/maps` pour isoler les allocations anonymes rendues\n"
    "                  exécutables (`r-xp`) avec un Inode à 0 sans ancrage disque, et déclenche\n"
    "                  la riposte réflexe `rap_reflex_dispatch`.\n"
    "\n"
    "Fonction 83 : mem_stealth.c -- mem_stealth_dispatch_diff_event -- Propagation différentielle\n"
    "  • Signature C : void mem_stealth_dispatch_diff_event(const StealthEvent* ev);\n"
    "  • Utilité     : Injecte les alertes forensiques d'injection dans le tampon circulaire du\n"
    "                  Cahier Différentiel (Onglet 4).\n"
    "\n"
    "================================================================================\n"
    "SECTION 14 : DÉTECTION DES DÉTOURNEMENTS LD_PRELOAD (src/core/preload_hunter.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 84 : preload_hunter.c -- preload_hunter_check_global -- Audit intégrité globale\n"
    "  • Signature C : bool preload_hunter_check_global(char* out_path, size_t out_len);\n"
    "  • Utilité     : Détecte la persistance d'un rootkit utilisateur en vérifiant l'accès\n"
    "                  et le contenu de `/etc/ld.so.preload`.\n"
    "\n"
    "Fonction 85 : preload_hunter.c -- preload_hunter_scan_process_env -- Audit variables d'environnement\n"
    "  • Signature C : bool preload_hunter_scan_process_env(pid_t pid, char* out_preload_lib, size_t out_len);\n"
    "  • Utilité     : Scanne `/proc/[PID]/environ` (délimiteurs `\\0`) pour intercepter\n"
    "                  toute injection ciblée via la variable `LD_PRELOAD=`.\n"
    "\n"
    "================================================================================\n"
    "SECTION 15 : NOTIFICATIONS BUREAU ASYNCHRONES (src/core/raptor_notify.c)\n"
    "================================================================================\n"
    "\n"
    "Fonction 86 : raptor_notify.c -- raptor_notify_send_desktop -- Émission asynchrone FreeDesktop\n"
    "  • Signature C : void raptor_notify_send_desktop(const char* title, const char* body, bool is_critical);\n"
    "  • Utilité     : Envoie une notification via `notify-send` avec double fork (`fork()` / `_exit(0)`)\n"
    "                  garantissant zéro saccade graphique sur la boucle Raylib (60 FPS).\n"
    "\n"
    "Fonction 87 : raptor_notify.c -- raptor_notify_evaluate_threats -- Arbitrage d'alertes\n"
    "  • Signature C : void raptor_notify_evaluate_threats(int current_threats, const char* primary_threat_name);\n"
    "  • Utilité     : Filtre anti-rebond temporel (30 secondes) déclenchant l'alerte sur\n"
    "                  transition critique et signalant le retour au calme.\n"
    "\n"
    "================================================================================\n"
    "SECTION 16 : MODE CLI DE SECOURS & OUTILS CHIRURGICAUX (src/cli/)\n"
    "================================================================================\n"
    "\n"
    "Fonction 88 : rap_main.c -- rap_dispatch_command -- Micro-routeur déclaratif\n"
    "  • Signature C : int rap_dispatch_command(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Achemine les requêtes CLI vers les gestionnaires spécialisés via la\n"
    "                  table déclarative `g_commands`.\n"
    "\n"
    "Fonction 89 : cmd_status.c -- rap_cmd_status -- Bilan vital instantané\n"
    "  • Signature C : int rap_cmd_status(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Diagnostic vital en moins de 15 ms (RAM totale, processus actifs,\n"
    "                  intégrité de `/etc/ld.so.preload`).\n"
    "\n"
    "Fonction 90 : cmd_top.c -- rap_cmd_top -- Top consommateurs VmRSS\n"
    "  • Signature C : int rap_cmd_top(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Trie les processus par mémoire résidente physique réelle sans\n"
    "                  allocation sur le tas (tri d'insertion statique sur la pile).\n"
    "\n"
    "Fonction 91 : cmd_hunt.c -- rap_cmd_hunt -- Chasseur universel combiné & riposte active\n"
    "  • Signature C : int rap_cmd_hunt(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Enchaîne séquentiellement la traque Inode 0 (`rx`), les détournements\n"
    "                  dynamiques (`preload`) et l'analyse heuristique (`heur`) avec support\n"
    "                  de l'armement réflexe (`--arm`) et simulation (`--dry-run`).\n"
    "\n"
    "Fonction 92 : cmd_rx.c -- rap_cmd_rx -- Traque mutations exécutables Inode 0\n"
    "  • Signature C : int rap_cmd_rx(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Repère les allocations mémoires anonymes exécutables (`r-xp`)\n"
    "                  dans `/proc/[PID]/maps` avec filtrage strict par `mem_whitelist.conf`\n"
    "                  et raccordement à la riposte cinétique réflexe (`rap_reflex_dispatch`).\n"
    "\n"
    "Fonction 93 : cmd_preload.c -- rap_cmd_preload -- Chasseur LD_PRELOAD\n"
    "  • Signature C : int rap_cmd_preload(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Détecte les rootkits userland via le contrôle double palier\n"
    "                  (`/etc/ld.so.preload` et `/proc/[PID]/environ`).\n"
    "\n"
    "Fonction 94 : cmd_net.c -- rap_cmd_net -- Audit réseau Net-Guard\n"
    "  • Signature C : int rap_cmd_net(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Analyse `/proc/net/tcp` et qualifie instantanément les connexions\n"
    "                  actives (APPROUVÉ, EXTERNE, SUSPECT, HOSTILE).\n"
    "\n"
    "Fonction 95 : cmd_kill.c -- rap_cmd_kill -- Neutralisation cinétique chirurgicale\n"
    "  • Signature C : int rap_cmd_kill(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Émet le signal non rattrapable `SIGKILL` avec validation préalable\n"
    "                  par `rap_check_safety_pid` et confirmation interactive (`-f` pour forcer).\n"
    "\n"
    "Fonction 96 : cmd_kill.c -- rap_cmd_freeze -- Suspension conservatoire forensique\n"
    "  • Signature C : int rap_cmd_freeze(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Gèle un processus suspect via `SIGSTOP` (état `T`) pour figer son\n"
    "                  activité tout en préservant sa mémoire vive pour analyse forensique.\n"
    "\n"
    "Fonction 97 : cmd_man.c -- rap_cmd_man -- Manuel de référence unifié embarqué\n"
    "  • Signature C : int rap_cmd_man(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Point d'entrée de la documentation embarquée. Gère l'affichage sélectif\n"
    "                  par commande (`rap man sentry`, `rap man rx`), la vue complète (`all`)\n"
    "                  ou le routage vers le dictionnaire des fonctions (`fn`).\n"
    "\n"
    "Fonction 98 : rap_common.c -- rap_output_formatter -- Formatage d'affichage universel\n"
    "  • Signature C : void rap_output_formatter(const RapConfig *cfg, const char *c1, const char *c2, const char *c3, const char *c4);\n"
    "  • Utilité     : Formate les flux d'audit sur 4 colonnes régulières en mode interactif\n"
    "                  ou en flux brut délimité par tabulations (`\\t`) en mode `--raw`.\n"
    "\n"
    "Fonction 99 : rap_common.c -- rap_check_safety_pid -- Barrière de sécurité cinétique\n"
    "  • Signature C : int rap_check_safety_pid(const RapConfig *cfg, pid_t pid);\n"
    "  • Utilité     : Empêche les arrêts accidentels en rejetant le tir sur PID 1,\n"
    "                  sur soi-même, sur le shell parent et sur les threads internes du noyau.\n"
    "\n"
    "Fonction 100 : rap_main.c -- rap_parse_flags -- Parseur d'arguments et drapeaux\n"
    "  • Signature C : int rap_parse_flags(int *argc, char ***argv, RapConfig *cfg);\n"
    "  • Utilité     : Extrait les drapeaux universels (`-q`, `-r`, `-f`, `-v`, `-a`, `--arm`,\n"
    "                  `--dry-run`, `-n`, `-h`), détecte le mode TTY et isole le verbe d'action.\n"
    "\n"
    "Fonction 107 : rap_common.c -- rap_is_mem_whitelisted -- Filtrage liste blanche mémoire\n"
    "  • Signature C : int rap_is_mem_whitelisted(const char *comm);\n"
    "  • Utilité     : Vérifie si un processus figure dans `mem_whitelist.conf` en gérant\n"
    "                  la troncature noyau à 15 caractères sans allocation dynamique.\n"
    "\n"
    "Fonction 108 : rap_common.c -- rap_net_eval_ip -- Qualification CIDR d'endpoint IP\n"
    "  • Signature C : int rap_net_eval_ip(uint32_t ip, uint16_t port, char *out_desc, size_t desc_size);\n"
    "  • Utilité     : Évalue une adresse IP contre les registres `fw_officiel.conf` et `fw_custom.conf`\n"
    "                  pour retourner son niveau de confiance (APPROUVÉ, STANDARD, SUSPECT, HOSTILE).\n"
    "\n"
    "Fonction 109 : cmd_heur.c -- rap_cmd_heur -- Traque des anomalies d'exécution\n"
    "  • Signature C : int rap_cmd_heur(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Parcourt `/proc`, résout `/proc/[PID]/exe` et projette les anomalies\n"
    "                  comportementales identifiées sur la console de secours.\n"
    "\n"
    "Fonction 110 : cmd_heur.c -- rap_heur_check_process -- Évaluation unitaire heuristique\n"
    "  • Signature C : int rap_heur_check_process(pid_t pid, const char *comm, const char *exe_path, char *out_anomaly, size_t out_sz);\n"
    "  • Utilité     : Évalue un processus selon trois axes sans allocation dynamique :\n"
    "                  1. Binaires supprimés du disque en cours d'exécution (`(deleted)`).\n"
    "                  2. Exécutions depuis des zones volatiles suspectes (`/tmp`, `/dev/shm`).\n"
    "                  3. Usurpation d'identité de démons systèmes (*Masquerading*).\n"
    "\n"
    "Fonction 123 : cmd_man.c -- rap_man_render -- Téléscripteur et projection de documentation\n"
    "  • Signature C : static void rap_man_render(const char *text, size_t len, const RapConfig *cfg, bool force_direct);\n"
    "  • Utilité     : Aiguille le flux textuel vers le pager autonome `rapless` en mode interactif\n"
    "                  ou projette directement sur STDOUT en mode tube, redirection ou `--raw`.\n"
    "\n"
    "Fonction 124 : cmd_man.c -- rap_man_handle_fn -- Moteur de navigation du dictionnaire API\n"
    "  • Signature C : static int rap_man_handle_fn(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Permet l'interrogation granulaire du dictionnaire des fonctions par numéro de\n"
    "                  section (`rap man fn <sec>`), par identifiant unique (`rap man fn <id>`) ou en intégralité.\n"
    "\n"
    "Fonction 125 : cmd_man.c -- rap_man_extract_topic -- Extraction sémantique de rubrique\n"
    "  • Signature C : static const char *rap_man_extract_topic(int argc, char **argv);\n"
    "  • Utilité     : Isole l'argument cible de documentation dans `argv` indépendamment des drapeaux\n"
    "                  et de la position des paramètres.\n"
    "\n"
    "Fonction 126 : cmd_man.c -- rap_man_lookup_page -- Recherche et résolution d'entrée man\n"
    "  • Signature C : static const RapManPageEntry *rap_man_lookup_page(const char *topic);\n"
    "  • Utilité     : Résout une clé de commande dans la table `g_rap_man_pages[]` avec gestion des alias\n"
    "                  courts (`st`, `se`, `he`) et résolution avec ou sans le préfixe `rap-`.\n"
    "\n"
    "================================================================================\n"
    "SECTION 17 : SENTINELLE PASSIVE ANTI-RANÇONGICIEL (FANOTIFY & LEURRES DENSES)\n"
    "================================================================================\n"
    "\n"
    "Fonction 111 : sentry_engine.c -- sentry_deploy_canary -- Déploiement d'appât dense\n"
    "  • Signature C : int sentry_deploy_canary(const char *dir_path);\n"
    "  • Utilité     : Génère un fichier binaire dense de 32 Ko (`!00_raptor_vault.kdbx`)\n"
    "                  doté de l'en-tête officiel KeePass KDBX4 (Magic Numbers authentiques)\n"
    "                  et d'un corps à haute entropie généré par LCG. Applique les permissions\n"
    "                  `0666` via `fchmod` afin de piéger tout compte utilisateur.\n"
    "  • Retour      : `0` en cas de synchronisation physique (`fdatasync`) réussie, `-1` en cas d'échec.\n"
    "\n"
    "Fonction 112 : sentry_engine.c -- sentry_init_fanotify -- Initialisation de la surveillance noyau\n"
    "  • Signature C : int sentry_init_fanotify(const SentryConfig *config);\n"
    "  • Utilité     : Configure le sous-système de notification noyau en mode permissionnel\n"
    "                  synchrone (`FAN_CLASS_CONTENT | FAN_NONBLOCK | FAN_CLOEXEC`). Déploie le\n"
    "                  leurre si nécessaire et pose une marque de verrouillage d'ouverture\n"
    "                  synchrone (`FAN_MARK_ADD`, `FAN_OPEN_PERM`) sur chaque cible configurée.\n"
    "  • Retour      : Descripteur de fichier `fan_fd` valide, ou `-1` en cas d'erreur de privilèges.\n"
    "\n"
    "Fonction 113 : sentry_engine.c -- sentry_listen_loop -- Boucle de veille passive bloquante\n"
    "  • Signature C : int sentry_listen_loop(int fanotify_fd, const SentryConfig *config);\n"
    "  • Utilité     : Assure le guet passif bloquant sur `poll()` (consommation de 0% CPU au repos).\n"
    "                  Dépouille les événements noyau dans un tampon statique de 4 Ko aligné\n"
    "                  sur 8 octets (`__attribute__((aligned(8)))`) sans allocation sur le tas (`0 heap`).\n"
    "  • Retour      : `0` lors d'un arrêt nominal par signal, `-1` en cas d'erreur de lecture.\n"
    "\n"
    "Fonction 114 : sentry_engine.c -- sentry_trigger_reflex -- Riposte cinétique synchrone\n"
    "  • Signature C : int sentry_trigger_reflex(int fanotify_fd, int fd_event, pid_t pid);\n"
    "  • Utilité     : Applique instantanément le rejet auprès du noyau via `FAN_DENY`,\n"
    "                  ferme le descripteur d'événement et achemine le PID agresseur vers\n"
    "                  `rap_reflex_dispatch` avec le motif `TRIGGER_CANARY_BREACH`.\n"
    "  • Retour      : `0` si la neutralisation a réussi, `-1` si le PID était protégé.\n"
    "\n"
    "Fonction 115 : sentry_engine.c -- sentry_enter_stealth -- Blindage et invisibilité mémoire\n"
    "  • Signature C : int sentry_enter_stealth(const char *fake_name);\n"
    "  • Utilité     : Masque le nom du processus dans la table noyau via `prctl(PR_SET_NAME, \"[kworker/u16:3]\")`,\n"
    "                  s'attribue l'immunité OOM maximale (`/proc/self/oom_score_adj = -1000`) et\n"
    "                  verrouille l'intégralité des pages physiques en RAM via `mlockall(MCL_CURRENT | MCL_FUTURE)`.\n"
    "  • Retour      : `0` en cas de succès opérationnel, `-1` si les limites système restreignent le verrouillage.\n"
    "\n"
    "Fonction 116 : cmd_sentry.c -- cmd_sentry_arm -- Armement des leurres sur disque\n"
    "  • Signature C : static int cmd_sentry_arm(int argc, char **argv);\n"
    "  • Utilité     : Déploie les fichiers canaris dans les cibles spécifiées en ligne de\n"
    "                  commande ou applique les répertoires sensibles par défaut (`/home`, `/var/www`, `/var/lib`, `/etc`).\n"
    "  • Retour      : `0` (succès) si au moins un appât est opérationnel, `2` en cas d'échec global.\n"
    "\n"
    "Fonction 117 : cmd_sentry.c -- cmd_sentry_disarm -- Purge et désarmement préventif\n"
    "  • Signature C : static int cmd_sentry_disarm(int argc, char **argv);\n"
    "  • Utilité     : Supprime proprement par déliaison physique (`unlink`) les fichiers\n"
    "                  leurres pour autoriser les opérations de maintenance sans fausse alerte.\n"
    "  • Retour      : Toujours `0` après affichage du bilan de neutralisation.\n"
    "\n"
    "Fonction 118 : cmd_sentry.c -- cmd_sentry_status -- Audit d'intégrité physique des appâts\n"
    "  • Signature C : static int cmd_sentry_status(int argc, char **argv);\n"
    "  • Utilité     : Vérifie l'existence et la stricte conformité de taille (32768 octets)\n"
    "                  des canaris sur l'ensemble des répertoires surveillés (`[ACTIF]`, `[ALTERE]`, `[NON ARME]`).\n"
    "  • Retour      : Toujours `0`.\n"
    "\n"
    "Fonction 119 : cmd_sentry.c -- cmd_sentry_run -- Démarrage du guet passif fanotify\n"
    "  • Signature C : static int cmd_sentry_run(int argc, char **argv);\n"
    "  • Utilité     : Initialise `fanotify`, prend en charge le détachement en démon d'arrière-plan\n"
    "                  (`-d` / `--daemon`) via `fork()` et `setsid()`, active l'invisibilité furtive\n"
    "                  (`sentry_enter_stealth`), redirige les flux vers `/dev/null` et lance la veille.\n"
    "  • Retour      : `0` en cas d'arrêt propre, `2` en cas de droits administrateur insuffisants.\n"
    "\n"
    "Fonction 120 : cmd_sentry.c -- cmd_sentry -- Aiguilleur CLI sentry\n"
    "  • Signature C : int cmd_sentry(int argc, char **argv);\n"
    "  • Utilité     : Point d'entrée principal de la sous-commande `rap sentry`. Parse les\n"
    "                  arguments et route vers `arm`, `disarm`, `status`, `dump` ou `run`.\n"
    "  • Retour      : Code de retour de la sous-commande, ou `2` si la commande est inconnue.\n"
    "\n"
    "Fonction 121 : rap_main.c -- rap_cmd_sentry -- Pont déclaratif sentinelle\n"
    "  • Signature C : int rap_cmd_sentry(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Relie la table déclarative universelle `g_commands` de RAPTOR au point\n"
    "                  d'entrée de la sentinelle anti-rançongiciel (`cmd_sentry`).\n"
    "  • Retour      : Code de retour POSIX transmis par `cmd_sentry`.\n"
    "\n"
    "Fonction 122 : cmd_sentry.c -- cmd_sentry_dump -- Restitution boîte noire médico-légale\n"
    "  • Signature C : static int cmd_sentry_dump(int argc, char **argv);\n"
    "  • Utilité     : Extrait et déverse les événements médico-légaux conservés dans l'anneau\n"
    "                  circulaire en mémoire vive (`/dev/shm/raptor_blackbox`) avec latences nanosecondes.\n"
    "  • Retour      : `0` après affichage, `2` si la boîte noire est vide ou inaccessible.\n"
    "\n"
    "================================================================================\n"
    "SECTION 18 : OUTILLAGE DE COMPILATION & SOURCE UNIQUE SSOT (tools/)\n"
    "================================================================================\n"
    "\n"
    "Outil 01 : gen_man.py -- parse_ssot_markdown -- Découpage multi-pages\n"
    "  • Signature Python : def parse_ssot_markdown(md_raw) -> dict\n"
    "  • Utilité          : Parse le fichier maître `docs/man/rap.1.md` et segmente chaque\n"
    "                       page de manuel délimitée par les balises `<!-- MAN: key | sec | titre -->`.\n"
    "\n"
    "Outil 02 : gen_man.py -- parse_functions_dict -- Segmentation du dictionnaire d'ingénierie\n"
    "  • Signature Python : def parse_functions_dict(dict_raw) -> list\n"
    "  • Utilité          : Extrait hiérarchiquement les Sections et les fonctions unitaires numérotées\n"
    "                       du dictionnaire pour alimenter la table C99 `g_rap_fn_entries[]`.\n"
    "\n"
    "Outil 03 : gen_man.py -- format_unix_header -- Bannière d'en-tête Unix\n"
    "  • Signature Python : def format_unix_header(key, sec) -> str\n"
    "  • Utilité          : Calibre et génère la bannière d'en-tête troff/groff centrée\n"
    "                       sur 70 colonnes (ex: `RAP(1)   RAPTOR DEFENSE   RAP(1)`).\n"
    "\n"
    "Outil 04 : gen_man.py -- escape_c_string -- Échappement de chaînes multilignes C99\n"
    "  • Signature Python : def escape_c_string(text) -> str\n"
    "  • Utilité          : Convertit un bloc textuel brut en constantes littérales C99 sécurisées\n"
    "                       sans débordement de pile pour les fichiers `.rodata`.\n"
    "\n"
    "Outil 05 : gen_man.py -- generate_cli_header -- Sérialisation .rodata C99\n"
    "  • Signature Python : def generate_cli_header(man_pages, fn_entries, full_dict_raw) -> str\n"
    "  • Utilité          : Génère `src/cli/rap_man_data.h` contenant les tables constantes\n"
    "                       `g_rap_man_pages[]`, `g_rap_man_data[]` et `g_rap_fn_entries[]` (zéro tas).\n"
    "\n"
    "Outil 06 : gen_man.py -- generate_js_database -- Sérialisation Web JSON/JS\n"
    "  • Signature Python : def generate_js_database(man_pages, fn_entries) -> str\n"
    "  • Utilité          : Construit la base de données JavaScript `docs/js/man_data.js`\n"
    "                       alimentant la visionneuse interactive de `docs/man.html`.\n"
    "\n"
    "Outil 07 : gen_man.py -- main -- Orchestrateur de compilation documentaire SSOT\n"
    "  • Signature Python : def main() -> None\n"
    "  • Utilité          : Point d'entrée vérifiant l'existence des sources, synchronisant le binaire C\n"
    "                       et la documentation web en une seule passe atomique.\n"
    "\n"
    "================================================================================\n"
    "SECTION 19 : MOTEUR CINÉTIQUE RÉFLEXE, BOÎTE NOIRE ET SURVIE MÉMOIRE (src/core/)\n"
    "================================================================================\n"
    "\n"
    "Fonction 127 : rap_reflex.c -- rap_reflex_init -- Initialisation du moteur réflexe\n"
    "  • Signature C : int rap_reflex_init(reflex_mode_t initial_mode);\n"
    "  • Utilité     : Initialise les structures internes de remédiation, remet à zéro les\n"
    "                  compteurs d'intervention et prépare l'accès à la boîte noire en mémoire vive.\n"
    "  • Retour      : `0` en cas de succès, `-1` en cas d'erreur d'ouverture de la boîte noire.\n"
    "\n"
    "Fonction 128 : rap_reflex.c -- rap_reflex_set_mode -- Commutation du mode d'armement\n"
    "  • Signature C : void rap_reflex_set_mode(reflex_mode_t mode);\n"
    "  • Utilité     : Modifie à chaud le comportement opérationnel du disjoncteur cinétique\n"
    "                  (`REFLEX_MODE_PASSIVE`, `REFLEX_MODE_DRYRUN`, `REFLEX_MODE_ARMED`).\n"
    "\n"
    "Fonction 129 : rap_reflex.c -- rap_reflex_dispatch -- Séquençage et frappe chirurgicale\n"
    "  • Signature C : int rap_reflex_dispatch(pid_t target_pid, reflex_trigger_t trigger);\n"
    "  • Utilité     : Exécute la riposte séquentielle déterministe en moins de 40 µs :\n"
    "                  1. Vérification des gardes-fous (PID <= 1, self, parent).\n"
    "                  2. Gel immédiat du processus et de son groupe de threads via `SIGSTOP`.\n"
    "                  3. Révocation forcée des descripteurs de sockets réseau dans `/proc/[PID]/fd`.\n"
    "                  4. Destruction physique de l'agresseur par émission de `SIGKILL`.\n"
    "                  5. Calcul de latence nanoseconde et injection dans la boîte noire RAM.\n"
    "  • Retour      : `0` si l'action a été appliquée ou simulée, `-1` en cas d'erreur.\n"
    "\n"
    "Fonction 130 : rap_reflex.c -- rap_reflex_get_stats -- Télémétrie des neutralisations\n"
    "  • Signature C : reflex_mode_t rap_reflex_get_stats(uint64_t *out_neutralized_count);\n"
    "  • Utilité     : Retourne le mode de fonctionnement actif et extrait le nombre cumulé\n"
    "                  d'anomalies abattues ou consignées depuis le démarrage.\n"
    "\n"
    "Fonction 131 : rap_blackbox.c -- rap_blackbox_open -- Initialisation anneau circulaire RAM\n"
    "  • Signature C : int rap_blackbox_open(void);\n"
    "  • Utilité     : Crée ou ouvre le segment mémoire partagé `/dev/shm/raptor_blackbox`\n"
    "                  (permissions `0666`) dimensionné pour 64 enregistrements FIFO avec\n"
    "                  verrouillage d'écriture sans concurrence.\n"
    "  • Retour      : `0` en cas d'initialisation réussie, `-1` en cas d'échec d'accès à `/dev/shm`.\n"
    "\n"
    "Fonction 132 : rap_blackbox.c -- rap_blackbox_push -- Enregistrement atomique d'incident\n"
    "  • Signature C : int rap_blackbox_push(const reflex_record_t *record);\n"
    "  • Utilité     : Inscrit un événement médico-légal complet (horodatage temps réel, PID,\n"
    "                  PPID, ligne de commande, cause, latence nanoseconde, statut de destruction)\n"
    "                  dans l'anneau circulaire avec écrasement rotatif en cas de saturation.\n"
    "  • Retour      : `0` en cas de succès, `-1` si la boîte noire n'est pas ouverte.\n"
    "\n"
    "Fonction 133 : rap_blackbox.c -- rap_blackbox_dump -- Restitution formatée boîte noire\n"
    "  • Signature C : int rap_blackbox_dump(void);\n"
    "  • Utilité     : Lit chronologiquement l'anneau circulaire en mémoire vive et affiche\n"
    "                  les fiches forensiques détaillées avec codes couleurs ANSI sans nécessiter\n"
    "                  les privilèges superutilisateur.\n"
    "  • Retour      : Nombre d'événements affichés, ou `-1` en cas d'erreur.\n"
    "\n"
    "Fonction 134 : rap_oom.c -- rap_oom_protect -- Immunité système anti-éviction OOM\n"
    "  • Signature C : int rap_oom_protect(void);\n"
    "  • Utilité     : Écrit la valeur `-1000` dans `/proc/self/oom_score_adj` pour conférer\n"
    "                  au binaire RAPTOR une immunité vitale absolue équivalente à `init` (PID 1)\n"
    "                  face au tueur de mémoire du noyau Linux (*Out-Of-Memory Killer*).\n"
    "  • Retour      : `0` en cas de succès, `-1` en cas de droits administratifs insuffisants.\n"
    "\n"
    "Fonction 135 : rap_oom.c -- rap_oom_lock_memory -- Verrouillage des pages physiques RAM\n"
    "  • Signature C : int rap_oom_lock_memory(void);\n"
    "  • Utilité     : Invoque `mlockall(MCL_CURRENT | MCL_FUTURE)` pour interdire tout swap\n"
    "                  sur disque des pages mémoires de RAPTOR, garantissant un temps de réponse\n"
    "                  sans latence de défaut de page (*page fault*) lors des frappes réflexes.\n"
    "  • Retour      : `0` en cas de verrouillage physique réussi, `-1` en cas de dépassement de quota.\n"
    "\n"
    "================================================================================\n"
    "SECTION 20 : CONSOLE TACTIQUE RÉSEAU TEMPS RÉEL (NETOP) & ROUTEUR DÉCOUPLÉ (src/cli/)\n"
    "================================================================================\n"
    "\n"
    "Fonction 136 : cmd_netop.c -- rap_cmd_netop -- Point d'entrée de la console netop\n"
    "  • Signature C : int rap_cmd_netop(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Point d'entrée principal de la commande `rap netop`. Initialise le contexte,\n"
    "                  parse les options de limitation de lignes (-n), active l'immunité OOM\n"
    "                  (rap_oom_protect), configure le routeur de commandes et pilote la boucle\n"
    "                  d'écoute non-bloquante STDIN via poll().\n"
    "  • Retour      : 0 (RAP_EXIT_OK) en sortie nominale, 2 (RAP_EXIT_ERROR) si STDIN n'est pas un TTY.\n"
    "\n"
    "Fonction 137 : cmd_netop.c -- netop_tui_init -- Initialisation mode brut TTY\n"
    "  • Signature C : int netop_tui_init(NetopContext *ctx);\n"
    "  • Utilité     : Sauvegarde les attributs originaux du terminal via tcgetattr, bascule\n"
    "                  en mode brut non-canonique sans écho (VMIN=0, VTIME=0), intercepte les signaux\n"
    "                  (SIGINT, SIGTERM, SIGQUIT) pour restauration d'urgence, masque le curseur\n"
    "                  (\\033[?25l) et purge l'écran (\\033[2J).\n"
    "  • Retour      : 0 en cas de succès, -1 en cas d'échec termios.\n"
    "\n"
    "Fonction 138 : cmd_netop.c -- netop_tui_render -- Moteur de rendu matriciel TUI\n"
    "  • Signature C : void netop_tui_render(NetopContext *ctx, const NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Orchestre le rendu graphique temps réel : délègue à l'overlay actif si présent,\n"
    "                  sinon projette la télémétrie supérieure, la base OSINT, le statut OOM,\n"
    "                  les modes de tri et de filtre actifs, la grille de sockets colorée par niveau\n"
    "                  de criticité (7 colonnes) et le pied de page dynamique avec purge basse (\\033[J).\n"
    "\n"
    "Fonction 139 : netop_engine.c -- netop_engine_scan -- Ingestion et confrontation passives\n"
    "  • Signature C : size_t netop_engine_scan(NetopContext *ctx, NetopEntry *out_entries, size_t max_entries);\n"
    "  • Utilité     : Lit directement /proc/net/tcp, décode les adresses et états de connexion,\n"
    "                  déclenche l'indexation ciblée des descripteurs, confronte les IP distantes\n"
    "                  à la table OSINT souveraine g_rap_threat_db, évalue les règles Net-Guard et\n"
    "                  applique les pierres tombales rémanentes.\n"
    "  • Retour      : Nombre exact de connexions ingérées et qualifiées.\n"
    "\n"
    "Fonction 140 : netop_engine.c -- netop_engine_build_inode_cache -- Résolution Inode/PID ciblée\n"
    "  • Signature C : static void netop_engine_build_inode_cache(const NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Parcourt /proc/[PID]/fd pour résoudre les descripteurs socket:[INODE]\n"
    "                  associés aux sockets actifs, extrait le nom du binaire (/proc/[PID]/comm)\n"
    "                  et alimente le cache statique 0-heap sans altération d'horodatage.\n"
    "\n"
    "Fonction 141 : netop_engine.c -- netop_engine_get_threat_count -- Dénombrement base OSINT\n"
    "  • Signature C : size_t netop_engine_get_threat_count(void);\n"
    "  • Utilité     : Restitue le volume exact de signatures d'adresses IP hostiles compilées\n"
    "                  en dur dans la section .rodata de RAPTOR.\n"
    "  • Retour      : Nombre d'enregistrements (19 867 signatures).\n"
    "\n"
    "Fonction 141b : netop_engine.c -- netop_engine_lookup_threat -- Recherche dichotomique OSINT\n"
    "  • Signature C : bool netop_engine_lookup_threat(uint32_t ip, uint8_t *out_cat, uint8_t *out_score);\n"
    "  • Utilité     : Exécute une recherche dichotomique O(log N) dans la table binaire triée\n"
    "                  g_rap_threat_db pour extraire la catégorie hostile (C2, Tor, Abus) et le score.\n"
    "  • Retour      : true si l'IP correspond à une menace répertoriée, false sinon.\n"
    "\n"
    "Fonction 142 : netop_cmd_kill.c -- netop_cmd_kill_exec -- Tir cinétique chirurgical 'k'\n"
    "  • Signature C : int netop_cmd_kill_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Valide le numéro de slot cible, contrôle les verrous d'inviolabilité absolue\n"
    "                  (PID 1, RAPTOR, shell parent), paralyse le processus via SIGSTOP puis le détruit\n"
    "                  via SIGKILL. Mesure la latence à la microseconde, consigne le slot dans le\n"
    "                  registre des tombstones (7 secondes) et actualise l'affichage en orange ambré.\n"
    "  • Retour      : 0 en cas de frappe réussie, -1 en cas de violation de sécurité ou cible invalide.\n"
    "\n"
    "Fonction 143 : netop_cmd_kill.c -- netop_cmd_kill_apply_tombstones -- Rémanence des cibles\n"
    "  • Signature C : void netop_cmd_kill_apply_tombstones(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Parcourt le registre statique des tombstones et marque comme neutralisés\n"
    "                  ([ABATTU], DISJONCTÉ, NETOP_SEV_KILLED) tous les sockets appartenant à un PID\n"
    "                  ou un endpoint abattu au cours des 7 dernières secondes.\n"
    "\n"
    "Fonction 144 : netop_router.c -- netop_router_init -- Initialisation de la table de dispatch\n"
    "  • Signature C : void netop_router_init(void);\n"
    "  • Utilité     : Réinitialise l'aiguilleur central, vide les tampons d'invites et inscrit de\n"
    "                  manière déclarative l'ensemble des greffons de commandes (k, b, s, f, i, Ctrl+B, p).\n"
    "\n"
    "Fonction 145 : netop_router.c -- netop_router_handle_key -- Aiguilleur central des frappes\n"
    "  • Signature C : void netop_router_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);\n"
    "  • Utilité     : Capture non-bloquante du flux clavier. Gère la machine à états de saisie\n"
    "                  des invites (séparateurs, virgules, minus, validation Entrée, annulation Échap),\n"
    "                  route vers les gestionnaires d'overlays si une modale est active, ou déclenche\n"
    "                  le greffon associé à la touche en mode normal.\n"
    "\n"
    "Fonction 146 : netop_router.c -- netop_router_get_footer -- Composition du pied de page\n"
    "  • Signature C : const char *netop_router_get_footer(NetopContext *ctx);\n"
    "  • Utilité     : Génère dynamiquement la ligne d'état inférieure : affiche le prompt de saisie\n"
    "                  avec curseur clignotant si une commande est en attente, les notifications\n"
    "                  médico-légales minutées, ou la liste des raccourcis enregistrés.\n"
    "\n"
    "Fonction 147 : netop_cmd_ban.c -- netop_cmd_ban_exec -- Embargo et déban multi-sélection 'b'\n"
    "  • Signature C : int netop_cmd_ban_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Parseur de sélections groupées (chiffres, virgules, espaces, signe moins pour déban,\n"
    "                  préfixe B pour index registre). Fige un snapshot du registre, déduplique les IP\n"
    "                  intra-batch, injecte la règle Netfilter (iptables -I INPUT -s IP -j DROP) avec repli\n"
    "                  sur route blackhole (ip route add blackhole), ou lève l'interdiction (iptables -D /\n"
    "                  ip route del). Mesure la latence en microsecondes et synchronise les flux.\n"
    "  • Retour      : 0 en cas de traitement nominal, -1 si privilèges root absents.\n"
    "\n"
    "Fonction 148 : netop_cmd_ban.c -- netop_cmd_ban_is_banned -- Statut d'embargo d'une IP\n"
    "  • Signature C : bool netop_cmd_ban_is_banned(const char *remote_ip);\n"
    "  • Utilité     : Interroge la table statique interne des embargos pour certifier si une adresse IP\n"
    "                  distante fait l'objet d'un verrouillage actif.\n"
    "  • Retour      : true si l'IP est sous embargo actif, false sinon.\n"
    "\n"
    "Fonction 149 : netop_cmd_ban.c -- netop_cmd_ban_toggle_list -- Bascule de la modale Ctrl+B\n"
    "  • Signature C : int netop_cmd_ban_toggle_list(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Intervertit l'affichage entre la grille réseau principale et le tableau d'embargo\n"
    "                  noyau lors de la combinaison Ctrl+B (ASCII 0x02).\n"
    "  • Retour      : Toujours 0.\n"
    "\n"
    "Fonction 150 : netop_cmd_ban.c -- netop_cmd_ban_render_list -- Rendu du registre d'embargo\n"
    "  • Signature C : bool netop_cmd_ban_render_list(NetopContext *ctx);\n"
    "  • Utilité     : Projette en plein écran la vue modale du registre des adresses bannies :\n"
    "                  index unique #B01, adresse IP, nombre de sockets impactés, durée d'activation\n"
    "                  et mécanisme noyau actif, avec effacement absolu des résidus graphiques.\n"
    "  • Retour      : true lorsque le rendu a été projeté.\n"
    "\n"
    "Fonction 151 : netop_router.c -- netop_router_render_overlay -- Délégation de vue modale\n"
    "  • Signature C : bool netop_router_render_overlay(NetopContext *ctx);\n"
    "  • Utilité     : Interroge le pointeur de fonction de l'overlay actif. S'il est non nul,\n"
    "                  exécute le rendu de la modale et suspend l'affichage de la grille principale.\n"
    "  • Retour      : true si une modale a pris la main sur l'affichage, false sinon.\n"
    "\n"
    "Fonction 152 : netop_router.c -- netop_router_set_overlay -- Gestion de transition d'écran\n"
    "  • Signature C : void netop_router_set_overlay(NetopOverlayRenderer renderer);\n"
    "  • Utilité     : Définit le gestionnaire d'affichage modal courant et purge physiquement\n"
    "                  l'écran (\\033[2J\\033[H) pour garantir une transition sans superposition de texte.\n"
    "\n"
    "Fonction 153 : netop_cmd_inspect.c -- netop_cmd_inspect_exec -- Déclencheur d'inspection 'i'\n"
    "  • Signature C : int netop_cmd_inspect_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Valide le numéro de slot sélectionné, lance la collecte des indicateurs médico-légaux\n"
    "                  via netop_cmd_inspect_collect et active la surcouche modale netop_cmd_inspect_render_modal.\n"
    "  • Retour      : 0 en cas de succès, -1 si le numéro de slot est hors plage.\n"
    "\n"
    "Fonction 154 : netop_cmd_inspect.c -- netop_cmd_inspect_collect -- Autopsie médico-légale /proc\n"
    "  • Signature C : static void netop_cmd_inspect_collect(NetopForensicRecord *f, const NetopEntry *e, int slot);\n"
    "  • Utilité     : Extrait sans allocation dynamique : le PID, le PPID (/proc/[PID]/status), le nom du\n"
    "                  processus parent, le chemin canonique de l'exécutable sur disque (readlink /proc/[PID]/exe),\n"
    "                  la ligne de commande complète (/proc/[PID]/cmdline), l'évaluation OSINT et l'état d'embargo.\n"
    "\n"
    "Fonction 155 : netop_cmd_inspect.c -- netop_cmd_inspect_detect_revshell -- Traque Reverse Shell\n"
    "  • Signature C : static void netop_cmd_inspect_detect_revshell(NetopForensicRecord *f);\n"
    "  • Utilité     : Énumère /proc/[PID]/fd/* et résout les cibles des descripteurs 0 (STDIN), 1 (STDOUT)\n"
    "                  et 2 (STDERR). Déclenche l'alerte critique REVERSE SHELL DÉTECTÉ si l'entrée et la sortie\n"
    "                  standard sont conjointement redirigées vers un socket réseau.\n"
    "\n"
    "Fonction 156 : netop_cmd_inspect.c -- netop_cmd_inspect_render_modal -- Rendu de la fiche forensic\n"
    "  • Signature C : bool netop_cmd_inspect_render_modal(NetopContext *ctx);\n"
    "  • Utilité     : Projette en plein écran la fiche d'autopsie structurée en 4 blocs : Identité & Généalogie,\n"
    "                  Ancrage Réseau & Socket, Traque des Descripteurs & Détournement, et Renseignement OSINT & Embargo.\n"
    "  • Retour      : true lorsque le rendu a été projeté.\n"
    "\n"
    "Fonction 157 : netop_cmd_inspect.c -- netop_cmd_inspect_handle_key -- Clavier contextuel en modale\n"
    "  • Signature C : bool netop_cmd_inspect_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);\n"
    "  • Utilité     : Intercepte les actions contextuelles en cours d'inspection : k (neutralise immédiatement\n"
    "                  le PID inspecté), b (bascule l'embargo Netfilter sur l'IP inspectée) ou q/Échap (fermeture de la fiche).\n"
    "  • Retour      : true si la touche a été traitée par la modale, false sinon.\n"
    "\n"
    "Fonction 158 : netop_cmd_sort.c -- netop_cmd_sort_exec -- Bascule cyclique de tri 's'\n"
    "  • Signature C : int netop_cmd_sort_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Commutation circulaire instantanée sans invite : DANGEROSITÉ -> PID -> PORT/ACTIVITÉ -> DANGEROSITÉ.\n"
    "                  Applique immédiatement l'ordonnancement et affiche une notification d'état temporaire.\n"
    "  • Retour      : Toujours 0.\n"
    "\n"
    "Fonction 159 : netop_cmd_sort.c -- netop_cmd_sort_apply -- Algorithme de tri stable 0-heap\n"
    "  • Signature C : void netop_cmd_sort_apply(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Réordonne le tableau d'entrées in-place via un algorithme de tri par insertion stable\n"
    "                  sans aucune allocation mémoire, garantissant une exécution déterministe en moins de 5 µs.\n"
    "\n"
    "Fonction 160 : netop_cmd_sort.c -- netop_cmd_sort_get_name -- Libellé du mode de tri\n"
    "  • Signature C : const char *netop_cmd_sort_get_name(void);\n"
    "  • Utilité     : Restitue la chaîne descriptive textuelle du tri actif (\"DANGEROSITÉ\", \"PID\", \"PORT/ACTIVITÉ\")\n"
    "                  pour intégration dynamique dans l'en-tête de télémétrie.\n"
    "\n"
    "Fonction 161 : netop_cmd_filter.c -- netop_cmd_filter_exec -- Bascule cyclique de filtre 'f'\n"
    "  • Signature C : int netop_cmd_filter_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Commutation circulaire instantanée des filtres de vue : TOUS -> HORS LOCAL -> ÉTABLIS ->\n"
    "                  ALERTES SEULES -> TOUS. Émet la notification d'état temporaire.\n"
    "  • Retour      : Toujours 0.\n"
    "\n"
    "Fonction 162 : netop_cmd_filter.c -- netop_cmd_filter_apply -- Compaction in-place des flux\n"
    "  • Signature C : size_t netop_cmd_filter_apply(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Filtre et compacte le tableau d'entrées directement en mémoire selon les critères du mode\n"
    "                  sélectionné (masquage de 127.0.0.1/LISTEN, conservation stricte d'ESTABLISHED, ou isolation\n"
    "                  des niveaux HOSTILE/WARN/KILLED/BAN).\n"
    "  • Retour      : Nombre d'entrées visibles après filtrage.\n"
    "\n"
    "Fonction 163 : netop_cmd_filter.c -- netop_cmd_filter_get_name -- Libellé du mode de filtre\n"
    "  • Signature C : const char *netop_cmd_filter_get_name(void);\n"
    "  • Utilité     : Restitue la chaîne descriptive textuelle du filtre actif (\"TOUS\", \"HORS LOCAL\", \"ÉTABLIS\",\n"
    "                  \"ALERTES SEULES\") pour affichage dans l'en-tête de télémétrie.\n"
    "\n"
    "================================================================================\n"
    "FIN DU RÉFÉRENTIEL TECHNIQUE OFFICIEL // RAPTOR DEFENSE v2.0\n"
    "================================================================================\n";

/* Structure de repérage hiérarchique du dictionnaire des fonctions */
typedef struct {
    int section;        /* 1 à 18 (0 si non spécifié) */
    int fn_id;          /* 0 = Section entière, > 0 = Fonction unitaire */
    const char *title;  /* Libellé d'identification */
    const char *content;/* Contenu technique formaté */
} RapFnDocEntry;

static const RapFnDocEntry g_rap_fn_entries[] = {
    {
        1,
        0,
        "SECTION 1 : MODÈLE DE DONNÉES UNIVERSEL (src/core/raptor_data.c)",
    "=== SECTION 1 : MODÈLE DE DONNÉES UNIVERSEL (src/core/raptor_data.c) ===\n"
    "\n"
    "Fonction 01 : raptor_data.c -- raptor_data_context_create -- Allocation du contexte\n"
    "  • Signature C : RaptorReportContext* raptor_data_context_create(void);\n"
    "  • Utilité     : Alloue dynamiquement en mémoire vive et initialise la structure\n"
    "                  centrale du rapport destinée à recevoir les données des greffons.\n"
    "\n"
    "Fonction 02 : raptor_data.c -- raptor_data_section_add -- Création de section\n"
    "  • Signature C : RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title);\n"
    "  • Utilité     : Ajoute un conteneur thématique autonome dans le contexte pour\n"
    "                  regrouper les cartes, tables et alertes produites par un greffon.\n"
    "\n"
    "Fonction 03 : raptor_data.c -- raptor_data_card_add -- Insertion de métrique clé\n"
    "  • Signature C : bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev);\n"
    "  • Utilité     : Enregistre un indicateur synthétique (valeur, pourcentage, niveau\n"
    "                  de criticité) affiché sous forme de tuile métrique dans l'interface.\n"
    "\n"
    "Fonction 04 : raptor_data.c -- raptor_data_table_create -- Initialisation de tableau\n"
    "  • Signature C : RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names);\n"
    "  • Utilité     : Instancie une structure de tableau bidimensionnel avec ses en-têtes\n"
    "                  de colonnes au sein d'une section donnée.\n"
    "\n"
    "Fonction 05 : raptor_data.c -- raptor_data_table_row_add -- Insertion de ligne de données\n"
    "  • Signature C : bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs);\n"
    "  • Utilité     : Ajoute une ligne complète de cellules textuelles associées à des\n"
    "                  codes de couleur/sévérité dans le tableau spécifié.\n"
    "\n"
    "Fonction 06 : raptor_data.c -- raptor_data_alert_add -- Notification de sécurité\n"
    "  • Signature C : bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message);\n"
    "  • Utilité     : Consigne un message d'avertissement ou de confirmation dans la section\n"
    "                  avec un niveau d'alerte défini (OK, AVERTISSEMENT, CRITIQUE).\n"
    "\n"
    "Fonction 07 : raptor_data.c -- raptor_data_get_section_count -- Dénombrement des sections\n"
    "  • Signature C : size_t raptor_data_get_section_count(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Renvoie le nombre total de sections actuellement peuplées dans le\n"
    "                  contexte de rapport actif.\n"
    "\n"
    "Fonction 08 : raptor_data.c -- raptor_data_get_section -- Accès sécurisé par index\n"
    "  • Signature C : const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index);\n"
    "  • Utilité     : Fournit un pointeur en lecture seule vers une section précise à partir\n"
    "                  de son rang séquentiel dans la collection.\n"
    "\n"
    "Fonction 09 : raptor_data.c -- raptor_data_context_clear -- Purge dynamique du contexte\n"
    "  • Signature C : void raptor_data_context_clear(RaptorReportContext* ctx);\n"
    "  • Utilité     : Libère récursivement l'ensemble des sections, tables et alertes\n"
    "                  tout en maintenant le conteneur principal alloué pour le Live Monitoring 3s.\n"
    "\n"
    "Fonction 10 : raptor_data.c -- raptor_data_context_destroy -- Destruction totale\n"
    "  • Signature C : void raptor_data_context_destroy(RaptorReportContext* ctx);\n"
    "  • Utilité     : Désalloue complètement l'arbre de mémoire du contexte et libère\n"
    "                  le pointeur racine lors de l'arrêt définitif de l'application.\n"
    },
    {
        1,
        1,
        "Fonction 1 : raptor_data.c -- raptor_data_context_create -- Allocation du contexte",
    "Fonction 01 : raptor_data.c -- raptor_data_context_create -- Allocation du contexte\n"
    "  • Signature C : RaptorReportContext* raptor_data_context_create(void);\n"
    "  • Utilité     : Alloue dynamiquement en mémoire vive et initialise la structure\n"
    "                  centrale du rapport destinée à recevoir les données des greffons.\n"
    },
    {
        1,
        2,
        "Fonction 2 : raptor_data.c -- raptor_data_section_add -- Création de section",
    "Fonction 02 : raptor_data.c -- raptor_data_section_add -- Création de section\n"
    "  • Signature C : RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title);\n"
    "  • Utilité     : Ajoute un conteneur thématique autonome dans le contexte pour\n"
    "                  regrouper les cartes, tables et alertes produites par un greffon.\n"
    },
    {
        1,
        3,
        "Fonction 3 : raptor_data.c -- raptor_data_card_add -- Insertion de métrique clé",
    "Fonction 03 : raptor_data.c -- raptor_data_card_add -- Insertion de métrique clé\n"
    "  • Signature C : bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev);\n"
    "  • Utilité     : Enregistre un indicateur synthétique (valeur, pourcentage, niveau\n"
    "                  de criticité) affiché sous forme de tuile métrique dans l'interface.\n"
    },
    {
        1,
        4,
        "Fonction 4 : raptor_data.c -- raptor_data_table_create -- Initialisation de tableau",
    "Fonction 04 : raptor_data.c -- raptor_data_table_create -- Initialisation de tableau\n"
    "  • Signature C : RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names);\n"
    "  • Utilité     : Instancie une structure de tableau bidimensionnel avec ses en-têtes\n"
    "                  de colonnes au sein d'une section donnée.\n"
    },
    {
        1,
        5,
        "Fonction 5 : raptor_data.c -- raptor_data_table_row_add -- Insertion de ligne de données",
    "Fonction 05 : raptor_data.c -- raptor_data_table_row_add -- Insertion de ligne de données\n"
    "  • Signature C : bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs);\n"
    "  • Utilité     : Ajoute une ligne complète de cellules textuelles associées à des\n"
    "                  codes de couleur/sévérité dans le tableau spécifié.\n"
    },
    {
        1,
        6,
        "Fonction 6 : raptor_data.c -- raptor_data_alert_add -- Notification de sécurité",
    "Fonction 06 : raptor_data.c -- raptor_data_alert_add -- Notification de sécurité\n"
    "  • Signature C : bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message);\n"
    "  • Utilité     : Consigne un message d'avertissement ou de confirmation dans la section\n"
    "                  avec un niveau d'alerte défini (OK, AVERTISSEMENT, CRITIQUE).\n"
    },
    {
        1,
        7,
        "Fonction 7 : raptor_data.c -- raptor_data_get_section_count -- Dénombrement des sections",
    "Fonction 07 : raptor_data.c -- raptor_data_get_section_count -- Dénombrement des sections\n"
    "  • Signature C : size_t raptor_data_get_section_count(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Renvoie le nombre total de sections actuellement peuplées dans le\n"
    "                  contexte de rapport actif.\n"
    },
    {
        1,
        8,
        "Fonction 8 : raptor_data.c -- raptor_data_get_section -- Accès sécurisé par index",
    "Fonction 08 : raptor_data.c -- raptor_data_get_section -- Accès sécurisé par index\n"
    "  • Signature C : const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index);\n"
    "  • Utilité     : Fournit un pointeur en lecture seule vers une section précise à partir\n"
    "                  de son rang séquentiel dans la collection.\n"
    },
    {
        1,
        9,
        "Fonction 9 : raptor_data.c -- raptor_data_context_clear -- Purge dynamique du contexte",
    "Fonction 09 : raptor_data.c -- raptor_data_context_clear -- Purge dynamique du contexte\n"
    "  • Signature C : void raptor_data_context_clear(RaptorReportContext* ctx);\n"
    "  • Utilité     : Libère récursivement l'ensemble des sections, tables et alertes\n"
    "                  tout en maintenant le conteneur principal alloué pour le Live Monitoring 3s.\n"
    },
    {
        1,
        10,
        "Fonction 10 : raptor_data.c -- raptor_data_context_destroy -- Destruction totale",
    "Fonction 10 : raptor_data.c -- raptor_data_context_destroy -- Destruction totale\n"
    "  • Signature C : void raptor_data_context_destroy(RaptorReportContext* ctx);\n"
    "  • Utilité     : Désalloue complètement l'arbre de mémoire du contexte et libère\n"
    "                  le pointeur racine lors de l'arrêt définitif de l'application.\n"
    },
    {
        2,
        0,
        "SECTION 2 : REGISTRE DE GREFFONS ET BUS SYSTÈME (src/core/raptor_registry.c)",
    "=== SECTION 2 : REGISTRE DE GREFFONS ET BUS SYSTÈME (src/core/raptor_registry.c) ===\n"
    "\n"
    "Fonction 11 : raptor_registry.c -- raptor_registry_init -- Initialisation du bus\n"
    "  • Signature C : void raptor_registry_init(void);\n"
    "  • Utilité     : Prépare la table interne d'enregistrement et réinitialise les\n"
    "                  compteurs de modules d'inspection disponibles.\n"
    "\n"
    "Fonction 12 : raptor_registry.c -- raptor_registry_register -- Enregistrement de greffon\n"
    "  • Signature C : bool raptor_registry_register(const RaptorPlugin* plugin);\n"
    "  • Utilité     : Inscrit un greffon d'audit dans la table globale sans nécessiter\n"
    "                  de modification des fichiers sources du noyau central.\n"
    "\n"
    "Fonction 13 : raptor_registry.c -- raptor_registry_run_all -- Déclenchement séquentiel\n"
    "  • Signature C : bool raptor_registry_run_all(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt l'ensemble des greffons actifs et exécute leur méthode\n"
    "                  `run_audit` pour peupler le contexte en une seule passe.\n"
    "\n"
    "Fonction 14 : raptor_registry.c -- raptor_registry_get_count -- Comptage des greffons\n"
    "  • Signature C : size_t raptor_registry_get_count(void);\n"
    "  • Utilité     : Retourne le nombre total de greffons actuellement enregistrés et\n"
    "                  disponibles pour les audits.\n"
    "\n"
    "Fonction 15 : raptor_registry.c -- raptor_registry_get_by_idx -- Accès greffon par index\n"
    "  • Signature C : const RaptorPlugin* raptor_registry_get_by_idx(size_t index);\n"
    "  • Utilité     : Permet d'interroger la configuration, le nom et les métadonnées d'un\n"
    "                  greffon via son index dans le registre.\n"
    "\n"
    "Fonction 16 : raptor_registry.c -- raptor_registry_cleanup -- Déchargement global\n"
    "  • Signature C : void raptor_registry_cleanup(void);\n"
    "  • Utilité     : Déclenche l'appel de nettoyage `cleanup()` sur chaque greffon et\n"
    "                  vide la table interne des pointeurs de modules.\n"
    },
    {
        2,
        11,
        "Fonction 11 : raptor_registry.c -- raptor_registry_init -- Initialisation du bus",
    "Fonction 11 : raptor_registry.c -- raptor_registry_init -- Initialisation du bus\n"
    "  • Signature C : void raptor_registry_init(void);\n"
    "  • Utilité     : Prépare la table interne d'enregistrement et réinitialise les\n"
    "                  compteurs de modules d'inspection disponibles.\n"
    },
    {
        2,
        12,
        "Fonction 12 : raptor_registry.c -- raptor_registry_register -- Enregistrement de greffon",
    "Fonction 12 : raptor_registry.c -- raptor_registry_register -- Enregistrement de greffon\n"
    "  • Signature C : bool raptor_registry_register(const RaptorPlugin* plugin);\n"
    "  • Utilité     : Inscrit un greffon d'audit dans la table globale sans nécessiter\n"
    "                  de modification des fichiers sources du noyau central.\n"
    },
    {
        2,
        13,
        "Fonction 13 : raptor_registry.c -- raptor_registry_run_all -- Déclenchement séquentiel",
    "Fonction 13 : raptor_registry.c -- raptor_registry_run_all -- Déclenchement séquentiel\n"
    "  • Signature C : bool raptor_registry_run_all(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt l'ensemble des greffons actifs et exécute leur méthode\n"
    "                  `run_audit` pour peupler le contexte en une seule passe.\n"
    },
    {
        2,
        14,
        "Fonction 14 : raptor_registry.c -- raptor_registry_get_count -- Comptage des greffons",
    "Fonction 14 : raptor_registry.c -- raptor_registry_get_count -- Comptage des greffons\n"
    "  • Signature C : size_t raptor_registry_get_count(void);\n"
    "  • Utilité     : Retourne le nombre total de greffons actuellement enregistrés et\n"
    "                  disponibles pour les audits.\n"
    },
    {
        2,
        15,
        "Fonction 15 : raptor_registry.c -- raptor_registry_get_by_idx -- Accès greffon par index",
    "Fonction 15 : raptor_registry.c -- raptor_registry_get_by_idx -- Accès greffon par index\n"
    "  • Signature C : const RaptorPlugin* raptor_registry_get_by_idx(size_t index);\n"
    "  • Utilité     : Permet d'interroger la configuration, le nom et les métadonnées d'un\n"
    "                  greffon via son index dans le registre.\n"
    },
    {
        2,
        16,
        "Fonction 16 : raptor_registry.c -- raptor_registry_cleanup -- Déchargement global",
    "Fonction 16 : raptor_registry.c -- raptor_registry_cleanup -- Déchargement global\n"
    "  • Signature C : void raptor_registry_cleanup(void);\n"
    "  • Utilité     : Déclenche l'appel de nettoyage `cleanup()` sur chaque greffon et\n"
    "                  vide la table interne des pointeurs de modules.\n"
    },
    {
        3,
        0,
        "SECTION 3 : MOTEUR D'ANALYSE DIFFÉRENTIELLE (src/core/raptor_differential.c)",
    "=== SECTION 3 : MOTEUR D'ANALYSE DIFFÉRENTIELLE (src/core/raptor_differential.c) ===\n"
    "\n"
    "Fonction 17 : raptor_differential.c -- raptor_diff_init -- Initialisation différentielle\n"
    "  • Signature C : void raptor_diff_init(void);\n"
    "  • Utilité     : Configure les structures mémoire destinées à conserver l'empreinte\n"
    "                  historique nécessaire au calcul des dérives temporelles.\n"
    "\n"
    "Fonction 18 : raptor_differential.c -- raptor_diff_compute -- Calcul des dérivées RAM\n"
    "  • Signature C : bool raptor_diff_compute(const RaptorReportContext* current, RaptorSection* out_diff_sec);\n"
    "  • Utilité     : Compare l'état T(n) avec l'état T(n-1), identifie les fuites de mémoire\n"
    "                  (Mo/min) et peuple la section du cahier différentiel.\n"
    "\n"
    "Fonction 19 : raptor_differential.c -- raptor_diff_snapshot_save -- Sauvegarde d'instantané\n"
    "  • Signature C : void raptor_diff_snapshot_save(const RaptorReportContext* current);\n"
    "  • Utilité     : Copie et fige les métriques de processus actuelles en tant que nouvel\n"
    "                  état de référence pour les comparaisons futures.\n"
    "\n"
    "Fonction 20 : raptor_differential.c -- raptor_diff_cleanup -- Nettoyage différentiel\n"
    "  • Signature C : void raptor_diff_cleanup(void);\n"
    "  • Utilité     : Libère l'intégralité des instantanés historiques archivés en RAM.\n"
    },
    {
        3,
        17,
        "Fonction 17 : raptor_differential.c -- raptor_diff_init -- Initialisation différentielle",
    "Fonction 17 : raptor_differential.c -- raptor_diff_init -- Initialisation différentielle\n"
    "  • Signature C : void raptor_diff_init(void);\n"
    "  • Utilité     : Configure les structures mémoire destinées à conserver l'empreinte\n"
    "                  historique nécessaire au calcul des dérives temporelles.\n"
    },
    {
        3,
        18,
        "Fonction 18 : raptor_differential.c -- raptor_diff_compute -- Calcul des dérivées RAM",
    "Fonction 18 : raptor_differential.c -- raptor_diff_compute -- Calcul des dérivées RAM\n"
    "  • Signature C : bool raptor_diff_compute(const RaptorReportContext* current, RaptorSection* out_diff_sec);\n"
    "  • Utilité     : Compare l'état T(n) avec l'état T(n-1), identifie les fuites de mémoire\n"
    "                  (Mo/min) et peuple la section du cahier différentiel.\n"
    },
    {
        3,
        19,
        "Fonction 19 : raptor_differential.c -- raptor_diff_snapshot_save -- Sauvegarde d'instantané",
    "Fonction 19 : raptor_differential.c -- raptor_diff_snapshot_save -- Sauvegarde d'instantané\n"
    "  • Signature C : void raptor_diff_snapshot_save(const RaptorReportContext* current);\n"
    "  • Utilité     : Copie et fige les métriques de processus actuelles en tant que nouvel\n"
    "                  état de référence pour les comparaisons futures.\n"
    },
    {
        3,
        20,
        "Fonction 20 : raptor_differential.c -- raptor_diff_cleanup -- Nettoyage différentiel",
    "Fonction 20 : raptor_differential.c -- raptor_diff_cleanup -- Nettoyage différentiel\n"
    "  • Signature C : void raptor_diff_cleanup(void);\n"
    "  • Utilité     : Libère l'intégralité des instantanés historiques archivés en RAM.\n"
    },
    {
        4,
        0,
        "SECTION 4 : JOURNALISATION FORENSIQUE PERMANENTE (src/report/report_audit_log.c)",
    "=== SECTION 4 : JOURNALISATION FORENSIQUE PERMANENTE (src/report/report_audit_log.c) ===\n"
    "\n"
    "Fonction 21 : report_audit_log.c -- audit_log_get_system_btime -- Extraction du boot time\n"
    "  • Signature C : static unsigned long long audit_log_get_system_btime(void);\n"
    "  • Utilité     : Lit `/proc/stat` et extrait le timestamp Unix `btime` correspondant\n"
    "                  à l'instant de démarrage matériel de la machine hôte.\n"
    "\n"
    "Fonction 22 : report_audit_log.c -- audit_log_compute_start_time -- Calcul de spawn\n"
    "  • Signature C : static void audit_log_compute_start_time(int pid, char* out_buf, size_t buf_sz);\n"
    "  • Utilité     : Parse le 22e champ (`starttime`) de `/proc/[PID]/stat` et convertit\n"
    "                  les ticks processeur en heure locale absolue de démarrage (HH:MM:SS).\n"
    "\n"
    "Fonction 23 : report_audit_log.c -- audit_log_get_next_sequence_number -- Séquençage auto\n"
    "  • Signature C : static int audit_log_get_next_sequence_number(void);\n"
    "  • Utilité     : Parcourt `AUDIT.TXT` à la recherche du motif `AUDIT NO #XX` pour\n"
    "                  déterminer et retourner le prochain numéro incrémental unique.\n"
    "\n"
    "Fonction 24 : report_audit_log.c -- raptor_audit_log_capture_target -- Capture pré-mortem\n"
    "  • Signature C : bool raptor_audit_log_capture_target(int pid, RaptorIncidentDetail* out_detail);\n"
    "  • Utilité     : Extrait dynamiquement toutes les preuves du processus dans `/proc`\n"
    "                  (binaire, CWD, start time, PPID, commande parent, CWD parent) avant arrêt.\n"
    "\n"
    "Fonction 25 : report_audit_log.c -- raptor_audit_log_commit -- Écriture persistante\n"
    "  • Signature C : bool raptor_audit_log_commit(const RaptorIncidentDetail* detail, const char* action_status);\n"
    "  • Utilité     : Formate et écrit le bloc d'audit horodaté dans `AUDIT.TXT` en mode\n"
    "                  ajout permanent (`a+`) avec forçage d'écriture physique sur disque.\n"
    "\n"
    "Fonction 26 : report_audit_log.c -- raptor_audit_log_notify_detection -- Alerte temps réel\n"
    "  • Signature C : bool raptor_audit_log_notify_detection(int pid, const char* classification);\n"
    "  • Utilité     : Déclenche l'écriture immédiate d'un audit dès qu'une anomalie heuristique\n"
    "                  apparaît, avec filtrage dédoublonné pour éviter la saturation du fichier.\n"
    "\n"
    "Fonction 27 : report_audit_log.c -- raptor_audit_log_forget_pid -- Purge du cache anti-flood\n"
    "  • Signature C : void raptor_audit_log_forget_pid(int pid);\n"
    "  • Utilité     : Supprime un identifiant PID de la table des processus déjà logués\n"
    "                  lors de sa destruction, autorisant la traque immédiate en cas de réapparition.\n"
    },
    {
        4,
        21,
        "Fonction 21 : report_audit_log.c -- audit_log_get_system_btime -- Extraction du boot time",
    "Fonction 21 : report_audit_log.c -- audit_log_get_system_btime -- Extraction du boot time\n"
    "  • Signature C : static unsigned long long audit_log_get_system_btime(void);\n"
    "  • Utilité     : Lit `/proc/stat` et extrait le timestamp Unix `btime` correspondant\n"
    "                  à l'instant de démarrage matériel de la machine hôte.\n"
    },
    {
        4,
        22,
        "Fonction 22 : report_audit_log.c -- audit_log_compute_start_time -- Calcul de spawn",
    "Fonction 22 : report_audit_log.c -- audit_log_compute_start_time -- Calcul de spawn\n"
    "  • Signature C : static void audit_log_compute_start_time(int pid, char* out_buf, size_t buf_sz);\n"
    "  • Utilité     : Parse le 22e champ (`starttime`) de `/proc/[PID]/stat` et convertit\n"
    "                  les ticks processeur en heure locale absolue de démarrage (HH:MM:SS).\n"
    },
    {
        4,
        23,
        "Fonction 23 : report_audit_log.c -- audit_log_get_next_sequence_number -- Séquençage auto",
    "Fonction 23 : report_audit_log.c -- audit_log_get_next_sequence_number -- Séquençage auto\n"
    "  • Signature C : static int audit_log_get_next_sequence_number(void);\n"
    "  • Utilité     : Parcourt `AUDIT.TXT` à la recherche du motif `AUDIT NO #XX` pour\n"
    "                  déterminer et retourner le prochain numéro incrémental unique.\n"
    },
    {
        4,
        24,
        "Fonction 24 : report_audit_log.c -- raptor_audit_log_capture_target -- Capture pré-mortem",
    "Fonction 24 : report_audit_log.c -- raptor_audit_log_capture_target -- Capture pré-mortem\n"
    "  • Signature C : bool raptor_audit_log_capture_target(int pid, RaptorIncidentDetail* out_detail);\n"
    "  • Utilité     : Extrait dynamiquement toutes les preuves du processus dans `/proc`\n"
    "                  (binaire, CWD, start time, PPID, commande parent, CWD parent) avant arrêt.\n"
    },
    {
        4,
        25,
        "Fonction 25 : report_audit_log.c -- raptor_audit_log_commit -- Écriture persistante",
    "Fonction 25 : report_audit_log.c -- raptor_audit_log_commit -- Écriture persistante\n"
    "  • Signature C : bool raptor_audit_log_commit(const RaptorIncidentDetail* detail, const char* action_status);\n"
    "  • Utilité     : Formate et écrit le bloc d'audit horodaté dans `AUDIT.TXT` en mode\n"
    "                  ajout permanent (`a+`) avec forçage d'écriture physique sur disque.\n"
    },
    {
        4,
        26,
        "Fonction 26 : report_audit_log.c -- raptor_audit_log_notify_detection -- Alerte temps réel",
    "Fonction 26 : report_audit_log.c -- raptor_audit_log_notify_detection -- Alerte temps réel\n"
    "  • Signature C : bool raptor_audit_log_notify_detection(int pid, const char* classification);\n"
    "  • Utilité     : Déclenche l'écriture immédiate d'un audit dès qu'une anomalie heuristique\n"
    "                  apparaît, avec filtrage dédoublonné pour éviter la saturation du fichier.\n"
    },
    {
        4,
        27,
        "Fonction 27 : report_audit_log.c -- raptor_audit_log_forget_pid -- Purge du cache anti-flood",
    "Fonction 27 : report_audit_log.c -- raptor_audit_log_forget_pid -- Purge du cache anti-flood\n"
    "  • Signature C : void raptor_audit_log_forget_pid(int pid);\n"
    "  • Utilité     : Supprime un identifiant PID de la table des processus déjà logués\n"
    "                  lors de sa destruction, autorisant la traque immédiate en cas de réapparition.\n"
    },
    {
        5,
        0,
        "SECTION 5 : GREFFON PROCESSUS, MÉMOIRE ET RWX (src/plugins/plugin_memory_rwx.c)",
    "=== SECTION 5 : GREFFON PROCESSUS, MÉMOIRE ET RWX (src/plugins/plugin_memory_rwx.c) ===\n"
    "\n"
    "Fonction 28 : plugin_memory_rwx.c -- proc_mem_is_trusted_path -- Filtrage des chemins sûrs\n"
    "  • Signature C : static bool proc_mem_is_trusted_path(const char* exe_path);\n"
    "  • Utilité     : Valide si le binaire provient strictement des répertoires systèmes\n"
    "                  légitimes (`/usr/bin/`, `/usr/lib/`, `/bin/`, `/sbin/`, `/app/`).\n"
    "\n"
    "Fonction 29 : plugin_memory_rwx.c -- proc_mem_is_legit_jit -- Détection des moteurs JIT\n"
    "  • Signature C : static bool proc_mem_is_legit_jit(const char* comm, const char* exe);\n"
    "  • Utilité     : Écarte les faux positifs en reconnaissant les interpréteurs légitimes\n"
    "                  allouant de la mémoire exécutable (navigateurs, Python, machines virtuelles).\n"
    "\n"
    "Fonction 30 : plugin_memory_rwx.c -- proc_mem_scan_maps_rwx -- Inspection des pages mémoire\n"
    "  • Signature C : static bool proc_mem_scan_maps_rwx(uint32_t pid, bool is_jit, bool is_trusted);\n"
    "  • Utilité     : Analyse `/proc/[PID]/maps` pour repérer les segments illégitimes\n"
    "                  possédant simultanément les droits Read-Write-Execute (`rwx`).\n"
    "\n"
    "Fonction 31 : plugin_memory_rwx.c -- proc_mem_read_stats -- Empreinte RAM du processus\n"
    "  • Signature C : static bool proc_mem_read_stats(uint32_t pid, char* out_comm, size_t comm_len, char* out_exe, size_len, uint64_t* out_rss);\n"
    "  • Utilité     : Lit le nom d'exécutable, résout le lien `/proc/[PID]/exe` et calcule\n"
    "                  la consommation résidente RSS en mémoire physique.\n"
    "\n"
    "Fonction 32 : plugin_memory_rwx.c -- proc_mem_run_audit -- Exécution de l'audit mémoire\n"
    "  • Signature C : static bool proc_mem_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt l'arborescence `/proc`, audite la mémoire de chaque tâche\n"
    "                  active et consigne les résultats dans la section dédiée du rapport.\n"
    "\n"
    "Fonction 33 : plugin_memory_rwx.c -- proc_mem_register -- Enregistrement automatique\n"
    "  • Signature C : __attribute__((constructor)) static void proc_mem_register(void);\n"
    "  • Utilité     : Enregistre le greffon mémoire auprès du registre central dès le\n"
    "                  chargement du binaire en mémoire par le système d'exploitation.\n"
    },
    {
        5,
        28,
        "Fonction 28 : plugin_memory_rwx.c -- proc_mem_is_trusted_path -- Filtrage des chemins sûrs",
    "Fonction 28 : plugin_memory_rwx.c -- proc_mem_is_trusted_path -- Filtrage des chemins sûrs\n"
    "  • Signature C : static bool proc_mem_is_trusted_path(const char* exe_path);\n"
    "  • Utilité     : Valide si le binaire provient strictement des répertoires systèmes\n"
    "                  légitimes (`/usr/bin/`, `/usr/lib/`, `/bin/`, `/sbin/`, `/app/`).\n"
    },
    {
        5,
        29,
        "Fonction 29 : plugin_memory_rwx.c -- proc_mem_is_legit_jit -- Détection des moteurs JIT",
    "Fonction 29 : plugin_memory_rwx.c -- proc_mem_is_legit_jit -- Détection des moteurs JIT\n"
    "  • Signature C : static bool proc_mem_is_legit_jit(const char* comm, const char* exe);\n"
    "  • Utilité     : Écarte les faux positifs en reconnaissant les interpréteurs légitimes\n"
    "                  allouant de la mémoire exécutable (navigateurs, Python, machines virtuelles).\n"
    },
    {
        5,
        30,
        "Fonction 30 : plugin_memory_rwx.c -- proc_mem_scan_maps_rwx -- Inspection des pages mémoire",
    "Fonction 30 : plugin_memory_rwx.c -- proc_mem_scan_maps_rwx -- Inspection des pages mémoire\n"
    "  • Signature C : static bool proc_mem_scan_maps_rwx(uint32_t pid, bool is_jit, bool is_trusted);\n"
    "  • Utilité     : Analyse `/proc/[PID]/maps` pour repérer les segments illégitimes\n"
    "                  possédant simultanément les droits Read-Write-Execute (`rwx`).\n"
    },
    {
        5,
        31,
        "Fonction 31 : plugin_memory_rwx.c -- proc_mem_read_stats -- Empreinte RAM du processus",
    "Fonction 31 : plugin_memory_rwx.c -- proc_mem_read_stats -- Empreinte RAM du processus\n"
    "  • Signature C : static bool proc_mem_read_stats(uint32_t pid, char* out_comm, size_t comm_len, char* out_exe, size_len, uint64_t* out_rss);\n"
    "  • Utilité     : Lit le nom d'exécutable, résout le lien `/proc/[PID]/exe` et calcule\n"
    "                  la consommation résidente RSS en mémoire physique.\n"
    },
    {
        5,
        32,
        "Fonction 32 : plugin_memory_rwx.c -- proc_mem_run_audit -- Exécution de l'audit mémoire",
    "Fonction 32 : plugin_memory_rwx.c -- proc_mem_run_audit -- Exécution de l'audit mémoire\n"
    "  • Signature C : static bool proc_mem_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt l'arborescence `/proc`, audite la mémoire de chaque tâche\n"
    "                  active et consigne les résultats dans la section dédiée du rapport.\n"
    },
    {
        5,
        33,
        "Fonction 33 : plugin_memory_rwx.c -- proc_mem_register -- Enregistrement automatique",
    "Fonction 33 : plugin_memory_rwx.c -- proc_mem_register -- Enregistrement automatique\n"
    "  • Signature C : __attribute__((constructor)) static void proc_mem_register(void);\n"
    "  • Utilité     : Enregistre le greffon mémoire auprès du registre central dès le\n"
    "                  chargement du binaire en mémoire par le système d'exploitation.\n"
    },
    {
        6,
        0,
        "SECTION 6 : GREFFON SURVEILLANCE RÉSEAU (src/plugins/plugin_network_sockets.c)",
    "=== SECTION 6 : GREFFON SURVEILLANCE RÉSEAU (src/plugins/plugin_network_sockets.c) ===\n"
    "\n"
    "Fonction 34 : plugin_network_sockets.c -- net_sock_build_inode_cache -- Résolution Inode/PID\n"
    "  • Signature C : static bool net_sock_build_inode_cache(void);\n"
    "  • Utilité     : Analyse tous les descripteurs ouverts dans `/proc/[PID]/fd` pour\n"
    "                  construire la table de correspondance rapide entre numéros d'inodes et PID.\n"
    "\n"
    "Fonction 35 : plugin_network_sockets.c -- net_sock_resolve_pid -- Recherche propriétaire O(1)\n"
    "  • Signature C : static uint32_t net_sock_resolve_pid(uint64_t target_inode);\n"
    "  • Utilité     : Retourne instantanément l'identifiant PID associé à un socket réseau\n"
    "                  à partir de son numéro d'inode noyau.\n"
    "\n"
    "Fonction 36 : plugin_network_sockets.c -- net_sock_decode_hex_endpoint -- Décodage IP/Port\n"
    "  • Signature C : static void net_sock_decode_hex_endpoint(const char* hex_ip, uint32_t hex_port, char* out_ip, size_t ip_len, uint16_t* out_port);\n"
    "  • Utilité     : Convertit la représentation hexadécimale brute (format endianess noyau)\n"
    "                  en adresse IP décimale lisible et port réseau standard.\n"
    "\n"
    "Fonction 37 : plugin_network_sockets.c -- net_sock_parse_file -- Analyse des tables réseau\n"
    "  • Signature C : static void net_sock_parse_file(const char* net_path, const char* proto, RaptorTable* out_table);\n"
    "  • Utilité     : Parse `/proc/net/tcp` et `/proc/net/udp` pour répertorier les ports en\n"
    "                  écoute (`LISTEN`) et les canaux établis (`ESTABLISHED`).\n"
    "\n"
    "Fonction 38 : plugin_network_sockets.c -- net_sock_run_audit -- Exécution de l'audit réseau\n"
    "  • Signature C : static bool net_sock_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Orchestre la collecte des flux réseau, résout les processus propriétaires\n"
    "                  et libère le cache des descripteurs après analyse.\n"
    "\n"
    "Fonction 39 : plugin_network_sockets.c -- net_sock_register -- Enregistrement automatique\n"
    "  • Signature C : __attribute__((constructor)) static void net_sock_register(void);\n"
    "  • Utilité     : Inscrit le greffon réseau dans le registre central dès l'initialisation\n"
    "                  du binaire exécutable.\n"
    },
    {
        6,
        34,
        "Fonction 34 : plugin_network_sockets.c -- net_sock_build_inode_cache -- Résolution Inode/PID",
    "Fonction 34 : plugin_network_sockets.c -- net_sock_build_inode_cache -- Résolution Inode/PID\n"
    "  • Signature C : static bool net_sock_build_inode_cache(void);\n"
    "  • Utilité     : Analyse tous les descripteurs ouverts dans `/proc/[PID]/fd` pour\n"
    "                  construire la table de correspondance rapide entre numéros d'inodes et PID.\n"
    },
    {
        6,
        35,
        "Fonction 35 : plugin_network_sockets.c -- net_sock_resolve_pid -- Recherche propriétaire O(1)",
    "Fonction 35 : plugin_network_sockets.c -- net_sock_resolve_pid -- Recherche propriétaire O(1)\n"
    "  • Signature C : static uint32_t net_sock_resolve_pid(uint64_t target_inode);\n"
    "  • Utilité     : Retourne instantanément l'identifiant PID associé à un socket réseau\n"
    "                  à partir de son numéro d'inode noyau.\n"
    },
    {
        6,
        36,
        "Fonction 36 : plugin_network_sockets.c -- net_sock_decode_hex_endpoint -- Décodage IP/Port",
    "Fonction 36 : plugin_network_sockets.c -- net_sock_decode_hex_endpoint -- Décodage IP/Port\n"
    "  • Signature C : static void net_sock_decode_hex_endpoint(const char* hex_ip, uint32_t hex_port, char* out_ip, size_t ip_len, uint16_t* out_port);\n"
    "  • Utilité     : Convertit la représentation hexadécimale brute (format endianess noyau)\n"
    "                  en adresse IP décimale lisible et port réseau standard.\n"
    },
    {
        6,
        37,
        "Fonction 37 : plugin_network_sockets.c -- net_sock_parse_file -- Analyse des tables réseau",
    "Fonction 37 : plugin_network_sockets.c -- net_sock_parse_file -- Analyse des tables réseau\n"
    "  • Signature C : static void net_sock_parse_file(const char* net_path, const char* proto, RaptorTable* out_table);\n"
    "  • Utilité     : Parse `/proc/net/tcp` et `/proc/net/udp` pour répertorier les ports en\n"
    "                  écoute (`LISTEN`) et les canaux établis (`ESTABLISHED`).\n"
    },
    {
        6,
        38,
        "Fonction 38 : plugin_network_sockets.c -- net_sock_run_audit -- Exécution de l'audit réseau",
    "Fonction 38 : plugin_network_sockets.c -- net_sock_run_audit -- Exécution de l'audit réseau\n"
    "  • Signature C : static bool net_sock_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Orchestre la collecte des flux réseau, résout les processus propriétaires\n"
    "                  et libère le cache des descripteurs après analyse.\n"
    },
    {
        6,
        39,
        "Fonction 39 : plugin_network_sockets.c -- net_sock_register -- Enregistrement automatique",
    "Fonction 39 : plugin_network_sockets.c -- net_sock_register -- Enregistrement automatique\n"
    "  • Signature C : __attribute__((constructor)) static void net_sock_register(void);\n"
    "  • Utilité     : Inscrit le greffon réseau dans le registre central dès l'initialisation\n"
    "                  du binaire exécutable.\n"
    },
    {
        7,
        0,
        "SECTION 7 : GREFFON HEURISTIQUE & LIGNÉE PARENT (src/plugins/plugin_heuristics_expert.c)",
    "=== SECTION 7 : GREFFON HEURISTIQUE & LIGNÉE PARENT (src/plugins/plugin_heuristics_expert.c) ===\n"
    "\n"
    "Fonction 40 : plugin_heuristics_expert.c -- heuristics_is_suspicious_path -- Détection chemins volatils\n"
    "  • Signature C : static bool heuristics_is_suspicious_path(const char* path);\n"
    "  • Utilité     : Identifie si un processus s'exécute depuis des zones temporaires ou\n"
    "                  volatiles suspectes (`/tmp`, `/var/tmp`, `/dev/shm`, `/run/user`).\n"
    "\n"
    "Fonction 41 : plugin_heuristics_expert.c -- heuristics_is_system_masquerade -- Détection d'usurpation\n"
    "  • Signature C : static bool heuristics_is_system_masquerade(const char* comm, const char* exe_target);\n"
    "  • Utilité     : Repère les processus non officiels usurpant le nom de démons critiques\n"
    "                  (`systemd`, `sshd`, `cron`, `kworker`) hors des dossiers systèmes.\n"
    "\n"
    "Fonction 42 : plugin_heuristics_expert.c -- heuristics_get_parent_info -- Traque généalogique (PPID)\n"
    "  • Signature C : static void heuristics_get_parent_info(const char* pid_str, char* out_ppid, size_t ppid_sz, char* out_pcomm, size_t pcomm_sz, char* out_pcmd, size_t pcmd_sz);\n"
    "  • Utilité     : Extrait le PID parent via `/proc/[PID]/stat` et reconstitue la ligne\n"
    "                  de commande exacte et l'exécutable créateur depuis `/proc/[PPID]/cmdline`.\n"
    "\n"
    "Fonction 43 : plugin_heuristics_expert.c -- heuristics_check_process -- Évaluation unitaire\n"
    "  • Signature C : static void heuristics_check_process(const char* pid_str, size_t* out_masquerades, size_t* out_deleted, size_t* out_suspicious_paths, RaptorTable* tbl);\n"
    "  • Utilité     : Exécute l'ensemble des filtres heuristiques sur un processus, remplit la table\n"
    "                  d'incident et déclenche la notification persistante dans `AUDIT.TXT`.\n"
    "\n"
    "Fonction 44 : plugin_heuristics_expert.c -- plugin_heuristics_init -- Initialisation greffon\n"
    "  • Signature C : static bool plugin_heuristics_init(void);\n"
    "  • Utilité     : Prépare les états internes et structures du greffon heuristique.\n"
    "\n"
    "Fonction 45 : plugin_heuristics_expert.c -- plugin_heuristics_run_audit -- Exécution globale heuristique\n"
    "  • Signature C : static bool plugin_heuristics_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt `/proc`, identifie les menaces comportementales, compile les cartes\n"
    "                  d'usurpation et qualifie la criticité défensive globale.\n"
    "\n"
    "Fonction 46 : plugin_heuristics_expert.c -- plugin_heuristics_cleanup -- Nettoyage greffon\n"
    "  • Signature C : static void plugin_heuristics_cleanup(void);\n"
    "  • Utilité     : Libère les structures locales allouées lors de l'exécution de l'audit.\n"
    "\n"
    "Fonction 47 : plugin_heuristics_expert.c -- plugin_heuristics_register_constructor -- Auto-enregistrement\n"
    "  • Signature C : __attribute__((constructor)) static void plugin_heuristics_register_constructor(void);\n"
    "  • Utilité     : Enregistre dynamiquement le greffon heuristique au chargement du programme.\n"
    },
    {
        7,
        40,
        "Fonction 40 : plugin_heuristics_expert.c -- heuristics_is_suspicious_path -- Détection chemins volatils",
    "Fonction 40 : plugin_heuristics_expert.c -- heuristics_is_suspicious_path -- Détection chemins volatils\n"
    "  • Signature C : static bool heuristics_is_suspicious_path(const char* path);\n"
    "  • Utilité     : Identifie si un processus s'exécute depuis des zones temporaires ou\n"
    "                  volatiles suspectes (`/tmp`, `/var/tmp`, `/dev/shm`, `/run/user`).\n"
    },
    {
        7,
        41,
        "Fonction 41 : plugin_heuristics_expert.c -- heuristics_is_system_masquerade -- Détection d'usurpation",
    "Fonction 41 : plugin_heuristics_expert.c -- heuristics_is_system_masquerade -- Détection d'usurpation\n"
    "  • Signature C : static bool heuristics_is_system_masquerade(const char* comm, const char* exe_target);\n"
    "  • Utilité     : Repère les processus non officiels usurpant le nom de démons critiques\n"
    "                  (`systemd`, `sshd`, `cron`, `kworker`) hors des dossiers systèmes.\n"
    },
    {
        7,
        42,
        "Fonction 42 : plugin_heuristics_expert.c -- heuristics_get_parent_info -- Traque généalogique (PPID)",
    "Fonction 42 : plugin_heuristics_expert.c -- heuristics_get_parent_info -- Traque généalogique (PPID)\n"
    "  • Signature C : static void heuristics_get_parent_info(const char* pid_str, char* out_ppid, size_t ppid_sz, char* out_pcomm, size_t pcomm_sz, char* out_pcmd, size_t pcmd_sz);\n"
    "  • Utilité     : Extrait le PID parent via `/proc/[PID]/stat` et reconstitue la ligne\n"
    "                  de commande exacte et l'exécutable créateur depuis `/proc/[PPID]/cmdline`.\n"
    },
    {
        7,
        43,
        "Fonction 43 : plugin_heuristics_expert.c -- heuristics_check_process -- Évaluation unitaire",
    "Fonction 43 : plugin_heuristics_expert.c -- heuristics_check_process -- Évaluation unitaire\n"
    "  • Signature C : static void heuristics_check_process(const char* pid_str, size_t* out_masquerades, size_t* out_deleted, size_t* out_suspicious_paths, RaptorTable* tbl);\n"
    "  • Utilité     : Exécute l'ensemble des filtres heuristiques sur un processus, remplit la table\n"
    "                  d'incident et déclenche la notification persistante dans `AUDIT.TXT`.\n"
    },
    {
        7,
        44,
        "Fonction 44 : plugin_heuristics_expert.c -- plugin_heuristics_init -- Initialisation greffon",
    "Fonction 44 : plugin_heuristics_expert.c -- plugin_heuristics_init -- Initialisation greffon\n"
    "  • Signature C : static bool plugin_heuristics_init(void);\n"
    "  • Utilité     : Prépare les états internes et structures du greffon heuristique.\n"
    },
    {
        7,
        45,
        "Fonction 45 : plugin_heuristics_expert.c -- plugin_heuristics_run_audit -- Exécution globale heuristique",
    "Fonction 45 : plugin_heuristics_expert.c -- plugin_heuristics_run_audit -- Exécution globale heuristique\n"
    "  • Signature C : static bool plugin_heuristics_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Parcourt `/proc`, identifie les menaces comportementales, compile les cartes\n"
    "                  d'usurpation et qualifie la criticité défensive globale.\n"
    },
    {
        7,
        46,
        "Fonction 46 : plugin_heuristics_expert.c -- plugin_heuristics_cleanup -- Nettoyage greffon",
    "Fonction 46 : plugin_heuristics_expert.c -- plugin_heuristics_cleanup -- Nettoyage greffon\n"
    "  • Signature C : static void plugin_heuristics_cleanup(void);\n"
    "  • Utilité     : Libère les structures locales allouées lors de l'exécution de l'audit.\n"
    },
    {
        7,
        47,
        "Fonction 47 : plugin_heuristics_expert.c -- plugin_heuristics_register_constructor -- Auto-enregistrement",
    "Fonction 47 : plugin_heuristics_expert.c -- plugin_heuristics_register_constructor -- Auto-enregistrement\n"
    "  • Signature C : __attribute__((constructor)) static void plugin_heuristics_register_constructor(void);\n"
    "  • Utilité     : Enregistre dynamiquement le greffon heuristique au chargement du programme.\n"
    },
    {
        8,
        0,
        "SECTION 8 : MOTEURS DE RESTITUTION UNIVERSELS (src/report/)",
    "=== SECTION 8 : MOTEURS DE RESTITUTION UNIVERSELS (src/report/) ===\n"
    "\n"
    "Fonction 48 : report_cli.c -- raptor_report_cli_print -- Sortie console formatée\n"
    "  • Signature C : void raptor_report_cli_print(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Rendu textuel intégral du rapport dans le terminal avec codes\n"
    "                  ANSI pour les couleurs de sévérité et colonnage structuré.\n"
    "\n"
    "Fonction 49 : report_html.c -- raptor_report_html_generate -- Export de rapport web\n"
    "  • Signature C : bool raptor_report_html_generate(const RaptorReportContext* ctx, const char* filepath);\n"
    "  • Utilité     : Génère un rapport d'audit au format HTML souverain autonome avec\n"
    "                  thème sombre, visualisations interactives et tableaux filtrables.\n"
    },
    {
        8,
        48,
        "Fonction 48 : report_cli.c -- raptor_report_cli_print -- Sortie console formatée",
    "Fonction 48 : report_cli.c -- raptor_report_cli_print -- Sortie console formatée\n"
    "  • Signature C : void raptor_report_cli_print(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Rendu textuel intégral du rapport dans le terminal avec codes\n"
    "                  ANSI pour les couleurs de sévérité et colonnage structuré.\n"
    },
    {
        8,
        49,
        "Fonction 49 : report_html.c -- raptor_report_html_generate -- Export de rapport web",
    "Fonction 49 : report_html.c -- raptor_report_html_generate -- Export de rapport web\n"
    "  • Signature C : bool raptor_report_html_generate(const RaptorReportContext* ctx, const char* filepath);\n"
    "  • Utilité     : Génère un rapport d'audit au format HTML souverain autonome avec\n"
    "                  thème sombre, visualisations interactives et tableaux filtrables.\n"
    },
    {
        9,
        0,
        "SECTION 9 : MOTEUR GRAPHIQUE ET CONTRÔLEURS DE VUES (src/ui/)",
    "=== SECTION 9 : MOTEUR GRAPHIQUE ET CONTRÔLEURS DE VUES (src/ui/) ===\n"
    "\n"
    "Fonction 50 : ui_engine.c -- raptor_ui_init -- Initialisation graphique Raylib\n"
    "  • Signature C : bool raptor_ui_init(int width, int height, const char* title);\n"
    "  • Utilité     : Configure la fenêtre Raylib, applique les dimensions, le titre,\n"
    "                  la synchronisation verticale (VSync) et la cible de 60 FPS.\n"
    "\n"
    "Fonction 51 : ui_engine.c -- raptor_ui_run -- Boucle de surveillance active\n"
    "  • Signature C : void raptor_ui_run(RaptorReportContext* ctx);\n"
    "  • Utilité     : Pilote la boucle de rendu principale, orchestre l'affichage des onglets\n"
    "                  et intègre le timer cyclique de 3s pour le rafraîchissement continu.\n"
    "\n"
    "Fonction 52 : ui_engine.c -- raptor_ui_shutdown -- Libération graphique\n"
    "  • Signature C : void raptor_ui_shutdown(void);\n"
    "  • Utilité     : Décharge les textures vectorielles (icônes) et détruit proprement\n"
    "                  la fenêtre graphique Raylib à la fermeture.\n"
    "\n"
    "Fonction 53 : view_ai.c -- view_ai_detect_os -- Identification du système hôte\n"
    "  • Signature C : static void view_ai_detect_os(char* out_name, size_t max_len);\n"
    "  • Utilité     : Interroge `/etc/os-release` pour afficher le nom exact de la distribution\n"
    "                  (ex: Linux Mint Debian Edition) dans le bandeau du rapport IA.\n"
    "\n"
    "Fonction 54 : view_ai.c -- view_ai_draw_pid_badge -- Affichage badges d'alerte PID\n"
    "  • Signature C : static float view_ai_draw_pid_badge(const char* role_title, const char* pid_val, float posX, float posY);\n"
    "  • Utilité     : Dessine un conteneur visuel haute visibilité (bordure jaune vif 2 px,\n"
    "                  fond sombre, texte blanc) mettant en évidence les identifiants cibles et parents.\n"
    "\n"
    "Fonction 55 : view_ai.c -- ui_view_ai_draw -- Contrôleur du rapport IA [ 05 ]\n"
    "  • Signature C : void ui_view_ai_draw(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Consolide l'audit explicatif souverain, affiche le modèle Z-Score, trace\n"
    "                  la lignée du spawn et intègre le garde-fou strict interdisant le kill du PID 1.\n"
    "\n"
    "Fonction 56 : view_processes.c -- ui_view_processes_draw -- Contrôleur Processus & RAM [ 02 ]\n"
    "  • Signature C : void ui_view_processes_draw(RaptorReportContext* ctx);\n"
    "  • Utilité     : Affiche la table filtrable des processus, gère la sélection interactive,\n"
    "                  exécute la capture forensique, ordonne le signal de kill et consigne l'événement.\n"
    },
    {
        9,
        50,
        "Fonction 50 : ui_engine.c -- raptor_ui_init -- Initialisation graphique Raylib",
    "Fonction 50 : ui_engine.c -- raptor_ui_init -- Initialisation graphique Raylib\n"
    "  • Signature C : bool raptor_ui_init(int width, int height, const char* title);\n"
    "  • Utilité     : Configure la fenêtre Raylib, applique les dimensions, le titre,\n"
    "                  la synchronisation verticale (VSync) et la cible de 60 FPS.\n"
    },
    {
        9,
        51,
        "Fonction 51 : ui_engine.c -- raptor_ui_run -- Boucle de surveillance active",
    "Fonction 51 : ui_engine.c -- raptor_ui_run -- Boucle de surveillance active\n"
    "  • Signature C : void raptor_ui_run(RaptorReportContext* ctx);\n"
    "  • Utilité     : Pilote la boucle de rendu principale, orchestre l'affichage des onglets\n"
    "                  et intègre le timer cyclique de 3s pour le rafraîchissement continu.\n"
    },
    {
        9,
        52,
        "Fonction 52 : ui_engine.c -- raptor_ui_shutdown -- Libération graphique",
    "Fonction 52 : ui_engine.c -- raptor_ui_shutdown -- Libération graphique\n"
    "  • Signature C : void raptor_ui_shutdown(void);\n"
    "  • Utilité     : Décharge les textures vectorielles (icônes) et détruit proprement\n"
    "                  la fenêtre graphique Raylib à la fermeture.\n"
    },
    {
        9,
        53,
        "Fonction 53 : view_ai.c -- view_ai_detect_os -- Identification du système hôte",
    "Fonction 53 : view_ai.c -- view_ai_detect_os -- Identification du système hôte\n"
    "  • Signature C : static void view_ai_detect_os(char* out_name, size_t max_len);\n"
    "  • Utilité     : Interroge `/etc/os-release` pour afficher le nom exact de la distribution\n"
    "                  (ex: Linux Mint Debian Edition) dans le bandeau du rapport IA.\n"
    },
    {
        9,
        54,
        "Fonction 54 : view_ai.c -- view_ai_draw_pid_badge -- Affichage badges d'alerte PID",
    "Fonction 54 : view_ai.c -- view_ai_draw_pid_badge -- Affichage badges d'alerte PID\n"
    "  • Signature C : static float view_ai_draw_pid_badge(const char* role_title, const char* pid_val, float posX, float posY);\n"
    "  • Utilité     : Dessine un conteneur visuel haute visibilité (bordure jaune vif 2 px,\n"
    "                  fond sombre, texte blanc) mettant en évidence les identifiants cibles et parents.\n"
    },
    {
        9,
        55,
        "Fonction 55 : view_ai.c -- ui_view_ai_draw -- Contrôleur du rapport IA [ 05 ]",
    "Fonction 55 : view_ai.c -- ui_view_ai_draw -- Contrôleur du rapport IA [ 05 ]\n"
    "  • Signature C : void ui_view_ai_draw(const RaptorReportContext* ctx);\n"
    "  • Utilité     : Consolide l'audit explicatif souverain, affiche le modèle Z-Score, trace\n"
    "                  la lignée du spawn et intègre le garde-fou strict interdisant le kill du PID 1.\n"
    },
    {
        9,
        56,
        "Fonction 56 : view_processes.c -- ui_view_processes_draw -- Contrôleur Processus & RAM [ 02 ]",
    "Fonction 56 : view_processes.c -- ui_view_processes_draw -- Contrôleur Processus & RAM [ 02 ]\n"
    "  • Signature C : void ui_view_processes_draw(RaptorReportContext* ctx);\n"
    "  • Utilité     : Affiche la table filtrable des processus, gère la sélection interactive,\n"
    "                  exécute la capture forensique, ordonne le signal de kill et consigne l'événement.\n"
    },
    {
        10,
        0,
        "SECTION 10 : POINT D'ENTRÉE DU SYSTÈME (src/main.c)",
    "=== SECTION 10 : POINT D'ENTRÉE DU SYSTÈME (src/main.c) ===\n"
    "\n"
    "Fonction 57 : main.c -- main -- Orchestrateur d'amorçage\n"
    "  • Signature C : int main(int argc, char** argv);\n"
    "  • Utilité     : Parse les arguments de ligne de commande (`--cli` ou interface GUI),\n"
    "                  initialise le registre, lance le premier audit et passe le relais à l'UI.\n"
    },
    {
        10,
        57,
        "Fonction 57 : main.c -- main -- Orchestrateur d'amorçage",
    "Fonction 57 : main.c -- main -- Orchestrateur d'amorçage\n"
    "  • Signature C : int main(int argc, char** argv);\n"
    "  • Utilité     : Parse les arguments de ligne de commande (`--cli` ou interface GUI),\n"
    "                  initialise le registre, lance le premier audit et passe le relais à l'UI.\n"
    },
    {
        11,
        0,
        "SECTION 11 : GREFFON RAPTOR NET-GUARD DÉFENSIF (src/plugins/plugin_net_guard.c)",
    "=== SECTION 11 : GREFFON RAPTOR NET-GUARD DÉFENSIF (src/plugins/plugin_net_guard.c) ===\n"
    "\n"
    "Fonction 58 : plugin_net_guard.c -- net_guard_trim -- Assainissement de chaîne\n"
    "  • Signature C : char* net_guard_trim(char* str);\n"
    "  • Utilité     : Élimine les espaces blancs et sauts de ligne superflus en début\n"
    "                  et fin de chaîne pour fiabiliser le parsing des fichiers de règles.\n"
    "\n"
    "Fonction 59 : plugin_net_guard.c -- net_guard_validate_ip -- Validation arithmétique IP & CIDR\n"
    "  • Signature C : bool net_guard_validate_ip(const char* ip_str, uint32_t* out_ip, uint32_t* out_mask, uint16_t* out_port);\n"
    "  • Utilité     : Contrôle rigoureusement les plages IPv4 (0-255), extrait le port\n"
    "                  éventuel (1-65535) et calcule le masque binaire réseau à partir de la notation CIDR (/1 à /32).\n"
    "\n"
    "Fonction 60 : plugin_net_guard.c -- net_guard_load_file -- Parsing tolérant de registre .conf\n"
    "  • Signature C : bool net_guard_load_file(const char* filepath, bool is_custom_source);\n"
    "  • Utilité     : Lit ligne par ligne les fichiers de configuration, ignore les commentaires,\n"
    "                  segmente les champs par séparateur pipe (|) et peuple les tables mémoires DOM et IP.\n"
    "\n"
    "Fonction 61 : plugin_net_guard.c -- net_guard_init -- Initialisation et fusion en RAM\n"
    "  • Signature C : bool net_guard_init(void);\n"
    "  • Utilité     : Réinitialise les tables du registre et charge successivement le socle\n"
    "                  officiel (fw_officiel.conf) puis les ajouts locaux (fw_custom.conf) sans conflit.\n"
    "\n"
    "Fonction 62 : plugin_net_guard.c -- net_guard_add_custom_rule -- Sérialisation et persistance IHM\n"
    "  • Signature C : bool net_guard_add_custom_rule(NetGuardRuleType type, const char* target, const char* param, const char* desc);\n"
    "  • Utilité     : Formate avec espacements canoniques une nouvelle règle saisie dans l'IHM,\n"
    "                  l'écrit en mode ajout permanent dans fw_custom.conf et actualise la table active.\n"
    "\n"
    "Fonction 63 : plugin_net_guard.c -- net_guard_remove_rule -- Suppression atomique de règle\n"
    "  • Signature C : bool net_guard_remove_rule(NetGuardRuleType type, size_t index);\n"
    "  • Utilité     : Supprime une règle personnalisée selon son index mémoire et réécrit\n"
    "                  proprement l'intégralité du fichier fw_custom.conf sans toucher au registre officiel.\n"
    "\n"
    "Fonction 64 : plugin_net_guard.c -- net_guard_match_domain -- Détection anti-combosquatting\n"
    "  • Signature C : NetGuardVerdict net_guard_match_domain(const char* hostname, char* out_reason, size_t reason_sz);\n"
    "  • Utilité     : Analyse le nom de domaine demandé, vérifie la présence d'une marque protégée,\n"
    "                  valide les domaines officiels autorisés et lève l'alerte CRITIQUE si un mot-clé suspect y est accolé.\n"
    "\n"
    "Fonction 65 : plugin_net_guard.c -- net_guard_match_ip -- Qualification binaire des endpoints IP\n"
    "  • Signature C : NetGuardVerdict net_guard_match_ip(const char* remote_ip, uint16_t port, char* out_reason, size_t reason_sz);\n"
    "  • Utilité     : Applique le masque de sous-réseau sur l'adresse distante pour déterminer\n"
    "                  instantanément si le socket pointe vers une cible APPROUVÉ (vert) ou HOSTILE (rouge critique).\n"
    "\n"
    "Fonction 66 : plugin_net_guard.c -- net_guard_run_audit -- Moteur d'audit périodique réseau\n"
    "  • Signature C : bool net_guard_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Orchestre la lecture de /proc/net/tcp et /proc/net/udp, confronte chaque\n"
    "                  connexion active aux tables de règles et peuple la section générique du rapport de sécurité.\n"
    "\n"
    "Fonction 67 : plugin_net_guard.c -- net_guard_cleanup -- Déchargement et purge mémoire\n"
    "  • Signature C : void net_guard_cleanup(void);\n"
    "  • Utilité     : Remet à zéro l'ensemble des structures de données du registre Net-Guard\n"
    "                  lors de la fermeture propre de l'application.\n"
    "\n"
    "Fonction 68 : plugin_net_guard.c -- net_guard_register -- Auto-enregistrement constructeur GCC\n"
    "  • Signature C : __attribute__((constructor)) void net_guard_register(void);\n"
    "  • Utilité     : Enregistre automatiquement la structure RaptorPlugin de Net-Guard auprès\n"
    "                  du bus central raptor_registry dès le chargement du binaire en mémoire.\n"
    "\n"
    "Fonction 69 : plugin_net_guard.c -- net_guard_update_official_rules -- Rapatriement atomique GitHub\n"
    "  • Signature C : bool net_guard_update_official_rules(void);\n"
    "  • Utilité     : Télécharge la dernière mouture de fw_officiel.conf via curl vers un fichier\n"
    "                  temporaire (.tmp), valide l'en-tête de conformité et applique un remplacement atomique POSIX (rename).\n"
    },
    {
        11,
        58,
        "Fonction 58 : plugin_net_guard.c -- net_guard_trim -- Assainissement de chaîne",
    "Fonction 58 : plugin_net_guard.c -- net_guard_trim -- Assainissement de chaîne\n"
    "  • Signature C : char* net_guard_trim(char* str);\n"
    "  • Utilité     : Élimine les espaces blancs et sauts de ligne superflus en début\n"
    "                  et fin de chaîne pour fiabiliser le parsing des fichiers de règles.\n"
    },
    {
        11,
        59,
        "Fonction 59 : plugin_net_guard.c -- net_guard_validate_ip -- Validation arithmétique IP & CIDR",
    "Fonction 59 : plugin_net_guard.c -- net_guard_validate_ip -- Validation arithmétique IP & CIDR\n"
    "  • Signature C : bool net_guard_validate_ip(const char* ip_str, uint32_t* out_ip, uint32_t* out_mask, uint16_t* out_port);\n"
    "  • Utilité     : Contrôle rigoureusement les plages IPv4 (0-255), extrait le port\n"
    "                  éventuel (1-65535) et calcule le masque binaire réseau à partir de la notation CIDR (/1 à /32).\n"
    },
    {
        11,
        60,
        "Fonction 60 : plugin_net_guard.c -- net_guard_load_file -- Parsing tolérant de registre .conf",
    "Fonction 60 : plugin_net_guard.c -- net_guard_load_file -- Parsing tolérant de registre .conf\n"
    "  • Signature C : bool net_guard_load_file(const char* filepath, bool is_custom_source);\n"
    "  • Utilité     : Lit ligne par ligne les fichiers de configuration, ignore les commentaires,\n"
    "                  segmente les champs par séparateur pipe (|) et peuple les tables mémoires DOM et IP.\n"
    },
    {
        11,
        61,
        "Fonction 61 : plugin_net_guard.c -- net_guard_init -- Initialisation et fusion en RAM",
    "Fonction 61 : plugin_net_guard.c -- net_guard_init -- Initialisation et fusion en RAM\n"
    "  • Signature C : bool net_guard_init(void);\n"
    "  • Utilité     : Réinitialise les tables du registre et charge successivement le socle\n"
    "                  officiel (fw_officiel.conf) puis les ajouts locaux (fw_custom.conf) sans conflit.\n"
    },
    {
        11,
        62,
        "Fonction 62 : plugin_net_guard.c -- net_guard_add_custom_rule -- Sérialisation et persistance IHM",
    "Fonction 62 : plugin_net_guard.c -- net_guard_add_custom_rule -- Sérialisation et persistance IHM\n"
    "  • Signature C : bool net_guard_add_custom_rule(NetGuardRuleType type, const char* target, const char* param, const char* desc);\n"
    "  • Utilité     : Formate avec espacements canoniques une nouvelle règle saisie dans l'IHM,\n"
    "                  l'écrit en mode ajout permanent dans fw_custom.conf et actualise la table active.\n"
    },
    {
        11,
        63,
        "Fonction 63 : plugin_net_guard.c -- net_guard_remove_rule -- Suppression atomique de règle",
    "Fonction 63 : plugin_net_guard.c -- net_guard_remove_rule -- Suppression atomique de règle\n"
    "  • Signature C : bool net_guard_remove_rule(NetGuardRuleType type, size_t index);\n"
    "  • Utilité     : Supprime une règle personnalisée selon son index mémoire et réécrit\n"
    "                  proprement l'intégralité du fichier fw_custom.conf sans toucher au registre officiel.\n"
    },
    {
        11,
        64,
        "Fonction 64 : plugin_net_guard.c -- net_guard_match_domain -- Détection anti-combosquatting",
    "Fonction 64 : plugin_net_guard.c -- net_guard_match_domain -- Détection anti-combosquatting\n"
    "  • Signature C : NetGuardVerdict net_guard_match_domain(const char* hostname, char* out_reason, size_t reason_sz);\n"
    "  • Utilité     : Analyse le nom de domaine demandé, vérifie la présence d'une marque protégée,\n"
    "                  valide les domaines officiels autorisés et lève l'alerte CRITIQUE si un mot-clé suspect y est accolé.\n"
    },
    {
        11,
        65,
        "Fonction 65 : plugin_net_guard.c -- net_guard_match_ip -- Qualification binaire des endpoints IP",
    "Fonction 65 : plugin_net_guard.c -- net_guard_match_ip -- Qualification binaire des endpoints IP\n"
    "  • Signature C : NetGuardVerdict net_guard_match_ip(const char* remote_ip, uint16_t port, char* out_reason, size_t reason_sz);\n"
    "  • Utilité     : Applique le masque de sous-réseau sur l'adresse distante pour déterminer\n"
    "                  instantanément si le socket pointe vers une cible APPROUVÉ (vert) ou HOSTILE (rouge critique).\n"
    },
    {
        11,
        66,
        "Fonction 66 : plugin_net_guard.c -- net_guard_run_audit -- Moteur d'audit périodique réseau",
    "Fonction 66 : plugin_net_guard.c -- net_guard_run_audit -- Moteur d'audit périodique réseau\n"
    "  • Signature C : bool net_guard_run_audit(RaptorReportContext* ctx);\n"
    "  • Utilité     : Orchestre la lecture de /proc/net/tcp et /proc/net/udp, confronte chaque\n"
    "                  connexion active aux tables de règles et peuple la section générique du rapport de sécurité.\n"
    },
    {
        11,
        67,
        "Fonction 67 : plugin_net_guard.c -- net_guard_cleanup -- Déchargement et purge mémoire",
    "Fonction 67 : plugin_net_guard.c -- net_guard_cleanup -- Déchargement et purge mémoire\n"
    "  • Signature C : void net_guard_cleanup(void);\n"
    "  • Utilité     : Remet à zéro l'ensemble des structures de données du registre Net-Guard\n"
    "                  lors de la fermeture propre de l'application.\n"
    },
    {
        11,
        68,
        "Fonction 68 : plugin_net_guard.c -- net_guard_register -- Auto-enregistrement constructeur GCC",
    "Fonction 68 : plugin_net_guard.c -- net_guard_register -- Auto-enregistrement constructeur GCC\n"
    "  • Signature C : __attribute__((constructor)) void net_guard_register(void);\n"
    "  • Utilité     : Enregistre automatiquement la structure RaptorPlugin de Net-Guard auprès\n"
    "                  du bus central raptor_registry dès le chargement du binaire en mémoire.\n"
    },
    {
        11,
        69,
        "Fonction 69 : plugin_net_guard.c -- net_guard_update_official_rules -- Rapatriement atomique GitHub",
    "Fonction 69 : plugin_net_guard.c -- net_guard_update_official_rules -- Rapatriement atomique GitHub\n"
    "  • Signature C : bool net_guard_update_official_rules(void);\n"
    "  • Utilité     : Télécharge la dernière mouture de fw_officiel.conf via curl vers un fichier\n"
    "                  temporaire (.tmp), valide l'en-tête de conformité et applique un remplacement atomique POSIX (rename).\n"
    },
    {
        12,
        0,
        "SECTION 12 : RENSEIGNEMENT RÉSEAU, BGP & CDN INTEL (src/plugins/plugin_asn_intel.c)",
    "=== SECTION 12 : RENSEIGNEMENT RÉSEAU, BGP & CDN INTEL (src/plugins/plugin_asn_intel.c) ===\n"
    "\n"
    "Fonction 71 : plugin_asn_intel.c -- asn_intel_trim -- Nettoyage canonique\n"
    "  • Signature C : static char* asn_intel_trim(char* str);\n"
    "  • Utilité     : Nettoyage canonique des espaces blancs en tête et queue de chaîne.\n"
    "\n"
    "Fonction 72 : plugin_asn_intel.c -- asn_intel_parse_cidr -- Décomposition CIDR\n"
    "  • Signature C : static bool asn_intel_parse_cidr(const char* cidr, uint32_t* out_net, uint32_t* out_mask);\n"
    "  • Utilité     : Décomposition arithmétique d'une notation CIDR (X.X.X.X/YY), extraction de\n"
    "                  l'adresse réseau hôte et génération du masque binaire 32 bits.\n"
    "\n"
    "Fonction 73 : plugin_asn_intel.c -- asn_intel_load_file -- Parsing à chaud table_static.conf\n"
    "  • Signature C : static bool asn_intel_load_file(const char* filepath);\n"
    "  • Utilité     : Chargement dynamique de `assets/rules/table_static.conf` pour peupler la table\n"
    "                  des plages IP de confiance sans recompilation du binaire.\n"
    "\n"
    "Fonction 74 : plugin_asn_intel.c -- asn_intel_match_static -- Évaluation arithmétique locale\n"
    "  • Signature C : static bool asn_intel_match_static(uint32_t ip, char* out_name, size_t max_len);\n"
    "  • Utilité     : Évaluation instantanée (0 ms) contre les infrastructures répertoriées via\n"
    "                  l'opération binaire `(ip & mask) == (net & mask)`.\n"
    "\n"
    "Fonction 75 : plugin_asn_intel.c -- asn_intel_query_bgp -- Sonde DNS TXT BGP\n"
    "  • Signature C : static bool asn_intel_query_bgp(const char* ip_str, uint32_t* out_asn, char* out_cc, size_t cc_sz);\n"
    "  • Utilité     : Résolution DNS de secours vers le registre Team Cymru (`origin.asn.cymru.com`)\n"
    "                  pour identifier le numéro d'ASN et le code pays ISO des adresses inconnues.\n"
    "\n"
    "Fonction 76 : plugin_asn_intel.c -- asn_intel_lookup -- Moteur central en cascade\n"
    "  • Signature C : bool raptor_asn_intel_lookup(const char* ip_str, char* out_name, size_t max_len);\n"
    "  • Utilité     : Cascade d'investigation : 1. Cache dynamique -> 2. `table_static.conf` (0 ms) -> 3. Sonde DNS BGP.\n"
    "\n"
    "Fonction 77 : plugin_asn_intel.c -- asn_intel_init -- Initialisation greffon ASN\n"
    "  • Signature C : static bool asn_intel_init(void);\n"
    "  • Utilité     : Prépare le mutex d'exclusion mutuelle, configure les caches et charge les règles réseau.\n"
    "\n"
    "Fonction 78 : plugin_asn_intel.c -- asn_intel_cleanup -- Vidange et arrêt\n"
    "  • Signature C : static void asn_intel_cleanup(void);\n"
    "  • Utilité     : Purge l'intégralité des structures mémoires et détruit le verrou d'exclusion mutuelle.\n"
    "\n"
    "Fonction 79 : plugin_asn_intel.c -- asn_intel_register -- Auto-enregistrement\n"
    "  • Signature C : __attribute__((constructor)) static void asn_intel_register(void);\n"
    "  • Utilité     : Enregistrement constructeur GCC du greffon sur le bus RaptorPlugin.\n"
    },
    {
        12,
        71,
        "Fonction 71 : plugin_asn_intel.c -- asn_intel_trim -- Nettoyage canonique",
    "Fonction 71 : plugin_asn_intel.c -- asn_intel_trim -- Nettoyage canonique\n"
    "  • Signature C : static char* asn_intel_trim(char* str);\n"
    "  • Utilité     : Nettoyage canonique des espaces blancs en tête et queue de chaîne.\n"
    },
    {
        12,
        72,
        "Fonction 72 : plugin_asn_intel.c -- asn_intel_parse_cidr -- Décomposition CIDR",
    "Fonction 72 : plugin_asn_intel.c -- asn_intel_parse_cidr -- Décomposition CIDR\n"
    "  • Signature C : static bool asn_intel_parse_cidr(const char* cidr, uint32_t* out_net, uint32_t* out_mask);\n"
    "  • Utilité     : Décomposition arithmétique d'une notation CIDR (X.X.X.X/YY), extraction de\n"
    "                  l'adresse réseau hôte et génération du masque binaire 32 bits.\n"
    },
    {
        12,
        73,
        "Fonction 73 : plugin_asn_intel.c -- asn_intel_load_file -- Parsing à chaud table_static.conf",
    "Fonction 73 : plugin_asn_intel.c -- asn_intel_load_file -- Parsing à chaud table_static.conf\n"
    "  • Signature C : static bool asn_intel_load_file(const char* filepath);\n"
    "  • Utilité     : Chargement dynamique de `assets/rules/table_static.conf` pour peupler la table\n"
    "                  des plages IP de confiance sans recompilation du binaire.\n"
    },
    {
        12,
        74,
        "Fonction 74 : plugin_asn_intel.c -- asn_intel_match_static -- Évaluation arithmétique locale",
    "Fonction 74 : plugin_asn_intel.c -- asn_intel_match_static -- Évaluation arithmétique locale\n"
    "  • Signature C : static bool asn_intel_match_static(uint32_t ip, char* out_name, size_t max_len);\n"
    "  • Utilité     : Évaluation instantanée (0 ms) contre les infrastructures répertoriées via\n"
    "                  l'opération binaire `(ip & mask) == (net & mask)`.\n"
    },
    {
        12,
        75,
        "Fonction 75 : plugin_asn_intel.c -- asn_intel_query_bgp -- Sonde DNS TXT BGP",
    "Fonction 75 : plugin_asn_intel.c -- asn_intel_query_bgp -- Sonde DNS TXT BGP\n"
    "  • Signature C : static bool asn_intel_query_bgp(const char* ip_str, uint32_t* out_asn, char* out_cc, size_t cc_sz);\n"
    "  • Utilité     : Résolution DNS de secours vers le registre Team Cymru (`origin.asn.cymru.com`)\n"
    "                  pour identifier le numéro d'ASN et le code pays ISO des adresses inconnues.\n"
    },
    {
        12,
        76,
        "Fonction 76 : plugin_asn_intel.c -- asn_intel_lookup -- Moteur central en cascade",
    "Fonction 76 : plugin_asn_intel.c -- asn_intel_lookup -- Moteur central en cascade\n"
    "  • Signature C : bool raptor_asn_intel_lookup(const char* ip_str, char* out_name, size_t max_len);\n"
    "  • Utilité     : Cascade d'investigation : 1. Cache dynamique -> 2. `table_static.conf` (0 ms) -> 3. Sonde DNS BGP.\n"
    },
    {
        12,
        77,
        "Fonction 77 : plugin_asn_intel.c -- asn_intel_init -- Initialisation greffon ASN",
    "Fonction 77 : plugin_asn_intel.c -- asn_intel_init -- Initialisation greffon ASN\n"
    "  • Signature C : static bool asn_intel_init(void);\n"
    "  • Utilité     : Prépare le mutex d'exclusion mutuelle, configure les caches et charge les règles réseau.\n"
    },
    {
        12,
        78,
        "Fonction 78 : plugin_asn_intel.c -- asn_intel_cleanup -- Vidange et arrêt",
    "Fonction 78 : plugin_asn_intel.c -- asn_intel_cleanup -- Vidange et arrêt\n"
    "  • Signature C : static void asn_intel_cleanup(void);\n"
    "  • Utilité     : Purge l'intégralité des structures mémoires et détruit le verrou d'exclusion mutuelle.\n"
    },
    {
        12,
        79,
        "Fonction 79 : plugin_asn_intel.c -- asn_intel_register -- Auto-enregistrement",
    "Fonction 79 : plugin_asn_intel.c -- asn_intel_register -- Auto-enregistrement\n"
    "  • Signature C : __attribute__((constructor)) static void asn_intel_register(void);\n"
    "  • Utilité     : Enregistrement constructeur GCC du greffon sur le bus RaptorPlugin.\n"
    },
    {
        13,
        0,
        "SECTION 13 : TRAQUE MÉMOIRE ANONYME & MUTATIONS R-X (src/core/mem_stealth.c)",
    "=== SECTION 13 : TRAQUE MÉMOIRE ANONYME & MUTATIONS R-X (src/core/mem_stealth.c) ===\n"
    "\n"
    "Fonction 80 : mem_stealth.c -- mem_stealth_whitelist_load -- Chargement liste blanche\n"
    "  • Signature C : bool mem_stealth_whitelist_load(const char* filepath);\n"
    "  • Utilité     : Charge et analyse `assets/rules/mem_whitelist.conf` (format `TYPE | MOTIF | DESC`),\n"
    "                  éliminant les faux positifs structurels sur le code dynamique.\n"
    "\n"
    "Fonction 81 : mem_stealth.c -- mem_stealth_is_whitelisted -- Filtrage exceptions légitimes\n"
    "  • Signature C : bool mem_stealth_is_whitelisted(const char* comm, const char* exe);\n"
    "  • Utilité     : Vérifie si un processus correspond à une signature autorisée (navigateurs,\n"
    "                  Flatpak, serveurs web, serveurs d'affichage, bureaux Cinnamon/GNOME/XFCE/KDE).\n"
    "\n"
    "Fonction 82 : mem_stealth.c -- mem_stealth_scan_maps_anon_rx -- Inspection Inode 0 & riposte\n"
    "  • Signature C : bool mem_stealth_scan_maps_anon_rx(pid_t pid, const char* comm, const char* exe, StealthEvent* ev_out);\n"
    "  • Utilité     : Analyse `/proc/[PID]/maps` pour isoler les allocations anonymes rendues\n"
    "                  exécutables (`r-xp`) avec un Inode à 0 sans ancrage disque, et déclenche\n"
    "                  la riposte réflexe `rap_reflex_dispatch`.\n"
    "\n"
    "Fonction 83 : mem_stealth.c -- mem_stealth_dispatch_diff_event -- Propagation différentielle\n"
    "  • Signature C : void mem_stealth_dispatch_diff_event(const StealthEvent* ev);\n"
    "  • Utilité     : Injecte les alertes forensiques d'injection dans le tampon circulaire du\n"
    "                  Cahier Différentiel (Onglet 4).\n"
    },
    {
        13,
        80,
        "Fonction 80 : mem_stealth.c -- mem_stealth_whitelist_load -- Chargement liste blanche",
    "Fonction 80 : mem_stealth.c -- mem_stealth_whitelist_load -- Chargement liste blanche\n"
    "  • Signature C : bool mem_stealth_whitelist_load(const char* filepath);\n"
    "  • Utilité     : Charge et analyse `assets/rules/mem_whitelist.conf` (format `TYPE | MOTIF | DESC`),\n"
    "                  éliminant les faux positifs structurels sur le code dynamique.\n"
    },
    {
        13,
        81,
        "Fonction 81 : mem_stealth.c -- mem_stealth_is_whitelisted -- Filtrage exceptions légitimes",
    "Fonction 81 : mem_stealth.c -- mem_stealth_is_whitelisted -- Filtrage exceptions légitimes\n"
    "  • Signature C : bool mem_stealth_is_whitelisted(const char* comm, const char* exe);\n"
    "  • Utilité     : Vérifie si un processus correspond à une signature autorisée (navigateurs,\n"
    "                  Flatpak, serveurs web, serveurs d'affichage, bureaux Cinnamon/GNOME/XFCE/KDE).\n"
    },
    {
        13,
        82,
        "Fonction 82 : mem_stealth.c -- mem_stealth_scan_maps_anon_rx -- Inspection Inode 0 & riposte",
    "Fonction 82 : mem_stealth.c -- mem_stealth_scan_maps_anon_rx -- Inspection Inode 0 & riposte\n"
    "  • Signature C : bool mem_stealth_scan_maps_anon_rx(pid_t pid, const char* comm, const char* exe, StealthEvent* ev_out);\n"
    "  • Utilité     : Analyse `/proc/[PID]/maps` pour isoler les allocations anonymes rendues\n"
    "                  exécutables (`r-xp`) avec un Inode à 0 sans ancrage disque, et déclenche\n"
    "                  la riposte réflexe `rap_reflex_dispatch`.\n"
    },
    {
        13,
        83,
        "Fonction 83 : mem_stealth.c -- mem_stealth_dispatch_diff_event -- Propagation différentielle",
    "Fonction 83 : mem_stealth.c -- mem_stealth_dispatch_diff_event -- Propagation différentielle\n"
    "  • Signature C : void mem_stealth_dispatch_diff_event(const StealthEvent* ev);\n"
    "  • Utilité     : Injecte les alertes forensiques d'injection dans le tampon circulaire du\n"
    "                  Cahier Différentiel (Onglet 4).\n"
    },
    {
        14,
        0,
        "SECTION 14 : DÉTECTION DES DÉTOURNEMENTS LD_PRELOAD (src/core/preload_hunter.c)",
    "=== SECTION 14 : DÉTECTION DES DÉTOURNEMENTS LD_PRELOAD (src/core/preload_hunter.c) ===\n"
    "\n"
    "Fonction 84 : preload_hunter.c -- preload_hunter_check_global -- Audit intégrité globale\n"
    "  • Signature C : bool preload_hunter_check_global(char* out_path, size_t out_len);\n"
    "  • Utilité     : Détecte la persistance d'un rootkit utilisateur en vérifiant l'accès\n"
    "                  et le contenu de `/etc/ld.so.preload`.\n"
    "\n"
    "Fonction 85 : preload_hunter.c -- preload_hunter_scan_process_env -- Audit variables d'environnement\n"
    "  • Signature C : bool preload_hunter_scan_process_env(pid_t pid, char* out_preload_lib, size_t out_len);\n"
    "  • Utilité     : Scanne `/proc/[PID]/environ` (délimiteurs `\\0`) pour intercepter\n"
    "                  toute injection ciblée via la variable `LD_PRELOAD=`.\n"
    },
    {
        14,
        84,
        "Fonction 84 : preload_hunter.c -- preload_hunter_check_global -- Audit intégrité globale",
    "Fonction 84 : preload_hunter.c -- preload_hunter_check_global -- Audit intégrité globale\n"
    "  • Signature C : bool preload_hunter_check_global(char* out_path, size_t out_len);\n"
    "  • Utilité     : Détecte la persistance d'un rootkit utilisateur en vérifiant l'accès\n"
    "                  et le contenu de `/etc/ld.so.preload`.\n"
    },
    {
        14,
        85,
        "Fonction 85 : preload_hunter.c -- preload_hunter_scan_process_env -- Audit variables d'environnement",
    "Fonction 85 : preload_hunter.c -- preload_hunter_scan_process_env -- Audit variables d'environnement\n"
    "  • Signature C : bool preload_hunter_scan_process_env(pid_t pid, char* out_preload_lib, size_t out_len);\n"
    "  • Utilité     : Scanne `/proc/[PID]/environ` (délimiteurs `\\0`) pour intercepter\n"
    "                  toute injection ciblée via la variable `LD_PRELOAD=`.\n"
    },
    {
        15,
        0,
        "SECTION 15 : NOTIFICATIONS BUREAU ASYNCHRONES (src/core/raptor_notify.c)",
    "=== SECTION 15 : NOTIFICATIONS BUREAU ASYNCHRONES (src/core/raptor_notify.c) ===\n"
    "\n"
    "Fonction 86 : raptor_notify.c -- raptor_notify_send_desktop -- Émission asynchrone FreeDesktop\n"
    "  • Signature C : void raptor_notify_send_desktop(const char* title, const char* body, bool is_critical);\n"
    "  • Utilité     : Envoie une notification via `notify-send` avec double fork (`fork()` / `_exit(0)`)\n"
    "                  garantissant zéro saccade graphique sur la boucle Raylib (60 FPS).\n"
    "\n"
    "Fonction 87 : raptor_notify.c -- raptor_notify_evaluate_threats -- Arbitrage d'alertes\n"
    "  • Signature C : void raptor_notify_evaluate_threats(int current_threats, const char* primary_threat_name);\n"
    "  • Utilité     : Filtre anti-rebond temporel (30 secondes) déclenchant l'alerte sur\n"
    "                  transition critique et signalant le retour au calme.\n"
    },
    {
        15,
        86,
        "Fonction 86 : raptor_notify.c -- raptor_notify_send_desktop -- Émission asynchrone FreeDesktop",
    "Fonction 86 : raptor_notify.c -- raptor_notify_send_desktop -- Émission asynchrone FreeDesktop\n"
    "  • Signature C : void raptor_notify_send_desktop(const char* title, const char* body, bool is_critical);\n"
    "  • Utilité     : Envoie une notification via `notify-send` avec double fork (`fork()` / `_exit(0)`)\n"
    "                  garantissant zéro saccade graphique sur la boucle Raylib (60 FPS).\n"
    },
    {
        15,
        87,
        "Fonction 87 : raptor_notify.c -- raptor_notify_evaluate_threats -- Arbitrage d'alertes",
    "Fonction 87 : raptor_notify.c -- raptor_notify_evaluate_threats -- Arbitrage d'alertes\n"
    "  • Signature C : void raptor_notify_evaluate_threats(int current_threats, const char* primary_threat_name);\n"
    "  • Utilité     : Filtre anti-rebond temporel (30 secondes) déclenchant l'alerte sur\n"
    "                  transition critique et signalant le retour au calme.\n"
    },
    {
        16,
        0,
        "SECTION 16 : MODE CLI DE SECOURS & OUTILS CHIRURGICAUX (src/cli/)",
    "=== SECTION 16 : MODE CLI DE SECOURS & OUTILS CHIRURGICAUX (src/cli/) ===\n"
    "\n"
    "Fonction 88 : rap_main.c -- rap_dispatch_command -- Micro-routeur déclaratif\n"
    "  • Signature C : int rap_dispatch_command(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Achemine les requêtes CLI vers les gestionnaires spécialisés via la\n"
    "                  table déclarative `g_commands`.\n"
    "\n"
    "Fonction 89 : cmd_status.c -- rap_cmd_status -- Bilan vital instantané\n"
    "  • Signature C : int rap_cmd_status(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Diagnostic vital en moins de 15 ms (RAM totale, processus actifs,\n"
    "                  intégrité de `/etc/ld.so.preload`).\n"
    "\n"
    "Fonction 90 : cmd_top.c -- rap_cmd_top -- Top consommateurs VmRSS\n"
    "  • Signature C : int rap_cmd_top(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Trie les processus par mémoire résidente physique réelle sans\n"
    "                  allocation sur le tas (tri d'insertion statique sur la pile).\n"
    "\n"
    "Fonction 91 : cmd_hunt.c -- rap_cmd_hunt -- Chasseur universel combiné & riposte active\n"
    "  • Signature C : int rap_cmd_hunt(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Enchaîne séquentiellement la traque Inode 0 (`rx`), les détournements\n"
    "                  dynamiques (`preload`) et l'analyse heuristique (`heur`) avec support\n"
    "                  de l'armement réflexe (`--arm`) et simulation (`--dry-run`).\n"
    "\n"
    "Fonction 92 : cmd_rx.c -- rap_cmd_rx -- Traque mutations exécutables Inode 0\n"
    "  • Signature C : int rap_cmd_rx(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Repère les allocations mémoires anonymes exécutables (`r-xp`)\n"
    "                  dans `/proc/[PID]/maps` avec filtrage strict par `mem_whitelist.conf`\n"
    "                  et raccordement à la riposte cinétique réflexe (`rap_reflex_dispatch`).\n"
    "\n"
    "Fonction 93 : cmd_preload.c -- rap_cmd_preload -- Chasseur LD_PRELOAD\n"
    "  • Signature C : int rap_cmd_preload(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Détecte les rootkits userland via le contrôle double palier\n"
    "                  (`/etc/ld.so.preload` et `/proc/[PID]/environ`).\n"
    "\n"
    "Fonction 94 : cmd_net.c -- rap_cmd_net -- Audit réseau Net-Guard\n"
    "  • Signature C : int rap_cmd_net(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Analyse `/proc/net/tcp` et qualifie instantanément les connexions\n"
    "                  actives (APPROUVÉ, EXTERNE, SUSPECT, HOSTILE).\n"
    "\n"
    "Fonction 95 : cmd_kill.c -- rap_cmd_kill -- Neutralisation cinétique chirurgicale\n"
    "  • Signature C : int rap_cmd_kill(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Émet le signal non rattrapable `SIGKILL` avec validation préalable\n"
    "                  par `rap_check_safety_pid` et confirmation interactive (`-f` pour forcer).\n"
    "\n"
    "Fonction 96 : cmd_kill.c -- rap_cmd_freeze -- Suspension conservatoire forensique\n"
    "  • Signature C : int rap_cmd_freeze(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Gèle un processus suspect via `SIGSTOP` (état `T`) pour figer son\n"
    "                  activité tout en préservant sa mémoire vive pour analyse forensique.\n"
    "\n"
    "Fonction 97 : cmd_man.c -- rap_cmd_man -- Manuel de référence unifié embarqué\n"
    "  • Signature C : int rap_cmd_man(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Point d'entrée de la documentation embarquée. Gère l'affichage sélectif\n"
    "                  par commande (`rap man sentry`, `rap man rx`), la vue complète (`all`)\n"
    "                  ou le routage vers le dictionnaire des fonctions (`fn`).\n"
    "\n"
    "Fonction 98 : rap_common.c -- rap_output_formatter -- Formatage d'affichage universel\n"
    "  • Signature C : void rap_output_formatter(const RapConfig *cfg, const char *c1, const char *c2, const char *c3, const char *c4);\n"
    "  • Utilité     : Formate les flux d'audit sur 4 colonnes régulières en mode interactif\n"
    "                  ou en flux brut délimité par tabulations (`\\t`) en mode `--raw`.\n"
    "\n"
    "Fonction 99 : rap_common.c -- rap_check_safety_pid -- Barrière de sécurité cinétique\n"
    "  • Signature C : int rap_check_safety_pid(const RapConfig *cfg, pid_t pid);\n"
    "  • Utilité     : Empêche les arrêts accidentels en rejetant le tir sur PID 1,\n"
    "                  sur soi-même, sur le shell parent et sur les threads internes du noyau.\n"
    "\n"
    "Fonction 100 : rap_main.c -- rap_parse_flags -- Parseur d'arguments et drapeaux\n"
    "  • Signature C : int rap_parse_flags(int *argc, char ***argv, RapConfig *cfg);\n"
    "  • Utilité     : Extrait les drapeaux universels (`-q`, `-r`, `-f`, `-v`, `-a`, `--arm`,\n"
    "                  `--dry-run`, `-n`, `-h`), détecte le mode TTY et isole le verbe d'action.\n"
    "\n"
    "Fonction 107 : rap_common.c -- rap_is_mem_whitelisted -- Filtrage liste blanche mémoire\n"
    "  • Signature C : int rap_is_mem_whitelisted(const char *comm);\n"
    "  • Utilité     : Vérifie si un processus figure dans `mem_whitelist.conf` en gérant\n"
    "                  la troncature noyau à 15 caractères sans allocation dynamique.\n"
    "\n"
    "Fonction 108 : rap_common.c -- rap_net_eval_ip -- Qualification CIDR d'endpoint IP\n"
    "  • Signature C : int rap_net_eval_ip(uint32_t ip, uint16_t port, char *out_desc, size_t desc_size);\n"
    "  • Utilité     : Évalue une adresse IP contre les registres `fw_officiel.conf` et `fw_custom.conf`\n"
    "                  pour retourner son niveau de confiance (APPROUVÉ, STANDARD, SUSPECT, HOSTILE).\n"
    "\n"
    "Fonction 109 : cmd_heur.c -- rap_cmd_heur -- Traque des anomalies d'exécution\n"
    "  • Signature C : int rap_cmd_heur(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Parcourt `/proc`, résout `/proc/[PID]/exe` et projette les anomalies\n"
    "                  comportementales identifiées sur la console de secours.\n"
    "\n"
    "Fonction 110 : cmd_heur.c -- rap_heur_check_process -- Évaluation unitaire heuristique\n"
    "  • Signature C : int rap_heur_check_process(pid_t pid, const char *comm, const char *exe_path, char *out_anomaly, size_t out_sz);\n"
    "  • Utilité     : Évalue un processus selon trois axes sans allocation dynamique :\n"
    "                  1. Binaires supprimés du disque en cours d'exécution (`(deleted)`).\n"
    "                  2. Exécutions depuis des zones volatiles suspectes (`/tmp`, `/dev/shm`).\n"
    "                  3. Usurpation d'identité de démons systèmes (*Masquerading*).\n"
    "\n"
    "Fonction 123 : cmd_man.c -- rap_man_render -- Téléscripteur et projection de documentation\n"
    "  • Signature C : static void rap_man_render(const char *text, size_t len, const RapConfig *cfg, bool force_direct);\n"
    "  • Utilité     : Aiguille le flux textuel vers le pager autonome `rapless` en mode interactif\n"
    "                  ou projette directement sur STDOUT en mode tube, redirection ou `--raw`.\n"
    "\n"
    "Fonction 124 : cmd_man.c -- rap_man_handle_fn -- Moteur de navigation du dictionnaire API\n"
    "  • Signature C : static int rap_man_handle_fn(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Permet l'interrogation granulaire du dictionnaire des fonctions par numéro de\n"
    "                  section (`rap man fn <sec>`), par identifiant unique (`rap man fn <id>`) ou en intégralité.\n"
    "\n"
    "Fonction 125 : cmd_man.c -- rap_man_extract_topic -- Extraction sémantique de rubrique\n"
    "  • Signature C : static const char *rap_man_extract_topic(int argc, char **argv);\n"
    "  • Utilité     : Isole l'argument cible de documentation dans `argv` indépendamment des drapeaux\n"
    "                  et de la position des paramètres.\n"
    "\n"
    "Fonction 126 : cmd_man.c -- rap_man_lookup_page -- Recherche et résolution d'entrée man\n"
    "  • Signature C : static const RapManPageEntry *rap_man_lookup_page(const char *topic);\n"
    "  • Utilité     : Résout une clé de commande dans la table `g_rap_man_pages[]` avec gestion des alias\n"
    "                  courts (`st`, `se`, `he`) et résolution avec ou sans le préfixe `rap-`.\n"
    },
    {
        16,
        88,
        "Fonction 88 : rap_main.c -- rap_dispatch_command -- Micro-routeur déclaratif",
    "Fonction 88 : rap_main.c -- rap_dispatch_command -- Micro-routeur déclaratif\n"
    "  • Signature C : int rap_dispatch_command(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Achemine les requêtes CLI vers les gestionnaires spécialisés via la\n"
    "                  table déclarative `g_commands`.\n"
    },
    {
        16,
        89,
        "Fonction 89 : cmd_status.c -- rap_cmd_status -- Bilan vital instantané",
    "Fonction 89 : cmd_status.c -- rap_cmd_status -- Bilan vital instantané\n"
    "  • Signature C : int rap_cmd_status(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Diagnostic vital en moins de 15 ms (RAM totale, processus actifs,\n"
    "                  intégrité de `/etc/ld.so.preload`).\n"
    },
    {
        16,
        90,
        "Fonction 90 : cmd_top.c -- rap_cmd_top -- Top consommateurs VmRSS",
    "Fonction 90 : cmd_top.c -- rap_cmd_top -- Top consommateurs VmRSS\n"
    "  • Signature C : int rap_cmd_top(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Trie les processus par mémoire résidente physique réelle sans\n"
    "                  allocation sur le tas (tri d'insertion statique sur la pile).\n"
    },
    {
        16,
        91,
        "Fonction 91 : cmd_hunt.c -- rap_cmd_hunt -- Chasseur universel combiné & riposte active",
    "Fonction 91 : cmd_hunt.c -- rap_cmd_hunt -- Chasseur universel combiné & riposte active\n"
    "  • Signature C : int rap_cmd_hunt(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Enchaîne séquentiellement la traque Inode 0 (`rx`), les détournements\n"
    "                  dynamiques (`preload`) et l'analyse heuristique (`heur`) avec support\n"
    "                  de l'armement réflexe (`--arm`) et simulation (`--dry-run`).\n"
    },
    {
        16,
        92,
        "Fonction 92 : cmd_rx.c -- rap_cmd_rx -- Traque mutations exécutables Inode 0",
    "Fonction 92 : cmd_rx.c -- rap_cmd_rx -- Traque mutations exécutables Inode 0\n"
    "  • Signature C : int rap_cmd_rx(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Repère les allocations mémoires anonymes exécutables (`r-xp`)\n"
    "                  dans `/proc/[PID]/maps` avec filtrage strict par `mem_whitelist.conf`\n"
    "                  et raccordement à la riposte cinétique réflexe (`rap_reflex_dispatch`).\n"
    },
    {
        16,
        93,
        "Fonction 93 : cmd_preload.c -- rap_cmd_preload -- Chasseur LD_PRELOAD",
    "Fonction 93 : cmd_preload.c -- rap_cmd_preload -- Chasseur LD_PRELOAD\n"
    "  • Signature C : int rap_cmd_preload(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Détecte les rootkits userland via le contrôle double palier\n"
    "                  (`/etc/ld.so.preload` et `/proc/[PID]/environ`).\n"
    },
    {
        16,
        94,
        "Fonction 94 : cmd_net.c -- rap_cmd_net -- Audit réseau Net-Guard",
    "Fonction 94 : cmd_net.c -- rap_cmd_net -- Audit réseau Net-Guard\n"
    "  • Signature C : int rap_cmd_net(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Analyse `/proc/net/tcp` et qualifie instantanément les connexions\n"
    "                  actives (APPROUVÉ, EXTERNE, SUSPECT, HOSTILE).\n"
    },
    {
        16,
        95,
        "Fonction 95 : cmd_kill.c -- rap_cmd_kill -- Neutralisation cinétique chirurgicale",
    "Fonction 95 : cmd_kill.c -- rap_cmd_kill -- Neutralisation cinétique chirurgicale\n"
    "  • Signature C : int rap_cmd_kill(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Émet le signal non rattrapable `SIGKILL` avec validation préalable\n"
    "                  par `rap_check_safety_pid` et confirmation interactive (`-f` pour forcer).\n"
    },
    {
        16,
        96,
        "Fonction 96 : cmd_kill.c -- rap_cmd_freeze -- Suspension conservatoire forensique",
    "Fonction 96 : cmd_kill.c -- rap_cmd_freeze -- Suspension conservatoire forensique\n"
    "  • Signature C : int rap_cmd_freeze(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Gèle un processus suspect via `SIGSTOP` (état `T`) pour figer son\n"
    "                  activité tout en préservant sa mémoire vive pour analyse forensique.\n"
    },
    {
        16,
        97,
        "Fonction 97 : cmd_man.c -- rap_cmd_man -- Manuel de référence unifié embarqué",
    "Fonction 97 : cmd_man.c -- rap_cmd_man -- Manuel de référence unifié embarqué\n"
    "  • Signature C : int rap_cmd_man(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Point d'entrée de la documentation embarquée. Gère l'affichage sélectif\n"
    "                  par commande (`rap man sentry`, `rap man rx`), la vue complète (`all`)\n"
    "                  ou le routage vers le dictionnaire des fonctions (`fn`).\n"
    },
    {
        16,
        98,
        "Fonction 98 : rap_common.c -- rap_output_formatter -- Formatage d'affichage universel",
    "Fonction 98 : rap_common.c -- rap_output_formatter -- Formatage d'affichage universel\n"
    "  • Signature C : void rap_output_formatter(const RapConfig *cfg, const char *c1, const char *c2, const char *c3, const char *c4);\n"
    "  • Utilité     : Formate les flux d'audit sur 4 colonnes régulières en mode interactif\n"
    "                  ou en flux brut délimité par tabulations (`\\t`) en mode `--raw`.\n"
    },
    {
        16,
        99,
        "Fonction 99 : rap_common.c -- rap_check_safety_pid -- Barrière de sécurité cinétique",
    "Fonction 99 : rap_common.c -- rap_check_safety_pid -- Barrière de sécurité cinétique\n"
    "  • Signature C : int rap_check_safety_pid(const RapConfig *cfg, pid_t pid);\n"
    "  • Utilité     : Empêche les arrêts accidentels en rejetant le tir sur PID 1,\n"
    "                  sur soi-même, sur le shell parent et sur les threads internes du noyau.\n"
    },
    {
        16,
        100,
        "Fonction 100 : rap_main.c -- rap_parse_flags -- Parseur d'arguments et drapeaux",
    "Fonction 100 : rap_main.c -- rap_parse_flags -- Parseur d'arguments et drapeaux\n"
    "  • Signature C : int rap_parse_flags(int *argc, char ***argv, RapConfig *cfg);\n"
    "  • Utilité     : Extrait les drapeaux universels (`-q`, `-r`, `-f`, `-v`, `-a`, `--arm`,\n"
    "                  `--dry-run`, `-n`, `-h`), détecte le mode TTY et isole le verbe d'action.\n"
    },
    {
        16,
        107,
        "Fonction 107 : rap_common.c -- rap_is_mem_whitelisted -- Filtrage liste blanche mémoire",
    "Fonction 107 : rap_common.c -- rap_is_mem_whitelisted -- Filtrage liste blanche mémoire\n"
    "  • Signature C : int rap_is_mem_whitelisted(const char *comm);\n"
    "  • Utilité     : Vérifie si un processus figure dans `mem_whitelist.conf` en gérant\n"
    "                  la troncature noyau à 15 caractères sans allocation dynamique.\n"
    },
    {
        16,
        108,
        "Fonction 108 : rap_common.c -- rap_net_eval_ip -- Qualification CIDR d'endpoint IP",
    "Fonction 108 : rap_common.c -- rap_net_eval_ip -- Qualification CIDR d'endpoint IP\n"
    "  • Signature C : int rap_net_eval_ip(uint32_t ip, uint16_t port, char *out_desc, size_t desc_size);\n"
    "  • Utilité     : Évalue une adresse IP contre les registres `fw_officiel.conf` et `fw_custom.conf`\n"
    "                  pour retourner son niveau de confiance (APPROUVÉ, STANDARD, SUSPECT, HOSTILE).\n"
    },
    {
        16,
        109,
        "Fonction 109 : cmd_heur.c -- rap_cmd_heur -- Traque des anomalies d'exécution",
    "Fonction 109 : cmd_heur.c -- rap_cmd_heur -- Traque des anomalies d'exécution\n"
    "  • Signature C : int rap_cmd_heur(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Parcourt `/proc`, résout `/proc/[PID]/exe` et projette les anomalies\n"
    "                  comportementales identifiées sur la console de secours.\n"
    },
    {
        16,
        110,
        "Fonction 110 : cmd_heur.c -- rap_heur_check_process -- Évaluation unitaire heuristique",
    "Fonction 110 : cmd_heur.c -- rap_heur_check_process -- Évaluation unitaire heuristique\n"
    "  • Signature C : int rap_heur_check_process(pid_t pid, const char *comm, const char *exe_path, char *out_anomaly, size_t out_sz);\n"
    "  • Utilité     : Évalue un processus selon trois axes sans allocation dynamique :\n"
    "                  1. Binaires supprimés du disque en cours d'exécution (`(deleted)`).\n"
    "                  2. Exécutions depuis des zones volatiles suspectes (`/tmp`, `/dev/shm`).\n"
    "                  3. Usurpation d'identité de démons systèmes (*Masquerading*).\n"
    },
    {
        16,
        123,
        "Fonction 123 : cmd_man.c -- rap_man_render -- Téléscripteur et projection de documentation",
    "Fonction 123 : cmd_man.c -- rap_man_render -- Téléscripteur et projection de documentation\n"
    "  • Signature C : static void rap_man_render(const char *text, size_t len, const RapConfig *cfg, bool force_direct);\n"
    "  • Utilité     : Aiguille le flux textuel vers le pager autonome `rapless` en mode interactif\n"
    "                  ou projette directement sur STDOUT en mode tube, redirection ou `--raw`.\n"
    },
    {
        16,
        124,
        "Fonction 124 : cmd_man.c -- rap_man_handle_fn -- Moteur de navigation du dictionnaire API",
    "Fonction 124 : cmd_man.c -- rap_man_handle_fn -- Moteur de navigation du dictionnaire API\n"
    "  • Signature C : static int rap_man_handle_fn(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Permet l'interrogation granulaire du dictionnaire des fonctions par numéro de\n"
    "                  section (`rap man fn <sec>`), par identifiant unique (`rap man fn <id>`) ou en intégralité.\n"
    },
    {
        16,
        125,
        "Fonction 125 : cmd_man.c -- rap_man_extract_topic -- Extraction sémantique de rubrique",
    "Fonction 125 : cmd_man.c -- rap_man_extract_topic -- Extraction sémantique de rubrique\n"
    "  • Signature C : static const char *rap_man_extract_topic(int argc, char **argv);\n"
    "  • Utilité     : Isole l'argument cible de documentation dans `argv` indépendamment des drapeaux\n"
    "                  et de la position des paramètres.\n"
    },
    {
        16,
        126,
        "Fonction 126 : cmd_man.c -- rap_man_lookup_page -- Recherche et résolution d'entrée man",
    "Fonction 126 : cmd_man.c -- rap_man_lookup_page -- Recherche et résolution d'entrée man\n"
    "  • Signature C : static const RapManPageEntry *rap_man_lookup_page(const char *topic);\n"
    "  • Utilité     : Résout une clé de commande dans la table `g_rap_man_pages[]` avec gestion des alias\n"
    "                  courts (`st`, `se`, `he`) et résolution avec ou sans le préfixe `rap-`.\n"
    },
    {
        17,
        0,
        "SECTION 17 : SENTINELLE PASSIVE ANTI-RANÇONGICIEL (FANOTIFY & LEURRES DENSES)",
    "=== SECTION 17 : SENTINELLE PASSIVE ANTI-RANÇONGICIEL (FANOTIFY & LEURRES DENSES) ===\n"
    "\n"
    "Fonction 111 : sentry_engine.c -- sentry_deploy_canary -- Déploiement d'appât dense\n"
    "  • Signature C : int sentry_deploy_canary(const char *dir_path);\n"
    "  • Utilité     : Génère un fichier binaire dense de 32 Ko (`!00_raptor_vault.kdbx`)\n"
    "                  doté de l'en-tête officiel KeePass KDBX4 (Magic Numbers authentiques)\n"
    "                  et d'un corps à haute entropie généré par LCG. Applique les permissions\n"
    "                  `0666` via `fchmod` afin de piéger tout compte utilisateur.\n"
    "  • Retour      : `0` en cas de synchronisation physique (`fdatasync`) réussie, `-1` en cas d'échec.\n"
    "\n"
    "Fonction 112 : sentry_engine.c -- sentry_init_fanotify -- Initialisation de la surveillance noyau\n"
    "  • Signature C : int sentry_init_fanotify(const SentryConfig *config);\n"
    "  • Utilité     : Configure le sous-système de notification noyau en mode permissionnel\n"
    "                  synchrone (`FAN_CLASS_CONTENT | FAN_NONBLOCK | FAN_CLOEXEC`). Déploie le\n"
    "                  leurre si nécessaire et pose une marque de verrouillage d'ouverture\n"
    "                  synchrone (`FAN_MARK_ADD`, `FAN_OPEN_PERM`) sur chaque cible configurée.\n"
    "  • Retour      : Descripteur de fichier `fan_fd` valide, ou `-1` en cas d'erreur de privilèges.\n"
    "\n"
    "Fonction 113 : sentry_engine.c -- sentry_listen_loop -- Boucle de veille passive bloquante\n"
    "  • Signature C : int sentry_listen_loop(int fanotify_fd, const SentryConfig *config);\n"
    "  • Utilité     : Assure le guet passif bloquant sur `poll()` (consommation de 0% CPU au repos).\n"
    "                  Dépouille les événements noyau dans un tampon statique de 4 Ko aligné\n"
    "                  sur 8 octets (`__attribute__((aligned(8)))`) sans allocation sur le tas (`0 heap`).\n"
    "  • Retour      : `0` lors d'un arrêt nominal par signal, `-1` en cas d'erreur de lecture.\n"
    "\n"
    "Fonction 114 : sentry_engine.c -- sentry_trigger_reflex -- Riposte cinétique synchrone\n"
    "  • Signature C : int sentry_trigger_reflex(int fanotify_fd, int fd_event, pid_t pid);\n"
    "  • Utilité     : Applique instantanément le rejet auprès du noyau via `FAN_DENY`,\n"
    "                  ferme le descripteur d'événement et achemine le PID agresseur vers\n"
    "                  `rap_reflex_dispatch` avec le motif `TRIGGER_CANARY_BREACH`.\n"
    "  • Retour      : `0` si la neutralisation a réussi, `-1` si le PID était protégé.\n"
    "\n"
    "Fonction 115 : sentry_engine.c -- sentry_enter_stealth -- Blindage et invisibilité mémoire\n"
    "  • Signature C : int sentry_enter_stealth(const char *fake_name);\n"
    "  • Utilité     : Masque le nom du processus dans la table noyau via `prctl(PR_SET_NAME, \"[kworker/u16:3]\")`,\n"
    "                  s'attribue l'immunité OOM maximale (`/proc/self/oom_score_adj = -1000`) et\n"
    "                  verrouille l'intégralité des pages physiques en RAM via `mlockall(MCL_CURRENT | MCL_FUTURE)`.\n"
    "  • Retour      : `0` en cas de succès opérationnel, `-1` si les limites système restreignent le verrouillage.\n"
    "\n"
    "Fonction 116 : cmd_sentry.c -- cmd_sentry_arm -- Armement des leurres sur disque\n"
    "  • Signature C : static int cmd_sentry_arm(int argc, char **argv);\n"
    "  • Utilité     : Déploie les fichiers canaris dans les cibles spécifiées en ligne de\n"
    "                  commande ou applique les répertoires sensibles par défaut (`/home`, `/var/www`, `/var/lib`, `/etc`).\n"
    "  • Retour      : `0` (succès) si au moins un appât est opérationnel, `2` en cas d'échec global.\n"
    "\n"
    "Fonction 117 : cmd_sentry.c -- cmd_sentry_disarm -- Purge et désarmement préventif\n"
    "  • Signature C : static int cmd_sentry_disarm(int argc, char **argv);\n"
    "  • Utilité     : Supprime proprement par déliaison physique (`unlink`) les fichiers\n"
    "                  leurres pour autoriser les opérations de maintenance sans fausse alerte.\n"
    "  • Retour      : Toujours `0` après affichage du bilan de neutralisation.\n"
    "\n"
    "Fonction 118 : cmd_sentry.c -- cmd_sentry_status -- Audit d'intégrité physique des appâts\n"
    "  • Signature C : static int cmd_sentry_status(int argc, char **argv);\n"
    "  • Utilité     : Vérifie l'existence et la stricte conformité de taille (32768 octets)\n"
    "                  des canaris sur l'ensemble des répertoires surveillés (`[ACTIF]`, `[ALTERE]`, `[NON ARME]`).\n"
    "  • Retour      : Toujours `0`.\n"
    "\n"
    "Fonction 119 : cmd_sentry.c -- cmd_sentry_run -- Démarrage du guet passif fanotify\n"
    "  • Signature C : static int cmd_sentry_run(int argc, char **argv);\n"
    "  • Utilité     : Initialise `fanotify`, prend en charge le détachement en démon d'arrière-plan\n"
    "                  (`-d` / `--daemon`) via `fork()` et `setsid()`, active l'invisibilité furtive\n"
    "                  (`sentry_enter_stealth`), redirige les flux vers `/dev/null` et lance la veille.\n"
    "  • Retour      : `0` en cas d'arrêt propre, `2` en cas de droits administrateur insuffisants.\n"
    "\n"
    "Fonction 120 : cmd_sentry.c -- cmd_sentry -- Aiguilleur CLI sentry\n"
    "  • Signature C : int cmd_sentry(int argc, char **argv);\n"
    "  • Utilité     : Point d'entrée principal de la sous-commande `rap sentry`. Parse les\n"
    "                  arguments et route vers `arm`, `disarm`, `status`, `dump` ou `run`.\n"
    "  • Retour      : Code de retour de la sous-commande, ou `2` si la commande est inconnue.\n"
    "\n"
    "Fonction 121 : rap_main.c -- rap_cmd_sentry -- Pont déclaratif sentinelle\n"
    "  • Signature C : int rap_cmd_sentry(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Relie la table déclarative universelle `g_commands` de RAPTOR au point\n"
    "                  d'entrée de la sentinelle anti-rançongiciel (`cmd_sentry`).\n"
    "  • Retour      : Code de retour POSIX transmis par `cmd_sentry`.\n"
    "\n"
    "Fonction 122 : cmd_sentry.c -- cmd_sentry_dump -- Restitution boîte noire médico-légale\n"
    "  • Signature C : static int cmd_sentry_dump(int argc, char **argv);\n"
    "  • Utilité     : Extrait et déverse les événements médico-légaux conservés dans l'anneau\n"
    "                  circulaire en mémoire vive (`/dev/shm/raptor_blackbox`) avec latences nanosecondes.\n"
    "  • Retour      : `0` après affichage, `2` si la boîte noire est vide ou inaccessible.\n"
    },
    {
        17,
        111,
        "Fonction 111 : sentry_engine.c -- sentry_deploy_canary -- Déploiement d'appât dense",
    "Fonction 111 : sentry_engine.c -- sentry_deploy_canary -- Déploiement d'appât dense\n"
    "  • Signature C : int sentry_deploy_canary(const char *dir_path);\n"
    "  • Utilité     : Génère un fichier binaire dense de 32 Ko (`!00_raptor_vault.kdbx`)\n"
    "                  doté de l'en-tête officiel KeePass KDBX4 (Magic Numbers authentiques)\n"
    "                  et d'un corps à haute entropie généré par LCG. Applique les permissions\n"
    "                  `0666` via `fchmod` afin de piéger tout compte utilisateur.\n"
    "  • Retour      : `0` en cas de synchronisation physique (`fdatasync`) réussie, `-1` en cas d'échec.\n"
    },
    {
        17,
        112,
        "Fonction 112 : sentry_engine.c -- sentry_init_fanotify -- Initialisation de la surveillance noyau",
    "Fonction 112 : sentry_engine.c -- sentry_init_fanotify -- Initialisation de la surveillance noyau\n"
    "  • Signature C : int sentry_init_fanotify(const SentryConfig *config);\n"
    "  • Utilité     : Configure le sous-système de notification noyau en mode permissionnel\n"
    "                  synchrone (`FAN_CLASS_CONTENT | FAN_NONBLOCK | FAN_CLOEXEC`). Déploie le\n"
    "                  leurre si nécessaire et pose une marque de verrouillage d'ouverture\n"
    "                  synchrone (`FAN_MARK_ADD`, `FAN_OPEN_PERM`) sur chaque cible configurée.\n"
    "  • Retour      : Descripteur de fichier `fan_fd` valide, ou `-1` en cas d'erreur de privilèges.\n"
    },
    {
        17,
        113,
        "Fonction 113 : sentry_engine.c -- sentry_listen_loop -- Boucle de veille passive bloquante",
    "Fonction 113 : sentry_engine.c -- sentry_listen_loop -- Boucle de veille passive bloquante\n"
    "  • Signature C : int sentry_listen_loop(int fanotify_fd, const SentryConfig *config);\n"
    "  • Utilité     : Assure le guet passif bloquant sur `poll()` (consommation de 0% CPU au repos).\n"
    "                  Dépouille les événements noyau dans un tampon statique de 4 Ko aligné\n"
    "                  sur 8 octets (`__attribute__((aligned(8)))`) sans allocation sur le tas (`0 heap`).\n"
    "  • Retour      : `0` lors d'un arrêt nominal par signal, `-1` en cas d'erreur de lecture.\n"
    },
    {
        17,
        114,
        "Fonction 114 : sentry_engine.c -- sentry_trigger_reflex -- Riposte cinétique synchrone",
    "Fonction 114 : sentry_engine.c -- sentry_trigger_reflex -- Riposte cinétique synchrone\n"
    "  • Signature C : int sentry_trigger_reflex(int fanotify_fd, int fd_event, pid_t pid);\n"
    "  • Utilité     : Applique instantanément le rejet auprès du noyau via `FAN_DENY`,\n"
    "                  ferme le descripteur d'événement et achemine le PID agresseur vers\n"
    "                  `rap_reflex_dispatch` avec le motif `TRIGGER_CANARY_BREACH`.\n"
    "  • Retour      : `0` si la neutralisation a réussi, `-1` si le PID était protégé.\n"
    },
    {
        17,
        115,
        "Fonction 115 : sentry_engine.c -- sentry_enter_stealth -- Blindage et invisibilité mémoire",
    "Fonction 115 : sentry_engine.c -- sentry_enter_stealth -- Blindage et invisibilité mémoire\n"
    "  • Signature C : int sentry_enter_stealth(const char *fake_name);\n"
    "  • Utilité     : Masque le nom du processus dans la table noyau via `prctl(PR_SET_NAME, \"[kworker/u16:3]\")`,\n"
    "                  s'attribue l'immunité OOM maximale (`/proc/self/oom_score_adj = -1000`) et\n"
    "                  verrouille l'intégralité des pages physiques en RAM via `mlockall(MCL_CURRENT | MCL_FUTURE)`.\n"
    "  • Retour      : `0` en cas de succès opérationnel, `-1` si les limites système restreignent le verrouillage.\n"
    },
    {
        17,
        116,
        "Fonction 116 : cmd_sentry.c -- cmd_sentry_arm -- Armement des leurres sur disque",
    "Fonction 116 : cmd_sentry.c -- cmd_sentry_arm -- Armement des leurres sur disque\n"
    "  • Signature C : static int cmd_sentry_arm(int argc, char **argv);\n"
    "  • Utilité     : Déploie les fichiers canaris dans les cibles spécifiées en ligne de\n"
    "                  commande ou applique les répertoires sensibles par défaut (`/home`, `/var/www`, `/var/lib`, `/etc`).\n"
    "  • Retour      : `0` (succès) si au moins un appât est opérationnel, `2` en cas d'échec global.\n"
    },
    {
        17,
        117,
        "Fonction 117 : cmd_sentry.c -- cmd_sentry_disarm -- Purge et désarmement préventif",
    "Fonction 117 : cmd_sentry.c -- cmd_sentry_disarm -- Purge et désarmement préventif\n"
    "  • Signature C : static int cmd_sentry_disarm(int argc, char **argv);\n"
    "  • Utilité     : Supprime proprement par déliaison physique (`unlink`) les fichiers\n"
    "                  leurres pour autoriser les opérations de maintenance sans fausse alerte.\n"
    "  • Retour      : Toujours `0` après affichage du bilan de neutralisation.\n"
    },
    {
        17,
        118,
        "Fonction 118 : cmd_sentry.c -- cmd_sentry_status -- Audit d'intégrité physique des appâts",
    "Fonction 118 : cmd_sentry.c -- cmd_sentry_status -- Audit d'intégrité physique des appâts\n"
    "  • Signature C : static int cmd_sentry_status(int argc, char **argv);\n"
    "  • Utilité     : Vérifie l'existence et la stricte conformité de taille (32768 octets)\n"
    "                  des canaris sur l'ensemble des répertoires surveillés (`[ACTIF]`, `[ALTERE]`, `[NON ARME]`).\n"
    "  • Retour      : Toujours `0`.\n"
    },
    {
        17,
        119,
        "Fonction 119 : cmd_sentry.c -- cmd_sentry_run -- Démarrage du guet passif fanotify",
    "Fonction 119 : cmd_sentry.c -- cmd_sentry_run -- Démarrage du guet passif fanotify\n"
    "  • Signature C : static int cmd_sentry_run(int argc, char **argv);\n"
    "  • Utilité     : Initialise `fanotify`, prend en charge le détachement en démon d'arrière-plan\n"
    "                  (`-d` / `--daemon`) via `fork()` et `setsid()`, active l'invisibilité furtive\n"
    "                  (`sentry_enter_stealth`), redirige les flux vers `/dev/null` et lance la veille.\n"
    "  • Retour      : `0` en cas d'arrêt propre, `2` en cas de droits administrateur insuffisants.\n"
    },
    {
        17,
        120,
        "Fonction 120 : cmd_sentry.c -- cmd_sentry -- Aiguilleur CLI sentry",
    "Fonction 120 : cmd_sentry.c -- cmd_sentry -- Aiguilleur CLI sentry\n"
    "  • Signature C : int cmd_sentry(int argc, char **argv);\n"
    "  • Utilité     : Point d'entrée principal de la sous-commande `rap sentry`. Parse les\n"
    "                  arguments et route vers `arm`, `disarm`, `status`, `dump` ou `run`.\n"
    "  • Retour      : Code de retour de la sous-commande, ou `2` si la commande est inconnue.\n"
    },
    {
        17,
        121,
        "Fonction 121 : rap_main.c -- rap_cmd_sentry -- Pont déclaratif sentinelle",
    "Fonction 121 : rap_main.c -- rap_cmd_sentry -- Pont déclaratif sentinelle\n"
    "  • Signature C : int rap_cmd_sentry(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Relie la table déclarative universelle `g_commands` de RAPTOR au point\n"
    "                  d'entrée de la sentinelle anti-rançongiciel (`cmd_sentry`).\n"
    "  • Retour      : Code de retour POSIX transmis par `cmd_sentry`.\n"
    },
    {
        17,
        122,
        "Fonction 122 : cmd_sentry.c -- cmd_sentry_dump -- Restitution boîte noire médico-légale",
    "Fonction 122 : cmd_sentry.c -- cmd_sentry_dump -- Restitution boîte noire médico-légale\n"
    "  • Signature C : static int cmd_sentry_dump(int argc, char **argv);\n"
    "  • Utilité     : Extrait et déverse les événements médico-légaux conservés dans l'anneau\n"
    "                  circulaire en mémoire vive (`/dev/shm/raptor_blackbox`) avec latences nanosecondes.\n"
    "  • Retour      : `0` après affichage, `2` si la boîte noire est vide ou inaccessible.\n"
    },
    {
        18,
        0,
        "SECTION 18 : OUTILLAGE DE COMPILATION & SOURCE UNIQUE SSOT (tools/)",
    "=== SECTION 18 : OUTILLAGE DE COMPILATION & SOURCE UNIQUE SSOT (tools/) ===\n"
    "\n"
    "Outil 01 : gen_man.py -- parse_ssot_markdown -- Découpage multi-pages\n"
    "  • Signature Python : def parse_ssot_markdown(md_raw) -> dict\n"
    "  • Utilité          : Parse le fichier maître `docs/man/rap.1.md` et segmente chaque\n"
    "                       page de manuel délimitée par les balises `<!-- MAN: key | sec | titre -->`.\n"
    "\n"
    "Outil 02 : gen_man.py -- parse_functions_dict -- Segmentation du dictionnaire d'ingénierie\n"
    "  • Signature Python : def parse_functions_dict(dict_raw) -> list\n"
    "  • Utilité          : Extrait hiérarchiquement les Sections et les fonctions unitaires numérotées\n"
    "                       du dictionnaire pour alimenter la table C99 `g_rap_fn_entries[]`.\n"
    "\n"
    "Outil 03 : gen_man.py -- format_unix_header -- Bannière d'en-tête Unix\n"
    "  • Signature Python : def format_unix_header(key, sec) -> str\n"
    "  • Utilité          : Calibre et génère la bannière d'en-tête troff/groff centrée\n"
    "                       sur 70 colonnes (ex: `RAP(1)   RAPTOR DEFENSE   RAP(1)`).\n"
    "\n"
    "Outil 04 : gen_man.py -- escape_c_string -- Échappement de chaînes multilignes C99\n"
    "  • Signature Python : def escape_c_string(text) -> str\n"
    "  • Utilité          : Convertit un bloc textuel brut en constantes littérales C99 sécurisées\n"
    "                       sans débordement de pile pour les fichiers `.rodata`.\n"
    "\n"
    "Outil 05 : gen_man.py -- generate_cli_header -- Sérialisation .rodata C99\n"
    "  • Signature Python : def generate_cli_header(man_pages, fn_entries, full_dict_raw) -> str\n"
    "  • Utilité          : Génère `src/cli/rap_man_data.h` contenant les tables constantes\n"
    "                       `g_rap_man_pages[]`, `g_rap_man_data[]` et `g_rap_fn_entries[]` (zéro tas).\n"
    "\n"
    "Outil 06 : gen_man.py -- generate_js_database -- Sérialisation Web JSON/JS\n"
    "  • Signature Python : def generate_js_database(man_pages, fn_entries) -> str\n"
    "  • Utilité          : Construit la base de données JavaScript `docs/js/man_data.js`\n"
    "                       alimentant la visionneuse interactive de `docs/man.html`.\n"
    "\n"
    "Outil 07 : gen_man.py -- main -- Orchestrateur de compilation documentaire SSOT\n"
    "  • Signature Python : def main() -> None\n"
    "  • Utilité          : Point d'entrée vérifiant l'existence des sources, synchronisant le binaire C\n"
    "                       et la documentation web en une seule passe atomique.\n"
    },
    {
        18,
        1,
        "Outil 1 : gen_man.py -- parse_ssot_markdown -- Découpage multi-pages",
    "Outil 01 : gen_man.py -- parse_ssot_markdown -- Découpage multi-pages\n"
    "  • Signature Python : def parse_ssot_markdown(md_raw) -> dict\n"
    "  • Utilité          : Parse le fichier maître `docs/man/rap.1.md` et segmente chaque\n"
    "                       page de manuel délimitée par les balises `<!-- MAN: key | sec | titre -->`.\n"
    },
    {
        18,
        2,
        "Outil 2 : gen_man.py -- parse_functions_dict -- Segmentation du dictionnaire d'ingénierie",
    "Outil 02 : gen_man.py -- parse_functions_dict -- Segmentation du dictionnaire d'ingénierie\n"
    "  • Signature Python : def parse_functions_dict(dict_raw) -> list\n"
    "  • Utilité          : Extrait hiérarchiquement les Sections et les fonctions unitaires numérotées\n"
    "                       du dictionnaire pour alimenter la table C99 `g_rap_fn_entries[]`.\n"
    },
    {
        18,
        3,
        "Outil 3 : gen_man.py -- format_unix_header -- Bannière d'en-tête Unix",
    "Outil 03 : gen_man.py -- format_unix_header -- Bannière d'en-tête Unix\n"
    "  • Signature Python : def format_unix_header(key, sec) -> str\n"
    "  • Utilité          : Calibre et génère la bannière d'en-tête troff/groff centrée\n"
    "                       sur 70 colonnes (ex: `RAP(1)   RAPTOR DEFENSE   RAP(1)`).\n"
    },
    {
        18,
        4,
        "Outil 4 : gen_man.py -- escape_c_string -- Échappement de chaînes multilignes C99",
    "Outil 04 : gen_man.py -- escape_c_string -- Échappement de chaînes multilignes C99\n"
    "  • Signature Python : def escape_c_string(text) -> str\n"
    "  • Utilité          : Convertit un bloc textuel brut en constantes littérales C99 sécurisées\n"
    "                       sans débordement de pile pour les fichiers `.rodata`.\n"
    },
    {
        18,
        5,
        "Outil 5 : gen_man.py -- generate_cli_header -- Sérialisation .rodata C99",
    "Outil 05 : gen_man.py -- generate_cli_header -- Sérialisation .rodata C99\n"
    "  • Signature Python : def generate_cli_header(man_pages, fn_entries, full_dict_raw) -> str\n"
    "  • Utilité          : Génère `src/cli/rap_man_data.h` contenant les tables constantes\n"
    "                       `g_rap_man_pages[]`, `g_rap_man_data[]` et `g_rap_fn_entries[]` (zéro tas).\n"
    },
    {
        18,
        6,
        "Outil 6 : gen_man.py -- generate_js_database -- Sérialisation Web JSON/JS",
    "Outil 06 : gen_man.py -- generate_js_database -- Sérialisation Web JSON/JS\n"
    "  • Signature Python : def generate_js_database(man_pages, fn_entries) -> str\n"
    "  • Utilité          : Construit la base de données JavaScript `docs/js/man_data.js`\n"
    "                       alimentant la visionneuse interactive de `docs/man.html`.\n"
    },
    {
        18,
        7,
        "Outil 7 : gen_man.py -- main -- Orchestrateur de compilation documentaire SSOT",
    "Outil 07 : gen_man.py -- main -- Orchestrateur de compilation documentaire SSOT\n"
    "  • Signature Python : def main() -> None\n"
    "  • Utilité          : Point d'entrée vérifiant l'existence des sources, synchronisant le binaire C\n"
    "                       et la documentation web en une seule passe atomique.\n"
    },
    {
        19,
        0,
        "SECTION 19 : MOTEUR CINÉTIQUE RÉFLEXE, BOÎTE NOIRE ET SURVIE MÉMOIRE (src/core/)",
    "=== SECTION 19 : MOTEUR CINÉTIQUE RÉFLEXE, BOÎTE NOIRE ET SURVIE MÉMOIRE (src/core/) ===\n"
    "\n"
    "Fonction 127 : rap_reflex.c -- rap_reflex_init -- Initialisation du moteur réflexe\n"
    "  • Signature C : int rap_reflex_init(reflex_mode_t initial_mode);\n"
    "  • Utilité     : Initialise les structures internes de remédiation, remet à zéro les\n"
    "                  compteurs d'intervention et prépare l'accès à la boîte noire en mémoire vive.\n"
    "  • Retour      : `0` en cas de succès, `-1` en cas d'erreur d'ouverture de la boîte noire.\n"
    "\n"
    "Fonction 128 : rap_reflex.c -- rap_reflex_set_mode -- Commutation du mode d'armement\n"
    "  • Signature C : void rap_reflex_set_mode(reflex_mode_t mode);\n"
    "  • Utilité     : Modifie à chaud le comportement opérationnel du disjoncteur cinétique\n"
    "                  (`REFLEX_MODE_PASSIVE`, `REFLEX_MODE_DRYRUN`, `REFLEX_MODE_ARMED`).\n"
    "\n"
    "Fonction 129 : rap_reflex.c -- rap_reflex_dispatch -- Séquençage et frappe chirurgicale\n"
    "  • Signature C : int rap_reflex_dispatch(pid_t target_pid, reflex_trigger_t trigger);\n"
    "  • Utilité     : Exécute la riposte séquentielle déterministe en moins de 40 µs :\n"
    "                  1. Vérification des gardes-fous (PID <= 1, self, parent).\n"
    "                  2. Gel immédiat du processus et de son groupe de threads via `SIGSTOP`.\n"
    "                  3. Révocation forcée des descripteurs de sockets réseau dans `/proc/[PID]/fd`.\n"
    "                  4. Destruction physique de l'agresseur par émission de `SIGKILL`.\n"
    "                  5. Calcul de latence nanoseconde et injection dans la boîte noire RAM.\n"
    "  • Retour      : `0` si l'action a été appliquée ou simulée, `-1` en cas d'erreur.\n"
    "\n"
    "Fonction 130 : rap_reflex.c -- rap_reflex_get_stats -- Télémétrie des neutralisations\n"
    "  • Signature C : reflex_mode_t rap_reflex_get_stats(uint64_t *out_neutralized_count);\n"
    "  • Utilité     : Retourne le mode de fonctionnement actif et extrait le nombre cumulé\n"
    "                  d'anomalies abattues ou consignées depuis le démarrage.\n"
    "\n"
    "Fonction 131 : rap_blackbox.c -- rap_blackbox_open -- Initialisation anneau circulaire RAM\n"
    "  • Signature C : int rap_blackbox_open(void);\n"
    "  • Utilité     : Crée ou ouvre le segment mémoire partagé `/dev/shm/raptor_blackbox`\n"
    "                  (permissions `0666`) dimensionné pour 64 enregistrements FIFO avec\n"
    "                  verrouillage d'écriture sans concurrence.\n"
    "  • Retour      : `0` en cas d'initialisation réussie, `-1` en cas d'échec d'accès à `/dev/shm`.\n"
    "\n"
    "Fonction 132 : rap_blackbox.c -- rap_blackbox_push -- Enregistrement atomique d'incident\n"
    "  • Signature C : int rap_blackbox_push(const reflex_record_t *record);\n"
    "  • Utilité     : Inscrit un événement médico-légal complet (horodatage temps réel, PID,\n"
    "                  PPID, ligne de commande, cause, latence nanoseconde, statut de destruction)\n"
    "                  dans l'anneau circulaire avec écrasement rotatif en cas de saturation.\n"
    "  • Retour      : `0` en cas de succès, `-1` si la boîte noire n'est pas ouverte.\n"
    "\n"
    "Fonction 133 : rap_blackbox.c -- rap_blackbox_dump -- Restitution formatée boîte noire\n"
    "  • Signature C : int rap_blackbox_dump(void);\n"
    "  • Utilité     : Lit chronologiquement l'anneau circulaire en mémoire vive et affiche\n"
    "                  les fiches forensiques détaillées avec codes couleurs ANSI sans nécessiter\n"
    "                  les privilèges superutilisateur.\n"
    "  • Retour      : Nombre d'événements affichés, ou `-1` en cas d'erreur.\n"
    "\n"
    "Fonction 134 : rap_oom.c -- rap_oom_protect -- Immunité système anti-éviction OOM\n"
    "  • Signature C : int rap_oom_protect(void);\n"
    "  • Utilité     : Écrit la valeur `-1000` dans `/proc/self/oom_score_adj` pour conférer\n"
    "                  au binaire RAPTOR une immunité vitale absolue équivalente à `init` (PID 1)\n"
    "                  face au tueur de mémoire du noyau Linux (*Out-Of-Memory Killer*).\n"
    "  • Retour      : `0` en cas de succès, `-1` en cas de droits administratifs insuffisants.\n"
    "\n"
    "Fonction 135 : rap_oom.c -- rap_oom_lock_memory -- Verrouillage des pages physiques RAM\n"
    "  • Signature C : int rap_oom_lock_memory(void);\n"
    "  • Utilité     : Invoque `mlockall(MCL_CURRENT | MCL_FUTURE)` pour interdire tout swap\n"
    "                  sur disque des pages mémoires de RAPTOR, garantissant un temps de réponse\n"
    "                  sans latence de défaut de page (*page fault*) lors des frappes réflexes.\n"
    "  • Retour      : `0` en cas de verrouillage physique réussi, `-1` en cas de dépassement de quota.\n"
    },
    {
        19,
        127,
        "Fonction 127 : rap_reflex.c -- rap_reflex_init -- Initialisation du moteur réflexe",
    "Fonction 127 : rap_reflex.c -- rap_reflex_init -- Initialisation du moteur réflexe\n"
    "  • Signature C : int rap_reflex_init(reflex_mode_t initial_mode);\n"
    "  • Utilité     : Initialise les structures internes de remédiation, remet à zéro les\n"
    "                  compteurs d'intervention et prépare l'accès à la boîte noire en mémoire vive.\n"
    "  • Retour      : `0` en cas de succès, `-1` en cas d'erreur d'ouverture de la boîte noire.\n"
    },
    {
        19,
        128,
        "Fonction 128 : rap_reflex.c -- rap_reflex_set_mode -- Commutation du mode d'armement",
    "Fonction 128 : rap_reflex.c -- rap_reflex_set_mode -- Commutation du mode d'armement\n"
    "  • Signature C : void rap_reflex_set_mode(reflex_mode_t mode);\n"
    "  • Utilité     : Modifie à chaud le comportement opérationnel du disjoncteur cinétique\n"
    "                  (`REFLEX_MODE_PASSIVE`, `REFLEX_MODE_DRYRUN`, `REFLEX_MODE_ARMED`).\n"
    },
    {
        19,
        129,
        "Fonction 129 : rap_reflex.c -- rap_reflex_dispatch -- Séquençage et frappe chirurgicale",
    "Fonction 129 : rap_reflex.c -- rap_reflex_dispatch -- Séquençage et frappe chirurgicale\n"
    "  • Signature C : int rap_reflex_dispatch(pid_t target_pid, reflex_trigger_t trigger);\n"
    "  • Utilité     : Exécute la riposte séquentielle déterministe en moins de 40 µs :\n"
    "                  1. Vérification des gardes-fous (PID <= 1, self, parent).\n"
    "                  2. Gel immédiat du processus et de son groupe de threads via `SIGSTOP`.\n"
    "                  3. Révocation forcée des descripteurs de sockets réseau dans `/proc/[PID]/fd`.\n"
    "                  4. Destruction physique de l'agresseur par émission de `SIGKILL`.\n"
    "                  5. Calcul de latence nanoseconde et injection dans la boîte noire RAM.\n"
    "  • Retour      : `0` si l'action a été appliquée ou simulée, `-1` en cas d'erreur.\n"
    },
    {
        19,
        130,
        "Fonction 130 : rap_reflex.c -- rap_reflex_get_stats -- Télémétrie des neutralisations",
    "Fonction 130 : rap_reflex.c -- rap_reflex_get_stats -- Télémétrie des neutralisations\n"
    "  • Signature C : reflex_mode_t rap_reflex_get_stats(uint64_t *out_neutralized_count);\n"
    "  • Utilité     : Retourne le mode de fonctionnement actif et extrait le nombre cumulé\n"
    "                  d'anomalies abattues ou consignées depuis le démarrage.\n"
    },
    {
        19,
        131,
        "Fonction 131 : rap_blackbox.c -- rap_blackbox_open -- Initialisation anneau circulaire RAM",
    "Fonction 131 : rap_blackbox.c -- rap_blackbox_open -- Initialisation anneau circulaire RAM\n"
    "  • Signature C : int rap_blackbox_open(void);\n"
    "  • Utilité     : Crée ou ouvre le segment mémoire partagé `/dev/shm/raptor_blackbox`\n"
    "                  (permissions `0666`) dimensionné pour 64 enregistrements FIFO avec\n"
    "                  verrouillage d'écriture sans concurrence.\n"
    "  • Retour      : `0` en cas d'initialisation réussie, `-1` en cas d'échec d'accès à `/dev/shm`.\n"
    },
    {
        19,
        132,
        "Fonction 132 : rap_blackbox.c -- rap_blackbox_push -- Enregistrement atomique d'incident",
    "Fonction 132 : rap_blackbox.c -- rap_blackbox_push -- Enregistrement atomique d'incident\n"
    "  • Signature C : int rap_blackbox_push(const reflex_record_t *record);\n"
    "  • Utilité     : Inscrit un événement médico-légal complet (horodatage temps réel, PID,\n"
    "                  PPID, ligne de commande, cause, latence nanoseconde, statut de destruction)\n"
    "                  dans l'anneau circulaire avec écrasement rotatif en cas de saturation.\n"
    "  • Retour      : `0` en cas de succès, `-1` si la boîte noire n'est pas ouverte.\n"
    },
    {
        19,
        133,
        "Fonction 133 : rap_blackbox.c -- rap_blackbox_dump -- Restitution formatée boîte noire",
    "Fonction 133 : rap_blackbox.c -- rap_blackbox_dump -- Restitution formatée boîte noire\n"
    "  • Signature C : int rap_blackbox_dump(void);\n"
    "  • Utilité     : Lit chronologiquement l'anneau circulaire en mémoire vive et affiche\n"
    "                  les fiches forensiques détaillées avec codes couleurs ANSI sans nécessiter\n"
    "                  les privilèges superutilisateur.\n"
    "  • Retour      : Nombre d'événements affichés, ou `-1` en cas d'erreur.\n"
    },
    {
        19,
        134,
        "Fonction 134 : rap_oom.c -- rap_oom_protect -- Immunité système anti-éviction OOM",
    "Fonction 134 : rap_oom.c -- rap_oom_protect -- Immunité système anti-éviction OOM\n"
    "  • Signature C : int rap_oom_protect(void);\n"
    "  • Utilité     : Écrit la valeur `-1000` dans `/proc/self/oom_score_adj` pour conférer\n"
    "                  au binaire RAPTOR une immunité vitale absolue équivalente à `init` (PID 1)\n"
    "                  face au tueur de mémoire du noyau Linux (*Out-Of-Memory Killer*).\n"
    "  • Retour      : `0` en cas de succès, `-1` en cas de droits administratifs insuffisants.\n"
    },
    {
        19,
        135,
        "Fonction 135 : rap_oom.c -- rap_oom_lock_memory -- Verrouillage des pages physiques RAM",
    "Fonction 135 : rap_oom.c -- rap_oom_lock_memory -- Verrouillage des pages physiques RAM\n"
    "  • Signature C : int rap_oom_lock_memory(void);\n"
    "  • Utilité     : Invoque `mlockall(MCL_CURRENT | MCL_FUTURE)` pour interdire tout swap\n"
    "                  sur disque des pages mémoires de RAPTOR, garantissant un temps de réponse\n"
    "                  sans latence de défaut de page (*page fault*) lors des frappes réflexes.\n"
    "  • Retour      : `0` en cas de verrouillage physique réussi, `-1` en cas de dépassement de quota.\n"
    },
    {
        20,
        0,
        "SECTION 20 : CONSOLE TACTIQUE RÉSEAU TEMPS RÉEL (NETOP) & ROUTEUR DÉCOUPLÉ (src/cli/)",
    "=== SECTION 20 : CONSOLE TACTIQUE RÉSEAU TEMPS RÉEL (NETOP) & ROUTEUR DÉCOUPLÉ (src/cli/) ===\n"
    "\n"
    "Fonction 136 : cmd_netop.c -- rap_cmd_netop -- Point d'entrée de la console netop\n"
    "  • Signature C : int rap_cmd_netop(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Point d'entrée principal de la commande `rap netop`. Initialise le contexte,\n"
    "                  parse les options de limitation de lignes (-n), active l'immunité OOM\n"
    "                  (rap_oom_protect), configure le routeur de commandes et pilote la boucle\n"
    "                  d'écoute non-bloquante STDIN via poll().\n"
    "  • Retour      : 0 (RAP_EXIT_OK) en sortie nominale, 2 (RAP_EXIT_ERROR) si STDIN n'est pas un TTY.\n"
    "\n"
    "Fonction 137 : cmd_netop.c -- netop_tui_init -- Initialisation mode brut TTY\n"
    "  • Signature C : int netop_tui_init(NetopContext *ctx);\n"
    "  • Utilité     : Sauvegarde les attributs originaux du terminal via tcgetattr, bascule\n"
    "                  en mode brut non-canonique sans écho (VMIN=0, VTIME=0), intercepte les signaux\n"
    "                  (SIGINT, SIGTERM, SIGQUIT) pour restauration d'urgence, masque le curseur\n"
    "                  (\\033[?25l) et purge l'écran (\\033[2J).\n"
    "  • Retour      : 0 en cas de succès, -1 en cas d'échec termios.\n"
    "\n"
    "Fonction 138 : cmd_netop.c -- netop_tui_render -- Moteur de rendu matriciel TUI\n"
    "  • Signature C : void netop_tui_render(NetopContext *ctx, const NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Orchestre le rendu graphique temps réel : délègue à l'overlay actif si présent,\n"
    "                  sinon projette la télémétrie supérieure, la base OSINT, le statut OOM,\n"
    "                  les modes de tri et de filtre actifs, la grille de sockets colorée par niveau\n"
    "                  de criticité (7 colonnes) et le pied de page dynamique avec purge basse (\\033[J).\n"
    "\n"
    "Fonction 139 : netop_engine.c -- netop_engine_scan -- Ingestion et confrontation passives\n"
    "  • Signature C : size_t netop_engine_scan(NetopContext *ctx, NetopEntry *out_entries, size_t max_entries);\n"
    "  • Utilité     : Lit directement /proc/net/tcp, décode les adresses et états de connexion,\n"
    "                  déclenche l'indexation ciblée des descripteurs, confronte les IP distantes\n"
    "                  à la table OSINT souveraine g_rap_threat_db, évalue les règles Net-Guard et\n"
    "                  applique les pierres tombales rémanentes.\n"
    "  • Retour      : Nombre exact de connexions ingérées et qualifiées.\n"
    "\n"
    "Fonction 140 : netop_engine.c -- netop_engine_build_inode_cache -- Résolution Inode/PID ciblée\n"
    "  • Signature C : static void netop_engine_build_inode_cache(const NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Parcourt /proc/[PID]/fd pour résoudre les descripteurs socket:[INODE]\n"
    "                  associés aux sockets actifs, extrait le nom du binaire (/proc/[PID]/comm)\n"
    "                  et alimente le cache statique 0-heap sans altération d'horodatage.\n"
    "\n"
    "Fonction 141 : netop_engine.c -- netop_engine_get_threat_count -- Dénombrement base OSINT\n"
    "  • Signature C : size_t netop_engine_get_threat_count(void);\n"
    "  • Utilité     : Restitue le volume exact de signatures d'adresses IP hostiles compilées\n"
    "                  en dur dans la section .rodata de RAPTOR.\n"
    "  • Retour      : Nombre d'enregistrements (19 867 signatures).\n"
    "\n"
    "Fonction 141b : netop_engine.c -- netop_engine_lookup_threat -- Recherche dichotomique OSINT\n"
    "  • Signature C : bool netop_engine_lookup_threat(uint32_t ip, uint8_t *out_cat, uint8_t *out_score);\n"
    "  • Utilité     : Exécute une recherche dichotomique O(log N) dans la table binaire triée\n"
    "                  g_rap_threat_db pour extraire la catégorie hostile (C2, Tor, Abus) et le score.\n"
    "  • Retour      : true si l'IP correspond à une menace répertoriée, false sinon.\n"
    "\n"
    "Fonction 142 : netop_cmd_kill.c -- netop_cmd_kill_exec -- Tir cinétique chirurgical 'k'\n"
    "  • Signature C : int netop_cmd_kill_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Valide le numéro de slot cible, contrôle les verrous d'inviolabilité absolue\n"
    "                  (PID 1, RAPTOR, shell parent), paralyse le processus via SIGSTOP puis le détruit\n"
    "                  via SIGKILL. Mesure la latence à la microseconde, consigne le slot dans le\n"
    "                  registre des tombstones (7 secondes) et actualise l'affichage en orange ambré.\n"
    "  • Retour      : 0 en cas de frappe réussie, -1 en cas de violation de sécurité ou cible invalide.\n"
    "\n"
    "Fonction 143 : netop_cmd_kill.c -- netop_cmd_kill_apply_tombstones -- Rémanence des cibles\n"
    "  • Signature C : void netop_cmd_kill_apply_tombstones(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Parcourt le registre statique des tombstones et marque comme neutralisés\n"
    "                  ([ABATTU], DISJONCTÉ, NETOP_SEV_KILLED) tous les sockets appartenant à un PID\n"
    "                  ou un endpoint abattu au cours des 7 dernières secondes.\n"
    "\n"
    "Fonction 144 : netop_router.c -- netop_router_init -- Initialisation de la table de dispatch\n"
    "  • Signature C : void netop_router_init(void);\n"
    "  • Utilité     : Réinitialise l'aiguilleur central, vide les tampons d'invites et inscrit de\n"
    "                  manière déclarative l'ensemble des greffons de commandes (k, b, s, f, i, Ctrl+B, p).\n"
    "\n"
    "Fonction 145 : netop_router.c -- netop_router_handle_key -- Aiguilleur central des frappes\n"
    "  • Signature C : void netop_router_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);\n"
    "  • Utilité     : Capture non-bloquante du flux clavier. Gère la machine à états de saisie\n"
    "                  des invites (séparateurs, virgules, minus, validation Entrée, annulation Échap),\n"
    "                  route vers les gestionnaires d'overlays si une modale est active, ou déclenche\n"
    "                  le greffon associé à la touche en mode normal.\n"
    "\n"
    "Fonction 146 : netop_router.c -- netop_router_get_footer -- Composition du pied de page\n"
    "  • Signature C : const char *netop_router_get_footer(NetopContext *ctx);\n"
    "  • Utilité     : Génère dynamiquement la ligne d'état inférieure : affiche le prompt de saisie\n"
    "                  avec curseur clignotant si une commande est en attente, les notifications\n"
    "                  médico-légales minutées, ou la liste des raccourcis enregistrés.\n"
    "\n"
    "Fonction 147 : netop_cmd_ban.c -- netop_cmd_ban_exec -- Embargo et déban multi-sélection 'b'\n"
    "  • Signature C : int netop_cmd_ban_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Parseur de sélections groupées (chiffres, virgules, espaces, signe moins pour déban,\n"
    "                  préfixe B pour index registre). Fige un snapshot du registre, déduplique les IP\n"
    "                  intra-batch, injecte la règle Netfilter (iptables -I INPUT -s IP -j DROP) avec repli\n"
    "                  sur route blackhole (ip route add blackhole), ou lève l'interdiction (iptables -D /\n"
    "                  ip route del). Mesure la latence en microsecondes et synchronise les flux.\n"
    "  • Retour      : 0 en cas de traitement nominal, -1 si privilèges root absents.\n"
    "\n"
    "Fonction 148 : netop_cmd_ban.c -- netop_cmd_ban_is_banned -- Statut d'embargo d'une IP\n"
    "  • Signature C : bool netop_cmd_ban_is_banned(const char *remote_ip);\n"
    "  • Utilité     : Interroge la table statique interne des embargos pour certifier si une adresse IP\n"
    "                  distante fait l'objet d'un verrouillage actif.\n"
    "  • Retour      : true si l'IP est sous embargo actif, false sinon.\n"
    "\n"
    "Fonction 149 : netop_cmd_ban.c -- netop_cmd_ban_toggle_list -- Bascule de la modale Ctrl+B\n"
    "  • Signature C : int netop_cmd_ban_toggle_list(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Intervertit l'affichage entre la grille réseau principale et le tableau d'embargo\n"
    "                  noyau lors de la combinaison Ctrl+B (ASCII 0x02).\n"
    "  • Retour      : Toujours 0.\n"
    "\n"
    "Fonction 150 : netop_cmd_ban.c -- netop_cmd_ban_render_list -- Rendu du registre d'embargo\n"
    "  • Signature C : bool netop_cmd_ban_render_list(NetopContext *ctx);\n"
    "  • Utilité     : Projette en plein écran la vue modale du registre des adresses bannies :\n"
    "                  index unique #B01, adresse IP, nombre de sockets impactés, durée d'activation\n"
    "                  et mécanisme noyau actif, avec effacement absolu des résidus graphiques.\n"
    "  • Retour      : true lorsque le rendu a été projeté.\n"
    "\n"
    "Fonction 151 : netop_router.c -- netop_router_render_overlay -- Délégation de vue modale\n"
    "  • Signature C : bool netop_router_render_overlay(NetopContext *ctx);\n"
    "  • Utilité     : Interroge le pointeur de fonction de l'overlay actif. S'il est non nul,\n"
    "                  exécute le rendu de la modale et suspend l'affichage de la grille principale.\n"
    "  • Retour      : true si une modale a pris la main sur l'affichage, false sinon.\n"
    "\n"
    "Fonction 152 : netop_router.c -- netop_router_set_overlay -- Gestion de transition d'écran\n"
    "  • Signature C : void netop_router_set_overlay(NetopOverlayRenderer renderer);\n"
    "  • Utilité     : Définit le gestionnaire d'affichage modal courant et purge physiquement\n"
    "                  l'écran (\\033[2J\\033[H) pour garantir une transition sans superposition de texte.\n"
    "\n"
    "Fonction 153 : netop_cmd_inspect.c -- netop_cmd_inspect_exec -- Déclencheur d'inspection 'i'\n"
    "  • Signature C : int netop_cmd_inspect_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Valide le numéro de slot sélectionné, lance la collecte des indicateurs médico-légaux\n"
    "                  via netop_cmd_inspect_collect et active la surcouche modale netop_cmd_inspect_render_modal.\n"
    "  • Retour      : 0 en cas de succès, -1 si le numéro de slot est hors plage.\n"
    "\n"
    "Fonction 154 : netop_cmd_inspect.c -- netop_cmd_inspect_collect -- Autopsie médico-légale /proc\n"
    "  • Signature C : static void netop_cmd_inspect_collect(NetopForensicRecord *f, const NetopEntry *e, int slot);\n"
    "  • Utilité     : Extrait sans allocation dynamique : le PID, le PPID (/proc/[PID]/status), le nom du\n"
    "                  processus parent, le chemin canonique de l'exécutable sur disque (readlink /proc/[PID]/exe),\n"
    "                  la ligne de commande complète (/proc/[PID]/cmdline), l'évaluation OSINT et l'état d'embargo.\n"
    "\n"
    "Fonction 155 : netop_cmd_inspect.c -- netop_cmd_inspect_detect_revshell -- Traque Reverse Shell\n"
    "  • Signature C : static void netop_cmd_inspect_detect_revshell(NetopForensicRecord *f);\n"
    "  • Utilité     : Énumère /proc/[PID]/fd/* et résout les cibles des descripteurs 0 (STDIN), 1 (STDOUT)\n"
    "                  et 2 (STDERR). Déclenche l'alerte critique REVERSE SHELL DÉTECTÉ si l'entrée et la sortie\n"
    "                  standard sont conjointement redirigées vers un socket réseau.\n"
    "\n"
    "Fonction 156 : netop_cmd_inspect.c -- netop_cmd_inspect_render_modal -- Rendu de la fiche forensic\n"
    "  • Signature C : bool netop_cmd_inspect_render_modal(NetopContext *ctx);\n"
    "  • Utilité     : Projette en plein écran la fiche d'autopsie structurée en 4 blocs : Identité & Généalogie,\n"
    "                  Ancrage Réseau & Socket, Traque des Descripteurs & Détournement, et Renseignement OSINT & Embargo.\n"
    "  • Retour      : true lorsque le rendu a été projeté.\n"
    "\n"
    "Fonction 157 : netop_cmd_inspect.c -- netop_cmd_inspect_handle_key -- Clavier contextuel en modale\n"
    "  • Signature C : bool netop_cmd_inspect_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);\n"
    "  • Utilité     : Intercepte les actions contextuelles en cours d'inspection : k (neutralise immédiatement\n"
    "                  le PID inspecté), b (bascule l'embargo Netfilter sur l'IP inspectée) ou q/Échap (fermeture de la fiche).\n"
    "  • Retour      : true si la touche a été traitée par la modale, false sinon.\n"
    "\n"
    "Fonction 158 : netop_cmd_sort.c -- netop_cmd_sort_exec -- Bascule cyclique de tri 's'\n"
    "  • Signature C : int netop_cmd_sort_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Commutation circulaire instantanée sans invite : DANGEROSITÉ -> PID -> PORT/ACTIVITÉ -> DANGEROSITÉ.\n"
    "                  Applique immédiatement l'ordonnancement et affiche une notification d'état temporaire.\n"
    "  • Retour      : Toujours 0.\n"
    "\n"
    "Fonction 159 : netop_cmd_sort.c -- netop_cmd_sort_apply -- Algorithme de tri stable 0-heap\n"
    "  • Signature C : void netop_cmd_sort_apply(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Réordonne le tableau d'entrées in-place via un algorithme de tri par insertion stable\n"
    "                  sans aucune allocation mémoire, garantissant une exécution déterministe en moins de 5 µs.\n"
    "\n"
    "Fonction 160 : netop_cmd_sort.c -- netop_cmd_sort_get_name -- Libellé du mode de tri\n"
    "  • Signature C : const char *netop_cmd_sort_get_name(void);\n"
    "  • Utilité     : Restitue la chaîne descriptive textuelle du tri actif (\"DANGEROSITÉ\", \"PID\", \"PORT/ACTIVITÉ\")\n"
    "                  pour intégration dynamique dans l'en-tête de télémétrie.\n"
    "\n"
    "Fonction 161 : netop_cmd_filter.c -- netop_cmd_filter_exec -- Bascule cyclique de filtre 'f'\n"
    "  • Signature C : int netop_cmd_filter_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Commutation circulaire instantanée des filtres de vue : TOUS -> HORS LOCAL -> ÉTABLIS ->\n"
    "                  ALERTES SEULES -> TOUS. Émet la notification d'état temporaire.\n"
    "  • Retour      : Toujours 0.\n"
    "\n"
    "Fonction 162 : netop_cmd_filter.c -- netop_cmd_filter_apply -- Compaction in-place des flux\n"
    "  • Signature C : size_t netop_cmd_filter_apply(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Filtre et compacte le tableau d'entrées directement en mémoire selon les critères du mode\n"
    "                  sélectionné (masquage de 127.0.0.1/LISTEN, conservation stricte d'ESTABLISHED, ou isolation\n"
    "                  des niveaux HOSTILE/WARN/KILLED/BAN).\n"
    "  • Retour      : Nombre d'entrées visibles après filtrage.\n"
    "\n"
    "Fonction 163 : netop_cmd_filter.c -- netop_cmd_filter_get_name -- Libellé du mode de filtre\n"
    "  • Signature C : const char *netop_cmd_filter_get_name(void);\n"
    "  • Utilité     : Restitue la chaîne descriptive textuelle du filtre actif (\"TOUS\", \"HORS LOCAL\", \"ÉTABLIS\",\n"
    "                  \"ALERTES SEULES\") pour affichage dans l'en-tête de télémétrie.\n"
    "\n"
    "================================================================================\n"
    "FIN DU RÉFÉRENTIEL TECHNIQUE OFFICIEL // RAPTOR DEFENSE v2.0\n"
    "================================================================================\n"
    },
    {
        20,
        136,
        "Fonction 136 : cmd_netop.c -- rap_cmd_netop -- Point d'entrée de la console netop",
    "Fonction 136 : cmd_netop.c -- rap_cmd_netop -- Point d'entrée de la console netop\n"
    "  • Signature C : int rap_cmd_netop(int argc, char **argv, const RapConfig *cfg);\n"
    "  • Utilité     : Point d'entrée principal de la commande `rap netop`. Initialise le contexte,\n"
    "                  parse les options de limitation de lignes (-n), active l'immunité OOM\n"
    "                  (rap_oom_protect), configure le routeur de commandes et pilote la boucle\n"
    "                  d'écoute non-bloquante STDIN via poll().\n"
    "  • Retour      : 0 (RAP_EXIT_OK) en sortie nominale, 2 (RAP_EXIT_ERROR) si STDIN n'est pas un TTY.\n"
    },
    {
        20,
        137,
        "Fonction 137 : cmd_netop.c -- netop_tui_init -- Initialisation mode brut TTY",
    "Fonction 137 : cmd_netop.c -- netop_tui_init -- Initialisation mode brut TTY\n"
    "  • Signature C : int netop_tui_init(NetopContext *ctx);\n"
    "  • Utilité     : Sauvegarde les attributs originaux du terminal via tcgetattr, bascule\n"
    "                  en mode brut non-canonique sans écho (VMIN=0, VTIME=0), intercepte les signaux\n"
    "                  (SIGINT, SIGTERM, SIGQUIT) pour restauration d'urgence, masque le curseur\n"
    "                  (\\033[?25l) et purge l'écran (\\033[2J).\n"
    "  • Retour      : 0 en cas de succès, -1 en cas d'échec termios.\n"
    },
    {
        20,
        138,
        "Fonction 138 : cmd_netop.c -- netop_tui_render -- Moteur de rendu matriciel TUI",
    "Fonction 138 : cmd_netop.c -- netop_tui_render -- Moteur de rendu matriciel TUI\n"
    "  • Signature C : void netop_tui_render(NetopContext *ctx, const NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Orchestre le rendu graphique temps réel : délègue à l'overlay actif si présent,\n"
    "                  sinon projette la télémétrie supérieure, la base OSINT, le statut OOM,\n"
    "                  les modes de tri et de filtre actifs, la grille de sockets colorée par niveau\n"
    "                  de criticité (7 colonnes) et le pied de page dynamique avec purge basse (\\033[J).\n"
    },
    {
        20,
        139,
        "Fonction 139 : netop_engine.c -- netop_engine_scan -- Ingestion et confrontation passives",
    "Fonction 139 : netop_engine.c -- netop_engine_scan -- Ingestion et confrontation passives\n"
    "  • Signature C : size_t netop_engine_scan(NetopContext *ctx, NetopEntry *out_entries, size_t max_entries);\n"
    "  • Utilité     : Lit directement /proc/net/tcp, décode les adresses et états de connexion,\n"
    "                  déclenche l'indexation ciblée des descripteurs, confronte les IP distantes\n"
    "                  à la table OSINT souveraine g_rap_threat_db, évalue les règles Net-Guard et\n"
    "                  applique les pierres tombales rémanentes.\n"
    "  • Retour      : Nombre exact de connexions ingérées et qualifiées.\n"
    },
    {
        20,
        140,
        "Fonction 140 : netop_engine.c -- netop_engine_build_inode_cache -- Résolution Inode/PID ciblée",
    "Fonction 140 : netop_engine.c -- netop_engine_build_inode_cache -- Résolution Inode/PID ciblée\n"
    "  • Signature C : static void netop_engine_build_inode_cache(const NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Parcourt /proc/[PID]/fd pour résoudre les descripteurs socket:[INODE]\n"
    "                  associés aux sockets actifs, extrait le nom du binaire (/proc/[PID]/comm)\n"
    "                  et alimente le cache statique 0-heap sans altération d'horodatage.\n"
    },
    {
        20,
        141,
        "Fonction 141 : netop_engine.c -- netop_engine_get_threat_count -- Dénombrement base OSINT",
    "Fonction 141 : netop_engine.c -- netop_engine_get_threat_count -- Dénombrement base OSINT\n"
    "  • Signature C : size_t netop_engine_get_threat_count(void);\n"
    "  • Utilité     : Restitue le volume exact de signatures d'adresses IP hostiles compilées\n"
    "                  en dur dans la section .rodata de RAPTOR.\n"
    "  • Retour      : Nombre d'enregistrements (19 867 signatures).\n"
    "\n"
    "Fonction 141b : netop_engine.c -- netop_engine_lookup_threat -- Recherche dichotomique OSINT\n"
    "  • Signature C : bool netop_engine_lookup_threat(uint32_t ip, uint8_t *out_cat, uint8_t *out_score);\n"
    "  • Utilité     : Exécute une recherche dichotomique O(log N) dans la table binaire triée\n"
    "                  g_rap_threat_db pour extraire la catégorie hostile (C2, Tor, Abus) et le score.\n"
    "  • Retour      : true si l'IP correspond à une menace répertoriée, false sinon.\n"
    },
    {
        20,
        142,
        "Fonction 142 : netop_cmd_kill.c -- netop_cmd_kill_exec -- Tir cinétique chirurgical 'k'",
    "Fonction 142 : netop_cmd_kill.c -- netop_cmd_kill_exec -- Tir cinétique chirurgical 'k'\n"
    "  • Signature C : int netop_cmd_kill_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Valide le numéro de slot cible, contrôle les verrous d'inviolabilité absolue\n"
    "                  (PID 1, RAPTOR, shell parent), paralyse le processus via SIGSTOP puis le détruit\n"
    "                  via SIGKILL. Mesure la latence à la microseconde, consigne le slot dans le\n"
    "                  registre des tombstones (7 secondes) et actualise l'affichage en orange ambré.\n"
    "  • Retour      : 0 en cas de frappe réussie, -1 en cas de violation de sécurité ou cible invalide.\n"
    },
    {
        20,
        143,
        "Fonction 143 : netop_cmd_kill.c -- netop_cmd_kill_apply_tombstones -- Rémanence des cibles",
    "Fonction 143 : netop_cmd_kill.c -- netop_cmd_kill_apply_tombstones -- Rémanence des cibles\n"
    "  • Signature C : void netop_cmd_kill_apply_tombstones(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Parcourt le registre statique des tombstones et marque comme neutralisés\n"
    "                  ([ABATTU], DISJONCTÉ, NETOP_SEV_KILLED) tous les sockets appartenant à un PID\n"
    "                  ou un endpoint abattu au cours des 7 dernières secondes.\n"
    },
    {
        20,
        144,
        "Fonction 144 : netop_router.c -- netop_router_init -- Initialisation de la table de dispatch",
    "Fonction 144 : netop_router.c -- netop_router_init -- Initialisation de la table de dispatch\n"
    "  • Signature C : void netop_router_init(void);\n"
    "  • Utilité     : Réinitialise l'aiguilleur central, vide les tampons d'invites et inscrit de\n"
    "                  manière déclarative l'ensemble des greffons de commandes (k, b, s, f, i, Ctrl+B, p).\n"
    },
    {
        20,
        145,
        "Fonction 145 : netop_router.c -- netop_router_handle_key -- Aiguilleur central des frappes",
    "Fonction 145 : netop_router.c -- netop_router_handle_key -- Aiguilleur central des frappes\n"
    "  • Signature C : void netop_router_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);\n"
    "  • Utilité     : Capture non-bloquante du flux clavier. Gère la machine à états de saisie\n"
    "                  des invites (séparateurs, virgules, minus, validation Entrée, annulation Échap),\n"
    "                  route vers les gestionnaires d'overlays si une modale est active, ou déclenche\n"
    "                  le greffon associé à la touche en mode normal.\n"
    },
    {
        20,
        146,
        "Fonction 146 : netop_router.c -- netop_router_get_footer -- Composition du pied de page",
    "Fonction 146 : netop_router.c -- netop_router_get_footer -- Composition du pied de page\n"
    "  • Signature C : const char *netop_router_get_footer(NetopContext *ctx);\n"
    "  • Utilité     : Génère dynamiquement la ligne d'état inférieure : affiche le prompt de saisie\n"
    "                  avec curseur clignotant si une commande est en attente, les notifications\n"
    "                  médico-légales minutées, ou la liste des raccourcis enregistrés.\n"
    },
    {
        20,
        147,
        "Fonction 147 : netop_cmd_ban.c -- netop_cmd_ban_exec -- Embargo et déban multi-sélection 'b'",
    "Fonction 147 : netop_cmd_ban.c -- netop_cmd_ban_exec -- Embargo et déban multi-sélection 'b'\n"
    "  • Signature C : int netop_cmd_ban_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Parseur de sélections groupées (chiffres, virgules, espaces, signe moins pour déban,\n"
    "                  préfixe B pour index registre). Fige un snapshot du registre, déduplique les IP\n"
    "                  intra-batch, injecte la règle Netfilter (iptables -I INPUT -s IP -j DROP) avec repli\n"
    "                  sur route blackhole (ip route add blackhole), ou lève l'interdiction (iptables -D /\n"
    "                  ip route del). Mesure la latence en microsecondes et synchronise les flux.\n"
    "  • Retour      : 0 en cas de traitement nominal, -1 si privilèges root absents.\n"
    },
    {
        20,
        148,
        "Fonction 148 : netop_cmd_ban.c -- netop_cmd_ban_is_banned -- Statut d'embargo d'une IP",
    "Fonction 148 : netop_cmd_ban.c -- netop_cmd_ban_is_banned -- Statut d'embargo d'une IP\n"
    "  • Signature C : bool netop_cmd_ban_is_banned(const char *remote_ip);\n"
    "  • Utilité     : Interroge la table statique interne des embargos pour certifier si une adresse IP\n"
    "                  distante fait l'objet d'un verrouillage actif.\n"
    "  • Retour      : true si l'IP est sous embargo actif, false sinon.\n"
    },
    {
        20,
        149,
        "Fonction 149 : netop_cmd_ban.c -- netop_cmd_ban_toggle_list -- Bascule de la modale Ctrl+B",
    "Fonction 149 : netop_cmd_ban.c -- netop_cmd_ban_toggle_list -- Bascule de la modale Ctrl+B\n"
    "  • Signature C : int netop_cmd_ban_toggle_list(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Intervertit l'affichage entre la grille réseau principale et le tableau d'embargo\n"
    "                  noyau lors de la combinaison Ctrl+B (ASCII 0x02).\n"
    "  • Retour      : Toujours 0.\n"
    },
    {
        20,
        150,
        "Fonction 150 : netop_cmd_ban.c -- netop_cmd_ban_render_list -- Rendu du registre d'embargo",
    "Fonction 150 : netop_cmd_ban.c -- netop_cmd_ban_render_list -- Rendu du registre d'embargo\n"
    "  • Signature C : bool netop_cmd_ban_render_list(NetopContext *ctx);\n"
    "  • Utilité     : Projette en plein écran la vue modale du registre des adresses bannies :\n"
    "                  index unique #B01, adresse IP, nombre de sockets impactés, durée d'activation\n"
    "                  et mécanisme noyau actif, avec effacement absolu des résidus graphiques.\n"
    "  • Retour      : true lorsque le rendu a été projeté.\n"
    },
    {
        20,
        151,
        "Fonction 151 : netop_router.c -- netop_router_render_overlay -- Délégation de vue modale",
    "Fonction 151 : netop_router.c -- netop_router_render_overlay -- Délégation de vue modale\n"
    "  • Signature C : bool netop_router_render_overlay(NetopContext *ctx);\n"
    "  • Utilité     : Interroge le pointeur de fonction de l'overlay actif. S'il est non nul,\n"
    "                  exécute le rendu de la modale et suspend l'affichage de la grille principale.\n"
    "  • Retour      : true si une modale a pris la main sur l'affichage, false sinon.\n"
    },
    {
        20,
        152,
        "Fonction 152 : netop_router.c -- netop_router_set_overlay -- Gestion de transition d'écran",
    "Fonction 152 : netop_router.c -- netop_router_set_overlay -- Gestion de transition d'écran\n"
    "  • Signature C : void netop_router_set_overlay(NetopOverlayRenderer renderer);\n"
    "  • Utilité     : Définit le gestionnaire d'affichage modal courant et purge physiquement\n"
    "                  l'écran (\\033[2J\\033[H) pour garantir une transition sans superposition de texte.\n"
    },
    {
        20,
        153,
        "Fonction 153 : netop_cmd_inspect.c -- netop_cmd_inspect_exec -- Déclencheur d'inspection 'i'",
    "Fonction 153 : netop_cmd_inspect.c -- netop_cmd_inspect_exec -- Déclencheur d'inspection 'i'\n"
    "  • Signature C : int netop_cmd_inspect_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Valide le numéro de slot sélectionné, lance la collecte des indicateurs médico-légaux\n"
    "                  via netop_cmd_inspect_collect et active la surcouche modale netop_cmd_inspect_render_modal.\n"
    "  • Retour      : 0 en cas de succès, -1 si le numéro de slot est hors plage.\n"
    },
    {
        20,
        154,
        "Fonction 154 : netop_cmd_inspect.c -- netop_cmd_inspect_collect -- Autopsie médico-légale /proc",
    "Fonction 154 : netop_cmd_inspect.c -- netop_cmd_inspect_collect -- Autopsie médico-légale /proc\n"
    "  • Signature C : static void netop_cmd_inspect_collect(NetopForensicRecord *f, const NetopEntry *e, int slot);\n"
    "  • Utilité     : Extrait sans allocation dynamique : le PID, le PPID (/proc/[PID]/status), le nom du\n"
    "                  processus parent, le chemin canonique de l'exécutable sur disque (readlink /proc/[PID]/exe),\n"
    "                  la ligne de commande complète (/proc/[PID]/cmdline), l'évaluation OSINT et l'état d'embargo.\n"
    },
    {
        20,
        155,
        "Fonction 155 : netop_cmd_inspect.c -- netop_cmd_inspect_detect_revshell -- Traque Reverse Shell",
    "Fonction 155 : netop_cmd_inspect.c -- netop_cmd_inspect_detect_revshell -- Traque Reverse Shell\n"
    "  • Signature C : static void netop_cmd_inspect_detect_revshell(NetopForensicRecord *f);\n"
    "  • Utilité     : Énumère /proc/[PID]/fd/* et résout les cibles des descripteurs 0 (STDIN), 1 (STDOUT)\n"
    "                  et 2 (STDERR). Déclenche l'alerte critique REVERSE SHELL DÉTECTÉ si l'entrée et la sortie\n"
    "                  standard sont conjointement redirigées vers un socket réseau.\n"
    },
    {
        20,
        156,
        "Fonction 156 : netop_cmd_inspect.c -- netop_cmd_inspect_render_modal -- Rendu de la fiche forensic",
    "Fonction 156 : netop_cmd_inspect.c -- netop_cmd_inspect_render_modal -- Rendu de la fiche forensic\n"
    "  • Signature C : bool netop_cmd_inspect_render_modal(NetopContext *ctx);\n"
    "  • Utilité     : Projette en plein écran la fiche d'autopsie structurée en 4 blocs : Identité & Généalogie,\n"
    "                  Ancrage Réseau & Socket, Traque des Descripteurs & Détournement, et Renseignement OSINT & Embargo.\n"
    "  • Retour      : true lorsque le rendu a été projeté.\n"
    },
    {
        20,
        157,
        "Fonction 157 : netop_cmd_inspect.c -- netop_cmd_inspect_handle_key -- Clavier contextuel en modale",
    "Fonction 157 : netop_cmd_inspect.c -- netop_cmd_inspect_handle_key -- Clavier contextuel en modale\n"
    "  • Signature C : bool netop_cmd_inspect_handle_key(NetopContext *ctx, NetopEntry *entries, size_t count, char ch);\n"
    "  • Utilité     : Intercepte les actions contextuelles en cours d'inspection : k (neutralise immédiatement\n"
    "                  le PID inspecté), b (bascule l'embargo Netfilter sur l'IP inspectée) ou q/Échap (fermeture de la fiche).\n"
    "  • Retour      : true si la touche a été traitée par la modale, false sinon.\n"
    },
    {
        20,
        158,
        "Fonction 158 : netop_cmd_sort.c -- netop_cmd_sort_exec -- Bascule cyclique de tri 's'",
    "Fonction 158 : netop_cmd_sort.c -- netop_cmd_sort_exec -- Bascule cyclique de tri 's'\n"
    "  • Signature C : int netop_cmd_sort_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Commutation circulaire instantanée sans invite : DANGEROSITÉ -> PID -> PORT/ACTIVITÉ -> DANGEROSITÉ.\n"
    "                  Applique immédiatement l'ordonnancement et affiche une notification d'état temporaire.\n"
    "  • Retour      : Toujours 0.\n"
    },
    {
        20,
        159,
        "Fonction 159 : netop_cmd_sort.c -- netop_cmd_sort_apply -- Algorithme de tri stable 0-heap",
    "Fonction 159 : netop_cmd_sort.c -- netop_cmd_sort_apply -- Algorithme de tri stable 0-heap\n"
    "  • Signature C : void netop_cmd_sort_apply(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Réordonne le tableau d'entrées in-place via un algorithme de tri par insertion stable\n"
    "                  sans aucune allocation mémoire, garantissant une exécution déterministe en moins de 5 µs.\n"
    },
    {
        20,
        160,
        "Fonction 160 : netop_cmd_sort.c -- netop_cmd_sort_get_name -- Libellé du mode de tri",
    "Fonction 160 : netop_cmd_sort.c -- netop_cmd_sort_get_name -- Libellé du mode de tri\n"
    "  • Signature C : const char *netop_cmd_sort_get_name(void);\n"
    "  • Utilité     : Restitue la chaîne descriptive textuelle du tri actif (\"DANGEROSITÉ\", \"PID\", \"PORT/ACTIVITÉ\")\n"
    "                  pour intégration dynamique dans l'en-tête de télémétrie.\n"
    },
    {
        20,
        161,
        "Fonction 161 : netop_cmd_filter.c -- netop_cmd_filter_exec -- Bascule cyclique de filtre 'f'",
    "Fonction 161 : netop_cmd_filter.c -- netop_cmd_filter_exec -- Bascule cyclique de filtre 'f'\n"
    "  • Signature C : int netop_cmd_filter_exec(NetopContext *ctx, NetopEntry *entries, size_t count, const char *arg);\n"
    "  • Utilité     : Commutation circulaire instantanée des filtres de vue : TOUS -> HORS LOCAL -> ÉTABLIS ->\n"
    "                  ALERTES SEULES -> TOUS. Émet la notification d'état temporaire.\n"
    "  • Retour      : Toujours 0.\n"
    },
    {
        20,
        162,
        "Fonction 162 : netop_cmd_filter.c -- netop_cmd_filter_apply -- Compaction in-place des flux",
    "Fonction 162 : netop_cmd_filter.c -- netop_cmd_filter_apply -- Compaction in-place des flux\n"
    "  • Signature C : size_t netop_cmd_filter_apply(NetopEntry *entries, size_t count);\n"
    "  • Utilité     : Filtre et compacte le tableau d'entrées directement en mémoire selon les critères du mode\n"
    "                  sélectionné (masquage de 127.0.0.1/LISTEN, conservation stricte d'ESTABLISHED, ou isolation\n"
    "                  des niveaux HOSTILE/WARN/KILLED/BAN).\n"
    "  • Retour      : Nombre d'entrées visibles après filtrage.\n"
    },
    {
        20,
        163,
        "Fonction 163 : netop_cmd_filter.c -- netop_cmd_filter_get_name -- Libellé du mode de filtre",
    "Fonction 163 : netop_cmd_filter.c -- netop_cmd_filter_get_name -- Libellé du mode de filtre\n"
    "  • Signature C : const char *netop_cmd_filter_get_name(void);\n"
    "  • Utilité     : Restitue la chaîne descriptive textuelle du filtre actif (\"TOUS\", \"HORS LOCAL\", \"ÉTABLIS\",\n"
    "                  \"ALERTES SEULES\") pour affichage dans l'en-tête de télémétrie.\n"
    "\n"
    "================================================================================\n"
    "FIN DU RÉFÉRENTIEL TECHNIQUE OFFICIEL // RAPTOR DEFENSE v2.0\n"
    "================================================================================\n"
    },
};

static const size_t g_rap_fn_entries_count = sizeof(g_rap_fn_entries) / sizeof(g_rap_fn_entries[0]);

#endif /* RAP_MAN_DATA_H */
