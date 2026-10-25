/* ==============================================================================
 * NOM DU SCRIPT : include/raptor/raptor_asn_intel.h
 * NUMÉRO DE RÉVISION : Rev 1.1
 * BLOC DESCRIPTIF DES INTÉGRATIONS :
 *   - Couche de définitions et contrats d'API pour le greffon RAPTOR NET-INTEL / ASN.
 *   - Élimination des tables codées en dur : chargement dynamique depuis table_static.conf.
 *   - Déclaration des fonctions 71 à 79 pour la Section 12 du Dictionnaire Officiel.
 * 
 * FONCTIONS DÉCLARÉES ET EXPORTÉES (SECTION 12) :
 *   71. asn_intel_trim
 *   72. asn_intel_parse_cidr
 *   73. asn_intel_load_file
 *   74. asn_intel_match_static
 *   75. asn_intel_query_bgp
 *   76. asn_intel_lookup
 *   77. asn_intel_init
 *   78. asn_intel_cleanup
 *   79. asn_intel_register
 * ============================================================================== */

#ifndef RAPTOR_ASN_INTEL_H
#define RAPTOR_ASN_INTEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "raptor/raptor_data.h"

#define RAPTOR_FILE_TABLE_STATIC  "assets/rules/table_static.conf"
#define ASN_STATIC_MAX_ENTRIES    512
#define ASN_INTEL_NAME_MAX        128
#define ASN_INTEL_CACHE_MAX       128

/* Structure d'un bloc CIDR dynamique issu de table_static.conf */
typedef struct {
    uint32_t network_ip;
    uint32_t netmask;
    char asn_tag[32];
    char name[ASN_INTEL_NAME_MAX];
} AsnStaticRange;

/* -----------------------------------------------------------------------------
 * PROTOTYPES DES FONCTIONS DU DICTIONNAIRE RAPTOR (SECTION 12 : 71 à 79)
 * ----------------------------------------------------------------------------- */

/* 71. Nettoyage des espaces blancs en tête et queue de chaîne */
char* asn_intel_trim(char* str);

/* 72. Extraction et validation mathématique d'un sous-réseau IPv4 CIDR */
bool asn_intel_parse_cidr(const char* cidr_str, uint32_t* out_ip, uint32_t* out_mask);

/* 73. Chargement et parsing dynamique du registre assets/rules/table_static.conf */
bool asn_intel_load_file(const char* filepath);

/* 74. Comparaison arithmétique binaire 0 ms contre les plages chargées en RAM */
bool asn_intel_match_static(uint32_t ip_host, char* out_buf, size_t out_sz);

/* 75. Requête DNS inverse BGP vers origin.asn.cymru.com pour les réseaux non répertoriés */
bool asn_intel_query_bgp(const char* ip_str, char* out_buf, size_t out_sz);

/* 76. Moteur d'investigation en cascade (Cache -> table_static.conf -> BGP distant) */
bool asn_intel_lookup(const char* ip_str, char* out_buf, size_t out_sz);

/* 77. Initialisation globale et chargement initial des fichiers de configuration */
bool asn_intel_init(void);

/* 78. Vidange complète du cache et des tables en mémoire vive */
void asn_intel_cleanup(void);

/* 79. Enregistrement automatique du greffon au démarrage sur le bus RaptorPlugin */
void asn_intel_register(void);

#endif /* RAPTOR_ASN_INTEL_H */
