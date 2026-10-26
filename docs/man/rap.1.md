<!-- MAN: rap | 1 | RAP(1) — Outils Généraux -->
NAME
     rap — Console chirurgicale, traque mémoire 0-heap et remédiation active réflexe

SYNOPSIS
     rap [-q] [-r] [-f] [-v] [-a|--arm] [--dry-run] [-n nombre] commande [arguments...]

DESCRIPTION
     rap est l'utilitaire autonome en ligne de commande de RAPTOR v2.0.
     Conçu selon la norme C99 pour fonctionner sous famine mémoire extrême
     (< 2 Mo RAM) ou en cas d'effondrement du serveur d'affichage (X11/Wayland),
     il audite passivement les structures du noyau (/proc) et neutralise les menaces.

     Dans sa révision réflexe, rap intègre un disjoncteur chirurgical de dernier
     recours (Last-Resort Watchdog) capable d'opérer sous contrainte d'épuisement
     mémoire (mlockall et score OOM -1000) pour paralyser et détruire les menaces
     en moins d'une milliseconde sans dépendre du disque ni du réseau.

OPTIONS UNIVERSELLES
     -q, --quiet
             Mode silencieux. Supprime les en-têtes et les lignes de séparation
             pour ne conserver que la sortie brute.

     -r, --raw
             Format machine TSV pur. Utilise des tabulations sans arrondir les
             unités pour faciliter le traitement via awk, grep ou cut.

     -f, --force
             Désactive les demandes de confirmation interactive lors des actions
             cinétiques (kill et freeze).

     -v, --verbose
             Mode d'audit forensique étendu. Révèle notamment les segments mémoires
             autorisés par mem_whitelist.conf avec le tag [LISTE BLANCHE].

     -a, --arm
             Arme la riposte cinétique immédiate (SIGSTOP -> Révocation réseau ->
             Extraction boîte noire -> SIGKILL). Active également l'immunité OOM
             (/proc/self/oom_score_adj = -1000) et le verrouillage mémoire physique.

     --dry-run
             Mode simulation opérationnelle. Exécute l'intégralité du pipeline réflexe
             et enregistre l'incident dans la boîte noire RAM (/dev/shm/raptor_blackbox)
             sans émettre les signaux de destruction (SIGSTOP/SIGKILL).

     -n nombre
             Limite le nombre de lignes projetées (applicable à top et net).

COMMANDES
     status, st   Diagnostic vital instantané de la machine hôte.
     top          Top des processus consommateurs de mémoire vive résidente.
     hunt         Audit combiné universel (Mémoire, Preload, Heuristiques) avec riposte réflexe.
     rx           Traque des pages mémoires anonymes exécutables (Inode 0) et neutralisation.
     preload      Audit d'intégrité du chargeur dynamique (/etc/ld.so.preload).
     heur, he     Analyse heuristique (binaires supprimés, chemins volatils).
     net          Audit TCP, corrélation PID et forensic IP (Net-Guard / OSINT).
     netop, no    Console tactique temps réel matricielle (TUI), audit continu et remédiation active.
     sentry, se   Sentinelle passive anti-rançongiciel (canaris, fanotify et boîte noire).
     kill <PID>   Neutralisation immédiate par émission du signal SIGKILL.
     freeze <PID> Gel conservatoire via signal SIGSTOP pour examen forensique.
     man [cmd]    Manuel de référence autonome embarqué (SSOT).

EXIT STATUS
     0 (RAP_EXIT_OK)      Système nominal. Aucune anomalie non autorisée.
     1 (RAP_EXIT_THREAT)  Menace critique active identifiée sur l'hôte.
     2 (RAP_EXIT_ERROR)   Erreur de syntaxe, droits insuffisants ou cible invalide.

AUTHORS
     Conçu par l'équipe d'ingénierie RAPTOR DEFENSE.

<!-- MAN: rap-status | 1 | RAP-STATUS(1) — Diagnostic Vital -->
NAME
     rap status — Diagnostic vital instantané de la machine hôte

SYNOPSIS
     rap status [-q] [-r]
     rap st

DESCRIPTION
     Extrait en moins de 15 ms les métriques fondamentales de santé du système :
     RAM totale et disponible (/proc/meminfo), dénombrement des processus
     actifs et vérification d'altération du fichier /etc/ld.so.preload.

EXIT STATUS
     0    Système sain et intègre.
     1    Présence suspecte du fichier /etc/ld.so.preload.

<!-- MAN: rap-top | 1 | RAP-TOP(1) — Consommation RAM -->
NAME
     rap top — Top des processus consommateurs de mémoire vive résidente (VmRSS)

