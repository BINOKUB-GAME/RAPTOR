/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_tooltip.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Composant d'interface utilisateur universel dédié à l'affichage
 *                   d'infobulles (Tooltips) contextuelles et adaptatives.
 *                   - Rendu garanti au premier plan (Z-order maximal).
 *                   - Redimensionnement dynamique selon la taille du contenu.
 *                   - Marges de sécurité anti-débordement sur les 4 bords d'écran.
 *                   - Gestion automatique de cycle de vie frame-by-frame.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Découpage modulaire : extraction de la logique d'infobulle hors des vues.
 *   - Définition des déclencheurs et du rendu universel multi-lignes.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_tooltip_trigger
 *   2. ui_tooltip_draw
 * ============================================================================
 */

#include "ui_common.h"

/* ------------------------------------------------------------------------- */
/* ÉTAT INTERNE DE L'INFOBULLE CONTEXTUELLE                                  */
/* ------------------------------------------------------------------------- */
static bool    s_tooltip_active = false;
static Vector2 s_tooltip_pos = {0};
static char    s_tooltip_line1[256] = {0};
static char    s_tooltip_line2[256] = {0};
static char    s_tooltip_line3[256] = {0};
static Color   s_tooltip_line3_col = { 255, 204, 0, 255 };

/* ========================================================================= */
/* 1. DÉCLENCHEMENT D'UNE INFOBULLE POUR LA FRAME COURANTE                   */
/* ========================================================================= */
void ui_tooltip_trigger(Vector2 mouse_pos, const char* line1, const char* line2, const char* line3, Color line3_col) {
    s_tooltip_active = true;
    s_tooltip_pos = mouse_pos;

    if (line1) {
        snprintf(s_tooltip_line1, sizeof(s_tooltip_line1), "%s", line1);
    } else {
        s_tooltip_line1[0] = '\0';
    }

    if (line2) {
        snprintf(s_tooltip_line2, sizeof(s_tooltip_line2), "%s", line2);
    } else {
        s_tooltip_line2[0] = '\0';
    }

    if (line3) {
        snprintf(s_tooltip_line3, sizeof(s_tooltip_line3), "%s", line3);
        s_tooltip_line3_col = line3_col;
    } else {
        s_tooltip_line3[0] = '\0';
    }
}

/* ========================================================================= */
/* 2. RENDU ET ADAPTATION DYNAMIQUE DE L'INFOBULLE                           */
/* ========================================================================= */
void ui_tooltip_draw(void) {
    if (!s_tooltip_active) return;

    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();

    /* 1. Mesure de la largeur maximale parmi les lignes actives */
    float w1 = (strlen(s_tooltip_line1) > 0) ? ui_measure_text(s_tooltip_line1, 10.0f) : 0.0f;
    float w2 = (strlen(s_tooltip_line2) > 0) ? ui_measure_text(s_tooltip_line2, 11.0f) : 0.0f;
    float w3 = (strlen(s_tooltip_line3) > 0) ? ui_measure_text(s_tooltip_line3, 10.0f) : 0.0f;

    float max_text_w = w1;
    if (w2 > max_text_w) max_text_w = w2;
    if (w3 > max_text_w) max_text_w = w3;

    /* 2. Calcul des dimensions du conteneur */
    float tip_w = max_text_w + 36.0f;
    if (tip_w < 280.0f) tip_w = 280.0f;

    float tip_h = (strlen(s_tooltip_line3) > 0) ? 72.0f : 54.0f;

    /* 3. Décalage et protection anti-débordement d'écran */
    float tip_x = s_tooltip_pos.x + 14.0f;
    float tip_y = s_tooltip_pos.y + 14.0f;

    if (tip_x + tip_w > (float)screen_w - 16.0f) {
        tip_x = (float)screen_w - tip_w - 16.0f;
    }
    if (tip_x < 16.0f) {
        tip_x = 16.0f;
    }

    if (tip_y + tip_h > (float)screen_h - 16.0f) {
        tip_y = s_tooltip_pos.y - tip_h - 10.0f;
    }
    if (tip_y < 16.0f) {
        tip_y = 16.0f;
    }

    /* 4. Dessin du cadre cyber sombre */
    DrawRectangle((int)tip_x, (int)tip_y, (int)tip_w, (int)tip_h, (Color){ 8, 14, 24, 250 });
    DrawRectangleLinesEx((Rectangle){ tip_x, tip_y, tip_w, tip_h }, 1.0f, COL_BORDER_FOCUS);
    DrawRectangle((int)tip_x, (int)tip_y, (int)tip_w, 2, COL_CYAN_ACCENT);

    /* 5. Dessin des lignes de texte */
    if (strlen(s_tooltip_line1) > 0) {
        ui_draw_text(s_tooltip_line1, tip_x + 14.0f, tip_y + 9.0f, 10.0f, COL_CYAN_ACCENT);
    }
    if (strlen(s_tooltip_line2) > 0) {
        ui_draw_text(s_tooltip_line2, tip_x + 14.0f, tip_y + 27.0f, 11.0f, COL_TEXT_WHITE);
    }
    if (strlen(s_tooltip_line3) > 0) {
        ui_draw_text(s_tooltip_line3, tip_x + 14.0f, tip_y + 48.0f, 10.0f, s_tooltip_line3_col);
    }

    /* Réinitialisation de l'état pour la frame suivante */
    s_tooltip_active = false;
}
