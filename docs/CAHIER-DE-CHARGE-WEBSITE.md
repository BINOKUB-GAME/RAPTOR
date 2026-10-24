# CAHIER DES CHARGES TECHNIQUE // PORTAIL WEB RAPTOR v2.0
## RÉFÉRENTIEL D'INGÉNIERIE & D'ARCHITECTURE DU SITE OFFICIEL

================================================================================
1. CONTEXTE & OBJECTIFS STRATÉGIQUES
================================================================================
Le présent document définit les spécifications techniques, fonctionnelles et
ergonomiques du portail web officiel de RAPTOR v2.0.

Objectifs fondamentaux :
1. Remplacer l'esthétique "hacker sombre" (noir et cyan fluorescent) par une
   identité sobre, souveraine et institutionnelle inspirée de l'écosystème
   UNIX / FreeBSD (dominance blanc pur et rouge vermillon).
2. Fournir un système documentaire vivant et modulaire :
   - Présentation et téléchargement souverain (sources Git, CMake).
   - Archive interactive des pages de manuel UNIX (man pages complètes en ligne).
   - Foire Aux Questions (FAQ) technique avec sommaire dynamique interactif.
   - Espace Communauté, gouvernance et sécurité.
3. Garantir une conception modulaire permettant l'ajout aisé de nouvelles
   sections sans refonte structurelle.
4. Maintenir une stricte souveraineté technique : zéro framework lourd (zéro
   React/Vue/Tailwind distants), HTML5 sémantique, CSS3 modulaire et JavaScript
   natif sans dépendance externe.

================================================================================
2. CHARTE GRAPHIQUE & IDENTITÉ VISUELLE
================================================================================
Inspiration directe : Portail officiel FreeBSD (bandeau supérieur d'autorité,
typographie nette, hiérarchie de lecture aérée).

2.1. Palette chromatique normalisée :
--------------------------------------------------------------------------------
• Rouge Institutionnel (Bandeau & En-têtes) : #8C1D18 (Rouge bordeaux FreeBSD)
• Rouge Accentuation (Survol & Alertes)     : #B71C1C / #D32F2F
• Fond Principal (Zone de Lecture)          : #FFFFFF (Blanc pur)
• Fond Secondaire (Cartes & Tableaux)       : #F8F9FA (Gris perle technique)
• Typographie Corps de Texte                : #1E232A (Gris ardoise profond)
• Blocs de Code & Console Man               : #181B1F (Fond sombre haute lisibilité)
• Bordures de Séparation                    : #E1E4E8 (Gris neutre)

2.2. Typographie :
--------------------------------------------------------------------------------
• Titres et Navigation : Police sans-serif moderne, nette et institutionnelle
  (Inter, system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif).
• Commandes, Man Pages & Code : Police à chasse fixe standardisée
  ('JetBrains Mono', 'Fira Code', 'DejaVu Sans Mono', 'Courier New', monospace).

================================================================================
3. TRANSFORMATION DU LOGO OFFICIEL (RAPTOR-ICONE)
================================================================================
3.1. Problématique du composant d'origine :
Le logo source actuel est un carré tramé matriciel (fond noir opaque avec
tracé cyan fluorescent).

