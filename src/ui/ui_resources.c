/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/ui_resources.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Gestionnaire de ressources graphiques et utilitaires visuels.
 *                   Instancie les variables globales d'interface, orchestre le
 *                   chargement et déchargement de la typographie TrueType avec
 *                   support étendu Latin-1, symboles et glyphes mathématiques (sigma).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Ajout explicite du point de code Unicode 0x03C3 (lettre grecque sigma)
 *     dans la table codepoints de LoadFontEx pour éviter le rendu de '?' dans l'UI.
 *   - Ajout du point de code 0x2714 (coche de validation).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_draw_text
 *   2. ui_measure_text
 *   3. ui_find_card
 *   4. ui_find_table
 *   5. ui_resources_load
 *   6. ui_resources_unload
 *   7. ui_views_unload_resources
 * ============================================================================
 */

#include "ui_common.h"
#include <time.h>
#include <ctype.h>

/* ------------------------------------------------------------------------- */
/* DÉFINITIONS DES VARIABLES GLOBALES D'INTERFACE                            */
/* ------------------------------------------------------------------------- */
int g_ui_active_tab = 0;
bool g_ui_show_guide_modal = false;
char g_ui_last_audit_time[64] = {0};

Font g_ui_font = {0};
bool g_ui_font_loaded = false;

Texture2D g_icon_dashboard = {0};
Texture2D g_icon_processes = {0};
Texture2D g_icon_network = {0};
Texture2D g_icon_diff = {0};
Texture2D g_icon_ai = {0};
bool g_ui_icons_loaded = false;

/* ------------------------------------------------------------------------- */
/* 1. DESSIN DE TEXTE AVEC MISE À L'ÉCHELLE GLOBALE                          */
/* ------------------------------------------------------------------------- */
void ui_draw_text(const char* text, float posX, float posY, float baseSize, Color color) {
    if (!text || text[0] == '\0') {
        return;
    }

    float scaledSize = baseSize * UI_FONT_SCALE;
    if (g_ui_font_loaded) {
        Vector2 pos = { posX, posY };
        DrawTextEx(g_ui_font, text, pos, scaledSize, 1.0f, color);
    } else {
        DrawText(text, (int)posX, (int)posY, (int)scaledSize, color);
    }
}

/* ------------------------------------------------------------------------- */
/* 2. MESURE DE LARGEUR DE TEXTE AVEC MISE À L'ÉCHELLE GLOBALE              */
/* ------------------------------------------------------------------------- */
float ui_measure_text(const char* text, float baseSize) {
    if (!text || text[0] == '\0') {
        return 0.0f;
    }

    float scaledSize = baseSize * UI_FONT_SCALE;
    if (g_ui_font_loaded) {
        return MeasureTextEx(g_ui_font, text, scaledSize, 1.0f).x;
    }
    return (float)MeasureText(text, (int)scaledSize);
}