SYNOPSIS
     rap top [-q] [-r] [-n nombre]

DESCRIPTION
     Analyse les structures /proc/[PID]/status pour extraire la mémoire physique
     réelle résidente (VmRSS). Élimine les threads noyau (RSS = 0) et classe les
     processus par tri statique sur la pile (0 octet sur le tas).

<!-- MAN: rap-hunt | 1 | RAP-HUNT(1) — Chasseur Universel -->
NAME
     rap hunt — Audit de sécurité universel combiné et riposte active

SYNOPSIS
     rap hunt [-q] [-r] [-v] [-a|--arm] [--dry-run]

DESCRIPTION
     Coordonne séquentiellement les trois grandes sondes d'investigation :
       1. Traque des mutations mémoires anonymes (rap rx).
       2. Traque des détournements LD_PRELOAD (rap preload).
       3. Analyse heuristique d'exécution (rap heur).

     En mode armé (-a, --arm), toute mutation Inode 0 R-X non autorisée ou
     détournement LD_PRELOAD déclenche immédiatement le moteur de remédiation
     rap_reflex (paralysie, isolation des descripteurs de socket et purge SIGKILL).

     L'option --dry-run permet de simuler la riposte et d'alimenter la boîte noire
     médico-légale sans impacter les processus audités.

EXIT STATUS
     0    Aucune anomalie détectée sur les trois vecteurs.
     1    Au moins une anomalie hostile détectée sur l'un des contrôles.

<!-- MAN: rap-rx | 1 | RAP-RX(1) — Traque Mémoire Inode 0 -->
NAME
     rap rx — Traque des pages mémoires anonymes exécutables (Inode 0)

SYNOPSIS
     rap rx [-q] [-r] [-v] [-a|--arm] [--dry-run]

DESCRIPTION
     Parcourt /proc/[PID]/maps pour identifier les régions mémoires marquées r-xp
     dont l'Inode est égal à 0 (aucun fichier adossé sur disque).
     
     Par défaut, filtre automatiquement le bruit des moteurs JIT (Chromium, Node)
     et des liaisons libffi (Cinnamon, GTK) selon assets/rules/mem_whitelist.conf.
     L'option -v permet de révéler les allocations légitimes approuvées.

     OPTIONS DE RÉACTION :
       -a, --arm    : Neutralise sur-le-champ tout processus non approuvé via SIGKILL.
       --dry-run    : Simule la riposte et consigne l'incident dans /dev/shm/raptor_blackbox.

EXAMPLES
     Chasser les injections mémoires et abattre immédiatement les processus hostiles :
     sudo rap rx --arm

     Auditer les mutations sans altérer l'état du système :
     sudo rap rx --dry-run

<!-- MAN: rap-preload | 1 | RAP-PRELOAD(1) — Audit Chargeur Dynamique -->
NAME
     rap preload — Détection des rootkits userland via LD_PRELOAD

SYNOPSIS
     rap preload [-q] [-r]

DESCRIPTION
     Effectue une double vérification défensive :
       1. Contrôle d'accès et d'intégrité de /etc/ld.so.preload.
       2. Parcours des blocs d'environnement (/proc/[PID]/environ) délimités par
          des octets nuls pour intercepter la variable hostile LD_PRELOAD=.

<!-- MAN: rap-heur | 1 | RAP-HEUR(1) — Analyse Heuristique -->
NAME
     rap heur — Analyse heuristique des anomalies de lancement et d'usurpation

SYNOPSIS
     rap heur [-q] [-r]
     rap he

DESCRIPTION
     Résout les liens /proc/[PID]/exe pour traquer :
       1. Les binaires fantômes supprimés du disque en cours d'exécution (deleted).
       2. Les processus exécutés depuis des répertoires temporaires (/tmp, /dev/shm).
       3. L'usurpation d'identité de démons officiels (systemd, sshd, kworker).

<!-- MAN: rap-net | 1 | RAP-NET(1) — Sonde Réseau & Forensic Net-Intel -->
NAME
     rap net — Surveillance réseau, corrélation PID et forensic IP souverain

SYNOPSIS
     rap net [-q] [-r] [-n nombre]
     rap net -i <ADRESSE_IPV4>
     rap net <ADRESSE_IPV4>
     rap net -l [c2 | tor | abuse | all]

DESCRIPTION
     rap net est le module d'audit passif des communications réseau et de
     renseignement tactique sur les menaces (Threat Intelligence).
     
     Il combine l'inspection locale des descripteurs de sockets du noyau Linux
     (/proc/net/tcp et /proc/[PID]/fd/) à une base de réputation OSINT souveraine
     embarquée directement en section .rodata (zéro allocation dynamique, 0 heap).
     Toutes les vérifications s'exécutent en mémoire pure sans émettre le moindre
     paquet sur le réseau extérieur (silence radio et furtivité absolue).

