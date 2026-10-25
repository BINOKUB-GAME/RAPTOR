# CAHIER DES CHARGES TECHNIQUE D'INGÉNIERIE : RAP_REFLEX
**Projet :** RAPTOR DEFENSE v2.0  
**Sous-système :** `src/core/rap_reflex` (Remédiation Active Chirurgicale & Survie Système)  
**Date d'établissement :** 25 Octobre 2026  
**Révision :** Spécification Rev 1.0  
**Norme de développement :** C99 strict (`-std=c99 -pedantic -Wall -Wextra -Werror`), 0-allocation dynamique (`0-heap`), POSIX.1-2008

---

## 1. CONTEXTE ET DOCTRINE OPÉRATIONNELLE

### 1.1. Positionnement Stratégique
Le module `rap_reflex` constitue le bras armé de l'architecture RAPTOR DEFENSE. Il opère selon la doctrine du « Disjoncteur de Dernier Recours » (*Last-Resort Watchdog*).
* Contrairement aux agents EDR lourds qui s'effondrent lors de saturations de mémoire vive ou de dénis de service locaux, `rap_reflex` est conçu pour fonctionner en milieu dégradé et sous famine mémoire totale.
* Il n'effectue aucune scrutation de flux de manière autonome : c'est un effecteur pur actionné uniquement sur réception d'un événement certifié émanant des capteurs d'intégrité (SENTRY, `hunt`, `net`).
* Son objectif opérationnel est de geler et neutraliser un processus agresseur en moins d'une milliseconde ($\le 1\text{ ms}$) sans perturber le reste de l'infrastructure de production.

### 1.2. Contraintes d'Ingénierie Non-Négociables
1. **Zéro Allocation Dynamique (0-Heap) :** Proscription intégrale de `malloc()`, `calloc()`, `realloc()` et `free()`. Toutes les structures, tampons de capture et contextes sont alloués statiquement dans les sections `.bss` et `.rodata` à l'édition des liens.
2. **Immunité Système Absolue (Anti-OOM) :** Verrouillage de la totalité des pages du binaire en mémoire physique via `mlockall()` et ajustement du score de sacrifice du noyau à `-1000` (`oom_score_adj`).
3. **Étanchéité et Zéro Trafic Émis :** Aucune ouverture de socket sortante, aucune dépendance réseau, aucune résolution DNS.
4. **Non-Blocage Déterministe :** Interdiction des appels d'E/S synchrones lents sur disque rotatif ou SSD en cours d'attaque. Utilisation d'un tampon circulaire binaire résidant en mémoire vive (`/dev/shm`).

---

## 2. SPÉCIFICATIONS DES INTERFACES ET DÉCLENCHEURS (TRIGGERS)

`rap_reflex` expose une interface fonctionnelle unifiée. L'exécution d'une riposte exige un niveau de certitude absolu (taux de faux positifs nul).

### 2.1. Matrice des Événements Déclencheurs

| Constante Déclencheur | Module Émetteur | Critère d'Activation Noyau | Action Appliquée |
| :--- | :--- | :--- | :--- |
| `TRIGGER_CANARY_BREACH` | SENTRY (`fanotify`/`inotify`) | Violation en écriture (`IN_MODIFY`), destruction (`IN_DELETE`) ou déplacement d'un canari leurre armé. | `SIGSTOP` $\rightarrow$ Coupure FD $\rightarrow$ Snapshot $\rightarrow$ `SIGKILL` |
| `TRIGGER_MEMORY_MUTATION`| `rap hunt` (`/proc/[pid]/maps`) | Détection d'une plage mémoire exécutable et modifiable (`rwx` ou `r-xp`) avec un **Inode 0** (injection sans fichier). | `SIGSTOP` $\rightarrow$ Coupure FD $\rightarrow$ Snapshot $\rightarrow$ `SIGKILL` |
| `TRIGGER_C2_ESTABLISHED` | `rap net` (Net-Intel `.rodata`) | Connexion TCP établie vers une IP hostile répertoriée dans la table figée des botnets bancaires. | Révocation immédiate des sockets $\rightarrow$ `SIGSTOP` $\rightarrow$ `SIGKILL` |
| `TRIGGER_BINARY_SEVERED` | `rap hunt` (`/proc/[pid]/exe`) | Exécution d'un binaire dont le fichier d'origine a été effacé du disque dur (`(deleted)`). | `SIGSTOP` $\rightarrow$ Snapshot $\rightarrow$ `SIGKILL` |

