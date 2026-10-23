#ifndef MEM_STEALTH_H
#define MEM_STEALTH_H

#include <stdbool.h>
#include <sys/types.h>

#define MAX_STEALTH_EVENTS 16
#define MAX_MEM_WHITELIST_ENTRIES 256

/* Structure d'un événement forensique poussé dans le Cahier Différentiel */
typedef struct {
    pid_t pid;
    char comm[64];
    unsigned long start_addr;
    unsigned long end_addr;
    size_t size_kb;
    char detail[128];
} StealthEvent;

/* Prototypes Section 13 (Fonctions 80 à 83) */
bool mem_stealth_whitelist_load(const char* filepath);
bool mem_stealth_is_whitelisted(const char* comm, const char* exe);
bool mem_stealth_scan_maps_anon_rx(pid_t pid, const char* comm, const char* exe, StealthEvent* ev_out);
void mem_stealth_dispatch_diff_event(const StealthEvent* ev);

/* Accesseurs pour l'interface graphique (Onglet 4) */
int  mem_stealth_get_diff_events(StealthEvent* out_events, int max_count);
void mem_stealth_clear_diff_events(void);

#endif /* MEM_STEALTH_H */
