/* ============================================================================
 * NOM DU SCRIPT   : docs/js/man-engine.js
 * RÉVISION        : Rev 1.4
 * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel & Hub Développeurs
 * RESPONSABILITÉ  : Moteur autonome d'affichage et de recherche des pages de
 *                   manuel Unix (Sections 1, 3 et 5) pour docs/man.html.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.4) :
 *   - Écoute active de la barre de recherche rapide de l'en-tête (.raptor-search-box input).
 *   - Synchronisation bidirectionnelle entre la recherche du haut et le viewer man.
 *   - Nettoyage des paramètres URL (?q=) lors de la navigation pour éviter les blocages au rafraîchissement.
 *   - Prise en charge des requêtes entrantes (?q=... ou ?search=...).
 * ============================================================================
 */

(function () {
    "use strict";

    const MAN_DATABASE = (typeof window !== "undefined" && window.RAP_MAN_DATABASE) ? window.RAP_MAN_DATABASE : {};

    const elSelect = document.getElementById("manCommandSelect");
    const elSection = document.getElementById("manSectionSelect");
    const elTitle = document.getElementById("manViewerTitle");
    const elContent = document.getElementById("manViewerContent");
    const elSearchInput = document.getElementById("manSearchInput");
    const elBtnMan = document.getElementById("btnManExec");
    const elBtnApropos = document.getElementById("btnAproposExec");
    const elBtnCopy = document.getElementById("btnCopyMan");

    /* Champ de recherche rapide situé dans l'en-tête rouge */
    const elHeaderSearch = document.querySelector(".raptor-header .raptor-search-box input");

    /* ========================================================================
     * 1. man_engine_populate_select
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

        if (elSection && elSection.value !== "all" && elSection.value !== item.section) {
            elSection.value = item.section;
            man_engine_populate_select(item.section);
        }

        elTitle.textContent = item.title.toUpperCase();

        let formatted = item.content
            .replace(/^([A-Z][A-Z\s]+)$/gm, '<span class="man-section-heading">$1</span>')
            .replace(/(\s)(-[a-zA-Z0-9_-]+)/g, '$1<span class="man-flag">$2</span>');

        elContent.innerHTML = formatted;
        if (elSelect) {
            elSelect.value = key;
        }

        /* Mise à jour propre de l'URL sans recharger et sans conserver les anciens ?q= */
        history.replaceState(null, null, `${window.location.pathname}#${key}`);
    }

    /* ========================================================================
     * 3. man_engine_search
     * ========================================================================
     */
    function man_engine_search(explicitTerm) {
        let term = explicitTerm;
        if (!term && elSearchInput) {
            term = elSearchInput.value;
        }
        if (!term) return;

        term = term.trim().toLowerCase();
        const cleanKey = term.replace(/\([0-9]\)/, "");

        /* Synchronisation visuelle des champs de recherche */
        if (elSearchInput) elSearchInput.value = term;
        if (elHeaderSearch) elHeaderSearch.value = term;

        if (MAN_DATABASE[cleanKey]) {
            man_engine_render(cleanKey);
        } else if (MAN_DATABASE["rap-" + cleanKey]) {
            man_engine_render("rap-" + cleanKey);
        } else if (MAN_DATABASE["sec-" + cleanKey]) {
            man_engine_render("sec-" + cleanKey);
        } else if (MAN_DATABASE["fn-" + cleanKey]) {
            man_engine_render("fn-" + cleanKey);
        } else {
            man_engine_apropos(term);
        }
    }

    /* ========================================================================
     * 4. man_engine_apropos
     * ========================================================================
     */
    function man_engine_apropos(explicitTerm) {
        if (!elTitle || !elContent) return;
        let term = explicitTerm;
        if (!term && elSearchInput) {
            term = elSearchInput.value;
        }

        if (!term) {
            elTitle.textContent = "APROPOS // GUIDE D'UTILISATION";
            elContent.innerHTML = `<span class="man-section-heading">RECHERCHE THÉMATIQUE APROPOS</span>\n\n` +
                `Entrez un mot-clé dans la barre de recherche ci-dessus, par exemple :\n` +
                `  &bull; <span style="color: #61AFEF;">hunt</span>         : Audit combiné et options de riposte active\n` +
                `  &bull; <span style="color: #61AFEF;">rx</span>           : Traque mémoire et allocations Inode 0\n` +
                `  &bull; <span style="color: #61AFEF;">sentry</span>       : Sentinelle passive et fanotify\n` +
                `  &bull; <span style="color: #61AFEF;">blackbox</span>     : Registre médico-légal en mémoire vive\n` +
                `  &bull; <span style="color: #61AFEF;">129</span>          : Fiche unitaire de la fonction 129 (Reflex)`;
            return;
        }

        term = term.trim().toLowerCase();
        let results = [];

        for (const key in MAN_DATABASE) {
            const item = MAN_DATABASE[key];
            const contentLower = item.content.toLowerCase();
            const titleLower = item.title.toLowerCase();

            if (key.includes(term) || contentLower.includes(term) || titleLower.includes(term)) {
                let desc = "";
                const lines = item.content.split("\n");
                for (let i = 0; i < lines.length; i++) {
                    if (lines[i].trim() === "NAME") {
                        desc = lines[i + 1] ? lines[i + 1].trim() : "";
                        break;
                    }
                }
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
     * ========================================================================
     */
    function man_engine_init() {
        man_engine_populate_select("1");

        /* 1. Écoute directe sur le champ de recherche de l'en-tête (en haut) */
        if (elHeaderSearch) {
            elHeaderSearch.addEventListener("keydown", function (e) {
                if (e.key === "Enter") {
                    e.preventDefault();
                    man_engine_search(this.value);
                }
            });
        }

        /* 2. Prise en charge des requêtes URL entrantes (?q=...) */
        const urlParams = new URLSearchParams(window.location.search);
        const queryTerm = urlParams.get("q") || urlParams.get("search");

        if (queryTerm) {
            man_engine_search(queryTerm);
        } else {
            const hash = window.location.hash.substring(1);
            if (hash && MAN_DATABASE[hash]) {
                man_engine_render(hash);
            } else {
                man_engine_render("rap");
            }
        }

        /* 3. Écouteurs de la barre d'outils locale */
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

        if (elBtnMan) elBtnMan.addEventListener("click", function() { man_engine_search(); });
        if (elBtnApropos) elBtnApropos.addEventListener("click", function() { man_engine_apropos(); });

        if (elSearchInput) {
            elSearchInput.addEventListener("keydown", function (e) {
                if (e.key === "Enter") {
                    e.preventDefault();
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