---

## 3. PROTOCOLE D'ACTION RÉFLEXE SÉQUENTIEL

Dès l'appel de l'aiguilleur `rap_reflex_dispatch()`, la séquence chronologique suivante s'exécute de façon atomique :

[ Événement Certifié ]
│
▼
┌─────────────────────────────────┐
│ ÉTAPE 1 : GEL HORLOGE (SIGSTOP) │ ──► Ordonnanceur Linux stoppe le processus et ses threads
└─────────────────────────────────┘
│
▼
┌─────────────────────────────────┐
│ ÉTAPE 2 : RÉVOCATION RÉSEAU     │ ──► Fermeture forcée des sockets via /proc/[pid]/fd
└─────────────────────────────────┘
│
▼
┌─────────────────────────────────┐
│ ÉTAPE 3 : INSTANTANÉ FORENSIQUE │ ──► Copie de l'état (PID, UID, cmdline, hash) dans /dev/shm
└─────────────────────────────────┘
│
▼
┌─────────────────────────────────┐
│ ÉTAPE 4 : PURGE DÉFINITIVE      │ ──► Émission du signal non interceptable kill -9
└─────────────────────────────────┘

### Étape 1 : Paralysie Immédiate (`SIGSTOP`)
* **Délai :** $< 100\text{ }\mu\text{s}$.
* **Opération :**
  ```c
  kill(target_pid, SIGSTOP);
  kill(-target_pid, SIGSTOP); /* Neutralisation de l'ensemble du process group */

Effet : Le processus passe dans l'état TASK_STOPPED. L'ordonnanceur du noyau suspend l'allocation de cycles d'horloge. Le chiffrement malveillant ou la corruption de fichiers est arrêté sur-le-champ.

Étape 2 : Révocation Sèche des Descripteurs Réseau
Opération : Parcours du répertoire /proc/[pid]/fd/. Pour chaque lien symbolique pointant vers un motif socket:[*], émission d'un appel système de rupture de liaison ou injection d'un filtre netfilter local temporaire sur l'inode de la socket.

Effet : Empêche la transmission d'un paquet de panique ou l'exfiltration d'une clé cryptographique vers le serveur C2.

Étape 3 : Capture Médico-Légale Statique
Opération : Extraction des métadonnées du processus figé sans faire appel à des buffers dynamiques :

Horodatage nanoseconde via clock_gettime(CLOCK_REALTIME, ...).

Identifiants : pid, ppid, uid, gid.

Ligne de commande : Lecture directe de /proc/[pid]/cmdline bornée à 256 octets.

Empreinte : Échantillon des 512 premiers octets de la région mémoire fautive.

Persistance : Écriture directe dans le tampon binaire circulaire rap_blackb

Étape 4 : Destruction Physique (SIGKILL)
Opération :

kill(target_pid, SIGKILL);
kill(-target_pid, SIGKILL);

Validation : Contrôle non bloquant via waitpid(target_pid, &status, WNOHANG).

Effet : Libération immédiate des pages de mémoire vive parasitées par l'attaquant.

4. ARCHITECTURE DU CODE ET DOCUMENTS DE RÉFÉRENCE
L'implémentation est découpée en modules C99 autonomes et indépendants, respectant strictement la norme de modularité du projet.

4.1. Organisation de l'Arborescence

src/core/
├── rap_reflex.h       # Définition des structures fixes, énumérations et prototypes
├── rap_reflex.c       # Moteur de séquençage et émission des signaux noyau
├── rap_oom.h          # Prototypes d'immunisation mémoire et verrouillage RAM
├── rap_oom.c          # Implémentation mlockall et ajustement oom_score_adj
├── rap_blackbox.h     # Interface de journalisation circulaire 0-allocation
└── rap_blackbox.c     # Gestion du fichier binaire persistant /dev/shm

5. MÉCANISME D'IMMUNITÉ SYSTÈME (rap_oom.c)
Pour garantir que RAPTOR ne sera pas abattu par le noyau lors d'une attaque par déni de service mémoire orchestrée pour faire tomber les EDR :

Ajustement OOM Score :

Ouverture du descripteur de fichier /proc/self/oom_score_adj.

Écriture de la chaîne -1000\n.

Le noyau Linux attribue au binaire une priorité de survie absolue identique à celle du processus init (systemd).

Verrouillage de la RAM (mlockall) :

Exécution de mlockall(MCL_CURRENT | MCL_FUTURE).

Interdiction formelle au sous-système de mémoire virtuelle de basculer les pages de RAPTOR sur la partition swap.

Plafond RSS Vérifié :

Consommation résidente en mémoire physique strictement bornée à moins de 2,5 Mo.

6. MODES D'EXPLOITATION ET CONFIGURATION
6.1. Interface en Ligne de Commande (CLI)
Le moteur SENTRY intègre l'armement de REFLEX par des drapeaux explicites :


# 1. Surveillance passive (Valeur par défaut - Sécurité maximale contre faux positifs)
rap sentry

# 2. Validation opérationnelle (Déroule tout le code mais ne tue pas le processus)
rap sentry --arm --dry-run

# 3. Armement opérationnel complet (Remédiation active instantanée)
rap sentry --arm

# 4. Consultation des enregistrements de la boîte noire
rap status --blackbox

6.2. Configuration du Démon Système (systemd)
Fichier d'unité de production /etc/systemd/system/raptor-sentry.service :

[Unit]
Description=RAPTOR Sovereign Sentry & Reflex Watchdog
Documentation=[https://raptor.binokub.com](https://raptor.binokub.com)
DefaultDependencies=no
After=local-fs.target

[Service]
Type=simple
ExecStart=/usr/local/bin/rap sentry --arm
Restart=always
RestartSec=1s

# Privilèges minimaux nécessaires selon les Linux Capabilities
CapabilityBoundingSet=CAP_KILL CAP_SYS_PTRACE CAP_SYS_RESOURCE CAP_DAC_READ_SEARCH
AmbientCapabilities=CAP_KILL CAP_SYS_PTRACE CAP_SYS_RESOURCE CAP_DAC_READ_SEARCH

# Immunité totale contre la famine mémoire
OOMScoreAdjust=-1000
LimitMEMLOCK=infinity

[Install]
WantedBy=sysinit.target

6.3. Interface Graphique (GUI raptor)
Widget d'Armement : Commutateur sécurisé exigeant une double confirmation pour basculer de l'état PASSIVE à l'état ARMED.

Indicateurs d'État :

Vert fixe : Surveillance passive active.

Rouge clignotant : Module réflexe armé.

Jaune/Orange : Mode simulation (dry-run).

Visualiseur Boîte Noire : Composant dédié lisant dynamiquement la zone mémoire partagée /dev/shm pour afficher la liste des neutralisations sans nécessiter d'analyse de fichiers journaux.


7. PROTOCOLE DE QUALIFICATION ET TESTS D'ACCEPTATIONToute mise en production du module rap_reflex doit obligatoirement valider les quatre tests suivants sur environnement d'intégration :Audit de Mémoire Statique (Valgrind) :Commande : valgrind --tool=memcheck --leak-check=full ./build/rap sentry --arm --dry-runRésultat attendu : 0 bytes in 0 blocks allocated lors des phases de traitement d'incident.Contrôle de Latence Chronométrique :Mesure matérielle de l'intervalle entre l'interception de l'alerte noyau et l'envoi du signal SIGSTOP.Critère d'acceptation : Latence médiane $\le 150\text{ }\mu\text{s}$, latence maximale absolue $\le 1000\text{ }\mu\text{s}$.Épreuve de Famine Mémoire Hostile :Lancement d'un processus parasite consommant 99 % de la mémoire vive pour déclencher l'OOM Killer du noyau.Critère d'acceptation : Le processus parasite est tué par l'OS ; rap sentry conserve son exécution et son état armé sans interruption.Validation de Non-Régression en Mode Dry-Run :Génération d'une violation contrôlée de fichier canari.Critère d'acceptation : Génération intégrale de l'enregistrement dans la boîte noire binaire, statut kill_confirmed = false, processus de test non interrompu.
