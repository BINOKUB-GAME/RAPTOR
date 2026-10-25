/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/modals/modal_guide.c
 * RÉVISION        : Rev 2.3
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Fenêtre modale d'aide interactive et guide analytique.
 *                   Présentation vulgarisée des 6 piliers défensifs :
 *                   indicateurs RSS/RWX/Inode-0, traque du chargeur dynamique
 *                   (Preload-Hunter), Net-Guard & BGP, sentinelle anti-rançongiciel
 *                   (Sentry/fanotify), trousse CLI 0-heap et documentation SSOT.
 *
 * BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 2.3) :
 *   - Intégration du Bloc 5 dédié à la Sentinelle passive Sentry (fanotify,
 *     canaris 32 Ko KeePass KDBX4, riposte réflexe SIGSTOP et FAN_DENY).
 *   - Intégration du Bloc 6 dédié à la Trousse CLI de secours (rap, rapless,
 *     architecture 0 heap) et au manuel centralisé SSOT (docs/man/rap.1.md).
 *   - Ajout d'un bouton d'action interactif ouvrant directement le manuel
 *     web officiel (docs/man.html) via le navigateur système par défaut.
 *   - Réorganisation harmonieuse en 6 blocs calibrés (3 gauches, 3 droites)
 *     maintenant une symétrie mathématique parfaite (518 px par colonne).
 *   - Typographie à 11.0f pour le corps et 12.0f pour les bandeaux de titre.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. ui_modal_guide_draw : Rendu de la modale d'aide et guide analytique.
 * ============================================================================
 */

