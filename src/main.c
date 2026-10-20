/**
 * ============================================================================
 * NOM DU SCRIPT   : src/main.c
 * RÉVISION        : Rev 1.3
 * PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
 * RESPONSABILITÉ  : Point d'entrée principal de l'exécutable unifié RAPTOR.
 *                   Traite les arguments de lancement (--cli, --html, --gui),
 *                   initialise le registre des greffons et le moteur différentiel,
 *                   orchestre l'audit défensif sur le conteneur universel,
 *                   génère systématiquement les rapports (HTML et TXT) à la racine
 *                   du projet sans inonder la sortie console.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.3) :
 *   - Suppression de l'affichage volumineux dans le shell lors de l'appel --cli.
 *   - Écriture silencieuse et immédiate des fichiers raptor_report.html et txt.
 *   - Simple accusé de réception sur la console avec indication des chemins.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. main
 * ============================================================================
 */

#include "raptor/raptor_data.h"
#include "raptor/raptor_plugin.h"
#include "raptor/raptor_registry.h"
#include "raptor/raptor_differential.h"
#include "raptor/raptor_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------- */
/* FONCTIONS AUXILIAIRES INTERNES (EXPORT TEXTE & CHEMINS RACINE)            */
/* ------------------------------------------------------------------------- */
static void main_export_txt_file(const RaptorReportContext* ctx, const char* filepath) {
    if (!ctx || !filepath) {
        return;
    }

    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        return;
    }

    fprintf(fp, "================================================================================\n");
    fprintf(fp, "  RAPTOR // SYSTEME D'AUDIT DEFENSIF & TRAQUE MEMOIRE (v2.0)\n");
    fprintf(fp, "  RAPTOR DEFENSE // AUDIT MEMOIRE & PROCESSUS  [ SURVEILLANCE ACTIVE ]\n");
    fprintf(fp, "================================================================================\n\n");

    size_t sec_count = raptor_data_get_section_count(ctx);
    for (size_t s = 0; s < sec_count; ++s) {
        const RaptorSection* sec = raptor_data_get_section(ctx, s);
        if (!sec) continue;

        fprintf(fp, "\n================================================================================\n");
        fprintf(fp, "  SECTION [%02zu] : %s\n", s + 1, sec->title);
        fprintf(fp, "  ID MODULE    : %s\n", sec->id);
        fprintf(fp, "================================================================================\n");

        if (sec->alert_count > 0) {
            fprintf(fp, "\n  -- NOTIFICATIONS & ALERTES --\n");
            for (size_t a = 0; a < sec->alert_count; ++a) {
                const char* sev_str = (sec->alerts[a].sev == RAPTOR_SEV_OK) ? "[ SUCCES ]" :
                                      (sec->alerts[a].sev == RAPTOR_SEV_WARNING) ? "[ ATTENTION ]" :
                                      (sec->alerts[a].sev == RAPTOR_SEV_CRITICAL) ? "[ CRITIQUE ]" : "[ INFO ]";
                fprintf(fp, "  %s %s\n", sev_str, sec->alerts[a].message);
            }
        }

        if (sec->card_count > 0) {
            fprintf(fp, "\n  -- METRIQUES CLES --\n");
            for (size_t c = 0; c < sec->card_count; ++c) {
                fprintf(fp, "  | %-24s : %-16s (%5.1f%%)\n",
                        sec->cards[c].label, sec->cards[c].val, sec->cards[c].pct);
            }
        }

        for (size_t t = 0; t < sec->table_count; ++t) {
            const RaptorTable* tbl = &sec->tables[t];
            fprintf(fp, "\n  >>> %s\n", tbl->title);

            for (size_t col = 0; col < tbl->col_count; ++col) {
                fprintf(fp, "%-22s\t", tbl->col_names[col] ? tbl->col_names[col] : "");
            }
            fprintf(fp, "\n--------------------------------------------------------------------------------\n");

            for (size_t r = 0; r < tbl->row_count; ++r) {
                for (size_t col = 0; col < tbl->col_count; ++col) {
                    const char* txt = (tbl->rows[r].cell_texts && tbl->rows[r].cell_texts[col])
                                      ? tbl->rows[r].cell_texts[col] : "";
                    fprintf(fp, "%-22s\t", txt);
                }
                fprintf(fp, "\n");
            }
        }
    }

    fprintf(fp, "\n--------------------------------------------------------------------------------\n");
    fprintf(fp, "  [RAPTOR] Rapport souverain consolide en local. Zero donnee externe.\n");
    fprintf(fp, "--------------------------------------------------------------------------------\n");

    fclose(fp);
}

