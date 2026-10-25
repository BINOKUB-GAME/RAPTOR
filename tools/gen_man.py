#!/usr/bin/env python3
# ============================================================================
# NOM DU SCRIPT   : tools/gen_man.py
# RÉVISION        : Rev 1.2
# PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
# RESPONSABILITÉ  : Générateur et synchroniseur de documentation unifiée (SSOT).
#                   Compile docs/man/rap.1.md vers :
#                     1) src/cli/rap_man_data.h (.rodata C99 pour le binaire 'rap')
#                     2) docs/js/man_data.js (base de données pour docs/man.html)
#
# BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.2) :
#   - Découpage multi-pages depuis la source unique docs/man/rap.1.md via balises <!-- MAN: key | sec | titre -->.
#   - Export vers docs/js/man_data.js (window.RAP_MAN_DATABASE) pour man.html.
#   - Formatage du header Unix aligné (80 colonnes) pour chaque commande.
#   - Production de src/cli/rap_man_data.h (.rodata sans heap) pour rap man et rapless.
#
# LISTE DES FONCTIONS NUMÉROTÉES :
#   1. parse_ssot_markdown     : Découpe le fichier Markdown en dictionnaire de pages.
#   2. format_unix_header      : Génère la bannière d'en-tête Unix centrée (80 colonnes).
#   3. generate_cli_header     : Construit l'en-tête C src/cli/rap_man_data.h.
#   4. generate_js_database    : Construit la base JavaScript docs/js/man_data.js.
#   5. main                    : Point d'entrée et orchestration du flux de compilation SSOT.
# ============================================================================

import sys
import os
import re
import json

SOURCE_MD = "docs/man/rap.1.md"
TARGET_H  = "src/cli/rap_man_data.h"
TARGET_JS = "docs/js/man_data.js"

# ============================================================================
# 1. parse_ssot_markdown
# ============================================================================
def parse_ssot_markdown(md_raw):
    pages = {}
    pattern = re.compile(r'<!--\s*MAN:\s*([a-zA-Z0-9_\-]+)\s*\|\s*([0-9]+)\s*\|\s*(.+?)\s*-->')
    
    parts = pattern.split(md_raw)
    if len(parts) < 4:
        # Fichier sans balise multi-pages : une seule page 'rap'
        pages["rap"] = {
            "section": "1",
            "title": "RAP(1) — Manuel Général",
            "content": md_raw.strip()
        }
        return pages

    # parts[0] = préambule
    # parts[1] = key, parts[2] = section, parts[3] = title, parts[4] = content
    i = 1
    while i < len(parts):
        key = parts[i].strip()
        sec = parts[i+1].strip()
        title = parts[i+2].strip()
        content = parts[i+3].strip()
        
        pages[key] = {
            "section": sec,
            "title": title,
            "content": content
        }
        i += 4

    return pages

# ============================================================================
# 2. format_unix_header
# ============================================================================
def format_unix_header(key, sec):
    tag = f"{key.upper()}({sec})"
    center = "RAPTOR DEFENSE"
    total_len = 70
    
    # Construction : TAG + espaces + CENTER + espaces + TAG
    spaces_before = max(2, (total_len - len(tag) * 2 - len(center)) // 2)
    spaces_after = max(2, total_len - len(tag) * 2 - len(center) - spaces_before)
    
    return f"{tag}{' ' * spaces_before}{center}{' ' * spaces_after}{tag}"

# ============================================================================
# 3. generate_cli_header
# ============================================================================
def generate_cli_header(pages):
    cli_lines = []
    
    # Pour la CLI, on produit un document combiné propre et continu
    for key, data in pages.items():
        header_banner = format_unix_header(key, data["section"])
        cli_lines.append("")
        cli_lines.append(header_banner)
        cli_lines.append("=" * len(header_banner))
        cli_lines.append("")
        cli_lines.append(data["content"])
        cli_lines.append("")

    # Échappement C strict
    escaped = []
    for line in "\n".join(cli_lines).splitlines():
        clean = line.replace('\\', '\\\\').replace('"', '\\"')
        escaped.append(f'    "{clean}\\n"')

    header_code = (
        "/* ============================================================================\n"
        " * NOM DU SCRIPT   : src/cli/rap_man_data.h\n"
        " * RÉVISION        : Généré automatiquement par tools/gen_man.py (SSOT)\n"
        " * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99\n"
        " * RESPONSABILITÉ  : Constante .rodata contenant le texte complet du manuel.\n"
        " * ============================================================================ */\n\n"
        "#ifndef RAP_MAN_DATA_H\n"
        "#define RAP_MAN_DATA_H\n\n"
        "static const char g_rap_man_data[] =\n"
        + "\n".join(escaped) + ";\n\n"
        "#endif /* RAP_MAN_DATA_H */\n"
    )
    return header_code

# ============================================================================
# 4. generate_js_database
# ============================================================================
def generate_js_database(pages):
    db = {}
    for key, data in pages.items():
        db[key] = {
            "section": data["section"],
            "title": data["title"],
            "header": format_unix_header(key, data["section"]),
            "content": "\n" + data["content"] + "\n"
        }

    js_code = (
        "/* ============================================================================\n"
        " * NOM DU SCRIPT   : docs/js/man_data.js\n"
        " * RÉVISION        : Généré automatiquement par tools/gen_man.py (SSOT)\n"
        " * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel\n"
        " * RESPONSABILITÉ  : Base de données globale des man pages pour docs/man.html.\n"
        " * ============================================================================ */\n\n"
        "window.RAP_MAN_DATABASE = "
        + json.dumps(db, indent=4, ensure_ascii=False)
        + ";\n"
    )
    return js_code

# ============================================================================
# 5. main
# ============================================================================
def main():
    if not os.path.exists(SOURCE_MD):
        print(f"[ERREUR] Source introuvable : {SOURCE_MD}", file=sys.stderr)
        sys.exit(1)

    with open(SOURCE_MD, "r", encoding="utf-8") as f:
        md_raw = f.read()

    pages = parse_ssot_markdown(md_raw)
    print(f"[SSOT] {len(pages)} pages de manuel extraites depuis {SOURCE_MD}")

    # 1. Génération de l'en-tête C (.rodata)
    c_header = generate_cli_header(pages)
    os.makedirs(os.path.dirname(TARGET_H), exist_ok=True)
    with open(TARGET_H, "w", encoding="utf-8") as f:
        f.write(c_header)
    print(f"[SSOT] En-tete C genere      : {TARGET_H}")

    # 2. Génération de la base JS pour le site web
    js_data = generate_js_database(pages)
    os.makedirs(os.path.dirname(TARGET_JS), exist_ok=True)
    with open(TARGET_JS, "w", encoding="utf-8") as f:
        f.write(js_data)
    print(f"[SSOT] Base JS web generee  : {TARGET_JS}")

if __name__ == "__main__":
    main()
