/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_views.c
 * RÉVISION        : Rev 2.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Routeur central et aiguilleur de rendu de l'interface Raylib.
 *                   Initialise le gestionnaire de ressources graphiques,
 *                   applique l'arrière-plan officiel Cyber Dark, invoque
 *                   le gabarit de navigation (en-tête + onglets), orchestre
 *                   l'affichage de la vue sélectionnée et superpose la modale.
 *
 * NOUVELLES INTÉGRATIONS (Rev 2.0) :
 *   - Refonte modulaire intégrale : passage de 1000+ lignes à moins de 60 lignes.
 *   - Élimination absolue du code monolithique.
 *   - Rôle exclusif d'aiguillage vers les sous-modules spécialisés de src/ui/.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. raptor_ui_render_tabs
 * ============================================================================
 */

#include "ui_common.h"

/* ------------------------------------------------------------------------- */
/* 1. DISPATCHER ET RENDU DE L'INTERFACE GRAPHIQUE MULTI-ONGLETS             */
/* ------------------------------------------------------------------------- */
void raptor_ui_render_tabs(const RaptorReportContext* ctx) {
    if (!ctx) {
        return;
    }

    /* 1. Chargement paresseux des polices et icônes */
    ui_resources_load();

    /* 2. Réinitialisation du tampon graphique avec la couleur de fond */
    ClearBackground(COL_BG_MAIN);

    /* 3. Rendu du squelette de navigation commun */
    ui_layout_draw_header();
    ui_layout_draw_tabs();

    /* 4. Aiguillage vers la vue métier active */
    switch (g_ui_active_tab) {
        case 0:
            ui_view_global_draw((RaptorReportContext*)ctx);
            break;
        case 1:
            ui_view_processes_draw((RaptorReportContext*)ctx);
            break;
        case 2:
            ui_view_network_draw(ctx);
            break;
        case 3:
            ui_view_diff_draw(ctx);
            break;
        case 4:
            ui_view_ai_draw(ctx);
            break;
        default:
            ui_view_global_draw((RaptorReportContext*)ctx);
            break;
    }

    /* 5. Affichage de la boîte modale en calque supérieur si activée */
    ui_modal_guide_draw();
}
