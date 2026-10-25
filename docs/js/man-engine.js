/* ============================================================================
 * NOM DU SCRIPT   : docs/js/man-engine.js
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel & Hub Développeurs
 * RESPONSABILITÉ  : Moteur autonome d'affichage et de recherche des pages de
 *                   manuel Unix (Sections 1, 3 et 5) pour docs/man.html.
 *                   Découplé des données : consomme window.RAP_MAN_DATABASE.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Gestion du clic 'apropos' à vide avec panneau d'aide guidée.
 *   - Extraction automatique des descriptions du Dictionnaire (Section 3).
 *   - Bascule automatique du sélecteur de section lors d'un clic sur un résultat apropos.
 *   - Coloration et navigation fluide pour les sections 1, 3 et 5.
 *
 * LISTE DES FONCTIONS NUMÉROTÉES :
 *   1. man_engine_populate_select : Remplissage dynamique du sélecteur par section.
 *   2. man_engine_render          : Rendu typographique et coloration des sections.
 *   3. man_engine_search          : Recherche directe par nom de commande ('man').
 *   4. man_engine_apropos         : Recherche textuelle par mots-clés ('apropos').
 *   5. man_engine_init            : Initialisation du DOM, gestion des URL hash et événements.
 * ============================================================================
 */

(function () {
    "use strict";

    /* Consommation de la base de données unifiée SSOT */
    const MAN_DATABASE = (typeof window !== "undefined" && window.RAP_MAN_DATABASE) ? window.RAP_MAN_DATABASE : {};

    const elSelect = document.getElementById("manCommandSelect");
    const elSection = document.getElementById("manSectionSelect");
    const elTitle = document.getElementById("manViewerTitle");
    const elContent = document.getElementById("manViewerContent");
    const elSearchInput = document.getElementById("manSearchInput");
    const elBtnMan = document.getElementById("btnManExec");
    const elBtnApropos = document.getElementById("btnAproposExec");
    const elBtnCopy = document.getElementById("btnCopyMan");

    /* ========================================================================
     * 1. man_engine_populate_select
     * Remplissage du menu déroulant selon la section active (1, 3, 5 ou all)
     * ========================================================================
     */
    function man_engine_populate_select(filterSection) {
        if (!elSelect) return;
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

    /* ========================================================================
     * 2. man_engine_render
     * Rendu typographique avec mise en relief des sections standard UNIX
     * ========================================================================
     */
    function man_engine_render(key) {
        if (!elTitle || !elContent) return;

        const item = MAN_DATABASE[key];
        if (!item) {
            elTitle.textContent = "ERREUR // PAGE INTROUVABLE";
            elContent.textContent = `Aucune page trouvée pour '${key}'.\nUtilisez le bouton 'apropos' pour rechercher par mot-clé dans l'ensemble du système.`;
            return;
        }

        /* Si la page appartient à une section différente du filtre actif, basculer le filtre */
        if (elSection && elSection.value !== "all" && elSection.value !== item.section) {
            elSection.value = item.section;
            man_engine_populate_select(item.section);
        }

        elTitle.textContent = item.title.toUpperCase();

        /* Coloration des en-têtes standard Unix et des drapeaux */
        let formatted = item.content
            .replace(/^([A-Z][A-Z\s]+)$/gm, '<span class="man-section-heading">$1</span>')
            .replace(/(\s)(-[a-zA-Z0-9_-]+)/g, '$1<span class="man-flag">$2</span>');

        elContent.innerHTML = formatted;
        if (elSelect) {
            elSelect.value = key;
        }

        /* Mise à jour de l'ancre URL */
        history.replaceState(null, null, `#${key}`);
    }

    /* ========================================================================
     * 3. man_engine_search
     * Recherche exacte par nom de commande ('man')
     * ========================================================================
     */
    function man_engine_search() {
        if (!elSearchInput) return;
        const term = elSearchInput.value.trim().toLowerCase();
        if (!term) return;

        const cleanKey = term.replace(/\([0-9]\)/, "");

        if (MAN_DATABASE[cleanKey]) {
            man_engine_render(cleanKey);
        } else if (MAN_DATABASE["rap-" + cleanKey]) {
            man_engine_render("rap-" + cleanKey);
        } else if (MAN_DATABASE["sec-" + cleanKey]) {
            man_engine_render("sec-" + cleanKey);
        } else if (MAN_DATABASE["fn-" + cleanKey]) {
            man_engine_render("fn-" + cleanKey);
        } else {
            man_engine_apropos();
        }
    }

    /* ========================================================================
     * 4. man_engine_apropos
     * Recherche textuelle transversale par mot-clé ('apropos')
     * ========================================================================
     */
    function man_engine_apropos() {
        if (!elSearchInput || !elTitle || !elContent) return;
        const term = elSearchInput.value.trim().toLowerCase();

        /* Cas champ vide : aide explicative immédiate */
        if (!term) {
            elTitle.textContent = "APROPOS // GUIDE D'UTILISATION";
            elContent.innerHTML = `<span class="man-section-heading">RECHERCHE THÉMATIQUE APROPOS</span>\n\n` +
                `La commande <span style="color: #E5C07B; font-weight: bold;">apropos</span> recherche un mot-clé dans l'ensemble des pages de manuel et du dictionnaire d'ingénierie.\n\n` +
                `Entrez un mot-clé dans la barre de recherche ci-dessus, par exemple :\n` +
                `  &bull; <span style="color: #61AFEF;">rx</span>          : Traque mémoire et allocations Inode 0\n` +
                `  &bull; <span style="color: #61AFEF;">sentry</span>      : Sentinelle passive et fanotify\n` +
                `  &bull; <span style="color: #61AFEF;">netguard</span>    : Qualification réseau et règles CIDR\n` +
                `  &bull; <span style="color: #61AFEF;">114</span>         : Fiche unitaire de la fonction 114\n` +
                `  &bull; <span style="color: #61AFEF;">preload</span>     : Détection des rootkits userland`;
            return;
        }

        let results = [];
        for (const key in MAN_DATABASE) {
            const item = MAN_DATABASE[key];
            const contentLower = item.content.toLowerCase();
            const titleLower = item.title.toLowerCase();

            if (key.includes(term) || contentLower.includes(term) || titleLower.includes(term)) {
                let desc = "";

                /* 1. Recherche du bloc NAME classique (Section 1 et 5) */
                const lines = item.content.split("\n");
                for (let i = 0; i < lines.length; i++) {
                    if (lines[i].trim() === "NAME") {
                        desc = lines[i + 1] ? lines[i + 1].trim() : "";
                        break;
                    }
                }

                /* 2. Repli pour le dictionnaire (Section 3) : extraction du titre propre */
                if (!desc && item.title) {
                    desc = item.title.replace(/^[A-Z0-9_\-]+\([0-9]\)\s*[—\-]\s*/i, "");
                }

                results.push({
                    key: key,
                    section: item.section,
                    desc: desc || key
                });
            }
        }

        elTitle.textContent = `APROPOS // RÉSULTATS POUR "${term.toUpperCase()}"`;
        if (results.length > 0) {
            let output = `<span class="man-section-heading">RÉSULTATS DE LA RECHERCHE (${results.length})</span>\n\n`;
            output += results.map(r => {
                const badge = `${r.key}(${r.section})`;
                const paddedBadge = badge.padEnd(18, ' ');
                return `&bull; <a href="#${r.key}" style="color: #E5C07B; font-weight: bold; text-decoration: none;">${paddedBadge}</a> - ${r.desc}`;
            }).join("\n");

            elContent.innerHTML = output;

            /* Liaison des clics sur les résultats */
            elContent.querySelectorAll("a").forEach(a => {
                a.addEventListener("click", function (e) {
                    e.preventDefault();
                    const targetKey = this.getAttribute("href").substring(1);
                    man_engine_render(targetKey);
                });
            });
        } else {
            elContent.innerHTML = `<span class="man-section-heading">AUCUNE CORRESPONDANCE</span>\nAucun résultat trouvé pour le mot-clé '${term}'.`;
        }
    }

    /* ========================================================================
     * 5. man_engine_init
     * Initialisation globale et écouteurs d'événements
     * ========================================================================
     */
    function man_engine_init() {
        man_engine_populate_select("1");

        const hash = window.location.hash.substring(1);
        if (hash && MAN_DATABASE[hash]) {
            man_engine_render(hash);
        } else {
            man_engine_render("rap");
        }

        if (elSelect) {
            elSelect.addEventListener("change", function () {
                man_engine_render(this.value);
            });
        }

        if (elSection) {
            elSection.addEventListener("change", function () {
                man_engine_populate_select(this.value);
                if (elSelect && elSelect.options.length > 0) {
                    man_engine_render(elSelect.options[0].value);
                }
            });
        }

        if (elBtnMan) elBtnMan.addEventListener("click", man_engine_search);
        if (elBtnApropos) elBtnApropos.addEventListener("click", man_engine_apropos);

        if (elSearchInput) {
            elSearchInput.addEventListener("keypress", function (e) {
                if (e.key === "Enter") {
                    man_engine_search();
                }
            });
        }

        if (elBtnCopy) {
            elBtnCopy.addEventListener("click", function () {
                const currentKey = elSelect ? elSelect.value : "rap";
                const item = MAN_DATABASE[currentKey];
                if (item) {
                    navigator.clipboard.writeText(item.content.trim()).then(() => {
                        const orig = elBtnCopy.textContent;
                        elBtnCopy.textContent = "Copié dans le presse-papier !";
                        setTimeout(() => { elBtnCopy.textContent = orig; }, 2000);
                    });
                }
            });
        }

        window.addEventListener("hashchange", function () {
            const h = window.location.hash.substring(1);
            if (h && MAN_DATABASE[h]) {
                man_engine_render(h);
            }
        });
    }

    document.addEventListener("DOMContentLoaded", man_engine_init);
})();
