/* ============================================================================
 * NOM DU SCRIPT   : docs/js/faq-nav.js
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel & Hub Développeurs
 * RESPONSABILITÉ  : Suivi automatique du défilement pour la barre latérale FAQ
 *                   (Sticky TOC). Active la classe .active sur le lien visible.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Utilisation de l'API moderne IntersectionObserver pour une détection
 *     fluide et performante à 60 FPS sans blocage du thread principal.
 *   - Défilement fluide lors du clic sur les ancres du sommaire.
 * ============================================================================
 */

(function () {
    "use strict";

    document.addEventListener("DOMContentLoaded", function () {
        const tocLinks = document.querySelectorAll(".faq-toc-list a");
        const sections = document.querySelectorAll(".faq-group");

        if (!sections.length || !tocLinks.length) return;

        // Configuration de l'observateur d'intersection
        const observerOptions = {
            root: null,
            rootMargin: "-80px 0px -60% 0px",
            threshold: 0
        };

        const observer = new IntersectionObserver(function (entries) {
            entries.forEach(function (entry) {
                if (entry.isIntersecting) {
                    const currentId = entry.target.getAttribute("id");
                    
                    tocLinks.forEach(function (link) {
                        if (link.getAttribute("href") === "#" + currentId) {
                            link.classList.add("active");
                        } else {
                            link.classList.remove("active");
                        }
                    });
                }
            });
        }, observerOptions);

        sections.forEach(function (section) {
            observer.observe(section);
        });

        // Défilement doux lors du clic sur les ancres
        tocLinks.forEach(function (link) {
            link.addEventListener("click", function (e) {
                e.preventDefault();
                const targetId = this.getAttribute("href").substring(1);
                const targetEl = document.getElementById(targetId);
                if (targetEl) {
                    targetEl.scrollIntoView({
                        behavior: "smooth"
                    });
                    history.replaceState(null, null, "#" + targetId);
                }
            });
        });
    });
})();
