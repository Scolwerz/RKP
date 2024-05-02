#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include<sys/stat.h>
#include<fcntl.h>
// #include <netdb.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
#include "functions.h"

#define MAX_PATH_LEN 256            // Fájl elérési útjának eltárolására beolvasására használt string max mérete
#define MAX_LINE_LEN 1024           // Sorok beolvasására használt string max mérete
#define BUFSIZE 1024                // Buffer max mérete
#define PORT_NO 3333                // Használt port

int server_timed_out = 1;


// Signal handler
void SignalHandler(int sig) {
    switch (sig) {
        case SIGINT:
            printf("\nA folyamat befejezodik, viszlat!\n");
            exit(0);
            // break;
        /*case SIGUSR1:
            fprintf(stderr, "\nHiba: A fajlon keresztuli kuldes szolgaltatas nem elerheto.\n");
            exit(1);
            break;
        case SIGALARM:
            if (server_timed_out) {
                fprintf(stderr, "\nHiba: A szerver nem valaszol.\n");
                exit(1);
            }
            break;*/
    }
}

// Mérések előállítása
int Measurement(int** Values) {
    if (*Values == NULL) { return -1; }

    srand(time(NULL));
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    int seconds = local_time->tm_min % 15 * 60 + local_time->tm_sec;    // Adott negyedórából eltelt másodpercek
    int size = seconds > 100 ? seconds : 100;      // seconds és 100 maximuma

    *Values = (int*)malloc(size * sizeof(int));    // Mérések tömb lefoglalása
    if (*Values == NULL) { return -1; }

    // Kezdőérték beállítása
    (*Values)[0] = 0;

    // Többi érték generálása
    for (int i = 1; i < size; i++)
    {
        double random = (double)rand() / RAND_MAX;
        if (random  < 0.428571)                 (*Values)[i] = (*Values)[i-1] + 1;
        else if (random > 1-((double)11 / 31))  (*Values)[i] = (*Values)[i-1] - 1;
        else                                    (*Values)[i] = (*Values)[i-1];
    }


    printf("# Meresek letrehozva.  n=%d\n",size);


    return size; // Visszatér az előállított értékek számával
}


// Visszaadja egy unsigned int bájtjaid hexadevimális formában
// uint8_t *bytes = (uint8_t *)&value;                    0       1        2         3
unsigned char* u_int_bytes(unsigned int value) {   // 10001110 10000110 11011001 00000111
    unsigned char* bytes = (unsigned char*)malloc(sizeof(unsigned int));
    if (bytes == NULL) {
        fprintf(stderr, "Hiba: Nem sikerult a memoriafoglalas.\n");
        exit(1);
    }
    // Bájtok tárolása az unsigned char tömbben
    for (int i = 0; i < sizeof(unsigned int); i++) {
        bytes[i] = (value >> (i * 8)) & 0xFF;
    }

    return bytes;
}

unsigned char* u_int_bytes_bigendian(unsigned int value) {   // 10001110 10000110 11011001 00000111
    unsigned char* bytes = (unsigned char*)malloc(sizeof(unsigned int));
    if (bytes == NULL) {
        fprintf(stderr, "Hiba: Nem sikerult a memoriafoglalas.\n");
        exit(1);
    }
    for (int i = sizeof(unsigned int) - 1; i >= 0; --i) {
       bytes[sizeof(unsigned int) - 1 - i] = (value >> (i * 8)) & 0xFF;
    }
    return bytes;
}

// 4 int értékből csinál egy unsigned int-et (segítség a u_int_bytes függvénynek)
unsigned int pack_rgba(int r, int g, int b, int a) {
    unsigned int packed_value = 0;      // Visszatérési érték
    int values[4] = {b, g, r, a};       // Little-endian szerint a sorrend b, g, r, a
    // |=  ==  +=
    for (int i = 0; i < sizeof(unsigned int); i++) {
        packed_value |= ((unsigned int)values[i] & 0xFF) << i * 8;
    }
    return packed_value;
}

void print_bits(unsigned int num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; --i) {
        if ((i+1)%8 == 0) printf(" ");
        printf("%d", (num >> i) & 1);
    }
    // printf("\n");
}