#include "../ui_common.h"
#include <stdio.h>
#include <stdlib.h>
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
    /* STRUCTURE DES COLONNES & CALIBRAGE                                    */
    /* --------------------------------------------------------------------- */
    float cur_y = modal_y + 48.0f;
    float col_w = (modal_w - 44.0f) * 0.5f;
    float left_x = modal_x + 16.0f;
    float right_x = modal_x + 28.0f + col_w;
    float card_h = 166.0f; /* 3 cartes x 166 px + 2 intervalles x 10 px = 518 px */

    /* ===================================================================== */
    /* COLONNE GAUCHE                                                        */
    /* ===================================================================== */

    /* BLOC 1 : SURVEILLANCE MÉMOIRE & EMPREINTE RSS */
    Rectangle rec_b1 = { left_x, cur_y, col_w, card_h };
    DrawRectangleRec(rec_b1, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b1, 1.0f, COL_BORDER);
    DrawRectangle((int)left_x, (int)cur_y, (int)col_w, 24, (Color){ 18, 28, 44, 255 });
    ui_draw_text("1. SURVEILLANCE MÉMOIRE ET DÉRIVE (RSS)", left_x + 10.0f, cur_y + 6.0f, 12.0f, (Color){ 255, 187, 0, 255 });

    ui_draw_text("• RSS (Resident Set Size) : Empreinte RAM physique réelle.", left_x + 12.0f, cur_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Cahier Différentiel (T-1 vs T0) : Échantillonne les variations", left_x + 12.0f, cur_y + 48.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  et dérives volumétriques entre cycles d'audit (Mo/min).", left_x + 12.0f, cur_y + 64.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Fuite Mémoire : Isole l'expansion continue de mémoire vive", left_x + 12.0f, cur_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  sans restitution lors des phases de repos.", left_x + 12.0f, cur_y + 98.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Filtrage Noyau : Ignore les threads système (RSS = 0).", left_x + 12.0f, cur_y + 118.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("• Équivalent CLI : rap top [-n limite] et rap status.", left_x + 12.0f, cur_y + 138.0f, 11.0f, COL_CYAN_ACCENT);

    /* BLOC 2 : ANALYSE DES MUTATIONS FORENSIQUES R-X (SECTION 13) */
    float b2_y = cur_y + card_h + 10.0f;
    Rectangle rec_b2 = { left_x, b2_y, col_w, card_h };
    DrawRectangleRec(rec_b2, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b2, 1.0f, (Color){ 255, 51, 85, 180 });
    DrawRectangle((int)left_x, (int)b2_y, (int)col_w, 24, (Color){ 44, 16, 24, 255 });
    ui_draw_text("2. MUTATIONS R-X & INODE-0 SANS FICHIER (SECTION 13)", left_x + 10.0f, b2_y + 6.0f, 12.0f, COL_CRIT_RED);

    ui_draw_text("• Règle W^X : Une plage mémoire ne doit jamais combiner", left_x + 12.0f, b2_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  l'écriture et l'exécution simultanées (Write xor Execute).", left_x + 12.0f, b2_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Évasion mprotect() : Traque l'écriture en RW suivie d'une", left_x + 12.0f, b2_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  bascule clandestine vers R-X pour exécuter un shellcode.", left_x + 12.0f, b2_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Inode 0 : Isole le code machine orphelin de fichier disque.", left_x + 12.0f, b2_y + 102.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Whitelist Externe : Registre souverain neutralisant les", left_x + 12.0f, b2_y + 122.0f, 11.0f, COL_CYAN_ACCENT);
    ui_draw_text("  moteurs JIT/libffi (assets/rules/mem_whitelist.conf).", left_x + 12.0f, b2_y + 138.0f, 11.0f, COL_CYAN_ACCENT);

    /* BLOC 3 : DÉTOURNEMENT DE CHARGEUR / PRELOAD-HUNTER (SECTION 14) */
    float b3_y = b2_y + card_h + 10.0f;
    Rectangle rec_b3 = { left_x, b3_y, col_w, card_h };
    DrawRectangleRec(rec_b3, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b3, 1.0f, (Color){ 255, 51, 85, 180 });
    DrawRectangle((int)left_x, (int)b3_y, (int)col_w, 24, (Color){ 44, 16, 24, 255 });
    ui_draw_text("3. TRAQUE DU CHARGEUR PRELOAD-HUNTER (SECTION 14)", left_x + 10.0f, b3_y + 6.0f, 12.0f, COL_CRIT_RED);

    ui_draw_text("• Persistance Globale : Audit direct de /etc/ld.so.preload.", left_x + 12.0f, b3_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  Tout module actif ici infecte l'ensemble des exécutables.", left_x + 12.0f, b3_y + 46.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Hijacking Ciblé : Inspection dans /proc/[PID]/environ.", left_x + 12.0f, b3_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  Isole la variable LD_PRELOAD détournant les appels système.", left_x + 12.0f, b3_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Intelligence Flatpak : Raccordement à la liste blanche", left_x + 12.0f, b3_y + 102.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("  pour valider les conteneurs sans aucun faux positif.", left_x + 12.0f, b3_y + 118.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("• Équivalent CLI : rap preload (audit rapide sans UI).", left_x + 12.0f, b3_y + 138.0f, 11.0f, COL_CYAN_ACCENT);

    /* ===================================================================== */
    /* COLONNE DROITE                                                        */
    /* ===================================================================== */

    /* BLOC 4 : SÉCURITÉ RÉSEAU & NET-GUARD (SECTIONS 11-12) */
    Rectangle rec_b4 = { right_x, cur_y, col_w, card_h };
    DrawRectangleRec(rec_b4, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b4, 1.0f, COL_BORDER);
    DrawRectangle((int)right_x, (int)cur_y, (int)col_w, 24, (Color){ 18, 28, 44, 255 });
    ui_draw_text("4. SÉCURITÉ RÉSEAU & NET-GUARD (SECTIONS 11-12)", right_x + 10.0f, cur_y + 6.0f, 12.0f, (Color){ 255, 187, 0, 255 });

    ui_draw_text("• Écoute Passive /proc/net : Corrélation en temps réel du", right_x + 12.0f, cur_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  triplet dynamique socket <-> Inode <-> PID propriétaire.", right_x + 12.0f, cur_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Filtrage CIDR Souverain : Qualification des flux via", right_x + 12.0f, cur_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  les registres fw_officiel.conf et fw_custom.conf.", right_x + 12.0f, cur_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Anti-Combosquatting : Détection Levenshtein d'imitation", right_x + 12.0f, cur_y + 102.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  de domaines légitimes et adresses d'infrastructure C2.", right_x + 12.0f, cur_y + 118.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Renseignement BGP/ASN : Sonde Team Cymru et table locale.", right_x + 12.0f, cur_y + 138.0f, 11.0f, COL_GREEN_OK);

    /* BLOC 5 : SENTINELLE PASSIVE ANTI-RANÇONGICIEL (SECTION 17) */
    float b5_y = cur_y + card_h + 10.0f;
    Rectangle rec_b5 = { right_x, b5_y, col_w, card_h };
    DrawRectangleRec(rec_b5, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b5, 1.0f, (Color){ 255, 51, 85, 180 });
    DrawRectangle((int)right_x, (int)b5_y, (int)col_w, 24, (Color){ 44, 16, 24, 255 });
    ui_draw_text("5. SENTINELLE NOYAU SENTRY (SECTION 17)", right_x + 10.0f, b5_y + 6.0f, 12.0f, COL_CRIT_RED);

    ui_draw_text("• Fanotify Permissionnel : Veille passive bloquante Ring 0", right_x + 12.0f, b5_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  au niveau du noyau Linux (0% CPU au repos).", right_x + 12.0f, b5_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Canaris Denses : Déploiement d'appâts (!00_raptor_vault.kdbx,", right_x + 12.0f, b5_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  32 Ko, signature KeePass KDBX4 et haute entropie LCG).", right_x + 12.0f, b5_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Riposte Synchrone : Gel immédiat de l'agresseur (SIGSTOP)", right_x + 12.0f, b5_y + 102.0f, 11.0f, (Color){ 255, 187, 0, 255 });
    ui_draw_text("  et rejet catégorique de l'ouverture fichier via FAN_DENY.", right_x + 12.0f, b5_y + 118.0f, 11.0f, (Color){ 255, 187, 0, 255 });
    ui_draw_text("• Démon Furtif : rap sentry run -d (prctl, oom -1000, mlockall).", right_x + 12.0f, b5_y + 138.0f, 11.0f, COL_CYAN_ACCENT);

    /* BLOC 6 : TROUSSE CLI 0-HEAP & SOURCE UNIQUE SSOT (SECTIONS 16 & 18) */
    float b6_y = b5_y + card_h + 10.0f;
    Rectangle rec_b6 = { right_x, b6_y, col_w, card_h };
    DrawRectangleRec(rec_b6, (Color){ 12, 18, 28, 255 });
    DrawRectangleLinesEx(rec_b6, 1.0f, COL_BORDER);
    DrawRectangle((int)right_x, (int)b6_y, (int)col_w, 24, (Color){ 14, 28, 48, 255 });
    ui_draw_text("6. TROUSSE CLI DE SECOURS & MANUEL SSOT", right_x + 10.0f, b6_y + 6.0f, 12.0f, COL_CYAN_ACCENT);

    ui_draw_text("• Survie 0 Heap : Utilitaires rap et rapless sans malloc/free", right_x + 12.0f, b6_y + 30.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("  pour opérer sous famine mémoire extrême (< 2 Mo RAM).", right_x + 12.0f, b6_y + 46.0f, 11.0f, COL_TEXT_WHITE);
    ui_draw_text("• Téléscripteur rapless : Pager d'urgence autonome indexant", right_x + 12.0f, b6_y + 66.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("  des milliers de lignes /proc en cas de panne X11/terminal.", right_x + 12.0f, b6_y + 82.0f, 11.0f, COL_TEXT_MUTED);
    ui_draw_text("• Riposte Protégée : rap kill et rap freeze avec barrière PID 1.", right_x + 12.0f, b6_y + 102.0f, 11.0f, COL_GREEN_OK);
    ui_draw_text("• Source Unique SSOT : Centralisation dans docs/man/rap.1.md", right_x + 12.0f, b6_y + 122.0f, 11.0f, COL_CYAN_ACCENT);
    ui_draw_text("  générant rap_man_data.h (.rodata) et docs/man.html.", right_x + 12.0f, b6_y + 138.0f, 11.0f, COL_CYAN_ACCENT);

    /* --------------------------------------------------------------------- */
    /* PIED DE MODALE : ACTIONS ET FERMETURE                                 */
    /* --------------------------------------------------------------------- */
    float btn_h = 32.0f;
    float footer_y = modal_y + modal_h - btn_h - 12.0f;

    /* 1. Bouton d'accès direct au Manuel Web Centralisé (SSOT) */
    float btn_man_w = 210.0f;
    Rectangle btn_man = { modal_x + 16.0f, footer_y, btn_man_w, btn_h };
    bool hover_man = CheckCollisionPointRec(mouse, btn_man);

    DrawRectangleRec(btn_man, hover_man ? (Color){ 24, 44, 68, 255 } : (Color){ 14, 24, 40, 255 });
    DrawRectangleLinesEx(btn_man, 1.0f, hover_man ? COL_CYAN_ACCENT : COL_BORDER);
    ui_draw_text("Manuel Officiel rap(1)", btn_man.x + 30.0f, btn_man.y + 9.0f, 12.0f, hover_man ? COL_TEXT_WHITE : COL_CYAN_ACCENT);

    if (hover_man && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        #if defined(__linux__) || defined(__unix__)
        int ret = system("xdg-open docs/man.html >/dev/null 2>&1 &");
        (void)ret;
        #elif defined(_WIN32)
        system("start docs/man.html");
        #endif
    }

    /* 2. Légende centrale de rappel SSOT */
    ui_draw_text("Source Unique SSOT : docs/man/rap.1.md  |  CLI : rap man | rapless",
                 modal_x + btn_man_w + 28.0f, footer_y + 10.0f, 11.0f, COL_TEXT_MUTED);

    /* 3. Bouton Fermer le guide */
    float btn_close_w = 150.0f;
    Rectangle btn_close_bottom = { modal_x + modal_w - btn_close_w - 16.0f, footer_y, btn_close_w, btn_h };
    bool hover_close_bottom = CheckCollisionPointRec(mouse, btn_close_bottom);

    DrawRectangleRec(btn_close_bottom, hover_close_bottom ? COL_BLUE_HOVER : (Color){ 20, 34, 52, 255 });
    DrawRectangleLinesEx(btn_close_bottom, 1.0f, COL_CYAN_ACCENT);
    ui_draw_text("Fermer le guide", btn_close_bottom.x + 22.0f, btn_close_bottom.y + 9.0f, 12.0f, COL_TEXT_WHITE);

    if (hover_close_bottom && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = false;
    }
}