MODES OPÉRATOIRES
     1. Surveillance active des flux établis :
        rap net [-q] [-r] [-n nombre]
        Énumère en temps réel les connexions TCP à l'état ESTABLISHED.
        Chaque flux est enrichi instantanément par :
          - La corrélation du socket Inode avec le PID et le binaire local (/proc/[PID]/comm).
          - La confrontation arithmétique O(log N) avec la table de menaces OSINT.
          - La qualification Net-Guard (APPROUVÉ, EXTERNE, SUSPECT, DANGER C2, ALERTE TOR).

     2. Enquête médico-légale unitaire :
        rap net -i <ADRESSE_IPV4>
        rap net <ADRESSE_IPV4>
        Édite la fiche forensique complète de l'adresse cible sans contact externe :
          - Reconnaissance immédiate des plages privées RFC 1918 et Loopback.
          - Score de menace consolidé (0 à 100) et qualification de la source hostile.
          - Détection de tout socket actif local connecté à cette IP avec chemin du binaire (/proc/[PID]/exe).
          - Génération immédiate des règles de neutralisation iptables et nftables.

     3. Inventaire et consultation de la base de signatures :
        rap net -l [c2 | tor | abuse | all]
        - Sans argument : Affiche le bilan vital de la base (date, volume, répartition).
        - Avec argument : Déverse les adresses filtrées par catégorie pour pagination
          dans le téléscripteur rapless ou exploitation dans des scripts d'administration.

CATÉGORIES OSINT INTÉGRÉES
     c2       Serveurs de commande et contrôle (C2), rançongiciels et botnets (abuse.ch Feodo).
     tor      Relais de sortie officiels du réseau d'anonymisation Tor.
     abuse    Adresses d'attaquants récurrents (scanners, brute-force SSH, fraudes IPsum).
     all      Ensemble exhaustif des signatures consolidées.

EXAMPLES
     Surveiller les flux et isoler les alertes :
     rap net

     Auditer chirurgicalement une IP suspecte repérée dans les journaux :
     rap net -i 194.26.29.112

     Inspecter les serveurs C2 actifs dans le téléscripteur zéro-heap :
     rap net -l c2 | rapless

     Extraire les IP malveillantes pour alimentation d'un pare-feu local :
     rap net -q -r -l c2 | awk '{print "iptables -A OUTPUT -d " $1 " -j DROP"}'

EXIT STATUS
     0 (RAP_EXIT_OK)      Trafic sain, IP non hostile ou bilan documentaire nominal.
     1 (RAP_EXIT_THREAT)  Connexion active vers un hôte hostile ou IP au score >= 70.
     2 (RAP_EXIT_ERROR)   Format IPv4 invalide, catégorie inconnue ou droits insuffisants.

<!-- MAN: rap-sentry | 1 | RAP-SENTRY(1) — Sentinelle Anti-Rançongiciel -->
NAME
     rap sentry — Sentinelle passive anti-rançongiciel (canaris, fanotify et boîte noire)

SYNOPSIS
     rap sentry <arm | disarm | status | dump | run [-d] [--arm|--dry-run]> [chemins...]
     rap se <arm | disarm | status | dump | run [-d] [--arm|--dry-run]> [chemins...]

DESCRIPTION
     rap sentry est un bouclier cinétique passif d'interception synchrone
     anti-rançongiciel basé sur l'infrastructure noyau fanotify.
     
     Contrairement aux antivirus conventionnels qui consomment des cycles CPU
     en scrutation continue, sentry délègue la veille passive au planificateur
     du noyau (0% CPU au repos). L'interception intervient au niveau Ring 0 dès
     l'instruction sys_open() sur les leurres denses.