// Létrehozza a "chart.bmp" fájlt
void BMPcreator(int *Values, int NumValues) {

    // Mindenki olvashatja de csak a tulajdonos írhatja !!!!!!!!!!!!!!!!!!!!!!!!

    FILE *file = fopen("chart.bmp", "w");
    if (file == NULL) {
        fprintf(stderr, "Hiba: Nem sikerult letrehozni a \"chart.bmp\" fajlt.\n");
        exit(3);
    }

    unsigned int width = NumValues;                                  // A kép szélessége
    unsigned int height = NumValues;                                 // A kép magassága
    int NumUInts = width % 32 == 0 ? width / 32 : width / 32 + 1;    // 1 sorban lévő unsigned int-ek (x * 32  bit)
    unsigned int size = 62 + NumUInts * height * 4;                  // A fájl mérete

    // Szürke - rgba(100, 100, 100, 0.5) #646464
    int grey_r = 0x64;
    int grey_g = 0x64;
    int grey_b = 0x64;
    int grey_a = 205;
    unsigned int u_grey = pack_rgba(grey_r, grey_g, grey_b, grey_a);            // Értékek becsomagolása egy unsigned int-be
    // Pink - rgba(255, 25, 150, 0.8) #FF1996
    int pink_r = 0xFF;
    int pink_g = 0x19;
    int pink_b = 0x96;
    int pink_a = 205;
    unsigned int u_pink = pack_rgba(pink_r, pink_g, pink_b, pink_a);            // Értékek becsomagolása egy unsigned int-be
    // Narancs - rgba(255, 150, 0, 0.8) #FF9600
    // int orange_r = 0xFF;
    // int orange_g = 0x96;
    // int orange_b = 0x00;
    // int orange_a = 205;
    // unsigned int u_orange = pack_rgba(orange_r, orange_g, orange_b, orange_a);  // Értékek becsomagolása egy unsigned int-be

    // Az unsigned int-ek bitjeinek szétszedése bájtonként
    unsigned char* grey_bytes = u_int_bytes(u_grey);
    unsigned char* pink_bytes = u_int_bytes(u_pink);
    // unsigned char* orange_bytes = u_int_bytes(u_orange);
    unsigned char* size_bytes = u_int_bytes(size);
    unsigned char* width_bytes = u_int_bytes(width);


    // Fejrész hexadecimális értékeinek írása a fájlba
    fprintf(file, "0x42, 0x4d, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x00, 0x00, 0x00, 0x00,\n",
            size_bytes[0], size_bytes[1], size_bytes[2], size_bytes[3]);
    fprintf(file, "0x3e, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x%02x, 0x%02x,\n",
            width_bytes[0], width_bytes[1]);
    fprintf(file, "0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x01, 0x00, 0x01, 0x00,\n",
            width_bytes[2], width_bytes[3], width_bytes[0], width_bytes[1], width_bytes[2], width_bytes[3]);
    fprintf(file, "0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x0f,\n");
    fprintf(file, "0x00, 0x00, 0x61, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\n");
    fprintf(file, "0x00, 0x00, 0x00, 0x00, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",
            grey_bytes[0], grey_bytes[1], grey_bytes[2], grey_bytes[3], pink_bytes[0], pink_bytes[1]);
    fprintf(file, "0x%02x, 0x%02x", pink_bytes[2], pink_bytes[3]);



    unsigned int* UInts = (unsigned int*)malloc(NumUInts * height * sizeof(unsigned int));
    for (int i = 0; i < NumUInts * height; i++) { UInts[i] = 0; }   // Tömb feltőltése 0-val

    int value_min = height % 2 == 0 ? -(height/2)+1 : -(height/2);  // Lehetséges legkisebb érték
    int value_max = height / 2;                                 // Lehetséges legnagyobb érték
    int pos_y = -(value_min);                                     // Az első érték y pozíciója (0 értékű sor) (sorindex)
    int pos_x = 0;                                              // u_int-ek oszlopindex
    unsigned int mask = 0x80000000;                                       // u_int bitindexe (base: 10000000000000000000000000000000)

    printf("\n\nwidth:%d  height:%d  NumUInts:%d  size:%d  min:%d  max:%d  y:%d  x:%d  mask:%x\n\n",
            width, height, NumUInts, size, value_min, value_max, pos_y, pos_x, mask);

    UInts[pos_y * NumUInts + pos_x / 32] |= mask;               // Első oszlop bitjének beállítása
    /*/ ### TEST ### //
    for (int i = NumUInts * height - 1; i >= 0; i--) {
        print_bits(UInts[i]);
        unsigned char* bytes = u_int_bytes_bigendian(UInts[i]);
        printf("   -   0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
            bytes[0], bytes[1], bytes[2], bytes[3]);
        free(bytes);
    }
    unsigned char* bytes = u_int_bytes_bigendian(UInts[pos_y * NumUInts + pos_x / 32]);
    printf("\nertek: %d    x:%d   y:%d   mask:%x   uint:%d",
    Values[pos_x], bytes[0], bytes[1], bytes[2], bytes[3], pos_x, pos_y, mask,(pos_y * NumUInts + pos_x / 32));
    printf("   mask:"); print_bits(mask);
    printf("\n\n--------------------------------------------------------\n\n");
    free(bytes);
    // ### TEST ### /*/
    pos_x++; mask /= 2;                                // Sor- és oszlop(bit)index növelése
    for (; pos_x < width; pos_x++, mask /= 2) {
        if (mask == 0x00) { mask = 0x80000000; }      // 10000000000000000000000000000000

        if (Values[pos_x] < Values[pos_x-1]) {        // Ha az előző érték kisebb, csökkentjük az oszlopindexet
            pos_y = pos_y <= 0 ? 0 : pos_y-1;
        }
        else if (Values[pos_x] > Values[pos_x-1]){    // Ha az előző érték nagyobb, növeljük az oszlopindexet
            pos_y = pos_y >= height-1 ? height-1 : pos_y+1;
        }
        UInts[pos_y * NumUInts + pos_x / 32] |= mask;

        /*/ ### TEST ### //
        for (int i = NumUInts * height - 1; i >= 0; i--) {
            print_bits(UInts[i]);
            unsigned char* bytes = u_int_bytes_bigendian(UInts[i]);
            printf("   -   0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                bytes[0], bytes[1], bytes[2], bytes[3]);
            free(bytes);
        }
        unsigned char* bytes = u_int_bytes_bigendian(UInts[pos_y * NumUInts + pos_x / 32]);
        printf("\nertek: %d    x:%d   y:%d   mask:%x   uint:%d",
        Values[pos_x], bytes[0], bytes[1], bytes[2], bytes[3], pos_x, pos_y, mask,(pos_y * NumUInts + pos_x / 32));
        printf("   mask:"); print_bits(mask);
        printf("\n\n--------------------------------------------------------\n\n");
        free(bytes);
        // ### TEST ### /*/
    }

    for (int i = 0; i < NumValues * NumUInts; i++) {
        unsigned char* bytes = u_int_bytes_bigendian(UInts[i]);
        fprintf(file, ", 0x%02x, 0x%02x, 0x%02x, 0x%02x", bytes[0], bytes[1], bytes[2], bytes[3]);
        free(bytes);
    }




    /*
    // Pixeltömb írása a fájlba
    int NumBits = NumUInts;
    unsigned int buffer = 0;
    unsigned char* buffer_bytes;
    int current;
    // lehetséges értékek == sorok száma
    for (int i = area_min; i < area_max; i++) {
        // sor hány u_intre osztható fel
        for (int j = 0; j < NumUInts/4; j++) {
            // u_int bitjei
            for (int k = 0; k < 32; k++) {
                current = j*32+k < NumValues ? Values[j*32+k] : INT32_MAX;
                // Ha első vagy utolsó sor akkor a kilógó értékeket is megjelenítjük
                if (i == area_min && current < i || i == area_max && current > i || current == i)
                    buffer |= 0x01;
                buffer << 1;
            }
            buffer_bytes = u_int_bytes(buffer);
            fprintf(file,", 0x%02x, 0x%02x, 0x%02x, 0x%02x ",buffer_bytes[3],buffer_bytes[2],buffer_bytes[1],buffer_bytes[0]);
            free(buffer_bytes);
            buffer = 0;
        }
    }
    */


    // Memóriaterületek felszabadítása
    free(grey_bytes);
    free(pink_bytes);
    // free(orange_bytes);
    free(size_bytes);
    free(width_bytes);
    free(UInts);
    // Fájl bezárása
    fclose(file);

    // Jogosultságok beállítása
    // if (chmod("chart.txt", S_IRUSR | S_IWUSR /* | S_IRGRP | S_IROTH */) != 0) {
    //    fprintf(stderr, "Hiba a jogosultsagok beallitasakor.\n");
    //    exit(1);
    //}


    printf("# BMP fajl letrehozva size=%d\n",size);
}