/* ------------------------------------------------------------------------- */
/* 3. RECHERCHE D'UNE CARTE MÉTRIQUE DANS LE CONTEXTE                        */
/* ------------------------------------------------------------------------- */
const RaptorCard* ui_find_card(const RaptorReportContext* ctx, const char* keyword) {
    if (!ctx || !keyword) {
        return NULL;
    }

    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];
        for (size_t c = 0; c < sec->card_count; ++c) {
            if (strstr(sec->cards[c].label, keyword) != NULL) {
                return &sec->cards[c];
            }
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* 4. RECHERCHE D'UNE TABLE DANS LE CONTEXTE                                 */
/* ------------------------------------------------------------------------- */
const RaptorTable* ui_find_table(const RaptorReportContext* ctx, const char* sec_id, const char* tbl_kw) {
    if (!ctx) {
        return NULL;
    }

    for (size_t s = 0; s < ctx->section_count; ++s) {
        const RaptorSection* sec = &ctx->sections[s];
        if (sec_id && strcmp(sec->id, sec_id) != 0) {
            continue;
        }

        for (size_t t = 0; t < sec->table_count; ++t) {
            if (!tbl_kw || strstr(sec->tables[t].title, tbl_kw) != NULL) {
                return &sec->tables[t];
            }
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------------- */
/* 5. CHARGEMENT DES RESSOURCES VISUELLES (POLICE & TEXTURES)                */
/* ------------------------------------------------------------------------- */
void ui_resources_load(void) {
    if (g_ui_last_audit_time[0] == '\0') {
        time_t n = time(NULL);
        struct tm* tm_i = localtime(&n);
        if (tm_i) {
            strftime(g_ui_last_audit_time, sizeof(g_ui_last_audit_time), "%Y-%m-%d %H:%M:%S", tm_i);
        } else {
            strncpy(g_ui_last_audit_time, "2026-10-20 12:00:00", sizeof(g_ui_last_audit_time) - 1);
            g_ui_last_audit_time[sizeof(g_ui_last_audit_time) - 1] = '\0';
        }
    }

    if (!g_ui_font_loaded) {
        int codepoints[512];
        int cp_count = 0;

        /* ASCII standard (32..126) */
        for (int i = 32; i <= 126; ++i) {
            codepoints[cp_count++] = i;
        }
        /* Latin-1 étendu pour les accents français (160..255) */
        for (int i = 160; i <= 255; ++i) {
            codepoints[cp_count++] = i;
        }
        /* Glyphes spécifiques de navigation et symboles */
        codepoints[cp_count++] = 0x2022; /* • */
        codepoints[cp_count++] = 0x25BC; /* ▼ */
        codepoints[cp_count++] = 0x25B6; /* ▶ */
        codepoints[cp_count++] = 0x2714; /* ✔ */
        codepoints[cp_count++] = 0x03C3; /* σ (sigma grec) */

        const char* font_candidates[] = {
            "assets/fonts/font.ttf",
            "../assets/fonts/font.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            NULL
        };

        for (int i = 0; font_candidates[i] != NULL; ++i) {
            if (FileExists(font_candidates[i])) {
                g_ui_font = LoadFontEx(font_candidates[i], (int)(32.0f * UI_FONT_SCALE), codepoints, cp_count);
                if (g_ui_font.baseSize > 0) {
                    SetTextureFilter(g_ui_font.texture, TEXTURE_FILTER_BILINEAR);
                    g_ui_font_loaded = true;
                    break;
                }
            }
        }
    }

    if (g_ui_icons_loaded) {
        return;
    }

    const char* p_dash = "assets/icons/icon_dashboard.png";
    const char* p_proc = "assets/icons/icon_processes.png";
    const char* p_net  = "assets/icons/icon_network.png";
    const char* p_diff = "assets/icons/icon_diff.png";
    const char* p_ai   = "assets/icons/icon_ai.png";

    const char* alt_dash = "../assets/icons/icon_dashboard.png";
    const char* alt_proc = "../assets/icons/icon_processes.png";
    const char* alt_net  = "../assets/icons/icon_network.png";
    const char* alt_diff = "../assets/icons/icon_diff.png";
    const char* alt_ai   = "../assets/icons/icon_ai.png";

    if (FileExists(p_dash)) g_icon_dashboard = LoadTexture(p_dash);
    else if (FileExists(alt_dash)) g_icon_dashboard = LoadTexture(alt_dash);

    if (FileExists(p_proc)) g_icon_processes = LoadTexture(p_proc);
    else if (FileExists(alt_proc)) g_icon_processes = LoadTexture(alt_proc);

    if (FileExists(p_net)) g_icon_network = LoadTexture(p_net);
    else if (FileExists(alt_net)) g_icon_network = LoadTexture(alt_net);

    if (FileExists(p_diff)) g_icon_diff = LoadTexture(p_diff);
    else if (FileExists(alt_diff)) g_icon_diff = LoadTexture(alt_diff);

    if (FileExists(p_ai)) g_icon_ai = LoadTexture(p_ai);
    else if (FileExists(alt_ai)) g_icon_ai = LoadTexture(alt_ai);

    g_ui_icons_loaded = true;
}

/* ------------------------------------------------------------------------- */
/* 6. DÉCHARGEMENT PROPRE DE LA MÉMOIRE GPU ET DES POLICES                   */
/* ------------------------------------------------------------------------- */
void ui_resources_unload(void) {
    if (g_ui_font_loaded) {
        UnloadFont(g_ui_font);
        g_ui_font_loaded = false;
    }

    if (!g_ui_icons_loaded) {
        return;
    }

    if (g_icon_dashboard.id > 0) UnloadTexture(g_icon_dashboard);
    if (g_icon_processes.id > 0) UnloadTexture(g_icon_processes);
    if (g_icon_network.id > 0)   UnloadTexture(g_icon_network);
    if (g_icon_diff.id > 0)      UnloadTexture(g_icon_diff);
    if (g_icon_ai.id > 0)        UnloadTexture(g_icon_ai);

    g_ui_icons_loaded = false;
}

/* ------------------------------------------------------------------------- */
/* 7. PASSERELLE DE LIBÉRATION POUR ui_engine.c                              */
/* ------------------------------------------------------------------------- */
void ui_views_unload_resources(void) {
    ui_resources_unload();
}
