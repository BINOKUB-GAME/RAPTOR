/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/modals/modal_guide.c
 * RÉVISION        : Rev 2.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Fenêtre modale d'aide interactive et guide analytique.
 *                   Vulgarise le fonctionnement des sondes forensiques, explicite
 *                   les indicateurs RSS/RWX/Inode-0, détaille la traque du
 *                   chargeur dynamique (Preload-Hunter) et documente le rôle
 *                   du registre différentiel temporel et des alertes bureau.
 *
 * NOUVELLES INTÉGRATIONS (Rev 2.2) :
 *   - Agrandissement de la typographie à 11.0f sur tout le corps de texte
 *     pour un confort de lecture optimal sur écran haute définition.
 *   - Agrandissement des titres des encadrés et du bouton de fermeture à 12.0f.
 *   - Recentrage horizontal du libellé d'action dans le bouton de pied.
 *   - Maintien strict de la symétrie parfaite des deux colonnes (518 px).
 *   - Zéro dépassement de cadre et respect du prototype void ui_modal_guide_draw(void).
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_modal_guide_draw
 * ============================================================================
 */

#include "../ui_common.h"
#include <stdio.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------- */
/* 1. AFFICHAGE DE LA MODALE DU GUIDE ANALYTIQUE DE RAPTOR v2.0             */
/* ------------------------------------------------------------------------- */
void ui_modal_guide_draw(void) {
    if (!g_ui_show_guide_modal) {
        return;
    }

    float screen_w = (float)GetScreenWidth();
    float screen_h = (float)GetScreenHeight();
    Vector2 mouse = GetMousePosition();

    /* Voile d'assombrissement d'arrière-plan */
    DrawRectangle(0, 0, (int)screen_w, (int)screen_h, (Color){ 4, 8, 14, 220 });

    /* Dimensions et centrage de la fenêtre modale */
    float modal_w = (screen_w > 1060.0f) ? 1020.0f : (screen_w - 40.0f);
    float modal_h = (screen_h > 700.0f) ? 650.0f : (screen_h - 30.0f);
    float modal_x = (screen_w - modal_w) * 0.5f;
    float modal_y = (screen_h - modal_h) * 0.5f;

    Rectangle rec_modal = { modal_x, modal_y, modal_w, modal_h };
    DrawRectangleRec(rec_modal, (Color){ 8, 14, 24, 255 });
    DrawRectangleLinesEx(rec_modal, 1.5f, COL_CYAN_ACCENT);

    /* En-tête de la modale */
    DrawRectangle((int)modal_x, (int)modal_y, (int)modal_w, 40, (Color){ 14, 24, 40, 255 });
    DrawLineEx((Vector2){ modal_x, modal_y + 40.0f },
               (Vector2){ modal_x + modal_w, modal_y + 40.0f }, 1.0f, COL_BORDER);

    ui_draw_text("GUIDE D'AIDE & COMPOSANTES ANALYTIQUES // RAPTOR DEFENSE v2.0",
                 modal_x + 20.0f, modal_y + 12.0f, 12.0f, COL_CYAN_ACCENT);

    /* Bouton fermeture (croix 'X' en haut à droite) */
    Rectangle btn_close_top = { modal_x + modal_w - 34.0f, modal_y + 8.0f, 24.0f, 24.0f };
    bool hover_close_top = CheckCollisionPointRec(mouse, btn_close_top);
    DrawRectangleRec(btn_close_top, hover_close_top ? COL_CRIT_RED : (Color){ 20, 32, 50, 255 });
    DrawRectangleLinesEx(btn_close_top, 1.0f, COL_BORDER);
    ui_draw_text("X", btn_close_top.x + 8.0f, btn_close_top.y + 5.0f, 11.0f, COL_TEXT_WHITE);

    if (hover_close_top && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = false;
        return;
    }

    /* --------------------------------------------------------------------- */
    /* STRUCTURE DES COLONNES                                                */
    /* --------------------------------------------------------------------- */
    float cur_y = modal_y + 48.0f;
    float col_w = (modal_w - 44.0f) * 0.5f;
    float left_x = modal_x + 16.0f;
    float right_x = modal_x + 28.0f + col_w;

    /* ===================================================================== */
    /* COLONNE GAUCHE                                                        */
    /* ===================================================================== */

    /* BLOC 1 : SURVEILLANCE MÉMOIRE & EMPREINTE RSS */
    float b1_h = 138.0f;
    Rectangle rec_b1 = { left_x, cur_y, col_w, b1_h };
    DrawRectangleRec(rec_b1, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b1, 1.0f, COL_BORDER);
    DrawRectangle((int)left_x, (int)cur_y, (int)col_w, 24, (Color){ 18, 28, 44, 255 });
    ui_draw_text("1. SURVEILLANCE MÉMOIRE ET DÉRIVE (RSS)", left_x + 10.0f, cur_y + 6.0f, 12.0f, (Color){ 255, 187, 0, 255 });

    ui_draw_text("• RSS (Resident Set Size) : Empreinte RAM physique réelle.", left_x + 12.0f, cur_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Cahier Différentiel (T-1 vs T0) : Échantillonne les variations", left_x + 12.0f, cur_y + 48.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  et dérives volumétriques entre cycles d'audit.", left_x + 12.0f, cur_y + 64.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Fuite Mémoire : Isole l'expansion continue de mémoire", left_x + 12.0f, cur_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  sans restitution lors des phases de repos.", left_x + 12.0f, cur_y + 98.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Filtrage Noyau : Ignore les threads système (RSS = 0).", left_x + 12.0f, cur_y + 116.0f, 11.0f, COL_GREEN_OK);

    /* BLOC 2 : ANALYSE DES MUTATIONS FORENSIQUES R-X (SECTION 13) */
    float b2_y = cur_y + b1_h + 10.0f;
    float b2_h = 184.0f;
    Rectangle rec_b2 = { left_x, b2_y, col_w, b2_h };
    DrawRectangleRec(rec_b2, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b2, 1.0f, (Color){ 255, 51, 85, 180 });
    DrawRectangle((int)left_x, (int)b2_y, (int)col_w, 24, (Color){ 44, 16, 24, 255 });
    ui_draw_text("2. MUTATIONS R-X & INODE-0 SANS FICHIER (SECTION 13)", left_x + 10.0f, b2_y + 6.0f, 12.0f, COL_CRIT_RED);

    ui_draw_text("• Règle W^X : Une plage mémoire ne doit jamais combiner", left_x + 12.0f, b2_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  l'écriture et l'exécution simultanées (Write xor Execute).", left_x + 12.0f, b2_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Évasion mprotect() : Traque l'écriture en RW suivie d'une", left_x + 12.0f, b2_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  bascule clandestine vers R-X pour exécuter un shellcode.", left_x + 12.0f, b2_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Inode 0 : Détecte le code machine n'ayant aucun ancrage", left_x + 12.0f, b2_y + 102.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  de binaire officiel sur le support de stockage disque.", left_x + 12.0f, b2_y + 118.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Whitelist Externe : Registre souverain neutralisant les", left_x + 12.0f, b2_y + 138.0f, 11.0f, COL_CYAN_ACCENT);
    ui_draw_text("  moteurs JIT/libffi (assets/rules/mem_whitelist.conf).", left_x + 12.0f, b2_y + 154.0f, 11.0f, COL_CYAN_ACCENT);

    /* BLOC 3 : DÉTOURNEMENT DE CHARGEUR / PRELOAD-HUNTER (SECTION 14) */
    float b3_y = b2_y + b2_h + 10.0f;
    float b3_h = 176.0f;
    Rectangle rec_b3 = { left_x, b3_y, col_w, b3_h };
    DrawRectangleRec(rec_b3, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b3, 1.0f, (Color){ 255, 51, 85, 180 });
    DrawRectangle((int)left_x, (int)b3_y, (int)col_w, 24, (Color){ 44, 16, 24, 255 });
    ui_draw_text("3. TRAQUE DU CHARGEUR DYNAMIQUE (SECTION 14)", left_x + 10.0f, b3_y + 6.0f, 12.0f, COL_CRIT_RED);

    ui_draw_text("• Persistance Globale : Audit direct de /etc/ld.so.preload.", left_x + 12.0f, b3_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  Tout module actif ici infecte l'ensemble des exécutables.", left_x + 12.0f, b3_y + 46.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Hijacking Ciblé : Inspection dans /proc/[PID]/environ.", left_x + 12.0f, b3_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  Isole la variable LD_PRELOAD détournant les appels système.", left_x + 12.0f, b3_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Intelligence Flatpak : Raccordement à la liste blanche", left_x + 12.0f, b3_y + 102.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("  pour valider les conteneurs sans aucun faux positif.", left_x + 12.0f, b3_y + 118.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("• Visibilité Forensique : Transmission automatique des alertes", left_x + 12.0f, b3_y + 138.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  au registre LIFO du Cahier Différentiel.", left_x + 12.0f, b3_y + 154.0f, 11.0f, COL_TEXT_MUTED);

    /* ===================================================================== */
    /* COLONNE DROITE                                                        */
    /* ===================================================================== */

    /* BLOC 4 : SÉCURITÉ RÉSEAU & NET-GUARD */
    float b4_h = 246.0f;
    Rectangle rec_b4 = { right_x, cur_y, col_w, b4_h };
    DrawRectangleRec(rec_b4, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b4, 1.0f, COL_BORDER);
    DrawRectangle((int)right_x, (int)cur_y, (int)col_w, 24, (Color){ 18, 28, 44, 255 });
    ui_draw_text("4. SÉCURITÉ RÉSEAU, SOCKETS & NET-GUARD", right_x + 10.0f, cur_y + 6.0f, 12.0f, (Color){ 255, 187, 0, 255 });

    ui_draw_text("• Écoute Passive /proc/net : Corrélation en temps réel du", right_x + 12.0f, cur_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  triplet dynamique socket <-> Inode <-> PID.", right_x + 12.0f, cur_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Filtrage CIDR Souverain : Contrôle strict défini dans", right_x + 12.0f, cur_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  assets/rules/net_rules.conf pour chaque flux réseau.", right_x + 12.0f, cur_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Anti-Combosquatting : Calcul algorithmique Levenshtein", right_x + 12.0f, cur_y + 102.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  décelant l'imitation de domaines légitimes et bancaires.", right_x + 12.0f, cur_y + 118.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Renseignement BGP/ASN : Détection préventive de flux", right_x + 12.0f, cur_y + 138.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  vers des serveurs connus de commande et contrôle (C2).", right_x + 12.0f, cur_y + 154.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Analyse Contextuelle (deleted) : Différencie les mises à jour", right_x + 12.0f, cur_y + 174.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("  de paquets officielles des fichiers effacés par des malwares.", right_x + 12.0f, cur_y + 190.0f, 11.0f, COL_GREEN_OK);

    /* BLOC 5 : NOTIFICATIONS SYSTÈME & DIFFÉRENTIEL (SECTION 15) */
    float b5_y = cur_y + b4_h + 10.0f;
    float b5_h = 262.0f;
    Rectangle rec_b5 = { right_x, b5_y, col_w, b5_h };
    DrawRectangleRec(rec_b5, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b5, 1.0f, COL_BORDER);
    DrawRectangle((int)right_x, (int)b5_y, (int)col_w, 24, (Color){ 18, 28, 44, 255 });
    ui_draw_text("5. NOTIFICATIONS ASYNCHRONES & CAHIER FORENSIQUE", right_x + 10.0f, b5_y + 6.0f, 12.0f, (Color){ 255, 187, 0, 255 });

    ui_draw_text("• Bulles Freedesktop (Section 15) : Avertit l'opérateur sur le", right_x + 12.0f, b5_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  bureau même si la fenêtre est masquée ou réduite.", right_x + 12.0f, b5_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Double Fork Asynchrone : 0 ms d'impact sur Raylib", right_x + 12.0f, b5_y + 66.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("  (60 FPS stables) et 0 processus zombie persistant.", right_x + 12.0f, b5_y + 82.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("• Filtre Anti-Rebond : Évite le harcèlement de notifications", right_x + 12.0f, b5_y + 102.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  avec un temps de garde minimum de 30 secondes.", right_x + 12.0f, b5_y + 118.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Cahier Forensique (Onglet 4) : Affichage ordonné LIFO", right_x + 12.0f, b5_y + 138.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  avec bouton interactif de purge d'historique.", right_x + 12.0f, b5_y + 154.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Mention 'N/A' Rigoureuse : Appliquée aux alertes d'environnement", right_x + 12.0f, b5_y + 174.0f, 11.0f, COL_CYAN_ACCENT);
    ui_draw_text("  sans plage mémoire directe pour une clarté forensique totale.", right_x + 12.0f, b5_y + 190.0f, 11.0f, COL_CYAN_ACCENT);

    /* --------------------------------------------------------------------- */
    /* PIED DE MODALE : BOUTON FERMER                                        */
    /* --------------------------------------------------------------------- */
    float btn_w = 160.0f;
    float btn_h = 32.0f;
    Rectangle btn_close_bottom = { modal_x + modal_w - btn_w - 16.0f, modal_y + modal_h - btn_h - 10.0f, btn_w, btn_h };
    bool hover_close_bottom = CheckCollisionPointRec(mouse, btn_close_bottom);

    DrawRectangleRec(btn_close_bottom, hover_close_bottom ? COL_BLUE_HOVER : (Color){ 20, 34, 52, 255 });
    DrawRectangleLinesEx(btn_close_bottom, 1.0f, COL_CYAN_ACCENT);
    ui_draw_text("Fermer le guide", btn_close_bottom.x + 24.0f, btn_close_bottom.y + 9.0f, 12.0f, COL_TEXT_WHITE);

    if (hover_close_bottom && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = false;
    }
}