// ProcessID megkeresése
int FindPID() {

    /////  !!!!!!!!!!!!! int chdir(const char *path);

    DIR* dir;                       // Direcroty pointer
    FILE *file;                     // File pointer
    struct dirent *entry;           // Dirent struktúra
    char path[MAX_PATH_LEN];        // fájl path
    char line[MAX_LINE_LEN];        // sor string
    int pid = -1;                   // process azonosító (-1, ha nem találtuk meg a kívánt folyamatot)
    int own_pid = getpid();         // Saját process azonosító  (pind_t*)

    // A "/proc" könyvtár megnyitása
    dir = opendir("/proc");
    if (dir == NULL) {
        fprintf(stderr, "Hiba: a \"/proc\" konyvtar megnyitasakor.\n");
        exit(3);
    }

    // Végigmegyünk a könyvtár tartalmán
    while ((entry = readdir(dir)) != NULL) {
        // Ha számjeggyel kezdődik az alkönyvtár neve, akkor tovább vizsgáljuk
        if (isdigit(entry->d_name[0])) {
            // Elmentjük a fájl elérési útját
            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            file = fopen(path, "r");    // Fájl megnyitása olvasásra
            if (file != NULL) {
                // Beolvassuk az első sort
                fgets(line, sizeof(line), file);
                // Ha megtaláljuk a "chart" szót, elkezdjük beolvasni az egész fájlt
                if (strcmp(line, "Name:\tchart\n") == 0) {
                    // Keressük a "Pid\t"-vel kezdődő sort
                    while (fgets(line, sizeof(line), file)) {
                        if (strncmp(line, "Pid:\t", 5) == 0) {
                            // Elmentjük a proccess azonosítót és befejettük a beolvasást
                            pid = atoi(line + strlen("Pid:\t"));
                            break;
                        }
                    }
                }
                fclose(file);   // Bezárjuk a megnyitott fájlt
                // Ha találtunk pid-et és az a sajátunktól különbpző, kilépünk a főciklusból
                if (pid != -1 && pid != own_pid) { break; }
                pid = -1;
            }
        }
    }

    closedir(dir);  // Könyvtár bezárása
    return pid;     // ProcessID visszaadása
}

