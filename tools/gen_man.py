#!/usr/bin/env python3
# ============================================================================
# NOM DU SCRIPT   : tools/gen_man.py
# RÉVISION        : Rev 1.4
# PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
# RESPONSABILITÉ  : Générateur et synchroniseur de documentation unifiée (SSOT).
#                   Compile docs/man/rap.1.md ET le dictionnaire des fonctions vers :
#                     1) src/cli/rap_man_data.h (.rodata C99 : manuel indexé + dictionnaire)
#                     2) docs/js/man_data.js (base de données pour docs/man.html)
#
# BLOC DESCRIPTIF DES NOUVELLES INTÉGRATIONS (Rev 1.4) :
#   - Génération de la table d'indexation des pages RapManPageEntry g_rap_man_pages[].
#   - Permet la projection ciblée par rubrique ('rap man sentry', 'rap man rx', etc.).
#   - Préservation de g_rap_man_data pour la consultation intégrale sans argument.
# ============================================================================

import sys
import os
import re
import json
import glob

SOURCE_MAN  = "docs/man/rap.1.md"
TARGET_H    = "src/cli/rap_man_data.h"
TARGET_JS   = "docs/js/man_data.js"

DICT_CANDIDATES = [
    "docs/DICTIONNAIRE-DES-FONCTIONS-22-10-2026.md",
    "docs/man/functions.md"
] + glob.glob("docs/DICTIONNAIRE-DES-FONCTIONS*.md")

# ============================================================================
# 1. parse_ssot_markdown
# ============================================================================
def parse_ssot_markdown(md_raw):
    pages = {}
    pattern = re.compile(r'<!--\s*MAN:\s*([a-zA-Z0-9_\-]+)\s*\|\s*([0-9]+)\s*\|\s*(.+?)\s*-->')
    parts = pattern.split(md_raw)
    
    if len(parts) < 4:
        pages["rap"] = {
            "section": "1",
            "title": "RAP(1) — Manuel Général",
            "content": md_raw.strip()
        }
        return pages

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
# 2. parse_functions_dict
# ============================================================================
def parse_functions_dict(dict_raw):
    entries = []
    sec_splitter = re.compile(r'={40,}\s*\n\s*SECTION\s+(\d+)\s*[:—\-]\s*([^\n]+)\s*\n\s*={40,}', re.MULTILINE)
    splits = sec_splitter.split(dict_raw)
    
    idx = 1
    while idx < len(splits):
        sec_num = int(splits[idx].strip())
        sec_title = splits[idx+1].strip()
        sec_body = splits[idx+2].strip()
        
        entries.append({
            "section": sec_num,
            "fn_id": 0,
            "title": f"SECTION {sec_num} : {sec_title}",
            "content": f"=== SECTION {sec_num} : {sec_title} ===\n\n{sec_body}"
        })
        
        fn_pattern = re.compile(r'^\s*(Fonction|Outil)\s+(\d+)\s*:\s*([^\n]+)', re.MULTILINE)
        matches = list(fn_pattern.finditer(sec_body))
        
        for m_idx, match in enumerate(matches):
            fn_id = int(match.group(2))
            fn_title = match.group(3).strip()
            start_pos = match.start()
            end_pos = matches[m_idx + 1].start() if (m_idx + 1 < len(matches)) else len(sec_body)
            fn_content = sec_body[start_pos:end_pos].strip()
            
            entries.append({
                "section": sec_num,
                "fn_id": fn_id,
                "title": f"{match.group(1)} {fn_id} : {fn_title}",
                "content": fn_content
            })
            
        idx += 3

    return entries

