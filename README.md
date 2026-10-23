# 🦅 RAPTOR v2.0 // Système d'Audit Défensif & Traque Mémoire

[![Licence](https://img.shields.io/badge/Licence-GPLv3%2FPropri%C3%A9taire-00d2ff.svg)](#)
[![Langage](https://img.shields.io/badge/Langage-C99%20%2F%20POSIX-00e676.svg)](#)
[![Plateforme](https://img.shields.io/badge/Plateforme-Linux%20x86__64-ff3355.svg)](#)
[![Interface](https://img.shields.io/badge/UI-Raylib%205.0-ffbb00.svg)](#)
[![Statut](https://img.shields.io/badge/Statut-Production%20%2F%20Souverain-00d2ff.svg)](#)
[![Communauté](https://img.shields.io/badge/Reddit-r%2FRaptorSecurity-ff4500.svg?logo=reddit)](https://www.reddit.com/r/RaptorSecurity/)

**RAPTOR** est une solution d'audit de sécurité défensive (EDR léger / HIDS) et d'analyse passive en temps réel conçue pour les environnements Linux souverains. Écrit en **C99 natif** sans dépendance logicielle lourde ni télémétrie distante, RAPTOR surveille l'intégrité de l'espace d'adressage des processus, traque les injections de code en mémoire vive (RWX et mémoire anonyme R-X), filtre les sockets réseau actifs et neutralise chirurgicalement les menaces identifiées.

---

## 🎯 Capacités Clés & Piliers Défensifs

### 1. Traque des Injections & Violations Mémoire (RWX)
* Inspection continue des cartographies mémoire sous `/proc/[pid]/maps`.
* Détection en temps réel des allocations suspectes cumulant les permissions **Lecture, Écriture et Exécution (RWX)** en violation stricte du principe $W \oplus X$.
* Signalement immédiat des binaires fantômes résidents `(deleted)` résultant d'injections sans fichier (*fileless*) ou d'exécutions masquées depuis `/tmp` ou `/dev/shm`.

### 2. Forensique Avancé & Mutations Mémoire Anonymes (R-X - Section 13)
* **Détection du Contournement *mprotect* :** Neutralisation des ruses d'évasion où du code est injecté en lecture/écriture (`rw-`) puis converti dynamiquement en lecture/exécution (`r-x`).
* **Analyse d'Ancrage Disque (Inode 0) :** Signalement chirurgical de toute plage mémoire exécutable (`r-xp`) orpheline de fichier physique sur le stockage, à l'exception des vecteurs système légitimes (`[vdso]`, `[vsyscall]`).
* **Filtrage Heuristique JIT :** Contextualisation des moteurs d'exécution dynamique légitimes (Chromium, Firefox, Electron, Node, JVM) pour éliminer les faux positifs structurels.
* **Injection Temporelle :** Acheminement instantané des alertes de mutation vers le registre du Cahier Différentiel ($T_{-1}$ vs $T_0$).

### 3. Pare-feu Applicatif & Détection Phishing (Net-Guard - Section 11)
* Surveillance passive continue des sockets TCP / UDP ouverts (`/proc/net/tcp`, `/proc/net/udp`) avec corrélation directe Inode $\rightarrow$ PID.
* **Moteur Anti-Combosquatting (DOM) :** Détection de l'usurpation de marques institutionnelles et bancaires légitimes via analyse lexicale contextuelle.
* **Filtrage Binaire d'Adresses (IP) :** Qualification mathématique instantanée des destinations (masques CIDR et filtrage par port) en mode `APPROUVÉ` ou `HOSTILE`.
* **Neutralisation Chirurgicale :** Abattage direct des processus malveillants identifiés par émission ciblée de signaux noyau (`SIGKILL`).
* **Synchronisation Sécurisée :** Rapatriement différentiel atomique (`curl`) des règles officielles depuis GitHub, comparateur d'intégrité physique et contournement strict des caches CDN.

### 4. Renseignement Opérateur & BGP Intel (Net-Intel - Section 12)
* **Résolution DNS PTR Asynchrone :** Moteur multi-thread (`pthread`) non-bloquant maintenant une cadence d'affichage stable à 60 FPS.
* **Cascade Anti-Opacité (Sans PTR) :** Pour les serveurs Web, répartiteurs de charge et réseaux CDN ne déclarant aucun enregistrement inverse :
  1. *Cache dynamique thread-safe* pour un accès immédiat.
  2. *Évaluation binaire (0 ms)* contre les blocs CIDR d'infrastructures majeures définis dans `table_static.conf` (Fastly, Cloudflare, AWS, Google Cloud, Akamai, GitHub, Hetzner, OVHcloud, DigitalOcean).
  3. *Sonde DNS BGP de secours* via Team Cymru (`origin.asn.cymru.com`) pour extraire automatiquement le numéro d'ASN et le pays de déclaration.

### 5. Console Graphique Souveraine
* Interface cyber sombre accélérée par le moteur **Raylib 5.0**.
* Composant d'infobulles contextuelles universelles et adaptatives (`ui_tooltip`).
* Boîtes modales d'ajout de règles personnalisées à chaud (`fw_custom.conf`) et dialogues de confirmation avec acquittement administrateur.
* Journalisation structurée des audits et comparateur différentiel de sessions.

---

## 📂 Structure du Répertoire

```text
RAPTOR/
├── assets/
│   ├── fonts/                   # Polices typographiques vectorielles
│   ├── icons/                   # Ressources graphiques et textures UI
│   └── rules/                   # Registres de filtrage persistants
│       ├── fw_officiel.conf     # Règles communautaires officielles (DOM / IP)
│       ├── fw_custom.conf       # Règles locales définies par l'administrateur
│       └── table_static.conf    # Cartographie CIDR des opérateurs mondiaux & CDN
├── include/
│   └── raptor/                  # Contrats d'API et en-têtes officiels
│       ├── raptor_data.h        # Structures de données et conteneurs du rapport
│       ├── raptor_plugin.h      # Spécification du bus modulaire RaptorPlugin
│       ├── raptor_registry.h    # Registre central d'enregistrement des greffons
│       ├── raptor_net_guard.h   # API Net-Guard (Règles, Sockets, Filtrage)
│       └── raptor_asn_intel.h   # API Net-Intel (ASN, CIDR, Résolution BGP)
├── src/
│   ├── core/                    # Socle universel d'orchestration
│   │   ├── mem_stealth.h        # En-tête de la sonde forensique mémoire
│   │   ├── mem_stealth.c        # Moteur Section 13 (Traque mutations anonymes R-X)
│   │   └── diff_engine.c        # Moteur de comparaison temporelle (T-1 vs T0)
│   ├── plugins/                 # Moteurs d'audit défensif autonomes
│   │   ├── plugin_memory_rwx.c
│   │   ├── plugin_network_sockets.c
│   │   ├── plugin_heuristics_expert.c
│   │   ├── plugin_anomaly_engine.c
│   │   ├── plugin_net_guard.c
│   │   └── plugin_asn_intel.c
│   ├── report/                  # Moteurs de rendu (CLI, HTML, Logs d'audit)
│   ├── ui/                      # Architecture modulaire de l'interface graphique
│   │   ├── ui_engine.c
│   │   ├── ui_layout.c
│   │   ├── ui_resources.c
│   │   ├── ui_tooltip.c         # Composant infobulle universel adaptatif
│   │   ├── modals/              # Boîtes modales d'interaction
│   │   └── views/               # Vues graphiques spécialisées
│   │       ├── view_global.c
│   │       ├── view_processes.c
│   │       ├── view_network.c   # Vue Sockets, Net-Guard & Traque RWX
│   │       ├── view_diff.c      # Cahier Différentiel et alertes forensiques
│   │       └── view_ai.c
│   └── main.c                   # Point d'entrée principal
├── tools/
│   └── test_stealth.c           # Banc d'essai inoffensif de mutation mprotect()
└── CMakeLists.txt               # Recette de construction CMake

🛠️ Compilation & Installation
Prérequis Système (Linux)
Compilateur : GCC (gcc) compatible C99 / POSIX.

Système de build : CMake version 3.16 ou supérieure.

Bibliothèques de développement : pthread, libm, libdl, curl.

Utilitaire de diagnostic réseau : host (paquet bind9-host ou bind-utils).

Sur Debian / Ubuntu / Linux Mint :

sudo apt-get update
sudo apt-get install -y build-essential cmake libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev libgl1-mesa-dev bind9-host curl

Sur Fedora / RHEL :

sudo dnf install -y gcc cmake make libX11-devel libXcursor-devel libXrandr-devel libXinerama-devel libXi-devel mesa-libGL-devel bind-utils curl

Construction du Binaire Principal
Raylib 5.0 est automatiquement téléchargé, configuré et compilé en interne via FetchContent lors de la passe CMake.

# 1. Cloner le dépôt
git clone [https://github.com/BINOKUB-GAME/RAPTOR.git](https://github.com/BINOKUB-GAME/RAPTOR.git)
cd RAPTOR

# 2. Préparer l'environnement de compilation
mkdir -p build && cd build

# 3. Configurer et compiler en mode Release (optimisation native -O3)
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# 4. Lancer RAPTOR
./raptor

Compilation & Épreuve du Banc Forensique
Un programme de simulation autonome permet de valider le déclenchement chirurgical des alertes sans risque pour l'intégrité de la machine :

# Compiler le banc d'essai
gcc -Wall -O2 tools/test_stealth.c -o tools/test_stealth

# Exécuter l'épreuve de mutation en vol
./tools/test_stealth

⚙️ Configuration & Registres Défensifs
Les fichiers de configuration sont situés dans assets/rules/ et peuvent être édités directement :

fw_custom.conf
Enregistre les règles locales ajoutées manuellement ou via l'interface graphique de RAPTOR :


# Règle anti-phishing (Marque | Domaines autorisés | Mots-clés déclencheurs | Description)
DOM | desjardins      | desjardins.com,desjardins.ca | login,connexion,auth | Portail institutionnel

# Règle de filtrage IP (Cible CIDR ou IP:Port | Statut | Description)
IP  | 45.33.32.156     | HOSTILE                      | Relais C2 Cobalt Strike
IP  | 192.168.1.0/24   | APPROUVÉ                     | Sous-réseau d'administration local

table_static.conf
Contient la cartographie des blocs CIDR des grands fournisseurs mondiaux pour une identification en 0 ms :


NET | 151.101.0.0/16   | AS54113 | Fastly Global CDN Anycast
NET | 104.16.0.0/12    | AS13335 | Cloudflare Anycast Edge
NET | 13.32.0.0/15     | AS16509 | Amazon CloudFront CDN

🛡️ Dictionnaire des Fonctions & Architecture
RAPTOR structure l'ensemble de ses fonctions métier au sein d'un dictionnaire officiel numéroté garantissant l'intégrité et la documentation de chaque composant :

Sections 1 à 10 (Fonctions 1 à 57) : Socle universel, traque mémoire, analyse heuristique des descripteurs, gestion des anomalies et moteurs de rapports.

Section 11 (Fonctions 58 à 70) : Moteur Net-Guard (Parsing, normalisation, filtrage binaire, abattage SIGKILL, comparateur différentiel GitHub et worker DNS asynchrone).

Section 12 (Fonctions 71 à 79) : Moteur Net-Intel (Décomposition CIDR, matching statique 0 ms, traque ASN / BGP Team Cymru et cache thread-safe).

Section 13 (Fonctions 80 à 82) : Moteur Forensique & Traque de Mutations R-X (Whitelisting JIT heuristique, scanning maps Inode 0 et dispatch d'événements différentiels temporels).

💬 Communauté & Support
Subreddit officiel : r/RaptorSecurity — Échanges techniques, signalement de faux positifs et annonces de versions.

Dépôt du code source : BINOKUB-GAME/RAPTOR

📄 Licence & Souveraineté
Le projet RAPTOR est distribué sous licence open-source. Il garantit une totale souveraineté logicielle : zéro transmission de télémétrie, fonctionnement autonome hors-ligne et neutralité stricte vis-à-vis des écosystèmes d'analyse cloud tiers.

