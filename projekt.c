#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <omp.h>
#include "functions.h"

// Kiírja a program információit (többb szál használatával)
void print_version_info() {
    #pragma omp parallel sections
    {
        #pragma omp section
        printf("Program verzioszama: 1.0.0\n");
        #pragma omp section
        printf("Elkeszules datuma: 2024.04.29.\n");
        #pragma omp section
        printf("Fejleszto neve: Szabolcsi Daniel\n");
    }
}

// Kiírja a program használatához szükséges információkat
void print_help_info() {
    printf("A program hasznalata:\n");
    printf("./chart {--version | --help} [-send | -receive] [-file | -socket]\n");
    printf("\nA Lehetsges parancssori argumentumok jelentesei:\n");
    printf("--version  |   Kiirja a program verzioszamat, elkeszultenek datumat es a fejleszto nevet.\n");
    printf("--help     |   Segitseget nyujt a program hasznalatahoz es opcioihoz.\n");
    printf("-send      |   Kuldokent viselkedik a rendszer. (alapertelmezett)\n");
    printf("-receive   |   Fogadokent viselkedik a rendszer.\n");
    printf("-file      |   Fajlt hasznal a kommunikaciora. (alapertelmezett)\n");
    printf("-socket    |   Socketeket hasznal a kommunikaciora.\n");
}

int main(int argc, char* argv[]) {
    // Ha nem "chart" a program neve akkor nem indul el
    if (strcmp(argv[0], "./chart") != 0) { fprintf(stderr, "Hiba: A futtathato allomany neve nem megfelelo.\n"); exit(1); }

    // Ha nincs argumentum, vagy túl sok van
    if (argc == 1 || argc > 3) { print_help_info(); exit(2); }

    // Dupla kapcsolós argumentumok ellenőrzése
    if (strcmp(argv[1], "--version") == 0)   { print_version_info(); return 0; }
    else if (strcmp(argv[1], "--help") == 0) { print_help_info(); return 0; }

    int mode = 1;               // 1 - Send   |   0 - Recieve
    int communication = 1;      // 1 - File   |   0 - Socket

    // Egy kapcsolós arhumentumok ellenőrzése
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-send") == 0)          { mode = 1; }
        else if (strcmp(argv[i], "-receive") == 0)  { mode = 0; }
        else if (strcmp(argv[i], "-file") == 0)     { communication = 1; }
        else if (strcmp(argv[i], "-socket") == 0)   { communication = 0; }
        else { print_help_info(); exit(2); }
    }

    // Signal kezelés
    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);

    int* measurements;      // Mérések helyére mutató pointer
    int size;               // Mérések területének mérete

    if (mode) {
        // Mérések elvégzése
        size = Measurement(&measurements);
        if (size == -1) {}
        if (communication) { SendViaFile(measurements, size); }   // Send, File
        else               { SendViaSocket(measurements, size); } // Send, Socket
    }
    else {
        if (communication) {                                      // Receive, File
            // Végtelenített futás, várakozás signálra
            while (1) { signal(SIGUSR1, ReceiveViaFile); }
        }
        else { ReceiveViaSocket(); }                              // Receive, Socket
    }

    // Lefoglalt memória felszabadítása
    free(measurements);

    return EXIT_SUCCESS;
}
