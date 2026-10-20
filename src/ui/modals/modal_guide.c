/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/modals/modal_guide.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Boîte de dialogue modale du Guide Analytique de RAPTOR.
 *                   Affiche l'écran d'aide contextuel en superposition avec
 *                   fond obscurci, commandes de fermeture (croix X et bouton)
 *                   et système d'accordéon interactif détaillant le fonctionnement
 *                   des sondes (RSS/VSZ, Inode->PID, RWX, Différentiel, IA).
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Découpage modulaire : isolation stricte du code de la boîte modale.
 *   - Conservation de l'état d'accordéon localisé dans cette unité de compilation.
 *   - Pilotage de l'ouverture/fermeture via le booléen global g_ui_show_guide_modal.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_modal_guide_draw
 * ============================================================================
 */

#include "../ui_common.h"

static int s_guide_accordion_open = 0;

/* ------------------------------------------------------------------------- */
/* 1. RENDU DE LA MODALE DU GUIDE ANALYTIQUE EN SUPERPOSITION                */
/* ------------------------------------------------------------------------- */
void ui_modal_guide_draw(void) {
    if (!g_ui_show_guide_modal) {
        return;
    }

    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    Vector2 mouse = GetMousePosition();

    /* 1. Voile d'obscurcissement arrière-plan */
    DrawRectangle(0, 0, screen_w, screen_h, (Color){ 0, 0, 0, 180 });

    /* 2. Cadre central de la modale */
    float mw = 840.0f;
    float mh = 490.0f;
    float mx = ((float)screen_w - mw) / 2.0f;
    float my = ((float)screen_h - mh) / 2.0f;

    Rectangle modal_rec = { mx, my, mw, mh };
    DrawRectangleRec(modal_rec, (Color){ 10, 18, 28, 255 });
    DrawRectangleLinesEx(modal_rec, 1.0f, (Color){ 0, 119, 204, 255 });

    /* 3. Barre de titre de la modale */
    DrawRectangle((int)mx, (int)my, (int)mw, 32, (Color){ 16, 28, 44, 255 });
    ui_draw_text("Guide d'Aide & Composantes Analytiques de RAPTOR", mx + 16.0f, my + 8.0f, 12.0f, COL_TEXT_WHITE);

    /* Bouton croix de fermeture [X] */
    Rectangle close_x = { mx + mw - 32.0f, my, 32.0f, 32.0f };
    bool hover_close_x = CheckCollisionPointRec(mouse, close_x);
    ui_draw_text("X", close_x.x + 10.0f, close_x.y + 8.0f, 13.0f, hover_close_x ? COL_CRIT_RED : COL_TEXT_MUTED);

    if (hover_close_x && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = false;
        return;
    }

    ui_draw_text("COMPRENDRE LE FONCTIONNEMENT DU DIAGNOSTIC RAPTOR", mx + 16.0f, my + 46.0f, 12.0f, COL_CYAN_ACCENT);

    /* 4. Liste des volets de l'accordéon */
    const char* items[5] = {
        "1. Surveillance de la Mémoire et Détection de Fuites",
        "2. Mouchards, Télémétrie & Activité Réseau",
        "3. Intrus et Signatures d'Attaque (RWX)",
        "4. Cahier de Bord & Moteur Différentiel (T-1 vs T0)",
        "5. Moteur d'IA Locale (Rapport Humain)"
    };

    float cur_y = my + 76.0f;
    for (int i = 0; i < 5; ++i) {
        Rectangle item_rec = { mx + 16.0f, cur_y, mw - 32.0f, 28.0f };
        bool hover_item = CheckCollisionPointRec(mouse, item_rec);

        DrawRectangleRec(item_rec, (s_guide_accordion_open == i) ? (Color){ 24, 42, 66, 255 } : (Color){ 14, 24, 38, 255 });
        DrawRectangleLinesEx(item_rec, 1.0f, COL_BORDER);

        ui_draw_text((s_guide_accordion_open == i) ? "\xE2\x96\xBC" : "\xE2\x96\xB6", item_rec.x + 10.0f, item_rec.y + 7.0f, 11.0f, COL_CYAN_ACCENT);
        ui_draw_text(items[i], item_rec.x + 30.0f, item_rec.y + 7.0f, 12.0f, COL_TEXT_WHITE);

        if (hover_item && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            s_guide_accordion_open = (s_guide_accordion_open == i) ? -1 : i;
        }

        cur_y += 32.0f;

        /* Contenu dépliant si le volet est sélectionné */
        if (s_guide_accordion_open == i) {
            float body_h = 104.0f;
            Rectangle body_rec = { mx + 16.0f, cur_y, mw - 32.0f, body_h };
            DrawRectangleRec(body_rec, (Color){ 8, 14, 22, 255 });
            DrawRectangleLinesEx(body_rec, 1.0f, (Color){ 16, 28, 44, 255 });

            if (i == 0) {
                ui_draw_text("•  RSS (Resident Set Size) : Mémoire RAM physique réellement occupée par le programme.",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  VSZ (Virtual Size) : Espace d'adressage virtuel réservé (inclut les bibliothèques partagées).",
                             body_rec.x + 14.0f, body_rec.y + 30.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Détection de Dérive (Leak) : Surveillance continue calculant l'augmentation monotone de la RAM.",
                             body_rec.x + 14.0f, body_rec.y + 50.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Si un processus voit sa mémoire augmenter régulièrement sans jamais la restituer lors des phases",
                             body_rec.x + 14.0f, body_rec.y + 72.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("d'inactivité, RAPTOR le signale comme une fuite anormale.",
                             body_rec.x + 14.0f, body_rec.y + 86.0f, 11.0f, COL_TEXT_MUTED);
            } else if (i == 1) {
                ui_draw_text("•  Résolution Inode -> PID : Corrélation instantanée des sockets réseau TCP/UDP avec le binaire source.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Permet de détecter immédiatement tout flux de communication clandestin initié en tâche de fond.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            } else if (i == 2) {
                ui_draw_text("•  Pages RWX (Read-Write-Execute) : Violation du principe W^X (Write XOR Execute).",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Alerte critique si du code exécutable réside sur la pile ou sans correspondance binaire légitime.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            } else if (i == 3) {
                ui_draw_text("•  Différentiel Temporel : Analyse des variations entre instantanés consécutifs T-1 et T0.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Calcul dynamique du taux de dérive en Mo/minute et classification automatique du niveau de risque.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            } else {
                ui_draw_text("•  Moteur IA Souverain : Traduction contextuelle des métriques techniques en diagnostic lisible.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Traitement intégralement local réalisé sans aucun recours à une API distante ni fuite de données.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_MUTED);
            }

            cur_y += body_h + 6.0f;
        }
    }

    /* 5. Bouton de fermeture inférieur */
    Rectangle btn_close = { mx + 16.0f, my + mh - 40.0f, 130.0f, 28.0f };
    bool hover_btn_close = CheckCollisionPointRec(mouse, btn_close);
    DrawRectangleRec(btn_close, hover_btn_close ? COL_BLUE_HOVER : COL_PANEL_BG);
    DrawRectangleLinesEx(btn_close, 1.0f, hover_btn_close ? COL_BORDER_FOCUS : COL_BORDER);
    ui_draw_text("Fermer le guide", btn_close.x + 16.0f, btn_close.y + 7.0f, 12.0f, COL_TEXT_WHITE);

    if (hover_btn_close && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = false;
    }
}
