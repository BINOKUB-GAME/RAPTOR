#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

int main(void) {
    size_t page_size = sysconf(_SC_PAGESIZE);
    pid_t pid = getpid();

    printf("[BANC D'ESSAI] PID : %d\n", pid);
    printf("[PHASE 1] Allocation d'une page anonyme en RW- (lecture/écriture)...\n");

    /* 1. Allocation mémoire RW- */
    void* buffer = mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (buffer == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    /* Écriture d'une instruction inoffensive (opcode RET 0xC3) */
    memset(buffer, 0xC3, page_size);

    printf("[PHASE 2] Veille de 6 secondes (capture de l'état T-1)...\n");
    sleep(6);

    printf("[PHASE 3] Déclenchement de la mutation mprotect() vers R-X...\n");
    /* 2. Transformation clandestine en R-X */
    if (mprotect(buffer, page_size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        munmap(buffer, page_size);
        return EXIT_FAILURE;
    }

    printf("[PHASE 4] Mutation active en RAM. Vérifiez l'Onglet 4 (Cahier Différentiel).\n");
    printf("[BANC D'ESSAI] Attente 120 secondes avant auto-libération...\n");
    sleep(120);

    munmap(buffer, page_size);
    printf("[BANC D'ESSAI] Fin de séquence propre.\n");
    return EXIT_SUCCESS;
}
