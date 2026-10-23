#ifndef MEM_STEALTH_H
#define MEM_STEALTH_H

#include <stdbool.h>
#include <sys/types.h>

/* Structure d'un événement forensique poussé dans le Cahier Différentiel */
typedef struct {
    pid_t pid;
    char comm[64];
    unsigned long start_addr;
    unsigned long end_addr;
    size_t size_kb;
    char detail[128];
} StealthEvent;

/* Prototypes Section 13 */
bool mem_stealth_is_jit_whitelisted(const char* comm);
bool mem_stealth_scan_maps_anon_rx(pid_t pid, const char* comm, StealthEvent* ev_out);
void mem_stealth_dispatch_diff_event(const StealthEvent* ev);

#endif /* MEM_STEALTH_H */
