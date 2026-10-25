<!-- MAN: rap | 1 | RAP(1) — Outils Généraux -->
NAME
     rap — Console chirurgicale et trousse de survie défensive pour Linux

SYNOPSIS
     rap [-q] [-r] [-f] [-v] [-n nombre] commande [arguments...]

DESCRIPTION
     rap est l'utilitaire autonome en ligne de commande de RAPTOR v2.0.
     Conçu selon la norme C99 pour fonctionner sous famine mémoire extrême
     (< 2 Mo RAM) ou en cas d'effondrement du serveur d'affichage (X11/Wayland),
     il audite passivement les structures du noyau (/proc) et neutralise les menaces.

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

     -n nombre
             Limite le nombre de lignes projetées (applicable à top et net).

COMMANDES
     status, st   Bilan vital instantané de la machine hôte.
     top          Top des processus consommateurs de mémoire vive résidente.
     hunt         Audit combiné universel (Mémoire, Preload, Heuristiques).
     rx           Traque des pages mémoires anonymes exécutables (Inode 0).
     preload      Audit d'intégrité du chargeur dynamique (/etc/ld.so.preload).
     heur, he     Analyse heuristique (binaires supprimés, chemins volatils).
     net          Audit et qualification des sockets TCP (Net-Guard).
     sentry, se   Sentinelle passive anti-rançongiciel (canaris & fanotify).
     kill <PID>   Neutralisation immédiate par émission du signal SIGKILL.
     freeze <PID> Gel conservatoire via signal SIGSTOP pour examen forensique.
     man [cmd]    Manuel de référence autonome embarqué.

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
     rap hunt — Audit de sécurité universel combiné

SYNOPSIS
     rap hunt [-q] [-r] [-v]

DESCRIPTION
     Coordonne séquentiellement les trois grandes sondes d'investigation :
       1. Traque des mutations mémoires anonymes (rap rx).
       2. Traque des détournements LD_PRELOAD (rap preload).
       3. Analyse heuristique d'exécution (rap heur).

     Conçu pour être invoqué dans les scripts de surveillance cron ou par
     le centre opérationnel de sécurité (SOC).

EXIT STATUS
     0    Aucune anomalie détectée sur les trois vecteurs.
     1    Au moins une anomalie hostile détectée sur l'un des contrôles.

<!-- MAN: rap-rx | 1 | RAP-RX(1) — Traque Mémoire Inode 0 -->
NAME
     rap rx — Traque des pages mémoires anonymes exécutables (Inode 0)

SYNOPSIS
     rap rx [-q] [-r] [-v]

DESCRIPTION
     Parcourt /proc/[PID]/maps pour identifier les régions mémoires marquées r-xp
     dont l'Inode est égal à 0 (aucun fichier adossé sur disque).
     
     Par défaut, filtre automatiquement le bruit des moteurs JIT (Chromium, Node)
     et des liaisons libffi (Cinnamon, GTK) selon assets/rules/mem_whitelist.conf.
     L'option -v permet de révéler les allocations légitimes approuvées.

EXAMPLES
     Abattre automatiquement tout processus inconnu présentant une injection r-xp :
     rap rx -q | awk '{print $1}' | xargs -r rap kill -f

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

<!-- MAN: rap-net | 1 | RAP-NET(1) — Sonde Réseau Net-Guard -->
NAME
     rap net — Audit des communications TCP actives et qualification Net-Guard

SYNOPSIS
     rap net [-q] [-r] [-n nombre]

DESCRIPTION
     Décode directement les sockets à l'état TCP_ESTABLISHED dans /proc/net/tcp.
     Chaque flux distant est évalué mathématiquement par calcul de masque CIDR
     contre les registres fw_officiel.conf et fw_custom.conf :
       - APPROUVÉ : Segments privés RFC 1918 (192.168.0.0/16, etc.) ou loopback.
       - EXTERNE  : Ports standards sécurisés (443 HTTPS, 80 HTTP, 53 DNS).
       - SUSPECT  : Ports distants exotiques non homologués.
       - [HOSTILE]: Adresses de relais C2 ou nœuds d'attaque répertoriés.

EXIT STATUS
     0    Trafic nominal ou absence de flux hostile.
     1    Au moins une connexion vers un endpoint HOSTILE identifiée.