/* ------------------------------------------------------------------------- */
/* 1. POINT D'ENTRÉE PRINCIPAL (API 3.7.1)                                   */
/* ------------------------------------------------------------------------- */
int main(int argc, char** argv) {
    bool mode_cli = false;
    bool mode_html = false;
    const char* user_html_path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--cli") == 0 || strcmp(argv[i], "-c") == 0) {
            mode_cli = true;
        } else if ((strcmp(argv[i], "--html") == 0 || strcmp(argv[i], "-o") == 0) && (i + 1 < argc)) {
            mode_html = true;
            user_html_path = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  --cli, -c              Executer l'audit et generer les fichiers (HTML et TXT) sans flood.\n");
            printf("  --html <fichier>, -o   Exporter le rapport d'audit souverain au format HTML.\n");
            printf("  --gui                  Lancer l'interface graphique interactive (mode par defaut).\n");
            printf("  --help, -h             Afficher cette notice d'aide.\n");
            return 0;
        }
    }

    const char* root_prefix = "";
    FILE* check_root = fopen("../CMakeLists.txt", "r");
    if (check_root) {
        fclose(check_root);
        root_prefix = "../";
    }

    char root_html_path[256];
    char root_txt_path[256];

    if (user_html_path) {
        snprintf(root_html_path, sizeof(root_html_path), "%s", user_html_path);
    } else {
        snprintf(root_html_path, sizeof(root_html_path), "%sraptor_report.html", root_prefix);
    }
    snprintf(root_txt_path, sizeof(root_txt_path), "%sraptor_report.txt", root_prefix);

    /* 1. Initialisation des sous-systèmes du cœur */
    raptor_registry_init();
    raptor_diff_init();

    /* 2. Création du contexte de rapport universel */
    RaptorReportContext* ctx = raptor_data_context_create();
    if (!ctx) {
        fprintf(stderr, "[ERREUR RAPTOR] Echec critique lors de l'allocation du contexte universel.\n");
        raptor_diff_cleanup();
        raptor_registry_cleanup();
        return 1;
    }

    /* 3. Exécution séquentielle des greffons actifs */
    raptor_registry_run_all(ctx);

    /* 4. Calcul différentiel temporel */
    RaptorSection* diff_sec = raptor_data_section_add(
        ctx,
        "sec_differential",
        "CAHIER DE BORD // ANALYSE DIFFERENTIELLE TEMPORELLE"
    );
    if (diff_sec) {
        raptor_diff_compute(ctx, diff_sec);
    }
    raptor_diff_snapshot_save(ctx);

    /* 5. Génération systématique des rapports sur disque */
    raptor_report_html_generate(ctx, root_html_path);
    main_export_txt_file(ctx, root_txt_path);

    if (root_prefix[0] != '\0') {
        raptor_report_html_generate(ctx, "raptor_report.html");
        main_export_txt_file(ctx, "raptor_report.txt");
    }

    /* 6. Aiguillage de restitution */
    if (mode_cli) {
        /* Ne flood plus le shell : affiche uniquement l'accusé de réception */
        printf("\n================================================================================\n");
        printf("  RAPTOR // AUDIT DEFENSIF EXECUTE AVEC SUCCES\n");
        printf("================================================================================\n");
        printf("  [OK] Rapport HTML enregistre : %s\n", root_html_path);
        printf("  [OK] Rapport TXT enregistre  : %s\n", root_txt_path);
        printf("================================================================================\n\n");
    } else if (mode_html) {
        printf("[RAPTOR] Rapport HTML souverain genere avec succes : %s\n", root_html_path);
    } else {
        const int window_width = 1200;
        const int window_height = 800;
        const char* window_title = "RAPTOR // Systeme d'Audit Defensif & Traque Memoire";

        if (raptor_ui_init(window_width, window_height, window_title)) {
            raptor_ui_run(ctx);
            raptor_ui_shutdown();
        } else {
            fprintf(stderr, "[AVERTISSEMENT RAPTOR] Serveur graphique indisponible. Generation des rapports sur disque.\n");
            printf("[RAPTOR] Rapport HTML genere : %s\n", root_html_path);
        }
    }

    /* 7. Libération mémoire */
    raptor_data_context_destroy(ctx);
    raptor_diff_cleanup();
    raptor_registry_cleanup();

    return 0;
}