// Adatok fájlba írása
void SendViaFile(int *Values, int NumValues) {
    //  az adott felhasználó saját alapértelmezett könyvtárában !!!!!!!!!!!!!!!!!!!!!
    const char *home = getenv("HOME");
    if (home == NULL) { fprintf(stderr, "Hiba a $HOME lekerdezese kozben"); exit(1); }
    if (chdir(home) == -1) { fprintf(stderr, "Hiba az alapertelmezett konyvtarba lepes soran"); exit(1); }



    // "Measurement.txt" megnyitása írásra
    FILE *file = fopen("Measurement.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Hiba: nem sikerult megnyitni a \"Measurement.txt\" fajlt\n");
        exit(3);
    }

    // Értékek kiírása a fájlba
    for (int i = 0; i < NumValues; i++) {
        fprintf(file, "%d\n", Values[i]);
    }
    fclose(file);   // Fájl bezárása

    // FindPID függvény meghívása
    int pid = FindPID();
    if (pid == -1) {
        fprintf(stderr, "Hiba: Nem talalhato fogado uzemmodban mokodo folyamat.\n");
        exit(3);
    }
    // Ha FindPID más értékkel tér vissza, küldünk a folyamatnak SIGUSR1 szignált
    //kill(pid, SIGUSR1);
}

