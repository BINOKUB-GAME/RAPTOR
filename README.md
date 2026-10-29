# 🦅 RAPTOR v2.0 // Disjoncteur Système & Traque Mémoire

[![Licence](https://img.shields.io/badge/Licence-GPLv3%20Souverain-00d2ff.svg)](#)
[![Langage](https://img.shields.io/badge/Langage-C99%20Strict%20%2F%20POSIX-00e676.svg)](#)
[![Mémoire](https://img.shields.io/badge/M%C3%A9moire-0--Heap%20%2F%20%3C2%20Mo%20RAM-ff3355.svg)](#)
[![Plateforme](https://img.shields.io/badge/Plateforme-Linux%20x86__64%20%7C%20FreeBSD%20(WIP)-7928ca.svg)](#)
[![Discussions](https://img.shields.io/badge/GitHub-Discussions%20Hub-blue.svg?logo=github)](https://github.com/BINOKUB-GAME/RAPTOR/discussions)
[![Communauté](https://img.shields.io/badge/Reddit-r%2FRaptorSecurity-ff4500.svg?logo=reddit)](https://www.reddit.com/r/RaptorSecurity/)
[![Rapport de test](https://img.shields.io/badge/Test_Distro-Rapporter_votre_syst%C3%A8me_(1--Clic)-brightgreen.svg?style=for-the-badge&logo=linux)](https://github.com/BINOKUB-GAME/RAPTOR/issues/new?template=distro_test.yml)

**RAPTOR** est une suite défensive d'urgence et d'investigation mémoire conçue pour opérer sur les systèmes d'exploitation sous contraintes opérationnelles extrêmes. Développé en **C99 strict**, RAPTOR assure le rôle de disjoncteur différentiel logiciel lorsque la machine subit une saturation mémoire (OOM), une attaque par rançongiciel ou une corruption avancée de l'espace utilisateur.

Le projet propose une **double architecture complémentaire** :
1. **La Suite Tactique CLI (`rap`) :** Des utilitaires d'intervention ultra-légers fonctionnant sur un modèle **0-heap** (aucune allocation dynamique `malloc`/`calloc`, buffers bornés sur pile ou `.rodata`, empreinte résidente inférieure à 2 Mo, immunité OOM `oom_score_adj = -1000`).
2. **La Station de Surveillance Visuelle (`raptor`) :** Une interface graphique de supervision complète en **Raylib 5.0** avec cartographie des flux réseau, registre forensique temporel différentiel (LIFO) et alertes asynchrones.

---

👉 **[Contribuer à la matrice de compatibilité : Soumettre un rapport de test de votre distribution en 1 clic](https://github.com/BINOKUB-GAME/RAPTOR/issues/new?template=distro_test.yml)**

---

## 🎯 Piliers Techniques & Capacités Opérationnelles

### 1. Console Tactique Réseau 0-Heap (`rap netop` / `rap no`)
* **Décodage Noyau Direct :** Parsing temps réel des sockets `/proc/net/tcp` et `/proc/net/udp` sans invoquer de sous-processus externe (`ss`, `lsof`, `netstat`).
* **Base de Renseignement Embarquée :** Corrélation instantanée de chaque flux sortant contre **19 927 signatures hostiles locales** (C2, botnets, nœuds d'exfiltration).
* **Détection de Reverse Shells :** Identification immédiate des redirections de descripteurs de fichiers suspects (`fd/0` et `fd/1` pointant vers des sockets réseau).
* **Remédiation Immédiate en Ligne de Commande :**
  * Touche `b` : Injection instantanée d'une règle Netfilter (`iptables -I INPUT -s <IP> -j DROP`) ou route trou noir.
  * Touche `k` : Abattage chirurgical du processus incriminé par signal `SIGKILL`.

### 2. Sentinelle Synchrone Anti-Ransomware (`rap sentry`)
* **Interception Préventive Noyau :** Utilisation de l'API synchrone Linux `fanotify(7)` avec le masque d'accès bloquant `FAN_OPEN_PERM`.
* **Empreinte 0 % CPU au Repos :** Mise en sommeil passive dans le noyau. Le thread hostile est figé par le noyau *avant* d'avoir pu altérer le fichier surveillé.
* **Décapitation Immédiate :** Abattage en moins de 40 microsecondes du processus malveillant tentant de toucher aux répertoires protégés ou aux fichiers leurres (*canary tokens*).

### 3. Traque Mémoire & Mutations Anonymes Inode 0 (`rap rx` & Section 13)
* **Contournement de `mprotect` :** Neutralisation des tactiques d'évasion consistant à allouer de la mémoire en lecture/écriture (`rw-`) puis à la commuter dynamiquement en exécution (`r-x`).
* **Forensique Inode 0 :** Détection chirurgicale des segments mémoires exécutables (`r-xp`) orphelins de fichier disque (charge virale injectée directement en RAM), en isolant les vecteurs système (`[vdso]`, `[vsyscall]`).
* **Registre d'Exclusion (`mem_whitelist.conf`) :** Filtrage externalisé des faux positifs légitimes (moteurs JIT V8, SpiderMonkey, trampolines `libffi`) sans aucune chaîne codée en dur.

### 4. Détection des Rootkits Userland & Détournements (`rap preload` & Section 14)
* **Audit Global :** Contrôle passif direct de `/etc/ld.so.preload` pour neutraliser les injections affectant la globalité de l'espace utilisateur.
* **Hijacking Ciblé :** Parsing chirurgical de `/proc/[PID]/environ` sur les délimiteurs nuls (`\0`) pour intercepter les tentatives d'évasion locale via la variable `LD_PRELOAD`.

### 5. Téléscripteur Forensique Déterministe (`rapless`)
* Pager de terminal 0-heap conçu pour inspecter en continu les flux système volumineux (`/proc/kallsyms`, cartographies mémoires de plusieurs gigaoctets) sans risquer de déclencher l'OOM-Killer par dépassement de mémoire tampon.

### 6. Station Graphique Raylib & Notifications Bureau
* **Interface Haute Définition :** Visualisation matricielle temps réel (60 FPS) avec thème cyber sombre et infobulles contextuelles adaptatives.
* **Cahier Différentiel Temporel :** Journalisation chronologique LIFO comparant l'état du système ($T_{-1}$ vs $T_0$) avec horodatage d'injection.
* **Notifications Asynchrones :** Émission sur le bureau via `notify-send` découplé par un double fork pour maintenir la fluidité du moteur de rendu sans créer de processus zombie.

---

## 📂 Architecture du Dépôt

```text
RAPTOR/
├── .github/
│   └── ISSUE_TEMPLATE/
│       └── distro_test.yml      # Formulaire officiel de test par distribution
├── assets/
│   ├── fonts/                   # Polices typographiques vectorielles
│   ├── icons/                   # Ressources graphiques de la console
│   └── rules/                   # Registres défensifs s
│       ├── fw_officiel.conf     # Règles de filtrage réseau officielles
│       ├── fw_custom.conf       # Règles locales définies par l'opérateur
│       ├── mem_whitelist.conf   # Whitelist des moteurs JIT et libffi
│       └── table_static.conf    # Cartographie CIDR des opérateurs et CDN
├── include/
│   └── raptor/                  # Contrats d'API et en-têtes C99
│       ├── raptor_data.h        # Structures de données mémoires bornées
│       ├── raptor_plugin.h      # Spécification modulaire des greffons
│       ├── raptor_net_guard.h   # API Net-Guard & Sockets
│       └── raptor_asn_intel.h   # Moteur de résolution ASN/CIDR
├── src/
│   ├── cli/                     # Outils d'intervention 0-Heap (<2 Mo RAM)
│   │   ├── rap_netop.c          # Console interactive /proc/net/tcp & riposte
│   │   ├── rap_sentry.c         # Sentinelle synchrone fanotify
│   │   ├── rap_rx.c             # Scanner chirurgical Inode 0
│   │   └── rapless.c            # Pager de secours sans allocation
│   ├── core/                    # Moteurs forensiques et interception
│   │   ├── mem_stealth.c        # Analyse des cartographies maps
│   │   ├── preload_hunter.c     # Inspection des chargeurs dynamiques
│   │   └── diff_engine.c        # Moteur différentiel temporel
│   ├── plugins/                 # Greffons d'audit système
│   ├── ui/                      # Station graphique Raylib 5.0
│   │   ├── modals/              # Boîtes d'action et guide interactif
│   │   └── views/               # Vues spécialisées (Processus, Réseau, Diff)
│   └── main.c                   # Point d'entrée de la station graphique
├── tools/
│   ├── test_stealth.c           # Banc d'essai inoffensif d'injection mprotect
│   └── test_preload.c           # Banc d'essai inoffensif de hook LD_PRELOAD
└── CMakeLists.txt               # Recette de construction CMake

🛠️ Compilation & Déploiement
Prérequis Système
Compilateur : GCC ou Clang (support C99 et POSIX requis).

Système de construction : CMake 3.16 ou supérieur.

Bibliothèques : pthread, libm, libdl, curl.

Pour la station graphique (optionnel) : Bibliothèques de développement X11 et OpenGL.

Sur Debian / Ubuntu / Linux Mint :

sudo apt-get update
sudo apt-get install -y build-essential cmake libx11-dev libxcursor-dev \
    libxrandr-dev libxinerama-dev libxi-dev libgl1-mesa-dev curl libnotify-bin

Sur Fedora / RHEL / AlmaLinux :

sudo dnf install -y gcc cmake make libX11-devel libXcursor-devel \
    libXrandr-devel libXinerama-devel libXi-devel mesa-libGL-devel curl libnotify


Construction des Binaires

# 1. Cloner le dépôt
git clone [https://github.com/BINOKUB-GAME/RAPTOR.git](https://github.com/BINOKUB-GAME/RAPTOR.git)
cd RAPTOR

# 2. Préparer l'environnement de compilation
mkdir -p build && cd build

# 3. Configurer et compiler l'ensemble des modules
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

Guide d'Utilisation Rapide (CLI rap)
Les binaires CLI compilés sont directement utilisables sur un terminal physique ou via une session SSH de secours :

# Lancer la console tactique réseau plein écran (détection reverse shells & signatures)
sudo ./bin/rap netop

# Activer la sentinelle anti-ransomware sur un répertoire sensible
sudo ./bin/rap sentry /var/www/html /home

# Scanner la mémoire vive pour traquer les injections anonymes (Inode 0)
sudo ./bin/rap rx

# Lancer la station graphique complète (si serveur X11 actif)
./bin/raptor

Bancs d'Essai Forensiques
Deux utilitaires inoffensifs permettent de valider la réactivité de RAPTOR sans altérer la stabilité de la machine hôte :

# Compiler les bancs de simulation
gcc -Wall -O2 ../tools/test_stealth.c -o tools/test_stealth
gcc -Wall -O2 ../tools/test_preload.c -o tools/test_preload

# Test 1 : Validation de l'interception de mutation de page (mprotect rw- -> r-x)
./tools/test_stealth

# Test 2 : Validation de l'interception de détournement de bibliothèque
LD_PRELOAD=/lib/x86_64-linux-gnu/libm.so.6 ./tools/test_preload

Feuille de Route : Portage FreeBSD (UNIX Portability)
Le portage de RAPTOR vers les architectures FreeBSD (ciblant les plateformes sécurisées OPNsense, pfSense et serveurs TrueNAS) est en cours de modélisation :

Inspection des Processus : Remplacement de /proc par l'API binaire native sysctl(3) (KERN_PROC_PROC, KERN_PROC_VMMAP, KERN_PROC_FILEDESC).

Interception des I/O : Évaluation des notifications de descripteurs via kqueue(2) / EVFILT_VNODE et des mécanismes d'audit de sécurité BSM.

Remédiation Réseau : Raccordement direct à l'interface ioctl /dev/pf de Packet Filter (PF) pour alimenter atomiquement des tables d'adresses bannies (table <raptor_drop> persist).

Rejoignez les discussions d'ingénierie ouvertes sur notre Forum GitHub Discussions pour suivre ou contribuer à ce chantier.

💬 Communauté & Support
Rapports de compatibilité distributions : Formulaire 1-Clic

Hub d'ingénierie & RFC : GitHub Discussions

Subreddit officiel : r/RaptorSecurity


### 📦 Téléchargements Directs (Prêts à l'emploi)

| Format | Architecture | Lien de téléchargement direct |
| :--- | :--- | :--- |
| **Paquet Debian / Ubuntu / Mint (`.deb`)** | `x86_64` (amd64) | [Télécharger le .deb]
(https://github.com/BINOKUB-GAME/RAPTOR/releases/download/v2.0/raptor-v2.0.0-Linux-x86_64.deb) |
| **Archive universelle CLI + Raylib (`.tar.gz`)** | `x86_64` | [Télécharger le .tar.gz]
(https://github.com/BINOKUB-GAME/RAPTOR/releases/download/v2.0/raptor-v2.0.0-linux-x86_64.tar.gz) |

#### Installation rapide via le paquet .deb :
```bash
sudo dpkg -i raptor_2.0.0_amd64.deb
sudo apt-get install -f   # Si des dépendances runtime X11 manquent

Exécution directe depuis l'archive .tar.gz :

tar -xzf raptor-v2.0-linux-x86_64.tar.gz
cd raptor-v2.0
sudo ./rap netop   # Lancer immédiatement la suite CLI 0-heap

---

### Vérification de l'intégrité (Pratique recommandée en cybersécurité)

Avant de téléverser vos fichiers, générez leurs sommes de contrôle dans votre terminal sous Debian :

```bash
sha256sum raptor_2.0.0_amd64.deb raptor-v2.0-linux-x86_64.tar.gz

📄 Licence & Philosophie
RAPTOR est distribué sous licence open-source. Il garantit une indépendance totale : aucune télémétrie distante, fonctionnement hors-ligne garanti et déconnexion stricte vis-à-vis des infrastructures cloud tierces.


