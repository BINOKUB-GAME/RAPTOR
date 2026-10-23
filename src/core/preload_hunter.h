#ifndef PRELOAD_HUNTER_H
#define PRELOAD_HUNTER_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define LD_PRELOAD_GLOBAL_PATH "/etc/ld.so.preload"

/* Structure d'alerte spécifique au détournement LD_PRELOAD */
typedef struct {
    pid_t pid;
    char comm[64];
    char injected_path[512];
    bool is_global; /* true si /etc/ld.so.preload, false si par processus */
} PreloadThreat;

/* Prototypes Section 14 (Fonctions 84 et 85) */
bool preload_hunter_check_global(char* out_path, size_t out_len);
bool preload_hunter_scan_process_env(pid_t pid, char* out_preload_lib, size_t out_len);

#endif /* PRELOAD_HUNTER_H */