// Adatok feldolgozása fájlból
void ReceiveViaFile(int sig) {
    // az  adott  felhasználó  alapértelmezett  könyvtárában  lévő  ”Measurement.txt”
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // SIGUSR1 handle !!!!!!!!!!!!!!!!!!!!!!!
    // signal(2, SignalHandler);

    // "Measurement.txt" megnyitása olvasásra
    FILE *file = fopen("Measurement.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Hiba: nem sikerult megnyitni a \"Measurement.txt\" fajlt\n");
        exit(3);
    }

    // Dinamikus memóriaterület létrehozása
    int capacity = 10;  // Kezdeti kapacitás
    int numValues = 0;  // Elemszám
    int *values = (int *)malloc(capacity * sizeof(int));
    if (values == NULL) {
        fprintf(stderr, "Hiba: nem sikerult a memoriafoglalas.\n");
        exit(3);
    }

    // "Measurement.txt" beolvasása és értékek tárolása
    int value;  // Aktuális érték
    while (fscanf(file, "%d", &value) == 1) {
         // Ha elfogyott a memóriaterület, akkor bővítjük
        if (numValues == capacity) {
            capacity *= 2;
            int* temp = (int*)realloc(values, capacity * sizeof(int));
            if (temp == NULL) {
                fprintf(stderr, "Hiba: nem sikerult boviteni a memoriateruletet.\n");
                // free(values);
                exit(3);
            }
            // free(values);
            values = temp;
        }
        values[numValues++] = value;
    }
    fclose(file);   // "Measurement.txt" bezárása

    // BMPcreator eljárás meghívása
    BMPcreator(values, numValues);

    // Memóriaterület felszabadítása
    free(values);
}

// UDP CLIENT
void SendViaSocket(int *Values, int NumValues) {

    printf("### SendViaSocket kezdete.\n");
    /*

    // UDP localhost (IPv4  cím:  127.0.0.1) 3333 portját figyelő fogadó üzemmódú kommunikál.

    int s;                          // Socket ID
    int bytes;                      // Elküldött/Fogadott bájtok
    int flag;                       // Átküldési flag
    char on;                        // sockopt beállítás
    unsigned int server_size;       // sockaddr_in szerver hossza
    struct sockaddr_in server;      // Szerver címe
    int message1;                   // Szervernek küldött egész szám
    int* message2;                  // Szervernek küldött tömb (int)
    int response;                   // Szerver válasza

    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port        = htons(PORT_NO);
    server_size            = sizeof server;

    // Signalkezelés
    signal(SIGALRM, SignalHandler);

    // Socket létrehozása
    s = socket(AF_INET, SOCK_DGRAM, 0 );
    if (s < 0) {
        fprintf(stderr, "Hiba: A socketet nem sikerult letrehozni.\n");
        exit(3);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);



    // Adat küldése (int - Tömb mérete)
    message = NumValues;
    bytes = sendto(s, message, sizeof(int) + 1, flag, (struct sockaddr *) &server, server_size);
    if (bytes <= 0) {
        fprintf(stderr, " Hiba az adat kuldese soran.\n");
        exit(4);
    }
    printf ("##### %i bytes have been sent to server.\n", bytes-1);


    // Időzítő indítása
    alarm(1);


   // Válasz fogadása (int)
   bytes = recvfrom(s, response, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
    if (bytes < 0) {
        fprintf(stderr, "Hiba az valasz fogadasa soran.\n");
        exit(4);
    }
    printf("##### Server's (%s:%d) acknowledgement:\n  %d\n",
           inet_ntoa(server.sin_addr), ntohs(server.sin_port), response);


    // Szerver válasz flag
    server_timed_out = 0;

    // Üzenet és válasz egyezésének ellenőrzése
    if (message != response) {
        fprintf(stderr, "Hiba: A kuldott es kapott ertekek elteroek.\n");
        exit(5);
    }


    // Adat küldése (Values tömb elemei)
    message2 = Values;
    bytes = sendto(s, message2, sizeof(int)*NumValues + 1, flag, (struct sockaddr *) &server, server_size);
    if (bytes <= 0) {
        fprintf(stderr, " Hiba az adat kuldese soran.\n");
        exit(4);
    }
    printf ("##### %i bytes have been sent to server.\n", bytes-1);

   // Válasz fogadása (int)
   bytes = recvfrom(s, response, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
    if (bytes < 0) {
        fprintf(stderr, "Hiba az valasz fogadasa soran.\n");
        exit(4);
    }
    printf("##### Server's (%s:%d) acknowledgement:\n  %d\n",
           inet_ntoa(server.sin_addr), ntohs(server.sin_port), response);



    if (bytes != response) {
        fprintf(stderr, "Hiba: A kuldott tomb merete bajtban es kapott ertekek elteroek.\n");
        exit(5);
    }

    // Socket bezárása
    close(s);

    */

   printf("# SendViaSocket vege.\n");
}



