# CAHIER DES CHARGES TECHNIQUE // PROJET RAPTOR v2.0
Architecture Modulaire Integrale, Greffons Autonomes et Dictionnaire d'API

## 1. PRINCIPES FONDAMENTAUX ET REGLES D'INGENIERIE

1. Agnosticisme absolu du Noyau (Core) : Le moteur central (raptor_core) ne manipule aucun concept metier specifique (ni structure de processus, ni structure reseau). Il gere exclusivement le cycle de vie de l'application, l'ordonnancement temporel et un conteneur de donnees generique.
2. Architecture Greffons Stricte (Bus d'Enregistrement) : Tout audit systeme est obligatoirement isole dans son propre greffon (RaptorPlugin). Un greffon ne depend d'aucun autre greffon.
3. Auto-enregistrement a la compilation (Zero retouche de fichier central) : L'ajout ou le retrait d'un greffon s'effectue par simple ajout ou suppression de son fichier source dans le dossier de compilation. L'enregistrement aupres du bus central est opere a l'initialisation du binaire via l'attribut constructeur GCC (__attribute__((constructor))). Aucun fichier existant (main.c, core, en-tetes) ne doit etre ouvert ni modifie pour integrer une nouvelle sonde.
4. Contrat de Donnees Universel : Les greffons publient leurs resultats sous forme de structures abstraites (cartes de metriques cles, alertes textuelles, tableaux dynamiques de colonnes et cellules). Les moteurs de restitution (CLI, HTML, GUI) consomment ces structures sans savoir si les donnees proviennent d'une sonde reseau, d'une sonde memoire ou d'un capteur materiel.
5. Norme de Documentation et de Proprete : Zero code temporaire ou non documente. Chaque fichier source commence obligatoirement par un en-tete mentionnant le nom du script, son numero de revision, un bloc descriptif des integrations et la liste numerotee exhaustive de ses fonctions internes et exportees.

## 2. ARBORESCENCE OFFICIELLE DU DEPOT

RAPTOR/
|-- CMakeLists.txt
|-- assets/
|   `-- icons/
|-- include/
|   `-- raptor/
|       |-- raptor_data.h
|       |-- raptor_plugin.h
|       |-- raptor_registry.h
|       |-- raptor_differential.h
|       `-- raptor_report.h
|-- src/
|   |-- main.c
|   |-- core/
|   |   |-- raptor_data.c
|   |   |-- raptor_registry.c
|   |   `-- raptor_differential.c
|   |-- plugins/
|   |   |-- plugin_memory_rwx.c
|   |   `-- plugin_network_sockets.c
|   |-- report/
|   |   |-- report_cli.c
|   |   `-- report_html.c
|   `-- ui/
|       |-- ui_engine.c
|       `-- ui_views.c
`-- build/




## 3. DICTIONNAIRE OFFICIEL DES STRUCTURES ET FONCTIONS (API FIGEE)

### 3.1. Modele de Donnees Universel (include/raptor/raptor_data.h & src/core/raptor_data.c)

Fonction 1 : raptor_data_context_create
Signature C : RaptorReportContext* raptor_data_context_create(void);
Responsabilite : Alloue et initialise un nouveau contexte de rapport vide.

Fonction 2 : raptor_data_section_add
Signature C : RaptorSection* raptor_data_section_add(RaptorReportContext* ctx, const char* id, const char* title);
Responsabilite : Instancie une nouvelle section autonome rattachee au contexte.

Fonction 3 : raptor_data_card_add
Signature C : bool raptor_data_card_add(RaptorSection* sec, const char* label, const char* val, float pct, RaptorSeverity sev);
Responsabilite : Ajoute une metrique en carte visuelle a une section.

Fonction 4 : raptor_data_table_create
Signature C : RaptorTable* raptor_data_table_create(RaptorSection* sec, const char* title, size_t col_count, const char** col_names);
Responsabilite : Cree un tableau tabulaire dans la section avec ses en-tetes.

Fonction 5 : raptor_data_table_row_add
Signature C : bool raptor_data_table_row_add(RaptorTable* table, const char** cell_texts, const RaptorSeverity* cell_sevs);
Responsabilite : Insere une ligne complete de cellules dans le tableau.

Fonction 6 : raptor_data_alert_add
Signature C : bool raptor_data_alert_add(RaptorSection* sec, RaptorSeverity sev, const char* message);
Responsabilite : Enregistre une notification ou un avertissement dans la section.

Fonction 7 : raptor_data_get_section_count
Signature C : size_t raptor_data_get_section_count(const RaptorReportContext* ctx);
Responsabilite : Retourne le nombre total de sections enregistrees dans le contexte.

Fonction 8 : raptor_data_get_section
Signature C : const RaptorSection* raptor_data_get_section(const RaptorReportContext* ctx, size_t index);
Responsabilite : Fournit un acces securise en lecture a une section via son index.

Fonction 9 : raptor_data_context_destroy
Signature C : void raptor_data_context_destroy(RaptorReportContext* ctx);
Responsabilite : Libere recursivement l'ensemble de la memoire allouee au contexte.

### 3.2. Contrat de Greffon et Registre (include/raptor/raptor_plugin.h & src/core/raptor_registry.c)

Structure universelle :
typedef struct {
    const char* id;
    const char* name;
    const char* version;
    bool (*init)(void);
    bool (*run_audit)(RaptorReportContext* ctx);
    void (*cleanup)(void);
} RaptorPlugin;

Fonction 1 : raptor_registry_init
Signature C : void raptor_registry_init(void);
Responsabilite : Initialise la table interne des pointeurs de greffons.

Fonction 2 : raptor_registry_register
Signature C : bool raptor_registry_register(const RaptorPlugin* plugin);
Responsabilite : Enregistre un greffon dans la liste globale.

Fonction 3 : raptor_registry_run_all
Signature C : bool raptor_registry_run_all(RaptorReportContext* ctx);
Responsabilite : Lance sequentiellement la methode run_audit de chaque greffon actif.

Fonction 4 : raptor_registry_get_count
Signature C : size_t raptor_registry_get_count(void);
Responsabilite : Retourne le nombre total de greffons actuellement branches.

Fonction 5 : raptor_registry_get_by_idx
Signature C : const RaptorPlugin* raptor_registry_get_by_idx(size_t index);
Responsabilite : Fournit l'acces en lecture a un greffon via son index.

Fonction 6 : raptor_registry_cleanup
Signature C : void raptor_registry_cleanup(void);
Responsabilite : Appelle cleanup() sur chaque greffon et vide le registre.

### 3.3. Moteur d'Analyse Differentielle (include/raptor/raptor_differential.h & src/core/raptor_differential.c)

Fonction 1 : raptor_diff_init
Signature C : void raptor_diff_init(void);
Responsabilite : Initialise les structures de mise en cache differentielle.

Fonction 2 : raptor_diff_compute
Signature C : bool raptor_diff_compute(const RaptorReportContext* current, RaptorSection* out_diff_sec);
Responsabilite : Compare Tn et Tn-1, calcule les derives (Mo/min) et peuple la section differentielle.

Fonction 3 : raptor_diff_snapshot_save
Signature C : void raptor_diff_snapshot_save(const RaptorReportContext* current);
Responsabilite : Archive l'etat actuel comme point de repere pour le prochain audit.

Fonction 4 : raptor_diff_cleanup
Signature C : void raptor_diff_cleanup(void);
Responsabilite : Libere les instantanes d'historique conserves en memoire vive.

### 3.4. Greffon 1 : Processus, Memoire et RWX (src/plugins/plugin_memory_rwx.c)

Fonction 1 : proc_mem_is_trusted_path
Signature C : static bool proc_mem_is_trusted_path(const char* exe_path);
Responsabilite : Verifie que l'executable provient strictement de /usr/bin/, /usr/lib/, /app/.

Fonction 2 : proc_mem_is_legit_jit
Signature C : static bool proc_mem_is_legit_jit(const char* comm, const char* exe);
Responsabilite : Valide la signature des moteurs JIT/libffi (Chromium, GTK, Python).

Fonction 3 : proc_mem_scan_maps_rwx
Signature C : static bool proc_mem_scan_maps_rwx(uint32_t pid, bool is_jit, bool is_trusted);
Responsabilite : Analyse /proc/[PID]/maps (alerte immediate sur [stack] ou faux chemin).

Fonction 4 : proc_mem_read_stats
Signature C : static bool proc_mem_read_stats(uint32_t pid, char* out_comm, size_t comm_len, char* out_exe, size_t exe_len, uint64_t* out_rss);
Responsabilite : Recupere nom, binaire et RSS sans fuite de descripteur de fichier.

Fonction 5 : proc_mem_run_audit
Signature C : static bool proc_mem_run_audit(RaptorReportContext* ctx);
Responsabilite : Parcourt /proc, qualifie les menaces et remplit la section generique du rapport.

Fonction 6 : proc_mem_register
Signature C : __attribute__((constructor)) static void proc_mem_register(void);
Responsabilite : Auto-enregistre le greffon memoire au chargement du binaire.

### 3.5. Greffon 2 : Surveillance Reseau et Sockets (src/plugins/plugin_network_sockets.c)

Fonction 1 : net_sock_build_inode_cache
Signature C : static bool net_sock_build_inode_cache(void);
Responsabilite : Scanne /proc/*/fd et construit l'index d'association inode -> PID.

Fonction 2 : net_sock_resolve_pid
Signature C : static uint32_t net_sock_resolve_pid(uint64_t target_inode);
Responsabilite : Retourne instantanement le PID proprietaire d'un inode (O(1)).

Fonction 3 : net_sock_decode_hex_endpoint
Signature C : static void net_sock_decode_hex_endpoint(const char* hex_ip, uint32_t hex_port, char* out_ip, size_t ip_len, uint16_t* out_port);
Responsabilite : Convertit le format hexadecimal noyau en IP:Port lisible.

Fonction 4 : net_sock_parse_file
Signature C : static void net_sock_parse_file(const char* net_path, const char* proto, RaptorTable* out_table);
Responsabilite : Parse /proc/net/tcp et /udp et peuple le tableau generique.

Fonction 5 : net_sock_run_audit
Signature C : static bool net_sock_run_audit(RaptorReportContext* ctx);
Responsabilite : Orchestre la resolution reseau et libere les tables d'inodes.

Fonction 6 : net_sock_register
Signature C : __attribute__((constructor)) static void net_sock_register(void);
Responsabilite : Auto-enregistre le greffon reseau au chargement du binaire.

### 3.6. Moteurs de Restitution Universels (src/report/ et src/ui/)

Fonction 1 : raptor_report_cli_print
Signature C : void raptor_report_cli_print(const RaptorReportContext* ctx);
Responsabilite : Affiche l'integralite du rapport textuel formate sur le terminal.

Fonction 2 : raptor_report_html_generate
Signature C : bool raptor_report_html_generate(const RaptorReportContext* ctx, const char* filepath);
Responsabilite : Exporte le rapport HTML souverain avec theme sombre et tableaux interactifs.

Fonction 3 : raptor_ui_init
Signature C : bool raptor_ui_init(int width, int height, const char* title);
Responsabilite : Demarre Raylib et prepare le contexte graphique ImGui.

Fonction 4 : raptor_ui_run
Signature C : void raptor_ui_run(RaptorReportContext* ctx);
Responsabilite : Boucle principale (while (!WindowShouldClose())) gerant les interactions et audits.

Fonction 5 : raptor_ui_render_tabs
Signature C : void raptor_ui_render_tabs(const RaptorReportContext* ctx);
Responsabilite : Boucle sur le contexte pour instancier un onglet dynamique par section/greffon.

Fonction 6 : raptor_ui_shutdown
Signature C : void raptor_ui_shutdown(void);
Responsabilite : Libere les contextes graphiques et ferme la fenetre.

### 3.7. Point d'Entree Principal (src/main.c)

Fonction 1 : main
Signature C : int main(int argc, char** argv);
Responsabilite : Parse les options (--cli ou GUI), initialise le registre, lance l'audit et pilote la sortie.

## 4. REGLE STRICTE D'AJOUT D'UN FUTUR GREFFON

1. Creer uniquement le fichier source dedie : src/plugins/plugin_nom.c.
2. Implementer la fonction d'audit et la structure RaptorPlugin.
3. Ajouter la balise de constructeur GCC :
   __attribute__((constructor))
   static void auto_register_plugin(void) {
       raptor_registry_register(&g_my_plugin);
   }
4. Ajouter le fichier a la liste des sources dans CMakeLists.txt et compiler.