SOUS-COMMANDES
     arm [chemins...]
             Déploie des fichiers leurres denses (!00_raptor_vault.kdbx, 32 Ko,
             signature KeePass KDBX4 authentique et entropie LCG). Cibles par
             défaut : /home, /var/www, /var/lib, /etc.

     disarm [chemins...]
             Supprime proprement les fichiers canaris sans déclencher d'alerte
             pour préparer des opérations de maintenance.

     status
             Vérifie la présence physique, la conformité de taille et l'état
             d'armement des canaris ainsi que les statistiques du moteur réflexe.

     dump, blackbox
             Restitue l'ensemble des enregistrements médico-légaux conservés dans
             la boîte noire circulaire en mémoire vive (/dev/shm/raptor_blackbox).
             Accessible sans privilèges root (permissions 0666).

     run [-d] [--arm|--dry-run] [chemins...]
             Enclenche le guet passif noyau FAN_OPEN_PERM.
             - Sans option : Mode passif direct (rejet FAN_DENY et alerte seule).
             - Option -a, --arm : Remédiation active (gel SIGSTOP, coupure socket, SIGKILL).
             - Option --dry-run : Simulation de neutralisation avec trace en boîte noire.
             - Option -d   : Détachement en démon d'arrière-plan autonome (fork/setsid),
                             camouflage prctl ([kworker/u16:3]), verrouillage mémoire
                             physique (mlockall) et immunité OOM (/proc/self/oom_score_adj = -1000).

MÉCANISME DE RIPOSTE RÉFLEXE
     1. Un processus hostile tente d'ouvrir le fichier leurre (!00_raptor_vault.kdbx).
     2. Le noyau Linux suspend immédiatement l'agresseur dans son appel open().
     3. Contrôle des gardes-fous autonomes (PID <= 1, sentinelle, shell parent).
     4. Rejet catégorique de l'ouverture auprès du noyau via FAN_DENY.
     5. En mode armé : paralysie immédiate du process group par SIGSTOP.
     6. Révocation forcée des descripteurs de socket (/proc/[pid]/fd).
     7. Élimination définitive de l'agresseur par émission de SIGKILL.
     8. Enregistrement médico-légal atomique dans l'anneau circulaire en RAM (/dev/shm).

EXIT STATUS
     0 (RAP_EXIT_OK)      Guet nominal interrompu proprement par signal.
     1 (RAP_EXIT_THREAT)  Atteinte à un canari détectée et riposte réflexe appliquée.
     2 (RAP_EXIT_ERROR)   Droits insuffisants (CAP_SYS_ADMIN requis) ou cible invalide.

DROITS SYSTÈME REQUIS
     Requiert les privilèges root ou les capacités :
     setcap 'cap_sys_admin,cap_kill,cap_ipc_lock,cap_sys_ptrace=+ep' /usr/local/bin/rap

<!-- MAN: rap-kill | 1 | RAP-KILL(1) — Riposte Cinétique -->
NAME
     rap kill — Neutralisation chirurgicale d'un processus hostile

SYNOPSIS
     rap kill [-f] <PID>

DESCRIPTION
     Transmet le signal non rattrapable SIGKILL (9) au PID spécifié.
     
     GARDES-FOUS INVIOLABLES :
     Refuse systématiquement le tir si la cible est le PID 1 (init/systemd),
     le PID de l'utilitaire, le shell parent (PPID) ou un thread interne du noyau.
     Exige une confirmation manuelle [o/N] sauf si l'option -f est présente.

<!-- MAN: rap-freeze | 1 | RAP-FREEZE(1) — Gel Conservatoire Forensique -->
NAME
     rap freeze — Gel instantané d'une menace pour investigation mémoire

SYNOPSIS
     rap freeze <PID>

DESCRIPTION
     Émet le signal SIGSTOP sur le processus cible pour le placer dans l'état
     noyau T (Stopped). Permet de suspendre toute activité malveillante tout en
     conservant l'intégralité de sa mémoire virtuelle intacte pour copie forensique.

<!-- MAN: rapless | 1 | RAPLESS(1) — Téléscripteur Zéro-Heap -->
NAME
     rapless — Pager de survie autonome sans allocation dynamique

SYNOPSIS
     rapless [fichier]
     commande | rapless

