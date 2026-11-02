#!/bin/sh
# ==============================================================================
# SCRIPT      : install.sh
# RÉVISION    : Rev 1.0 (Autonomous Zero-Clone Deployment One-Liner)
# PROJET      : RAP-EDIT / RAPTOR DEFENSE COMPANION
# INTÉGRATION : Déploiement chirurgical du binaire statique ELF et de sa page de
#               manuel sans nécessiter le clonage du dépôt git complet.
#               Détection d'architecture machine, contrôle des privilèges et
#               validation de l'installation.
# FONCTIONS   :
#   1. check_architecture (Vérification de la compatibilité matérielle x86_64)
#   2. check_privileges (Détermination du répertoire cible selon les droits)
#   3. fetch_and_install (Téléchargement synchrone du binaire et de la man page)
#   4. verify_installation (Validation de l'intégrité de l'exécutable déployé)
# ==============================================================================

set -e

# Palette de couleurs ANSI
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

BASE_URL="https://raptor.binokub.com/rap-editor"
BIN_URL="${BASE_URL}/bin/rap-edit"
MAN_URL="${BASE_URL}/rap-edit.1"

# 1. Vérification de l'architecture matérielle
check_architecture() {
    ARCH=$(uname -m)
    if [ "$ARCH" != "x86_64" ]; then
        echo -e "${RED}[!] Erreur : Le binaire statique RAP-EDIT est forgé pour x86_64 (architecture actuelle : ${ARCH}).${RESET}"
        exit 1
    fi
}

# 2. Détermination des répertoires cibles selon les privilèges
check_privileges() {
    if [ "$(id -u)" -eq 0 ]; then
        TARGET_BIN="/usr/local/bin"
        TARGET_MAN="/usr/local/share/man/man1"
    else
        TARGET_BIN="${HOME}/.local/bin"
        TARGET_MAN="${HOME}/.local/share/man/man1"
        echo -e "${YELLOW}[i] Installation sans droits root : cible vers ${TARGET_BIN}${RESET}"
    fi
    mkdir -p "${TARGET_BIN}" "${TARGET_MAN}"
}

# 3. Rapatriement et pose physique des composants
fetch_and_install() {
    echo -e "${CYAN}${BOLD}>>> DÉPLOIEMENT RAP-EDIT (RAPTOR DEFENSE) <<<${RESET}"
    
    echo -n "[1/2] Téléchargement du binaire statique ELF 64-bit... "
    curl -sSL "${BIN_URL}" -o "${TARGET_BIN}/rap-edit"
    chmod 755 "${TARGET_BIN}/rap-edit"
    echo -e "${GREEN}[OK]${RESET}"

    echo -n "[2/2] Installation de la page de manuel UNIX... "
    curl -sSL "${MAN_URL}" -o "${TARGET_MAN}/rap-edit.1"
    chmod 644 "${TARGET_MAN}/rap-edit.1"
    echo -e "${GREEN}[OK]${RESET}"
}

# 4. Contrôle d'intégrité final
verify_installation() {
    if [ -x "${TARGET_BIN}/rap-edit" ]; then
        echo -e "\n${GREEN}${BOLD}[✓] RAP-EDIT a été installé avec succès :${RESET}"
        echo -e "    - Exécutable : ${TARGET_BIN}/rap-edit"
        echo -e "    - Manuel     : ${TARGET_MAN}/rap-edit.1"
        echo -e "\n${BOLD}Exemples d'utilisation :${RESET}"
        echo -e "    ${CYAN}rap-edit <fichier1> [fichier2]${RESET}"
        echo -e "    ${CYAN}man rap-edit${RESET}\n"
    else
        echo -e "${RED}[!] Échec critique de l'installation.${RESET}"
        exit 1
    fi
}

# Séquence d'exécution
check_architecture
check_privileges
fetch_and_install
verify_installation