# ============================================================================
# 3. format_unix_header
# ============================================================================
def format_unix_header(key, sec):
    tag = f"{key.upper()}({sec})"
    center = "RAPTOR DEFENSE"
    total_len = 70
    spaces_before = max(2, (total_len - len(tag) * 2 - len(center)) // 2)
    spaces_after = max(2, total_len - len(tag) * 2 - len(center) - spaces_before)
    return f"{tag}{' ' * spaces_before}{center}{' ' * spaces_after}{tag}"

# ============================================================================
# 4. escape_c_string
# ============================================================================
def escape_c_string(text):
    escaped = []
    for line in text.splitlines():
        clean = line.replace('\\', '\\\\').replace('"', '\\"')
        escaped.append(f'    "{clean}\\n"')
    return "\n".join(escaped)

# ============================================================================
# 5. generate_cli_header
# ============================================================================
def generate_cli_header(man_pages, fn_entries, full_dict_raw):
    # 1. Manuel complet concaténé
    cli_lines = []
    for key, data in man_pages.items():
        banner = format_unix_header(key, data["section"])
        cli_lines.append("")
        cli_lines.append(banner)
        cli_lines.append("=" * len(banner))
        cli_lines.append("")
        cli_lines.append(data["content"])
        cli_lines.append("")
    
    escaped_man = escape_c_string("\n".join(cli_lines))
    escaped_fn_full = escape_c_string(full_dict_raw.strip())
    
    # 2. Table structurée des pages individuelles g_rap_man_pages[]
    page_lines = []
    for key, data in man_pages.items():
        banner = format_unix_header(key, data["section"])
        page_text = f"{banner}\n{'=' * len(banner)}\n\n{data['content']}\n"
        esc_title = data["title"].replace('\\', '\\\\').replace('"', '\\"')
        esc_page_content = escape_c_string(page_text)
        page_lines.append("    {")
        page_lines.append(f"        \"{key}\",")
        page_lines.append(f"        \"{data['section']}\",")
        page_lines.append(f"        \"{esc_title}\",")
        page_lines.append(f"{esc_page_content}")
        page_lines.append("    },")

    # 3. Table structurée du dictionnaire g_rap_fn_entries[]
    table_lines = []
    for e in fn_entries:
        esc_title = e["title"].replace('\\', '\\\\').replace('"', '\\"')
        esc_content = escape_c_string(e["content"])
        table_lines.append("    {")
        table_lines.append(f"        {e['section']},")
        table_lines.append(f"        {e['fn_id']},")
        table_lines.append(f"        \"{esc_title}\",")
        table_lines.append(f"{esc_content}")
        table_lines.append("    },")

    header_code = f"""/* ============================================================================
 * NOM DU SCRIPT   : src/cli/rap_man_data.h
 * RÉVISION        : Généré automatiquement par tools/gen_man.py (SSOT)
 * PROJET          : RAPTOR DEFENSE v2.0 // Arsenal Souverain C99
 * RESPONSABILITÉ  : Constantes .rodata pour le manuel indexé et le dictionnaire.
 * ============================================================================ */

#ifndef RAP_MAN_DATA_H
#define RAP_MAN_DATA_H

#include <stddef.h>

/* Manuel CLI complet concaténé (Section 1 et 5) */
static const char g_rap_man_data[] =
{escaped_man};

/* Structure d'indexation unitaire des pages de manuel (0 heap) */
typedef struct {{
    const char *name;       /* Clé de commande (ex: "rap", "rap-sentry") */
    const char *section;    /* Numéro de section Unix (ex: "1") */
    const char *title;      /* Titre descriptif */
    const char *content;    /* Contenu unitaire formaté avec bannière */
}} RapManPageEntry;

static const RapManPageEntry g_rap_man_pages[] = {{
{chr(10).join(page_lines)}
}};

static const size_t g_rap_man_pages_count = sizeof(g_rap_man_pages) / sizeof(g_rap_man_pages[0]);

/* Dictionnaire intégral brut pour consultation paginée (rap man fn) */
static const char g_rap_fn_full_data[] =
{escaped_fn_full};

/* Structure de repérage hiérarchique du dictionnaire des fonctions */
typedef struct {{
    int section;        /* 1 à 18 (0 si non spécifié) */
    int fn_id;          /* 0 = Section entière, > 0 = Fonction unitaire */
    const char *title;  /* Libellé d'identification */
    const char *content;/* Contenu technique formaté */
}} RapFnDocEntry;

static const RapFnDocEntry g_rap_fn_entries[] = {{
{chr(10).join(table_lines)}
}};

static const size_t g_rap_fn_entries_count = sizeof(g_rap_fn_entries) / sizeof(g_rap_fn_entries[0]);

#endif /* RAP_MAN_DATA_H */
"""
    return header_code

# ============================================================================
# 6. generate_js_database
# ============================================================================
def generate_js_database(man_pages, fn_entries):
    db = {}
    
    for key, data in man_pages.items():
        db[key] = {
            "section": data["section"],
            "title": data["title"],
            "header": format_unix_header(key, data["section"]),
            "content": "\n" + data["content"] + "\n"
        }
        
    for e in fn_entries:
        if e["fn_id"] == 0:
            key = f"sec-{e['section']}"
            title = f"SEC-{e['section']}(3) — {e['title']}"
            hdr = format_unix_header(f"SEC-{e['section']}", "3")
        else:
            key = f"fn-{e['fn_id']}"
            title = f"FN-{e['fn_id']}(3) — {e['title']}"
            hdr = format_unix_header(f"FN-{e['fn_id']}", "3")
            
        db[key] = {
            "section": "3",
            "title": title,
            "header": hdr,
            "content": "\n" + e["content"] + "\n"
        }

    js_code = (
        "/* ============================================================================\n"
        " * NOM DU SCRIPT   : docs/js/man_data.js\n"
        " * RÉVISION        : Généré automatiquement par tools/gen_man.py (SSOT)\n"
        " * PROJET          : RAPTOR DEFENSE v2.0 // Portail Officiel\n"
        " * RESPONSABILITÉ  : Base de données globale unifiée (Sections 1, 3 et 5).\n"
        " * ============================================================================ */\n\n"
        "window.RAP_MAN_DATABASE = "
        + json.dumps(db, indent=4, ensure_ascii=False)
        + ";\n"
    )
    return js_code

# ============================================================================
# 7. main
# ============================================================================
def main():
    if not os.path.exists(SOURCE_MAN):
        print(f"[ERREUR] Manuel introuvable : {SOURCE_MAN}", file=sys.stderr)
        sys.exit(1)

    dict_path = None
    for cand in DICT_CANDIDATES:
        if cand and os.path.exists(cand):
            dict_path = cand
            break

    if not dict_path:
        print(f"[ERREUR] Dictionnaire des fonctions introuvable !", file=sys.stderr)
        sys.exit(1)

    with open(SOURCE_MAN, "r", encoding="utf-8") as f:
        man_raw = f.read()

    with open(dict_path, "r", encoding="utf-8") as f:
        dict_raw = f.read()

    man_pages = parse_ssot_markdown(man_raw)
    fn_entries = parse_functions_dict(dict_raw)

    print(f"[SSOT] Source Man         : {SOURCE_MAN} ({len(man_pages)} pages)")
    print(f"[SSOT] Source Dico        : {dict_path} ({len(fn_entries)} entrées indexées)")

    c_header = generate_cli_header(man_pages, fn_entries, dict_raw)
    os.makedirs(os.path.dirname(TARGET_H), exist_ok=True)
    with open(TARGET_H, "w", encoding="utf-8") as f:
        f.write(c_header)
    print(f"[SSOT] En-tête C compilé  : {TARGET_H}")

    js_db = generate_js_database(man_pages, fn_entries)
    os.makedirs(os.path.dirname(TARGET_JS), exist_ok=True)
    with open(TARGET_JS, "w", encoding="utf-8") as f:
        f.write(js_db)
    print(f"[SSOT] Base JS générée    : {TARGET_JS}")

if __name__ == "__main__":
    main()
