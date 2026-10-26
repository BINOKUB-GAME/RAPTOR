#!/usr/bin/env python3a# ==============================================================================
# NOM DU SCRIPT   : tools/gen_threats.py
# RÉVISION        : Rev 1.0
# PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
# RESPONSABILITÉ  : Pipeline autonome d'extraction et de compilation des flux OSINT.
#                   - Téléchargement résilient des flux publics de réputation IP.
#                   - Filtrage chirurgical et consolidation des scores de menace (1-100).
#                   - Déduplication et tri croissant pour recherche binaire en C99.
#                   - Production du fichier d'en-tête C99 (src/cli/rap_threat_data.h).
#                   - Production du module de recherche web (docs/js/threat_data.js).
#
# BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.0) :
#   - Intégration du flux Feodo Tracker (abuse.ch) : Botnets, rançongiciels et C2.
#   - Intégration du flux IPsum (Stamparm) : Agrégateur multi-listes noires.
#   - Seuil de réputation configurable (MIN_SEVERITY_LEVEL) pour éliminer les bruits.
#   - Génération C99 100% .rodata pour exécution 0-heap dans 'rap net -i'.
#   - Génération Web structurée en table de hachage O(1) pour analyseur HTML.
#
# FONCTIONS NUMÉROTÉES :
#   1. ip_to_uint32(ip_str)            : Conversion IPv4 chaîne vers entier hôte 32 bits.
#   2. uint32_to_ip(ip_int)            : Conversion entier 32 bits vers chaîne pointée.
#   3. is_private_ip(ip_int)           : Détection des plages locales, réservées et loopback.
#   4. fetch_feed(url, label)          : Téléchargement HTTP/HTTPS résilient avec timeout.
#   5. parse_feodo_tracker(raw_text)   : Extraction des serveurs C2 / Botnets actifs.
#   6. parse_ipsum(raw_text, min_lvl)  : Extraction et pondération du flux multi-listes.
#   7. merge_and_sort_threats(...)     : Fusion, arbitrage des scores et tri arithmétique.
#   8. generate_c_header(threats, out) : Compilation du header C99 (table .rodata).
#   9. generate_web_js(threats, out)   : Compilation du dictionnaire JavaScript O(1).
#  10. main()                          : Orchestrateur général de construction SSOT.
# ==============================================================================

import os
import re
import sys
import time
import socket
import urllib.request
import urllib.error

# ------------------------------------------------------------------------------
# CONSTANTES DE CONFIGURATION
# ------------------------------------------------------------------------------
URL_FEODO_TRACKER = "https://feodotracker.abuse.ch/downloads/ipblocklist.txt"
URL_IPSUM = "https://raw.githubusercontent.com/stamparm/ipsum/master/ipsum.txt"

# Seuil minimum d'apparition sur IPsum (3 = listée par au moins 3 sources indépendantes)
IPSUM_MIN_LEVEL = 3

# Catégories de menaces normalisées
CAT_UNKNOWN = 0
CAT_C2 = 1        # Botnet / Ransomware Command & Control (Critique)
CAT_TOR = 2       # Relais de sortie Tor (Anonymisation)
CAT_ABUSE = 3     # Attaques récurrentes (Bruteforce SSH, scanners, fraudes)

# Chemins de sortie relatifs à la racine du dépôt
OUTPUT_C_HEADER = "src/cli/rap_threat_data.h"
OUTPUT_WEB_JS = "docs/js/threat_data.js"


# ------------------------------------------------------------------------------
# 1. CONVERSION IPV4 VERS ENTIER 32 BITS NON SIGNÉ
# ------------------------------------------------------------------------------
def ip_to_uint32(ip_str: str) -> int:
    parts = ip_str.strip().split(".")
    if len(parts) != 4:
        return 0
    try:
        return (int(parts[0]) << 24) | (int(parts[1]) << 16) | (int(parts[2]) << 8) | int(parts[3])
    except ValueError:
        return 0


# ------------------------------------------------------------------------------
# 2. CONVERSION ENTIER 32 BITS VERS NOTATION POINTÉE
# ------------------------------------------------------------------------------
def uint32_to_ip(ip_int: int) -> str:
    return f"{(ip_int >> 24) & 0xFF}.{(ip_int >> 16) & 0xFF}.{(ip_int >> 8) & 0xFF}.{ip_int & 0xFF}"


# ------------------------------------------------------------------------------
# 3. FILTRE DES PLAGES PRIVÉES / LOCALES (RFC 1918 & RFC 5735)
# ------------------------------------------------------------------------------
def is_private_ip(ip_int: int) -> bool:
    # 10.0.0.0/8
    if (ip_int & 0xFF000000) == 0x0A000000:
        return True
    # 127.0.0.0/8 (Loopback)
    if (ip_int & 0xFF000000) == 0x7F000000:
        return True
    # 172.16.0.0/12
    if (ip_int & 0xFFF00000) == 0xAC100000:
        return True
    # 192.168.0.0/16
    if (ip_int & 0xFFFF0000) == 0xC0A80000:
        return True
    # 169.254.0.0/16 (Link-Local)
    if (ip_int & 0xFFFF0000) == 0xA9FE0000:
        return True
    return False