DESCRIPTION
     rapless est un téléscripteur d'urgence écrit en C99 n'effectuant aucun
     appel à malloc/calloc (0 heap). Il indexe par pagination statique les flux
     massifs (testé jusqu'à 25 000 lignes sous /proc/kallsyms) sans saturer la
     mémoire résidente de la machine hôte.

EXIT STATUS
     0    Fermeture nominale du visualiseur (touche 'q').
     1    Erreur d'accès au fichier source ou flux vide.

<!-- MAN: rap-netop | 1 | RAP-NETOP(1) — Console Tactique Temps Réel -->
NAME
     rap netop — Console tactique réseau temps réel matricielle, 0-heap et remédiation active

SYNOPSIS
     rap netop [-n nombre]
     rap no [-n nombre]

DESCRIPTION
     rap netop projette une console d'audit et de défense réseau active opérant
     sans aucune allocation dynamique (0-heap) et sans scintillement (flicker-free).
     
     Il ingère en flux continu les structures /proc/net/tcp du noyau Linux, résout
     les descripteurs Inode vers les PIDs et processus propriétaires, et confronte
     chaque flux distant à la base souveraine de renseignement OSINT (19 867 signatures
     actives). L'architecture repose sur un routeur de commandes découplé permettant
     la neutralisation cinétique, l'embargo réseau et l'autopsie médico-légale sans
     jamais interrompre la surveillance.

OPTIONS
     -n nombre
             Fixe le nombre maximal de lignes de sockets affichées simultanément
             sur la grille matricielle (valeur par défaut : 20, max : 256).

COMMANDES INTERACTIVES AU CLAVIER (HOTKEYS)
     k       Neutralisation réflexe chirurgicale (Kill).
             Ouvre l'invite [FRAPPE REFLEXE]. Neutralise le processus par émission
             séquentielle de SIGSTOP puis SIGKILL, mesure la latence à la microseconde
             et active une trace rémanente ambrée (tombstone) de 7 secondes.
             Gardes-fous stricts : tir proscrit sur PID 1, RAPTOR et shell parent.

     b       Embargo et réhabilitation réseau (Ban/Déban IP).
             Ouvre l'invite [BAN IP / DÉBAN]. Permet d'isoler une ou plusieurs cibles
             séparées par une virgule ou un espace (ex: b 4,6,9).
             L'injection applique une règle Netfilter (iptables -I INPUT -s IP -j DROP)
             avec repli automatique sur une route trou noir (ip route add blackhole).
             Le préfixe moins permet de lever l'embargo (ex: b -4, -6 ou b -B1).

     Ctrl+B  Registre interactif des embargos noyau.
             Bascule l'affichage vers la surcouche modale recensant l'ensemble des IP
             verrouillées (#B01, #B02...), la durée d'activation, le mécanisme actif
             et le volume de sockets associés. Le déban peut être exécuté directement
             depuis cette vue via b 1, 2 ou b -1 -2.

     s       Bascule instantanée du mode de tri (Sort Toggle).
             Alterne cycliquement l'ordonnancement des flux sans réallocation mémoire :
               1. DANGEROSITÉ : Float-to-top (KILLED > HOSTILE > WARN > ESTABLISHED > LISTEN).
               2. PID         : Ordre croissant des identifiants processus système.
               3. PORT/ACT.   : Regroupement par port distant et activité.

     f       Filtre dynamique de flux (Filter Toggle).
             Alterne instantanément le masque d'affichage par compaction in-place :
               1. TOUS         : Vue exhaustive de tous les sockets noyau.
               2. HORS LOCAL   : Masque le bruit 127.0.0.1, 0.0.0.0 et l'écoute LISTEN.
               3. ÉTABLIS      : Isole strictement les connexions connectées ESTABLISHED.
               4. ALERTES      : Cockpit de crise (HOSTILE, WARN, KILLED et BAN).

     i       Inspection médico-légale approfondie (Forensic Overlay).
             Ouvre l'invite [INSPECTION MÉDICO-LÉGALE] et projette l'autopsie du slot :
               - Généalogie complète : PID, PPID et nom du processus parent (/proc).
               - Intégrité binaire   : Ligne de commande brute et exécutable réel (/proc/[PID]/exe).
               - Anti-Reverse Shell  : Analyse des flux STDIN (fd/0), STDOUT (fd/1) et détection
                                       de détournement vers un socket réseau.
               - Renseignement OSINT : Catégorisation de la menace (C2, Tor, Abus) et score.
               - Actions directes    : Frappe 'k' ou embargo 'b' exécutables depuis la fiche.

     p, SPC  Pause / Reprise de la capture.
             Fige l'affichage du flux réseau. Les opérations de tri ('s') et de filtrage
             ('f') restent immédiatement opérationnelles sur le snapshot gelé.

     q, ESC  Fermeture de modale ou sortie propre de l'application.
             Ferme la vue modale active (Ctrl+B ou inspection), ou restaure la configuration
             du terminal (mode canonique, réaffichage du curseur) et quitte l'outil.

DROITS SYSTÈME REQUIS
     L'affichage passif et l'inspection ne requièrent que des droits utilisateurs standard.
     L'application des embargos réseau (b / iptables) et les frappes cinétiques (k / kill)
     requièrent les privilèges superutilisateur (sudo) ou les capacités Linux :
     setcap 'cap_net_admin,cap_kill,cap_sys_ptrace=+ep' /usr/local/bin/rap

EXIT STATUS
     0 (RAP_EXIT_OK)      Fermeture nominale de la console interactive.
     2 (RAP_EXIT_ERROR)   Terminal non interactif (absence de TTY) ou erreur d'initialisation.