<!-- MAN: rap-sentry | 1 | RAP-SENTRY(1) — Sentinelle Anti-Rançongiciel -->
NAME
     rap sentry — Sentinelle passive anti-rançongiciel (canaris & fanotify)

SYNOPSIS
     rap sentry <arm | disarm | status | run [-d]> [chemins...]
     rap se <arm | disarm | status | run [-d]> [chemins...]

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
             d'armement des canaris sur les cibles configurées.

     run [-d] [chemins...]
             Enclenche le guet passif noyau FAN_OPEN_PERM.
             - Sans option : Mode console direct avec alertes en temps réel.
             - Option -d   : Détachement en démon d'arrière-plan autonome (fork/setsid),
                             camouflage prctl ([kworker/u16:3]), verrouillage mémoire
                             physique (mlockall) et immunité OOM (/proc/self/oom_score_adj = -1000).

MÉCANISME DE RIPOSTE CINÉTIQUE
     1. Un processus hostile tente d'ouvrir le fichier leurre (!00_raptor_vault.kdbx).
     2. Le noyau Linux suspend immédiatement l'agresseur dans son appel open().
     3. Contrôle des gardes-fous autonomes (PID <= 1, sentinelle, shell parent).
     4. Gel immédiat du processus pirate par émission du signal SIGSTOP (état T).
     5. Rejet catégorique de l'ouverture auprès du noyau via FAN_DENY.
     6. Alerte rouge générée avec PID pour analyse (rap hunt) ou destruction (rap kill).

EXIT STATUS
     0 (RAP_EXIT_OK)      Guet nominal interrompu proprement par signal.
     1 (RAP_EXIT_THREAT)  Atteinte à un canari détectée et gel conservatoire appliqué.
     2 (RAP_EXIT_ERROR)   Droits insuffisants (CAP_SYS_ADMIN requis) ou cible invalide.

DROITS SYSTÈME REQUIS
     Requiert les privilèges root ou les capacités :
     setcap 'cap_sys_admin,cap_kill,cap_ipc_lock=+ep' /usr/local/bin/rap

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
     massifs (testé jusqu'à 25 000 lignes sous /proc/kallsyms).

CONTRÔLES CLAVIER
     Espace       Fait défiler d'une page complète vers l'avant.
     Entrée       Fait défiler d'une ligne vers l'avant.
     b            Revient d'une page vers l'arrière.
     q            Quitte le téléscripteur et restaure le terminal.

<!-- MAN: mem_whitelist | 5 | MEM_WHITELIST(5) — Format Registre Mémoire -->
NAME
     mem_whitelist.conf — Registre des processus autorisés aux pages anonymes R-X

DESCRIPTION
     Définit les exceptions légitimes pour le moteur d'audit mémoire (rap rx).
     Élimine les faux positifs causés par les compilateurs JIT (Just-In-Time)
     et les trampolines de fermeture libffi.

SYNTAXE DU FICHIER
     TYPE | MOTIF_RECHERCHE | DESCRIPTION CONTEXTUELLE

     TYPE :
       JIT   Moteurs d'exécution dynamique (navigateurs, JVM, Node.js).
       FFI   Trampolines d'interfaces graphiques (Cinnamon, GTK, Python).

EXEMPLES
     JIT | chromium         | Navigateurs basés sur Chromium / V8
     FFI | cinnamon         | Gestionnaire du bureau Cinnamon
     FFI | csd-             | Préfixe des démons de configuration Cinnamon

<!-- MAN: fw_officiel | 5 | FW_OFFICIEL(5) — Format Registre Net-Guard -->
NAME
     fw_officiel.conf — Registre officiel de souveraineté IP et Net-Guard

DESCRIPTION
     Fichier immuable de règles réseau distribué avec RAPTOR DEFENSE.
     Parse les adresses unitaires, les blocs CIDR et les noms de domaines.

SYNTAXE DES RÈGLES IP
     IP | ADRESSE_OU_CIDR | STATUT | DESCRIPTION

     STATUT :
       APPROUVÉ   Plage privée locale (RFC 1918) ou service vital autorisé.
       HOSTILE    Relais C2, scanner malveillant ou infrastructure d'attaque.

EXEMPLES
     IP | 192.168.0.0/16    | APPROUVÉ | Réseau local privé (RFC 1918)
     IP | 45.33.32.156      | HOSTILE  | Relais C2 Cobalt Strike répertorié