# ------------------------------------------------------------------------------
# 4. TÉLÉCHARGEMENT HTTP/HTTPS SÉCURISÉ AVEC TIMEOUT
# ------------------------------------------------------------------------------
def fetch_feed(url: str, label: str) -> str:
    print(f"[*] Téléchargement du flux [{label}]...")
    req = urllib.request.Request(
        url,
        headers={"User-Agent": "RAPTOR-Threat-Pipeline/2.0 (+https://raptor.binokub.com)"}
    )
    try:
        with urllib.request.urlopen(req, timeout=20) as response:
            return response.read().decode("utf-8", errors="ignore")
    except Exception as e:
        print(f"[!] Erreur lors de la récupération de {label} ({e})")
        return ""


# ------------------------------------------------------------------------------
# 5. EXTRACTION : FEODO TRACKER (ABUSE.CH)
# ------------------------------------------------------------------------------
def parse_feodo_tracker(raw_text: str) -> dict:
    threats = {}
    if not raw_text:
        return threats

    ipv4_pattern = re.compile(r"^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})")
    for line in raw_text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        match = ipv4_pattern.match(line)
        if match:
            ip_str = match.group(1)
            ip_val = ip_to_uint32(ip_str)
            if ip_val and not is_private_ip(ip_val):
                threats[ip_val] = {
                    "ip_str": ip_str,
                    "cat": CAT_C2,
                    "score": 98,  # Sévérité critique immédiate pour botnet/ransomware
                    "tag": "Feodo_Botnet_C2"
                }
    print(f"[+] Feodo Tracker : {len(threats)} serveurs C2 actifs extraits.")
    return threats


# ------------------------------------------------------------------------------
# 6. EXTRACTION : IPSUM (MULTI-LISTES MONDIALES)
# ------------------------------------------------------------------------------
def parse_ipsum(raw_text: str, min_level: int) -> dict:
    threats = {}
    if not raw_text:
        return threats

    for line in raw_text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) >= 2:
            ip_str = parts[0]
            try:
                level = int(parts[1])
            except ValueError:
                continue

            # Ne retenir que les adresses persistantes
            if level < min_level:
                continue

            ip_val = ip_to_uint32(ip_str)
            if not ip_val or is_private_ip(ip_val):
                continue

            # Calcul du score normalisé de 50 à 95 selon la sévérité (level 3 à 8)
            score = min(50 + (level * 7), 95)
            threats[ip_val] = {
                "ip_str": ip_str,
                "cat": CAT_ABUSE,
                "score": score,
                "tag": f"Blacklist_{level}_Sources"
            }

    print(f"[+] IPsum (Niveau >= {min_level}) : {len(threats)} IP malveillantes extraites.")
    return threats


# ------------------------------------------------------------------------------
# 7. FUSION, CONSOLIDATION ET TRI CROISSANT
# ------------------------------------------------------------------------------
def merge_and_sort_threats(feodo_dict: dict, ipsum_dict: dict) -> list:
    merged = {}

    # Insérer IPsum d'abord
    for ip_val, data in ipsum_dict.items():
        merged[ip_val] = data

    # Fusionner Feodo (priorité maximale sur catégorie et score)
    for ip_val, data in feodo_dict.items():
        if ip_val in merged:
            merged[ip_val]["cat"] = CAT_C2
            merged[ip_val]["score"] = 98
            merged[ip_val]["tag"] = "C2_Multi_Blacklist"
        else:
            merged[ip_val] = data

    # Tri arithmétique strict par entier IP (obligatoire pour bsearch)
    sorted_threats = sorted(merged.values(), key=lambda x: ip_to_uint32(x["ip_str"]))
    print(f"[+] Total net consolidé : {len(sorted_threats)} adresses indexées.")
    return sorted_threats


