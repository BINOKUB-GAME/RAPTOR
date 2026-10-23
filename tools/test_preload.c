/**
 * ============================================================================
 * NOM DU SCRIPT   : tools/test_preload.c
 * PROJET          : RAPTOR v2.0 - Banc d'essai Preload-Hunter
 * RESPONSABILITÉ  : Lance un processus dormant avec une variable LD_PRELOAD
 *                   simulant une injection de bibliothèque clandestine.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    pid_t pid = getpid();
    printf("====================================================\n");
    printf("[BANC D'ESSAI PRELOAD] PID : %d\n", (int)pid);
    printf("[BANC D'ESSAI PRELOAD] Simulation d'une injection LD_PRELOAD...\n");
    printf("[BANC D'ESSAI PRELOAD] Maintien actif pendant 60 secondes...\n");
    printf("====================================================\n");

    sleep(60);

    printf("[BANC D'ESSAI PRELOAD] Fin de séquence normale.\n");
    return 0;
}