3.2. Spécifications de conversion :
• Vectorisation intégrale au format SVG pur (mise à l'échelle infinie sans perte).
• Réticule de visée et circuits logiques : Teinte Rouge Vermillon (#8C1D18).
• Tête de raptor cybernétique : Lignes d'armature en rouge vif (#B71C1C) et
  blindage contrasté en blanc franc (#FFFFFF) ou gris clair (#ECEFF1).
• Fond : Transparence alpha 100 % absolue (aucune boîte carrée résiduelle).
• Intégration : Le logo doit s'incruster indifféremment sur le bandeau rouge
  du header en version blanche/claire, et sur fond blanc dans le corps de page.

================================================================================
4. ARCHITECTURE TECHNIQUE & ARBORESCENCE DU DÉPÔT WEB
================================================================================
L'arborescence est isolée dans le sous-dossier `docs/` pour publication directe
sur GitHub Pages ou tout serveur HTTP statique autonome (Nginx, Apache, Caddy) :

docs/
├── index.html              # Portail d'accueil, vision, architecture et téléchargement
├── man.html                # Archive interactive des man pages Unix (Section Man)
├── faq.html                # Base de connaissances technique & FAQ à deux colonnes
├── community.html          # Gouvernance, contribution, dépôts Git et sécurité
├── css/
│   ├── raptor-core.css     # Variables CSS (rouge/blanc), réinitialisation, grille
│   ├── raptor-header.css   # Navigation supérieure institutionnelle et responsive
│   ├── raptor-man.css      # Mise en page fidèle aux sorties troff/groff Unix
│   └── raptor-faq.css      # Mise en page spécifique FAQ (sommaire flottant)
├── js/
│   ├── man-engine.js       # Moteur de recherche et de rendu instantané des pages man
│   └── faq-nav.js          # Génération automatique du sommaire et défilement fluide
└── assets/
    ├── raptor-logo-red.svg # Logo officiel transparent rouge institutionnel
    ├── raptor-logo-white.svg # Variante blanche transparente pour fond rouge
    └── img/                # Schémas techniques et captures d'écran

================================================================================
5. SPÉCIFICATIONS FONCTIONNELLES DES MODULES
================================================================================

5.1. Module I : Portail Principal (`index.html`)
--------------------------------------------------------------------------------
• En-tête Institutionnel :
  - Logo vectoriel transparent avec mention "RAPTOR DEFENSE".
  - Menu principal : Accueil, Documentation / Man, FAQ, Communauté, GitHub.
  - Champ de recherche globale rapide.
• Bannière Principale (Hero Banner) :
  - Titre souverain : "RAPTOR DEFENSE // Surveillance Mémoire & Audit Linux".
  - Devise : "Le pouvoir d'auditer passivement, la rigueur de protéger."
  - Boutons d'action : [Obtenir RAPTOR v2.0] (lien vers archive/git) et
    [Consulter les Man Pages].
• Grille des Piliers Techniques (style cartes FreeBSD) :
  1. Traque R-X Inode 0 : Détection sans signature des shellcodes injectés.
  2. Net-Guard Souverain : Évaluation CIDR et blocage des vecteurs C2 hostiles.
  3. LD_PRELOAD Hunter : Audit double palier (fichier global et environnement).
  4. Mode CLI de Secours : Binaire 'rap' autonome opérant sous < 2 Mo de RAM.
  5. Téléscripteur Zéro-Heap : Pager 'rapless' autonome sans allocation dynamique.
  6. Journal d'Audit Forensique : Traçabilité immuable post-mortem sous AUDIT.TXT.
• Guide Rapide de Construction :
  - Commandes normalisées de clonage et compilation CMake Release (-O3).
  - Matrice synthétique des dépendances matérielles et logicielles.

5.2. Module II : Archive des Pages de Manuel en Ligne (`man.html`)
--------------------------------------------------------------------------------
• Barre d'Interrogation Interactive :
  - Champ de saisie : recherche par mot-clé avec bascule `man` ou `apropos`.
  - Sélecteur de section :
    * Section 1 : Commandes utilisateur et utilitaires de secours (rap, rapless).
    * Section 5 : Formats des fichiers de règles (.conf).
    * Section 8 : Administration de surveillance et détection d'intrusions.
  - Sélecteur de sous-commandes directes :
    rap(1), rap-status(1), rap-top(1), rap-hunt(1), rap-rx(1),
    rap-preload(1), rap-net(1), rap-kill(1), rap-freeze(1), rapless(1),
    mem_whitelist(5), fw_officiel(5).
• Rendu Typographique Standardisé UNIX :
  - Titres de rubriques en gras bordeaux : NAME, SYNOPSIS, DESCRIPTION,
    OPTIONS, EXIT STATUS, ENVIRONMENT, FILES, EXAMPLES, AUTHORS.
  - Coloration syntaxique discrète des options (-q, -r, -f, -v, -n).
  - Bouton de copie en un clic pour les pipelines tactiques.

5.3. Module III : Base de Connaissances & FAQ (`faq.html`)
--------------------------------------------------------------------------------
• Architecture Ergonomique à Deux Colonnes (façon FreeBSD FAQ) :
  - Colonne Latérale Fixe (Sticky Sidebar droite ou gauche) : Table des matières
    automatique indexant chaque question/réponse avec surbrillance dynamique au scroll.
  - Zone de Contenu Principale : Articles techniques détaillés classés par thèmes :
    1. Introduction & Principes de Conception (Pourquoi C99 sans heap ?).
    2. Gestion des Faux Positifs (Moteurs JIT JavaScript et trampolines libffi).
    3. Qualification Réseau & Net-Guard (Règles CIDR et classification des ports).
    4. Procédures d'Urgence (Comment neutraliser un implant avec rap kill/freeze).
    5. Intégration Continue & Automatisation (Exploitation des codes POSIX 0/1/2).

5.4. Module IV : Espace Communauté & Souveraineté (`community.html`)
--------------------------------------------------------------------------------
• Dépôt & Code Source : Liens directs vers le dépôt Git principal.
• Guide de Développement de Greffons : Spécification de l'interface `RaptorPlugin`.
• Signalement d'Incidents de Sécurité : Procédure de divulgation responsable.
• Licences & Souveraineté : Principes de transparence et d'indépendance logicielle.

================================================================================
6. CRITÈRES DE PERFORMANCE & DE QUALITÉ
================================================================================
• Temps de chargement initial inférieur à 100 ms sur connexion standard.
• Poids global de la page d'accueil inférieur à 300 Ko (hors images d'illustration).
• Compatibilité responsive complète : bureau, tablette et smartphone.
• Rendu typographique rigoureux sur tous les navigateurs (Chromium, Firefox, Safari).
• Modularité CSS : Ajout d'une nouvelle page ou section par simple inclusion
  de composants stylisés sans effet de bord.