/**
 * ============================================================================
 * NOM DU SCRIPT   : tools/sim_threat.c
 * RÉVISION        : Rev 1.0
 * PROJET          : RAPTOR v2.0 (Banc d'Essai & Simulation d'Anomalie)
 * RESPONSABILITÉ  : Simulateur contrôlé d'activités clandestines pour éprouver
 *                   les capacités de détection défensive de RAPTOR :
 *                   1. Allocation d'une page mémoire anonyme RWX (W^X violation).
 *                   2. Création d'un socket d'écoute réseau TCP furtif.
 *                   3. Maintien en mémoire pour inspection temps réel.
 *
 * NOUVELLES INTÉGRATIONS (Rev 1.0) :
 *   - Allocation mémoire RWX via mmap().
 *   - Ouverture d'un socket d'écoute TCP sur le port 4444.
 *   - Boucle de maintien passive sans impact sur le système.
 *
 * LISTE EXHAUSTIVE ET NUMÉROTÉE DES FONCTIONS :
 *   1. sim_allocate_rwx_page
 *   2. sim_open_listening_socket
 *   3. main
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SIM_PORT 4444

/* ------------------------------------------------------------------------- */
/* 1. ALLOCATION D'UNE PAGE MÉMOIRE RWX (READ-WRITE-EXECUTE)                 */
/* ------------------------------------------------------------------------- */
static void* sim_allocate_rwx_page(void) {
    /* Allocation anonyme d'une page de 4 Ko avec permissions R+W+X simultanées */
    void* addr = mmap(NULL, 4096,
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED) {
        perror("[SIM_THREAT] Échec mmap RWX");
        return NULL;
    }

    /* Écriture d'un faux bytecode (NOPs: 0x90, RET: 0xC3) dans la page mémoire */
    memset(addr, 0x90, 4096);
    ((unsigned char*)addr)[4095] = 0xC3;

    return addr;
}

/* ------------------------------------------------------------------------- */
/* 2. OUVERTURE D'UN SOCKET TCP D'ÉCOUTE SUR PORT SUSPECT                    */
/* ------------------------------------------------------------------------- */
static int sim_open_listening_socket(uint16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[SIM_THREAT] Échec création socket");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[SIM_THREAT] Échec bind socket");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("[SIM_THREAT] Échec listen socket");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

/* ------------------------------------------------------------------------- */
/* 3. POINT D'ENTRÉE PRINCIPAL DU SIMULATEUR                                 */
/* ------------------------------------------------------------------------- */
int main(void) {
    printf("====================================================================\n");
    printf("  [RAPTOR SIMULATOR] Lancement du processus témoin d'anomalie\n");
    printf("  PID du processus : %d\n", getpid());
    printf("====================================================================\n");

    /* 1. Déclenchement de l'anomalie mémoire RWX */
    void* rwx_ptr = sim_allocate_rwx_page();
    if (rwx_ptr) {
        printf("[OK] Page mémoire RWX injectée à l'adresse : %p\n", rwx_ptr);
    }

    /* 2. Déclenchement du flux réseau sur le port 4444 */
    int sock_fd = sim_open_listening_socket(SIM_PORT);
    if (sock_fd >= 0) {
        printf("[OK] Socket d'écoute furtif ouvert sur 0.0.0.0:%d\n", SIM_PORT);
    }

    printf("\n>>> Processus actif. Lancez RAPTOR pour tester sa détection.\n");
    printf(">>> Appuyez sur Ctrl+C ou utilisez le bouton RAPTOR pour l'arrêter.\n\n");

    /* Maintien du processus actif pour observation */
    while (1) {
        sleep(1);
    }

    if (rwx_ptr) munmap(rwx_ptr, 4096);
    if (sock_fd >= 0) close(sock_fd);

    return 0;
}