// UDP SERVER
void ReceiveViaSocket() {

    printf("### ReceiveViaSocket kezdete.\n");

    /*
    
    int s;                          // Socket ID
    int bytes;                      // Elküldött/Fogadott bájtok
    int err;                        // Error code
    int flag;                       // Átküldési flag
    char on;                        // sockopt beállítás
    char buffer[MAX_LINE_LEN];      // Bufferként használt karaktertümb
    unsigned int server_size;       // sockaddr_in server hossza
    unsigned int clien_size;        // sockaddr_in client hossza
    struct sockaddr_in server;      // Szerver címe
    struct sockaddr_in client;      // Kliens címe
    int NumValues;                  // Klienstől érkezett egész szám (tömb mérete)
    int* array;                     // Klienstől érkezett tömb (int)
    int response;                   // Szerver válasza

    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port        = htons(PORT_NO);
    server_size            = sizeof server;
    client_size            = sizeof client;
    signal(SIGINT, stop);
    signal(SIGTERM, stop);

    // Socket létrehozása
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        fprintf(stderr, "Hiba: A socketet nem sikerult letrehozni.\n");
        exit(3);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    // Socket bindelése
    err = bind(s, (struct sockaddr *) &server, server_size);
    if (err < 0) {
        fprintf(stderr,"Hiba a bindeles soran.\n");
        exit(3);
    }

    // Beérkező üzenetek folyamatos figyelése
    while(1) {
        // Adat fogadása (int - Tömb mérete)
        printf("\n ##### Waiting for a message...\n");
        bytes = recvfrom(s, NumValues, sizeof(int), flag, (struct sockaddr *) &client, &client_size);
        if (bytes <= 0) {
            fprintf(stderr, " Hiba az adat fogadasa soran.\n");
            exit(4);
        }
        printf ("##### %d bytes have been received from the client (%s:%d).\n Client's message:\n  %d",
               bytes-1, inet_ntoa(client.sin_addr), ntohs(client.sin_port), NumValues);

        // Válasz küldése (int - Visszaigazolás)
        response = NumValues;
        bytes = sendto(s, response, sizeof(int) + 1, flag, (struct sockaddr *) &client, client_size);
        if (bytes <= 0) {
            fprintf(stderr, " Hiba az valasz kuldese soran.\n");
            exit(4);
        }
        printf("##### Acknowledgement have been sent to client.\n");



        int* Values = (int*)malloc(sizeof(int) * NumValues);



        // Adat fogadása (Tömb elemei)
        printf("\n ##### Waiting for a message...\n");
        bytes = recvfrom(s, array, sizeof(int), flag, (struct sockaddr *) &client, &client_size);
        if (bytes <= 0) {
            fprintf(stderr, " Hiba az adat fogadasa soran.\n");
            exit(4);
        }
        printf ("##### %d bytes have been received from the client (%s:%d).\n Client's message:\n  %d",
               bytes-1, inet_ntoa(client.sin_addr), ntohs(client.sin_port), array);



        for (int i = 0; i < NumValues; i++) {
            Values[i] = array[i];
        }


        // Válasz küldése (int - Tömb mérete bájtban)
        response = sizeof(Values);
        bytes = sendto(s, response, sizeof(int) + 1, flag, (struct sockaddr *) &client, client_size);
        if (bytes <= 0) {
            fprintf(stderr, " Hiba az valasz kuldese soran.\n");
            exit(4);
        }
        printf("##### Acknowledgement have been sent to client.\n");



        // BMPcreator meghívása
        BMPcreator(Values, NumValues);
        // Memória felszabadítása
        free(Values);
    }

    */

   printf("# ReceiveViaSocket vege.\n");

}

void stop_server(int sig) {
    // close(server);
    printf("\n Szerver leallitva.\n");
    exit(0);
}

