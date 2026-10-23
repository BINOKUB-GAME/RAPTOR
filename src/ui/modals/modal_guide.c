/**
 * ============================================================================
 * NOM DU SCRIPT   : src/ui/modals/modal_guide.c
 * RÉVISION        : Rev 1.2
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Boîte de dialogue modale du Guide Analytique de RAPTOR.
 *                   Affiche l'écran d'aide contextuel en superposition avec
 *                   fond obscurci, commandes de fermeture (croix X et bouton)
 *                   et système d'accordéon interactif détaillant le fonctionnement
 *                   des sondes d'audit et des greffons défensifs.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.2) :
 *   - Intégration du volet Forensique Avancé (Section 13 : traque mémoire anonyme R-X
 *     consécutive aux bascules mprotect() clandestines et analyse Inode 0).
 *   - Enrichissement du volet Cahier Différentiel avec la traque des mutations dynamiques.
 *   - Passage de 6 à 7 volets d'accordéon interactifs avec recalibrage vertical (mh = 560 px).
 *   - Maintien du volet Net-Guard (anti-combosquatting) et Net-Intel (CDN / BGP / table_static).
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
    DrawRectangle(0, 0, screen_w, screen_h, (Color){ 0, 0, 0, 195 });

    /* 2. Cadre central de la modale */
    float mw = 880.0f;
    float mh = 560.0f;
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

    ui_draw_text("COMPRENDRE LE FONCTIONNEMENT DU DIAGNOSTIC SOUVERAIN", mx + 16.0f, my + 44.0f, 12.0f, COL_CYAN_ACCENT);

    /* 4. Liste des 7 volets de l'accordéon analytique */
    const char* items[7] = {
        "1. Mémoire Vive, Détection de Fuites & Binaires Fantômes (deleted)",
        "2. Net-Guard Défensif, Anti-Usurpation & Registres de Flux",
        "3. Net-Intel : Résolution ASN / BGP & Table Statique des CDN",
        "4. Intrusions Mémoire & Signatures d'Attaque (RWX)",
        "5. Forensique Avancé : Traque Mémoire Anonyme R-X (Section 13)",
        "6. Cahier de Bord & Moteur Différentiel (T-1 vs T0)",
        "7. Moteur d'IA Locale (Rapport d'Audit Souverain)"
    };

    float cur_y = my + 68.0f;
    for (int i = 0; i < 7; ++i) {
        Rectangle item_rec = { mx + 16.0f, cur_y, mw - 32.0f, 26.0f };
        bool hover_item = CheckCollisionPointRec(mouse, item_rec);

        DrawRectangleRec(item_rec, (s_guide_accordion_open == i) ? (Color){ 24, 42, 66, 255 } : (Color){ 14, 24, 38, 255 });
        DrawRectangleLinesEx(item_rec, 1.0f, COL_BORDER);

        ui_draw_text((s_guide_accordion_open == i) ? "\xE2\x96\xBC" : "\xE2\x96\xB6", item_rec.x + 10.0f, item_rec.y + 6.0f, 10.0f, COL_CYAN_ACCENT);
        ui_draw_text(items[i], item_rec.x + 28.0f, item_rec.y + 6.0f, 11.0f, COL_TEXT_WHITE);

        if (hover_item && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            s_guide_accordion_open = (s_guide_accordion_open == i) ? -1 : i;
        }

        cur_y += 30.0f;

        /* Contenu déployé du volet actif */
        if (s_guide_accordion_open == i) {
            float body_h = 106.0f;
            Rectangle body_rec = { mx + 16.0f, cur_y, mw - 32.0f, body_h };
            DrawRectangleRec(body_rec, (Color){ 8, 14, 22, 255 });
            DrawRectangleLinesEx(body_rec, 1.0f, (Color){ 16, 28, 44, 255 });

            if (i == 0) {
                ui_draw_text("•  RSS & VSZ : Mesure la RAM physique allouée et l'espace virtuel total de chaque processus.",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Dérive Mémoire (Leak) : Détecte les allocations monotones sans restitution en phase d'inactivité.",
                             body_rec.x + 14.0f, body_rec.y + 28.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Binaires Fantômes (deleted) : Alerte critique lorsqu'un programme tourne en RAM alors que son",
                             body_rec.x + 14.0f, body_rec.y + 48.0f, 11.0f, (Color){ 255, 187, 0, 255 });
                ui_draw_text("fichier source n'existe plus sur disque (binaire écrasé par mise à jour ou malware fileless masqué).",
                             body_rec.x + 14.0f, body_rec.y + 64.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("La neutralisation de ces processus purge les résidus et libère immédiatement l'espace mémoire.",
                             body_rec.x + 14.0f, body_rec.y + 82.0f, 11.0f, COL_GREEN_OK);
            } else if (i == 1) {
                ui_draw_text("•  Anti-Combosquatting (DOM) : Détecte les domaines contrefaits exploitant des marques protégées.",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Filtrage de Flux (IP) : Évalue chaque socket actif par masques CIDR et ports applicatifs.",
                             body_rec.x + 14.0f, body_rec.y + 28.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Registres Souverains : Gestion conjointe de la liste officielle GitHub (fw_officiel.conf)",
                             body_rec.x + 14.0f, body_rec.y + 48.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("et du fichier local privé (fw_custom.conf) avec neutralisation chirurgicale par SIGKILL.",
                             body_rec.x + 14.0f, body_rec.y + 64.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("La synchronisation en ligne calcule le différentiel strict sans écraser vos ajouts personnels.",
                             body_rec.x + 14.0f, body_rec.y + 82.0f, 11.0f, COL_CYAN_ACCENT);
            } else if (i == 2) {
                ui_draw_text("•  Élimination des 'PTR non déclarés' : Traque l'identité des serveurs dépourvus de DNS inverse.",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Table Statique d'Infrastructures : Résolution locale 0 ms via assets/rules/table_static.conf",
                             body_rec.x + 14.0f, body_rec.y + 28.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("identifiant instantanément les CDN mondiaux (Fastly, Cloudflare, Akamai, AWS, GitHub, Google).",
                             body_rec.x + 14.0f, body_rec.y + 48.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("•  Sonde ASN / BGP Distante : Interroge les registres de routage pour identifier les hébergeurs tiers.",
                             body_rec.x + 14.0f, body_rec.y + 68.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Toutes les requêtes s'exécutent en arrière-plan sans ralentir la fluidité de l'interface graphique.",
                             body_rec.x + 14.0f, body_rec.y + 84.0f, 11.0f, COL_GREEN_OK);
            } else if (i == 3) {
                ui_draw_text("•  Principe W^X (Write XOR Execute) : Une page mémoire ne doit jamais être modifiable ET exécutable.",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Traque d'Injections RWX : Détecte les shellcodes injectés en RAM et l'exécution sans binaire disque.",
                             body_rec.x + 14.0f, body_rec.y + 32.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Les compilateurs JIT légitimes (navigateurs, JVM) sont contextualisés pour isoler les anomalies réelles.",
                             body_rec.x + 14.0f, body_rec.y + 54.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("Tout processus hôte compromis peut être abattu immédiatement depuis la vue réseau.",
                             body_rec.x + 14.0f, body_rec.y + 76.0f, 11.0f, COL_CRIT_RED);
            } else if (i == 4) {
                ui_draw_text("•  Contournement mprotect() : Neutralise les ruses où le code bascule de RW- (écriture) à R-X (exécution).",
                             body_rec.x + 14.0f, body_rec.y + 10.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Analyse Inode 0 (Sans Ancrage Disque) : Isole tout segment exécutable orphelin de fichier physique.",
                             body_rec.x + 14.0f, body_rec.y + 30.0f, 11.0f, (Color){ 255, 187, 0, 255 });
                ui_draw_text("•  Filtrage Heuristique JIT : Exclut les moteurs légitimes (Chromium, Firefox) et segments système ([vdso]).",
                             body_rec.x + 14.0f, body_rec.y + 50.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("•  Signalement Forensique : Achemine l'anomalie en direct vers le Cahier Différentiel pour figer la preuve à T0.",
                             body_rec.x + 14.0f, body_rec.y + 70.0f, 11.0f, COL_CYAN_ACCENT);
                ui_draw_text("Permet de démasquer les ordinateurs zombies et les botnets sans binaire disque persistant.",
                             body_rec.x + 14.0f, body_rec.y + 88.0f, 11.0f, COL_GREEN_OK);
            } else if (i == 5) {
                ui_draw_text("•  Différentiel Temporel : Compare l'empreinte mémoire actuelle (T0) avec le cycle précédent (T-1).",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Dérive en Mo/min : Calcule la vitesse d'expansion de la mémoire pour anticiper les saturations OOM.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Journal Forensique des Mutations : Consigne les alertes critiques (mutations R-X et processus éphémères).",
                             body_rec.x + 14.0f, body_rec.y + 60.0f, 11.0f, COL_TEXT_MUTED);
                ui_draw_text("Permet d'isoler les activités clandestines même si le processus s'est éteint entre deux audits.",
                             body_rec.x + 14.0f, body_rec.y + 82.0f, 11.0f, COL_CYAN_ACCENT);
            } else {
                ui_draw_text("•  Moteur d'Audit Souverain : Traduit les métriques techniques complexes en synthèse compréhensible.",
                             body_rec.x + 14.0f, body_rec.y + 12.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("•  Confidentialité Absolue : Analyse 100 % locale, sans appel d'API distante ni fuite de télémétrie.",
                             body_rec.x + 14.0f, body_rec.y + 36.0f, 11.0f, COL_TEXT_WHITE);
                ui_draw_text("Délivre des recommandations précises sur les processus énergivores, les flux et les anomalies mémoire.",
                             body_rec.x + 14.0f, body_rec.y + 60.0f, 11.0f, COL_TEXT_MUTED);
            }

            cur_y += body_h + 6.0f;
        }
    }

    /* 5. Bouton de fermeture inférieur */
    Rectangle btn_close = { mx + 16.0f, my + mh - 38.0f, 130.0f, 26.0f };
    bool hover_btn_close = CheckCollisionPointRec(mouse, btn_close);
    DrawRectangleRec(btn_close, hover_btn_close ? COL_BLUE_HOVER : COL_PANEL_BG);
    DrawRectangleLinesEx(btn_close, 1.0f, hover_btn_close ? COL_BORDER_FOCUS : COL_BORDER);
    ui_draw_text("Fermer le guide", btn_close.x + 18.0f, btn_close.y + 6.0f, 11.0f, COL_TEXT_WHITE);

    if (hover_btn_close && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        g_ui_show_guide_modal = false;
    }
}
