/* ============================================================================
 * NOM DU SCRIPT   : docs/js/man_data.js
 * RÉVISION        : Généré automatiquement par tools/gen_man.py (SSOT)
 * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel
 * RESPONSABILITÉ  : Base de données globale des man pages pour docs/man.html.
 * ============================================================================ */

window.RAP_MAN_DATABASE = {
    "rap": {
        "section": "1",
        "title": "RAP(1) — Outils Généraux",
        "header": "RAP(1)                      RAPTOR DEFENSE                      RAP(1)",
        "content": "\nNAME\n     rap — Console chirurgicale et trousse de survie défensive pour Linux\n\nSYNOPSIS\n     rap [-q] [-r] [-f] [-v] [-n nombre] commande [arguments...]\n\nDESCRIPTION\n     rap est l'utilitaire autonome en ligne de commande de RAPTOR v2.0.\n     Conçu selon la norme C99 pour fonctionner sous famine mémoire extrême\n     (< 2 Mo RAM) ou en cas d'effondrement du serveur d'affichage (X11/Wayland),\n     il audite passivement les structures du noyau (/proc) et neutralise les menaces.\n\nOPTIONS UNIVERSELLES\n     -q, --quiet\n             Mode silencieux. Supprime les en-têtes et les lignes de séparation\n             pour ne conserver que la sortie brute.\n\n     -r, --raw\n             Format machine TSV pur. Utilise des tabulations sans arrondir les\n             unités pour faciliter le traitement via awk, grep ou cut.\n\n     -f, --force\n             Désactive les demandes de confirmation interactive lors des actions\n             cinétiques (kill et freeze).\n\n     -v, --verbose\n             Mode d'audit forensique étendu. Révèle notamment les segments mémoires\n             autorisés par mem_whitelist.conf avec le tag [LISTE BLANCHE].\n\n     -n nombre\n             Limite le nombre de lignes projetées (applicable à top et net).\n\nCOMMANDES\n     status, st   Bilan vital instantané de la machine hôte.\n     top          Top des processus consommateurs de mémoire vive résidente.\n     hunt         Audit combiné universel (Mémoire, Preload, Heuristiques).\n     rx           Traque des pages mémoires anonymes exécutables (Inode 0).\n     preload      Audit d'intégrité du chargeur dynamique (/etc/ld.so.preload).\n     heur, he     Analyse heuristique (binaires supprimés, chemins volatils).\n     net          Audit et qualification des sockets TCP (Net-Guard).\n     sentry, se   Sentinelle passive anti-rançongiciel (canaris & fanotify).\n     kill <PID>   Neutralisation immédiate par émission du signal SIGKILL.\n     freeze <PID> Gel conservatoire via signal SIGSTOP pour examen forensique.\n     man [cmd]    Manuel de référence autonome embarqué.\n\nEXIT STATUS\n     0 (RAP_EXIT_OK)      Système nominal. Aucune anomalie non autorisée.\n     1 (RAP_EXIT_THREAT)  Menace critique active identifiée sur l'hôte.\n     2 (RAP_EXIT_ERROR)   Erreur de syntaxe, droits insuffisants ou cible invalide.\n\nAUTHORS\n     Conçu par l'équipe d'ingénierie RAPTOR DEFENSE.\n"
    },
    "rap-status": {
        "section": "1",
        "title": "RAP-STATUS(1) — Diagnostic Vital",
        "header": "RAP-STATUS(1)               RAPTOR DEFENSE               RAP-STATUS(1)",
        "content": "\nNAME\n     rap status — Diagnostic vital instantané de la machine hôte\n\nSYNOPSIS\n     rap status [-q] [-r]\n     rap st\n\nDESCRIPTION\n     Extrait en moins de 15 ms les métriques fondamentales de santé du système :\n     RAM totale et disponible (/proc/meminfo), dénombrement des processus\n     actifs et vérification d'altération du fichier /etc/ld.so.preload.\n\nEXIT STATUS\n     0    Système sain et intègre.\n     1    Présence suspecte du fichier /etc/ld.so.preload.\n"
    },
    "rap-top": {
        "section": "1",
        "title": "RAP-TOP(1) — Consommation RAM",
        "header": "RAP-TOP(1)                  RAPTOR DEFENSE                  RAP-TOP(1)",
        "content": "\nNAME\n     rap top — Top des processus consommateurs de mémoire vive résidente (VmRSS)\n\nSYNOPSIS\n     rap top [-q] [-r] [-n nombre]\n\nDESCRIPTION\n     Analyse les structures /proc/[PID]/status pour extraire la mémoire physique\n     réelle résidente (VmRSS). Élimine les threads noyau (RSS = 0) et classe les\n     processus par tri statique sur la pile (0 octet sur le tas).\n"
    },
    "rap-hunt": {
        "section": "1",
        "title": "RAP-HUNT(1) — Chasseur Universel",
        "header": "RAP-HUNT(1)                 RAPTOR DEFENSE                 RAP-HUNT(1)",
        "content": "\nNAME\n     rap hunt — Audit de sécurité universel combiné\n\nSYNOPSIS\n     rap hunt [-q] [-r] [-v]\n\nDESCRIPTION\n     Coordonne séquentiellement les trois grandes sondes d'investigation :\n       1. Traque des mutations mémoires anonymes (rap rx).\n       2. Traque des détournements LD_PRELOAD (rap preload).\n       3. Analyse heuristique d'exécution (rap heur).\n\n     Conçu pour être invoqué dans les scripts de surveillance cron ou par\n     le centre opérationnel de sécurité (SOC).\n\nEXIT STATUS\n     0    Aucune anomalie détectée sur les trois vecteurs.\n     1    Au moins une anomalie hostile détectée sur l'un des contrôles.\n"
    },
    "rap-rx": {
        "section": "1",
        "title": "RAP-RX(1) — Traque Mémoire Inode 0",
        "header": "RAP-RX(1)                   RAPTOR DEFENSE                   RAP-RX(1)",
        "content": "\nNAME\n     rap rx — Traque des pages mémoires anonymes exécutables (Inode 0)\n\nSYNOPSIS\n     rap rx [-q] [-r] [-v]\n\nDESCRIPTION\n     Parcourt /proc/[PID]/maps pour identifier les régions mémoires marquées r-xp\n     dont l'Inode est égal à 0 (aucun fichier adossé sur disque).\n     \n     Par défaut, filtre automatiquement le bruit des moteurs JIT (Chromium, Node)\n     et des liaisons libffi (Cinnamon, GTK) selon assets/rules/mem_whitelist.conf.\n     L'option -v permet de révéler les allocations légitimes approuvées.\n\nEXAMPLES\n     Abattre automatiquement tout processus inconnu présentant une injection r-xp :\n     rap rx -q | awk '{print $1}' | xargs -r rap kill -f\n"
    },
    "rap-preload": {
        "section": "1",
        "title": "RAP-PRELOAD(1) — Audit Chargeur Dynamique",
        "header": "RAP-PRELOAD(1)              RAPTOR DEFENSE              RAP-PRELOAD(1)",
        "content": "\nNAME\n     rap preload — Détection des rootkits userland via LD_PRELOAD\n\nSYNOPSIS\n     rap preload [-q] [-r]\n\nDESCRIPTION\n     Effectue une double vérification défensive :\n       1. Contrôle d'accès et d'intégrité de /etc/ld.so.preload.\n       2. Parcours des blocs d'environnement (/proc/[PID]/environ) délimités par\n          des octets nuls pour intercepter la variable hostile LD_PRELOAD=.\n"
    },
    "rap-heur": {
        "section": "1",
        "title": "RAP-HEUR(1) — Analyse Heuristique",
        "header": "RAP-HEUR(1)                 RAPTOR DEFENSE                 RAP-HEUR(1)",
        "content": "\nNAME\n     rap heur — Analyse heuristique des anomalies de lancement et d'usurpation\n\nSYNOPSIS\n     rap heur [-q] [-r]\n     rap he\n\nDESCRIPTION\n     Résout les liens /proc/[PID]/exe pour traquer :\n       1. Les binaires fantômes supprimés du disque en cours d'exécution (deleted).\n       2. Les processus exécutés depuis des répertoires temporaires (/tmp, /dev/shm).\n       3. L'usurpation d'identité de démons officiels (systemd, sshd, kworker).\n"
    },
    "rap-net": {
        "section": "1",
        "title": "RAP-NET(1) — Sonde Réseau Net-Guard",
        "header": "RAP-NET(1)                  RAPTOR DEFENSE                  RAP-NET(1)",
        "content": "\nNAME\n     rap net — Audit des communications TCP actives et qualification Net-Guard\n\nSYNOPSIS\n     rap net [-q] [-r] [-n nombre]\n\nDESCRIPTION\n     Décode directement les sockets à l'état TCP_ESTABLISHED dans /proc/net/tcp.\n     Chaque flux distant est évalué mathématiquement par calcul de masque CIDR\n     contre les registres fw_officiel.conf et fw_custom.conf :\n       - APPROUVÉ : Segments privés RFC 1918 (192.168.0.0/16, etc.) ou loopback.\n       - EXTERNE  : Ports standards sécurisés (443 HTTPS, 80 HTTP, 53 DNS).\n       - SUSPECT  : Ports distants exotiques non homologués.\n       - [HOSTILE]: Adresses de relais C2 ou nœuds d'attaque répertoriés.\n\nEXIT STATUS\n     0    Trafic nominal ou absence de flux hostile.\n     1    Au moins une connexion vers un endpoint HOSTILE identifiée.\n"
    },
    "rap-sentry": {
        "section": "1",
        "title": "RAP-SENTRY(1) — Sentinelle Anti-Rançongiciel",
        "header": "RAP-SENTRY(1)               RAPTOR DEFENSE               RAP-SENTRY(1)",
        "content": "\nNAME\n     rap sentry — Sentinelle passive anti-rançongiciel (canaris & fanotify)\n\nSYNOPSIS\n     rap sentry <arm | disarm | status | run [-d]> [chemins...]\n     rap se <arm | disarm | status | run [-d]> [chemins...]\n\nDESCRIPTION\n     rap sentry est un bouclier cinétique passif d'interception synchrone\n     anti-rançongiciel basé sur l'infrastructure noyau fanotify.\n     \n     Contrairement aux antivirus conventionnels qui consomment des cycles CPU\n     en scrutation continue, sentry délègue la veille passive au planificateur\n     du noyau (0% CPU au repos). L'interception intervient au niveau Ring 0 dès\n     l'instruction sys_open() sur les leurres denses.\n\nSOUS-COMMANDES\n     arm [chemins...]\n             Déploie des fichiers leurres denses (!00_raptor_vault.kdbx, 32 Ko,\n             signature KeePass KDBX4 authentique et entropie LCG). Cibles par\n             défaut : /home, /var/www, /var/lib, /etc.\n\n     disarm [chemins...]\n             Supprime proprement les fichiers canaris sans déclencher d'alerte\n             pour préparer des opérations de maintenance.\n\n     status\n             Vérifie la présence physique, la conformité de taille et l'état\n             d'armement des canaris sur les cibles configurées.\n\n     run [-d] [chemins...]\n             Enclenche le guet passif noyau FAN_OPEN_PERM.\n             - Sans option : Mode console direct avec alertes en temps réel.\n             - Option -d   : Détachement en démon d'arrière-plan autonome (fork/setsid),\n                             camouflage prctl ([kworker/u16:3]), verrouillage mémoire\n                             physique (mlockall) et immunité OOM (/proc/self/oom_score_adj = -1000).\n\nMÉCANISME DE RIPOSTE CINÉTIQUE\n     1. Un processus hostile tente d'ouvrir le fichier leurre (!00_raptor_vault.kdbx).\n     2. Le noyau Linux suspend immédiatement l'agresseur dans son appel open().\n     3. Contrôle des gardes-fous autonomes (PID <= 1, sentinelle, shell parent).\n     4. Gel immédiat du processus pirate par émission du signal SIGSTOP (état T).\n     5. Rejet catégorique de l'ouverture auprès du noyau via FAN_DENY.\n     6. Alerte rouge générée avec PID pour analyse (rap hunt) ou destruction (rap kill).\n\nEXIT STATUS\n     0 (RAP_EXIT_OK)      Guet nominal interrompu proprement par signal.\n     1 (RAP_EXIT_THREAT)  Atteinte à un canari détectée et gel conservatoire appliqué.\n     2 (RAP_EXIT_ERROR)   Droits insuffisants (CAP_SYS_ADMIN requis) ou cible invalide.\n\nDROITS SYSTÈME REQUIS\n     Requiert les privilèges root ou les capacités :\n     setcap 'cap_sys_admin,cap_kill,cap_ipc_lock=+ep' /usr/local/bin/rap\n"
    },
    "rap-kill": {
        "section": "1",
        "title": "RAP-KILL(1) — Riposte Cinétique",
        "header": "RAP-KILL(1)                 RAPTOR DEFENSE                 RAP-KILL(1)",
        "content": "\nNAME\n     rap kill — Neutralisation chirurgicale d'un processus hostile\n\nSYNOPSIS\n     rap kill [-f] <PID>\n\nDESCRIPTION\n     Transmet le signal non rattrapable SIGKILL (9) au PID spécifié.\n     \n     GARDES-FOUS INVIOLABLES :\n     Refuse systématiquement le tir si la cible est le PID 1 (init/systemd),\n     le PID de l'utilitaire, le shell parent (PPID) ou un thread interne du noyau.\n     Exige une confirmation manuelle [o/N] sauf si l'option -f est présente.\n"
    },
    "rap-freeze": {
        "section": "1",
        "title": "RAP-FREEZE(1) — Gel Conservatoire Forensique",
        "header": "RAP-FREEZE(1)               RAPTOR DEFENSE               RAP-FREEZE(1)",
        "content": "\nNAME\n     rap freeze — Gel instantané d'une menace pour investigation mémoire\n\nSYNOPSIS\n     rap freeze <PID>\n\nDESCRIPTION\n     Émet le signal SIGSTOP sur le processus cible pour le placer dans l'état\n     noyau T (Stopped). Permet de suspendre toute activité malveillante tout en\n     conservant l'intégralité de sa mémoire virtuelle intacte pour copie forensique.\n"
    },
    "rapless": {
        "section": "1",
        "title": "RAPLESS(1) — Téléscripteur Zéro-Heap",
        "header": "RAPLESS(1)                  RAPTOR DEFENSE                  RAPLESS(1)",
        "content": "\nNAME\n     rapless — Pager de survie autonome sans allocation dynamique\n\nSYNOPSIS\n     rapless [fichier]\n     commande | rapless\n\nDESCRIPTION\n     rapless est un téléscripteur d'urgence écrit en C99 n'effectuant aucun\n     appel à malloc/calloc (0 heap). Il indexe par pagination statique les flux\n     massifs (testé jusqu'à 25 000 lignes sous /proc/kallsyms).\n\nCONTRÔLES CLAVIER\n     Espace       Fait défiler d'une page complète vers l'avant.\n     Entrée       Fait défiler d'une ligne vers l'avant.\n     b            Revient d'une page vers l'arrière.\n     q            Quitte le téléscripteur et restaure le terminal.\n"
    },
    "mem_whitelist": {
        "section": "5",
        "title": "MEM_WHITELIST(5) — Format Registre Mémoire",
        "header": "MEM_WHITELIST(5)            RAPTOR DEFENSE            MEM_WHITELIST(5)",
        "content": "\nNAME\n     mem_whitelist.conf — Registre des processus autorisés aux pages anonymes R-X\n\nDESCRIPTION\n     Définit les exceptions légitimes pour le moteur d'audit mémoire (rap rx).\n     Élimine les faux positifs causés par les compilateurs JIT (Just-In-Time)\n     et les trampolines de fermeture libffi.\n\nSYNTAXE DU FICHIER\n     TYPE | MOTIF_RECHERCHE | DESCRIPTION CONTEXTUELLE\n\n     TYPE :\n       JIT   Moteurs d'exécution dynamique (navigateurs, JVM, Node.js).\n       FFI   Trampolines d'interfaces graphiques (Cinnamon, GTK, Python).\n\nEXEMPLES\n     JIT | chromium         | Navigateurs basés sur Chromium / V8\n     FFI | cinnamon         | Gestionnaire du bureau Cinnamon\n     FFI | csd-             | Préfixe des démons de configuration Cinnamon\n"
    },
    "fw_officiel": {
        "section": "5",
        "title": "FW_OFFICIEL(5) — Format Registre Net-Guard",
        "header": "FW_OFFICIEL(5)              RAPTOR DEFENSE              FW_OFFICIEL(5)",
        "content": "\nNAME\n     fw_officiel.conf — Registre officiel de souveraineté IP et Net-Guard\n\nDESCRIPTION\n     Fichier immuable de règles réseau distribué avec RAPTOR DEFENSE.\n     Parse les adresses unitaires, les blocs CIDR et les noms de domaines.\n\nSYNTAXE DES RÈGLES IP\n     IP | ADRESSE_OU_CIDR | STATUT | DESCRIPTION\n\n     STATUT :\n       APPROUVÉ   Plage privée locale (RFC 1918) ou service vital autorisé.\n       HOSTILE    Relais C2, scanner malveillant ou infrastructure d'attaque.\n\nEXEMPLES\n     IP | 192.168.0.0/16    | APPROUVÉ | Réseau local privé (RFC 1918)\n     IP | 45.33.32.156      | HOSTILE  | Relais C2 Cobalt Strike répertorié\n"
    }
};