# ------------------------------------------------------------------------------
# 8. GÉNÉRATION DU FICHIER D'EN-TÊTE C99 (SRC/CLI/RAP_THREAT_DATA.H)
# ------------------------------------------------------------------------------
def generate_c_header(threats: list, output_path: str):
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime())
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("/* ==============================================================================\n")
        f.write(" * NOM DU SCRIPT   : src/cli/rap_threat_data.h\n")
        f.write(" * GÉNÉRATION      : AUTOMATIQUE VIA tools/gen_threats.py (SSOT)\n")
        f.write(f" * DATE PRODUCTION : {timestamp}\n")
        f.write(" * RESPONSABILITÉ  : Base locale C99 de renseignement sur les menaces (.rodata).\n")
        f.write(" *                   Table triée par entier IPv4 pour recherche binaire en O(log N).\n")
        f.write(" *                   Zéro allocation dynamique (0 heap), silence radio absolu.\n")
        f.write(" * ============================================================================== */\n\n")
        f.write("#ifndef RAP_THREAT_DATA_H\n")
        f.write("#define RAP_THREAT_DATA_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stddef.h>\n\n")
        f.write("#define RAP_THREAT_CAT_UNKNOWN  0\n")
        f.write("#define RAP_THREAT_CAT_C2       1  /* Botnet / Ransomware C2 */\n")
        f.write("#define RAP_THREAT_CAT_TOR      2  /* Relais de sortie Tor */\n")
        f.write("#define RAP_THREAT_CAT_ABUSE    3  /* Scanners / Bruteforce / Abus */\n\n")
        f.write("typedef struct {\n")
        f.write("    uint32_t ip;       /* Adresse IPv4 en entier hôte 32 bits */\n")
        f.write("    uint8_t  category; /* Catégorie de menace normalisée */\n")
        f.write("    uint8_t  score;    /* Score de réputation (1 à 100) */\n")
        f.write("} RapThreatRecord;\n\n")
        f.write(f"static const char *RAP_THREAT_DB_BUILD = \"{timestamp}\";\n")
        f.write(f"static const size_t RAP_THREAT_COUNT = {len(threats)};\n\n")
        f.write("static const RapThreatRecord g_rap_threat_db[] = {\n")

        for item in threats:
            ip_val = ip_to_uint32(item["ip_str"])
            f.write(f"    {{ 0x{ip_val:08X}U, {item['cat']}, {item['score']} }},\n")

        f.write("};\n\n")
        f.write("#endif /* RAP_THREAT_DATA_H */\n")

    print(f"[✓] Header C99 généré : {output_path} ({len(threats)} entrées)")


# ------------------------------------------------------------------------------
# 9. GÉNÉRATION DU MODULE WEB (DOCS/JS/THREAT_DATA.JS)
# ------------------------------------------------------------------------------
def generate_web_js(threats: list, output_path: str):
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime())
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("// ==============================================================================\n")
        f.write("// NOM DU SCRIPT   : docs/js/threat_data.js\n")
        f.write("// GÉNÉRATION      : AUTOMATIQUE VIA tools/gen_threats.py (SSOT)\n")
        f.write(f"// DATE PRODUCTION : {timestamp}\n")
        f.write("// RESPONSABILITÉ  : Dictionnaire de réputation IP O(1) pour l'investigation web.\n")
        f.write("//                   Exécution 100% côté client, aucune donnée transmise à un tiers.\n")
        f.write("// ==============================================================================\n\n")
        f.write(f"const RAPTOR_THREAT_TIMESTAMP = \"{timestamp}\";\n")
        f.write(f"const RAPTOR_THREAT_COUNT = {len(threats)};\n\n")
        f.write("// Format de stockage : \"IP\" : [Catégorie, Score, Tag]\n")
        f.write("const RAPTOR_THREAT_DB = {\n")

        for item in threats:
            f.write(f'  "{item["ip_str"]}": [{item["cat"]}, {item["score"]}, "{item["tag"]}"],\n')

        f.write("};\n")

    print(f"[✓] Module Web JS généré : {output_path} ({len(threats)} entrées)")


# ------------------------------------------------------------------------------
# 10. ORCHESTRATEUR PRINCIPAL
# ------------------------------------------------------------------------------
def main():
    start_time = time.time()
    print("=================================================================")
    print("   RAPTOR v2.0 // Générateur de Renseignement sur les Menaces   ")
    print("=================================================================")

    # Déterminer la racine du projet
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, ".."))
    os.chdir(project_root)

    # 1. Télécharger les flux
    raw_feodo = fetch_feed(URL_FEODO_TRACKER, "abuse.ch Feodo Tracker")
    raw_ipsum = fetch_feed(URL_IPSUM, "IPsum Threat Intelligence")

    # 2. Analyser les données
    dict_feodo = parse_feodo_tracker(raw_feodo)
    dict_ipsum = parse_ipsum(raw_ipsum, IPSUM_MIN_LEVEL)

    # 3. Consolider et trier
    consolidated = merge_and_sort_threats(dict_feodo, dict_ipsum)
    if not consolidated:
        print("[!] Erreur critique : Aucune donnée de menace extraite. Abandon.")
        sys.exit(1)

    # 4. Écrire les sorties
    generate_c_header(consolidated, OUTPUT_C_HEADER)
    generate_web_js(consolidated, OUTPUT_WEB_JS)

    elapsed = time.time() - start_time
    print("-----------------------------------------------------------------")
    print(f"[✓] Synchronisation terminée avec succès en {elapsed:.2f} secondes.")
    print("=================================================================")


if __name__ == "__main__":
    main()
