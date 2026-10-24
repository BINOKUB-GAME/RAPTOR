/* ============================================================================
 * NOM DU SCRIPT   : docs/js/man-engine.js
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel & Hub Développeurs
 * RESPONSABILITÉ  : Moteur autonome d'affichage et de recherche des pages de
 *                   manuel Unix (troff/groff) pour docs/man.html.
 *                   Zéro dépendance, gestion des ancres d'URL et coloration.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Base documentaire .rodata embarquée pour l'ensemble des 12 modules CLI
 *     et des registres de configuration (.conf).
 *   - Prise en charge des commandes 'man' (affichage exact) et 'apropos' (mots-clés).
 *   - Coloration syntaxique des en-têtes standard Unix (NAME, SYNOPSIS, etc.).
 *   - Gestion du bouton de copie rapide du manuel dans le presse-papier.
 *   - Synchronisation bidirectionnelle avec les hash d'URL (#rap-rx, etc.).
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *   1. man_engine_init
 *   2. man_engine_render
 *   3. man_engine_search
 *   4. man_engine_apropos
 *   5. man_engine_populate_select
 * ============================================================================
 */

(function () {
    "use strict";

    /* ========================================================================
     * BASE DE DONNÉES DES MAN PAGES // FORMAT STANDARD UNIX
     * ========================================================================
     */
    const MAN_DATABASE = {
        "rap": {
            section: "1",
            title: "RAP(1) — Outils Généraux",
            header: "RAP(1)                     RAPTOR DEFENSE                    RAP(1)",
            content: `
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
     kill <PID>   Neutralisation immédiate par émission du signal SIGKILL.
     freeze <PID> Gel conservatoire via signal SIGSTOP pour examen forensique.
     man [cmd]    Manuel de référence autonome embarqué.

EXIT STATUS
     0 (RAP_EXIT_OK)      Système nominal. Aucune anomalie non autorisée.
     1 (RAP_EXIT_THREAT)  Menace critique active identifiée sur l'hôte.
     2 (RAP_EXIT_ERROR)   Erreur de syntaxe, droits insuffisants ou cible invalide.

AUTHORS
     Conçu par l'équipe d'ingénierie RAPTOR DEFENSE.
`
        },

        "rap-status": {
            section: "1",
            title: "RAP-STATUS(1) — Diagnostic Vital",
            header: "RAP-STATUS(1)              RAPTOR DEFENSE             RAP-STATUS(1)",
            content: `
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
`
        },

        "rap-top": {
            section: "1",
            title: "RAP-TOP(1) — Consommation RAM",
            header: "RAP-TOP(1)                 RAPTOR DEFENSE                RAP-TOP(1)",
            content: `
NAME
     rap top — Top des processus consommateurs de mémoire vive résidente (VmRSS)

SYNOPSIS
     rap top [-q] [-r] [-n nombre]

DESCRIPTION
     Analyse les structures /proc/[PID]/status pour extraire la mémoire physique
     réelle résidente (VmRSS). Élimine les threads noyau (RSS = 0) et classe les
     processus par tri statique sur la pile (0 octet sur le tas).
`
        },

        "rap-hunt": {
            section: "1",
            title: "RAP-HUNT(1) — Chasseur Universel",
            header: "RAP-HUNT(1)                RAPTOR DEFENSE               RAP-HUNT(1)",
            content: `
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
`
        },

        "rap-rx": {
            section: "1",
            title: "RAP-RX(1) — Traque Mémoire Inode 0",
            header: "RAP-RX(1)                  RAPTOR DEFENSE                 RAP-RX(1)",
            content: `
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
`
        },

        "rap-preload": {
            section: "1",
            title: "RAP-PRELOAD(1) — Audit Chargeur Dynamique",
            header: "RAP-PRELOAD(1)             RAPTOR DEFENSE            RAP-PRELOAD(1)",
            content: `
NAME
     rap preload — Détection des rootkits userland via LD_PRELOAD

SYNOPSIS
     rap preload [-q] [-r]

DESCRIPTION
     Effectue une double vérification défensive :
       1. Contrôle d'accès et d'intégrité de /etc/ld.so.preload.
       2. Parcours des blocs d'environnement (/proc/[PID]/environ) délimités par
          des octets nuls pour intercepter la variable hostile LD_PRELOAD=.
`
        },

        "rap-heur": {
            section: "1",
            title: "RAP-HEUR(1) — Analyse Heuristique",
            header: "RAP-HEUR(1)                RAPTOR DEFENSE               RAP-HEUR(1)",
            content: `
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
`
        },

        "rap-net": {
            section: "1",
            title: "RAP-NET(1) — Sonde Réseau Net-Guard",
            header: "RAP-NET(1)                 RAPTOR DEFENSE                RAP-NET(1)",
            content: `
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
`
        },

        "rap-kill": {
            section: "1",
            title: "RAP-KILL(1) — Riposte Cinétique",
            header: "RAP-KILL(1)                RAPTOR DEFENSE               RAP-KILL(1)",
            content: `
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
`
        },

        "rap-freeze": {
            section: "1",
            title: "RAP-FREEZE(1) — Gel Conservatoire Forensique",
            header: "RAP-FREEZE(1)              RAPTOR DEFENSE             RAP-FREEZE(1)",
            content: `
NAME
     rap freeze — Gel instantané d'une menace pour investigation mémoire

SYNOPSIS
     rap freeze <PID>

DESCRIPTION
     Émet le signal SIGSTOP sur le processus cible pour le placer dans l'état
     noyau T (Stopped). Permet de suspendre toute activité malveillante tout en
     conservant l'intégralité de sa mémoire virtuelle intacte pour copie forensique.
`
        },

        "rapless": {
            section: "1",
            title: "RAPLESS(1) — Téléscripteur Zéro-Heap",
            header: "RAPLESS(1)                 RAPTOR DEFENSE              RAPLESS(1)",
            content: `
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
`
        },

        "mem_whitelist": {
            section: "5",
            title: "MEM_WHITELIST(5) — Format Registre Mémoire",
            header: "MEM_WHITELIST(5)           RAPTOR DEFENSE          MEM_WHITELIST(5)",
            content: `
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
`
        },

        "fw_officiel": {
            section: "5",
            title: "FW_OFFICIEL(5) — Format Registre Net-Guard",
            header: "FW_OFFICIEL(5)            RAPTOR DEFENSE           FW_OFFICIEL(5)",
            content: `
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
`
        }
    };

    /* ========================================================================
     * FONCTIONS DU MOTEUR
     * ========================================================================
     */

    const elSelect = document.getElementById("manCommandSelect");
    const elSection = document.getElementById("manSectionSelect");
    const elTitle = document.getElementById("manViewerTitle");
    const elContent = document.getElementById("manViewerContent");
    const elSearchInput = document.getElementById("manSearchInput");
    const elBtnMan = document.getElementById("btnManExec");
    const elBtnApropos = document.getElementById("btnAproposExec");
    const elBtnCopy = document.getElementById("btnCopyMan");

    /* Fonction 1 : Peuplement du menu déroulant selon la section */
    function populateCommandSelect(filterSection) {
        elSelect.innerHTML = "";
        for (const key in MAN_DATABASE) {
            const item = MAN_DATABASE[key];
            if (filterSection === "all" || item.section === filterSection) {
                const opt = document.createElement("option");
                opt.value = key;
                opt.textContent = `${key}(${item.section})`;
                elSelect.appendChild(opt);
            }
        }
    }

    /* Fonction 2 : Rendu typographique avec mise en relief des sections UNIX */
    function renderManPage(key) {
        const item = MAN_DATABASE[key];
        if (!item) {
            elTitle.textContent = "ERREUR // PAGE INTROUVABLE";
            elContent.textContent = `Aucune page de manuel trouvée pour '${key}'.\nUtilisez 'apropos' pour une recherche par mot-clé.`;
            return;
        }

        elTitle.textContent = item.title.toUpperCase();

        // Coloration des sections standard
        let formatted = item.content
            .replace(/^([A-Z][A-Z\s]+)$/gm, '<span class="man-section-heading">$1</span>')
            .replace(/(\s)(-[a-zA-Z0-9_-]+)/g, '$1<span class="man-flag">$2</span>');

        elContent.innerHTML = formatted;
        elSelect.value = key;

        // Mise à jour de l'ancre URL sans recharger la page
        history.replaceState(null, null, `#${key}`);
    }

    /* Fonction 3 : Recherche par nom exact (man) */
    function handleManSearch() {
        const term = elSearchInput.value.trim().toLowerCase();
        if (!term) return;

        // Nettoyage des parenthèses de section éventuelles (ex: rx(1) -> rx)
        const cleanKey = term.replace(/\([0-9]\)/, "");

        if (MAN_DATABASE[cleanKey]) {
            renderManPage(cleanKey);
        } else if (MAN_DATABASE["rap-" + cleanKey]) {
            renderManPage("rap-" + cleanKey);
        } else {
            handleAproposSearch();
        }
    }

    /* Fonction 4 : Recherche par mot-clé dans les descriptions (apropos) */
    function handleAproposSearch() {
        const term = elSearchInput.value.trim().toLowerCase();
        if (!term) return;

        let results = [];
        for (const key in MAN_DATABASE) {
            const item = MAN_DATABASE[key];
            if (key.includes(term) || item.content.toLowerCase().includes(term)) {
                // Extraction de la ligne NAME
                const lines = item.content.split("\n");
                let desc = "";
                for (let i = 0; i < lines.length; i++) {
                    if (lines[i].startsWith("NAME")) {
                        desc = lines[i + 1] ? lines[i + 1].trim() : key;
                        break;
                    }
                }
                results.push(`${key}(${item.section})\t- ${desc}`);
            }
        }

        elTitle.textContent = `APROPOS // RÉSULTATS POUR "${term.toUpperCase()}"`;
        if (results.length > 0) {
            elContent.innerHTML = `<span class="man-section-heading">RÉSULTATS DE LA RECHERCHE</span>\n\n` + 
                results.map(r => `&bull; <a href="#${r.split('(')[0]}" style="color: #E5C07B; font-weight: bold;">${r.split('\t')[0]}</a>\t${r.split('\t')[1]}`).join("\n");
            
            // Écoute des liens générés dans les résultats apropos
            elContent.querySelectorAll("a").forEach(a => {
                a.addEventListener("click", function(e) {
                    e.preventDefault();
                    const targetKey = this.getAttribute("href").substring(1);
                    renderManPage(targetKey);
                });
            });
        } else {
            elContent.innerHTML = `<span class="man-section-heading">AUCUNE CORRESPONDANCE</span>\nAucun résultat trouvé pour le mot-clé '${term}'.`;
        }
    }

    /* ========================================================================
     * INITIALISATION & ÉCOUTEURS D'ÉVÉNEMENTS
     * ========================================================================
     */
    function init() {
        populateCommandSelect("1");

        // Détection de l'ancre dans l'URL (#rap-rx, etc.)
        const hash = window.location.hash.substring(1);
        if (hash && MAN_DATABASE[hash]) {
            renderManPage(hash);
        } else {
            renderManPage("rap");
        }

        // Sélecteur de commande
        elSelect.addEventListener("change", function () {
            renderManPage(this.value);
        });

        // Filtre de section
        elSection.addEventListener("change", function () {
            populateCommandSelect(this.value);
            if (elSelect.options.length > 0) {
                renderManPage(elSelect.options[0].value);
            }
        });

        // Boutons man et apropos
        elBtnMan.addEventListener("click", handleManSearch);
        elBtnApropos.addEventListener("click", handleAproposSearch);

        elSearchInput.addEventListener("keypress", function (e) {
            if (e.key === "Enter") {
                handleManSearch();
            }
        });

        // Copie du contenu brut
        elBtnCopy.addEventListener("click", function () {
            const currentKey = elSelect.value;
            const item = MAN_DATABASE[currentKey];
            if (item) {
                navigator.clipboard.writeText(item.content.trim()).then(() => {
                    const orig = elBtnCopy.textContent;
                    elBtnCopy.textContent = "Copié dans le presse-papier !";
                    setTimeout(() => { elBtnCopy.textContent = orig; }, 2000);
                });
            }
        });

        // Écouteur de changement d'ancre dans l'URL
        window.addEventListener("hashchange", function () {
            const h = window.location.hash.substring(1);
            if (h && MAN_DATABASE[h]) {
                renderManPage(h);
            }
        });
    }

    document.addEventListener("DOMContentLoaded", init);
})();
