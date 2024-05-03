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
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <omp.h>
#include "functions.h"

#define MAX_PATH_LEN 512            // Fájl elérési útjának eltárolására használt memóriaterület mérete
#define MAX_LINE_LEN 1024           // Sorok beolvasására használt memóriaterület mérete
#define BUFSIZE 1024                // Buffer memóriaterület mérete
#define PORT_NO 3333                // Socketekhez gasznált port

int server_timed_out = 1;           // 1 - A szerver nem elérhető   |  0 - A szerver elérhető
int s_s;                            // Socket ID (Szerver)


// Signal handler
void SignalHandler(int sig) {
    switch (sig) {
        case SIGINT:
            if (!server_timed_out) { close(s_s); }
            printf("\nA folyamat befejezodik, viszlat!\n");
            exit(0);
        case SIGUSR1:
            fprintf(stderr, "\nHiba: A fajlon keresztuli kuldes szolgaltatas nem elerheto.\n");
            exit(1);
        case SIGALRM:
            if (server_timed_out) {
                fprintf(stderr, "\nHiba: A szerver nem valaszol.\n");
                exit(1);
            }
            break;
    }
}

// "Mért" értékek előállítása
int Measurement(int** Values) {
    // if (*Values == NULL) { return -1; }

    srand(time(NULL));
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    int seconds = local_time->tm_min % 15 * 60 + local_time->tm_sec;    // Adott negyedórából eltelt másodpercek
    int size = seconds > 100 ? seconds : 100;                           // seconds és 100 maximuma
    double random;                                                      // Random generált szám
    /*// ### TEST ### ///
    size = 10;
    /// ### TEST ### //*/

    // Mérések tömb lefoglalása
    *Values = (int*)malloc(size * sizeof(int));
    if (*Values == NULL) { return -1; }

    // Kezdőérték beállítása
    (*Values)[0] = 0;

    // További értékek generálása
    for (int i = 1; i < size; i++)
    {
        random = (double)rand() / RAND_MAX;
        if (random  < 0.428571)                (*Values)[i] = (*Values)[i-1] + 1;
        else if (random > 1-((double)11 / 31)) (*Values)[i] = (*Values)[i-1] - 1;
        else                                   (*Values)[i] = (*Values)[i-1];
    }

    /*// ### TEST ### ///
    printf("# Meresek letrehozva.  n=%d\n",size);
    /// ### TEST ### //*/
    return size; // Visszatér az előállított értékek számával
}

// Visszaadja egy unsigned int bájtjait hexadecimális formában, little-endian bájtsorrendben
// uint8_t *bytes = (uint8_t *)&value;
unsigned char* u_int_bytes_little(unsigned int value) {
    unsigned char* bytes = (unsigned char*)malloc(sizeof(unsigned int));
    if (bytes == NULL) { fprintf(stderr, "Hiba: Nem sikerult a memoriafoglalas.\n"); exit(3); }
    // Bájtok tárolása az unsigned char tömbben
    for (int i = 0; i < sizeof(unsigned int); i++) {
        bytes[i] = (value >> (i * 8)) & 0xFF;
    }
    return bytes;
}

// Visszaadja egy unsigned int bájtjait hexadecimális formában, big-endian bájtsorrendben
unsigned char* u_int_bytes_big(unsigned int value) {
    unsigned char* bytes = (unsigned char*)malloc(sizeof(unsigned int));
    if (bytes == NULL) { fprintf(stderr, "Hiba: Nem sikerult a memoriafoglalas.\n"); exit(3); }
    // Bájtok tárolása az unsigned char tömbben
    for (int i = sizeof(unsigned int) - 1; i >= 0; --i) {
       bytes[sizeof(unsigned int) - 1 - i] = (value >> (i * 8)) & 0xFF;
    }
    return bytes;
}

// 4 int értékből csinál egy unsigned int-et (segítség a u_int_bytes_* függvényeknek)
unsigned int pack_rgba(int r, int g, int b, int a) {
    unsigned int packed_value = 0;      // Visszatérési érték
    int values[4] = {b, g, r, a};       // Little-endian szerint a sorrend b, g, r, a
    // |=  ==  +=
    for (int i = 0; i < sizeof(unsigned int); i++) {
        packed_value |= ((unsigned int)values[i] & 0xFF) << i * 8;
    }
    return packed_value;
}

/*// ### TEST ### ///
void print_bits(unsigned int num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; --i) {
        if ((i+1)%8 == 0) printf(" ");
        printf("%d", (num >> i) & 1);
    }
}
/// ### TEST ### //*/

// Létrehozza a "chart.bmp" fájlt
void BMPcreator(int *Values, int NumValues) {
    char* filename = "chart.bmp";                                  // Fájlnév
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;           // Fájl jogosultságai
    FILE* file = fopen(filename, "w");  // Fájl megnyitása
    if (file == NULL) {
        fprintf(stderr, "Hiba: Nem sikerult letrehozni a %s fajlt.\n", filename);
        exit(4);
    }

    unsigned int width = NumValues;                                  // A kép szélessége
    unsigned int height = NumValues;                                 // A kép magassága
    int NumUInts = width % 32 == 0 ? width / 32 : width / 32 + 1;    // 1 sorban lévő unsigned int-ek száma (x * 32  bit)
    unsigned int size = 62 + NumUInts * height * 4;                  // A fájl mérete bájtban

    // Szürke - rgba(100, 100, 100, 0.5) #646464
    int grey_r = 100;
    int grey_g = 100;
    int grey_b = 100;
    int grey_a = 205;
    unsigned int u_grey = pack_rgba(grey_r, grey_g, grey_b, grey_a); // Értékek becsomagolása egy unsigned int-be
    // Pink - rgba(255, 25, 150, 0.8) #FF1996
    int pink_r = 255;
    int pink_g = 25;
    int pink_b = 150;
    int pink_a = 205;
    unsigned int u_pink = pack_rgba(pink_r, pink_g, pink_b, pink_a); // Értékek becsomagolása egy unsigned int-be

    // Az unsigned int-ek bitjeinek szétszedése bájtonként
    unsigned char* grey_bytes = u_int_bytes_little(u_grey);
    unsigned char* pink_bytes = u_int_bytes_little(u_pink);
    unsigned char* size_bytes = u_int_bytes_little(size);
    unsigned char* width_bytes = u_int_bytes_little(width);


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

    // Unsigned int tömb a pixel bitek tárolására
    unsigned int* UInts = (unsigned int*)malloc(NumUInts * height * sizeof(unsigned int));
    if (UInts == NULL) { fprintf(stderr, "Hiba: Nem sikerult a memoriafoglalas.\n"); exit(3); }
    for (int i = 0; i < NumUInts * height; i++) { UInts[i] = 0; }   // Tömb feltőltése 0-val

    int value_min = height % 2 == 0 ? -(height/2)+1 : -(height/2);  // Lehetséges legkisebb érték
    int value_max = height / 2;                                     // Lehetséges legnagyobb érték
    int pos_y = -(value_min);                                       // Az első érték y pozíciója (0 értékű sor indexe alulról) (sorindex)
    int pos_x = 0;                                                  // iterátor és 32-vel osztva unsigned int-ek oszlopindexe
    unsigned int mask = 0x80000000;                                 // unsigned int bitindexe (start: 10000000000000000000000000000000)

    /*// ### TEST ### ///
    printf("\n\nwidth:%d  height:%d  NumUInts:%d  size:%d  min:%d  max:%d  y:%d  x:%d  mask:%x\n\n",
            width, height, NumUInts, size, value_min, value_max, pos_y, pos_x, mask);
    /// ### TEST ### //*/

    UInts[pos_y * NumUInts + pos_x / 32] |= mask;               // Első oszlop bitjének beállítása

    /*// ### TEST ### ///
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
    /// ### TEST ### //*/

    pos_x++; mask /= 2;                                // Sor- és oszlop(bit)index növelése
    for (; pos_x < width; pos_x++, mask /= 2) {
        if (mask == 0x00) { mask = 0x80000000; }      // 10000000000000000000000000000000

        if (Values[pos_x] < Values[pos_x-1]) {        // Ha az előző érték kisebb, csökkentjük az sorindexet
            pos_y = pos_y <= 0 ? 0 : pos_y-1;
        }
        else if (Values[pos_x] > Values[pos_x-1]){    // Ha az előző érték nagyobb, növeljük az sorindexet
            pos_y = pos_y >= height-1 ? height-1 : pos_y+1;
        }
        UInts[pos_y * NumUInts + pos_x / 32] |= mask;

        /*// ### TEST ### ///
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
        /// ### TEST ### //*/
    }

    // Bájtok fájlba írása
    for (int i = 0; i < NumValues * NumUInts; i++) {
        unsigned char* bytes = u_int_bytes_bigendian(UInts[i]);
        fprintf(file, ", 0x%02x, 0x%02x, 0x%02x, 0x%02x", bytes[0], bytes[1], bytes[2], bytes[3]);
        free(bytes);
    }

    // Lefoglalt memóriaterületek felszabadítása
    free(grey_bytes);
    free(pink_bytes);
    free(size_bytes);
    free(width_bytes);
    free(UInts);
    // Fájl bezárása
    fclose(file);

    // Fájl jogosultságainak beállítása
    if (chmod(filename, mode) == -1) {
        fprintf(stderr, "Hiba a jogosultsagok beallitasakor.\n");
        exit(3);
    }
    /*// ### TEST ### ///
    printf("# BMP fajl letrehozva size=%d\n",size);
    /// ### TEST ### //*/
}

// ProcessID megkeresése
int FindPID() {
    DIR* dir;                       // Direcroty pointer
    FILE *file;                     // File pointer
    struct dirent *entry;           // Dirent struktúra
    char path[MAX_PATH_LEN];        // Fájl elérési útvonala
    char line[MAX_LINE_LEN];        // Sor karaktertömb
    int pid = -1;                   // Process azonosító (-1, ha nem találtuk meg a kívánt folyamatot)
    int own_pid = getpid();         // Saját process azonosító

    // A "/proc" könyvtár megnyitása
    dir = opendir("/proc");
    if (dir == NULL) {
        fprintf(stderr, "Hiba: a \"/proc\" konyvtar megnyitasakor.\n");
        exit(3);
    }

    // Végigmegyünk a könyvtár tartalmán
    while ((entry = readdir(dir)) != NULL) {
        // Ha számjeggyel kezdődik az alkönyvtár neve, akkor azt tovább vizsgáljuk
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
                // Ha találtunk pid-et és az a sajátunktól különböző, kilépünk a főciklusból, egyébként folytatjuk a keresést
                if (pid != -1 && pid != own_pid) { break; }
                pid = -1;
            }
        }
    }
    closedir(dir);  // Könyvtár bezárása

    /*// ### TEST ### ///
    printf("\npid: %d\n", pid);
    /// ### TEST ### //*/

    return pid;     // ProcessID visszaadása
}

// Adatok fájlba írása
void SendViaFile(int *Values, int NumValues) {
    // Az adott felhasználó saját alapértelmezett könyvtárának beállítása
    const char *home = getenv("HOME");
    if (home == NULL) { fprintf(stderr, "Hiba a $HOME lekerdezese kozben"); exit(4); }
    if (chdir(home) == -1) { fprintf(stderr, "Hiba az alapertelmezett konyvtarba lepes soran"); exit(4); }


    // "Measurement.txt" megnyitása írásra
    FILE *file = fopen("Measurement.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Hiba: nem sikerult megnyitni a \"Measurement.txt\" fajlt\n");
        exit(4);
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
        exit(5);
    }
    // Ha FindPID más értékkel tér vissza, küldünk az adott folyamatnak egy SIGUSR1 szignált
    kill(pid, SIGUSR1);
}

// Adatok feldolgozása fájlból
void ReceiveViaFile(int sig) {
    // Az adott felhasználó saját alapértelmezett könyvtárának beállítása
    const char *home = getenv("HOME");
    if (home == NULL) { fprintf(stderr, "Hiba a $HOME lekerdezese kozben"); exit(4); }
    if (chdir(home) == -1) { fprintf(stderr, "Hiba az alapertelmezett konyvtarba lepes soran"); exit(4); }


    // "Measurement.txt" megnyitása olvasásra
    FILE *file = fopen("Measurement.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Hiba: nem sikerult megnyitni a \"Measurement.txt\" fajlt\n");
        exit(3);
    }

    // Dinamikus memóriaterület létrehozása
    int capacity = 100;     // Kapacitás (Kezdetben = minimum várható elemek)
    int numValues = 0;      // Elemszám
    int *values = (int *)malloc(capacity * sizeof(int));
    if (values == NULL) { fprintf(stderr, "Hiba: nem sikerult a memoriafoglalas.\n"); exit(3); }

    // "Measurement.txt" beolvasása és értékek tárolása
    int value;    // Aktuális érték
    while (fscanf(file, "%d", &value) == 1) {
        // Ha elfogyott a memóriaterület, akkor bővítjük
        if (numValues == capacity) {
            capacity += 20;
            int* temp = (int*)realloc(values, capacity * sizeof(int));
            if (temp == NULL) { fprintf(stderr, "Hiba: nem sikerult boviteni a memoriateruletet.\n"); exit(3); }
            values = temp;
        }
        values[numValues++] = value;
    }
    // "Measurement.txt" bezárása
    fclose(file);

    // BMPcreator eljárás meghívása
    BMPcreator(values, numValues);

    // Memóriaterület felszabadítása
    free(values);
}

// UDP CLIENT
void SendViaSocket(int *Values, int NumValues) {
    /*// ### TEST ### ///
    printf("##### SendViaSocket kezdete.\n");
    /// ### TEST ### //*/
    // UDP 127.0.0.1 3333 port

    int s_c;                        // Socket ID (Kliens)
    int bytes;                      // Elküldött/Fogadott bájtok
    int flag;                       // Átküldési flag
    char on;                        // sockopt beállítás
    unsigned int server_size;       // sockaddr_in szerver hossza
    struct sockaddr_in server;      // Szerver címe
    int message1;                   // Szervernek küldött egész szám
    int* message2;                  // Szervernek küldött tömb (int)
    int message2_size;              // Küldött tömb mérete
    int response;                   // Szerver válasza

    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port        = htons(PORT_NO);
    server_size            = sizeof server;

    // Socket létrehozása és beállítása
    s_c = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_c < 0) { fprintf(stderr, "Hiba: A socketet nem sikerult letrehozni.\n"); exit(5); }
    setsockopt(s_c, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s_c, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);



    // Adat küldése (int - Tömb mérete)
    message1 = NumValues;
    bytes = sendto(s_c, &message1, sizeof(int) + 1, flag, (struct sockaddr *) &server, server_size);
    if (bytes <= 0) { fprintf(stderr, " Hiba az adat kuldese soran.\n"); exit(5); }
    /*// ### TEST ### ///
    printf ("## %i bytes el lett kuldve a szervernek: %d\n", bytes-1, message1);
    /// ### TEST ### //*/

    // Signalkezelés
    signal(SIGALRM, SignalHandler);
    // Időzítő indítása
    alarm(1);

   // Válasz fogadása (int)
    bytes = recvfrom(s_c, &response, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
    if (bytes < 0) { fprintf(stderr, "Hiba az valasz fogadasa soran.\n"); exit(5); }
    /*// ### TEST ### ///
    printf("## Szerver(%s:%d) valasza: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port), response);
    /// ### TEST ### //*/

    // Szerver válasz flag
    server_timed_out = 0;

    // Üzenet és válasz egyezésének ellenőrzése
    if (message1 != response) {
        fprintf(stderr, "Hiba: A kuldott es kapott ertekek elteroek.\n");
        exit(10);
    }


    // Adat küldése (Values tömb elemei)
    message2 = Values;
    message2_size = sizeof(int) * NumValues;
    bytes = sendto(s_c, &message2, message2_size + 1, flag, (struct sockaddr *) &server, server_size);
    if (bytes <= 0) { fprintf(stderr, " Hiba az adat kuldese soran.\n"); exit(5); }
    /*// ### TEST ### ///
    printf ("## %i bytes el lett kuldve a szervernek: %d elemu tomb\n", bytes-1, message1);
    /// ### TEST ### //*/

    // Válasz fogadása (int)
    bytes = recvfrom(s_c, &response, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
    if (bytes < 0) { fprintf(stderr, "Hiba az valasz fogadasa soran.\n"); exit(5); }
    /*// ### TEST ### ///
    printf("## Szerver(%s:%d) valasza: %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port), response);
    /// ### TEST ### //*/

    if (message2_size != response) {
        fprintf(stderr, "Hiba: A kuldott tomb merete bajtban es kapott ertekek elteroek.\n");
        exit(11);
    }

    // Socket bezárása
    close(s_c);

    /*// ### TEST ### ///
    printf("##### SendViaSocket vege.\n");
    /// ### TEST ### //*/
}

// UDP SERVER
void ReceiveViaSocket() {
    /*// ### TEST ### ///
    printf("### ReceiveViaSocket kezdete.\n");
    /// ### TEST ### //*/

    int bytes;                      // Elküldött/Fogadott bájtok
    int err;                        // Error code
    int flag;                       // Átküldési flag
    char on;                        // sockopt beállítás
    char buffer[MAX_LINE_LEN];      // Bufferként használt karaktertümb
    unsigned int server_size;       // sockaddr_in server hossza
    unsigned int client_size;       // sockaddr_in client hossza
    struct sockaddr_in server;      // Szerver címe
    struct sockaddr_in client;      // Kliens címe
    int message;                    // Klienstől érkezett egész szám (tömb mérete)
    int response;                   // Szerver válasza

    on   = 1;
    flag = 0;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port        = htons(PORT_NO);
    server_size            = sizeof server;
    client_size            = sizeof client;

    // Signalkezelés
    signal(SIGINT, SignalHandler);

    // Socket létrehozása és beállítása
    s_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_s < 0) { fprintf(stderr, "Hiba: A socketet nem sikerult letrehozni.\n"); exit(5); }
    setsockopt(s_s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s_s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    // Socket bindelése
    err = bind(s_s, (struct sockaddr *) &server, server_size);
    if (err < 0) { fprintf(stderr,"Hiba a bindeles soran.\n"); exit(5); }

    // Beérkező üzenetek folyamatos figyelése
    while(1) {
        // Adat fogadása (int - Tömb mérete)
        printf("\n ##### Varakozas egy uzenetre...\n");
        bytes = recvfrom(s_s, &message, sizeof(int), flag, (struct sockaddr *) &client, &client_size);
        if (bytes < 0) { fprintf(stderr, " Hiba az adat fogadasa soran.\n"); exit(5); }
        /*// ### TEST ### ///
        printf ("## %d bytes megerkezett a klienstol(%s:%d). Uzenet: %d",
                bytes-1, inet_ntoa(client.sin_addr), ntohs(client.sin_port), message);
        /// ### TEST ### //*/

        // Válasz küldése (int - Visszaigazolás)
        response = message;
        bytes = sendto(s_s, &response, sizeof(int) + 1, flag, (struct sockaddr *) &client, client_size);
        if (bytes <= 0) { fprintf(stderr, " Hiba az valasz kuldese soran.\n"); exit(5); }
        /*// ### TEST ### ///
        printf("## Valasz el lett kuldve a kliensnek: %d\n", response);
        /// ### TEST ### //*/

        int NumValues = message;
        int* Values = (int*)malloc(sizeof(int) * NumValues);
        if (Values == NULL) { fprintf(stderr, "Hiba a memoriafoglalas soran.\n"); exit(3); }

        // Adat fogadása (Tömb elemei)
        printf("\n ## Waiting for a message...\n");
        bytes = recvfrom(s_s, Values, NumValues * sizeof(int), flag, (struct sockaddr *) &client, &client_size);
        if (bytes <= 0) { fprintf(stderr, " Hiba az adat fogadasa soran.\n"); exit(5); }
        /*// ### TEST ### ///
        printf ("## %d byte megerkezett a klienstol (%s:%d).\n Kliens uzenete: ",
               bytes-1, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        for (int i = 0; i < NumValues * sizeof(int); i++) {
            printf (" %d", Values[i]);
        }
        printf("\n");
        /// ### TEST ### //*/

        // Válasz küldése (int - Tömb mérete bájtban)
        response = NumValues * sizeof;
        bytes = sendto(s_s, &response, sizeof(int) + 1, flag, (struct sockaddr *) &client, client_size);
        if (bytes <= 0) { fprintf(stderr, " Hiba az valasz kuldese soran.\n"); exit(5); }
        /*// ### TEST ### ///
        printf("## Valasz el lett kuldve a kliensnek: %d\n", response);
        /// ### TEST ### //*/



        // BMPcreator meghívása
        BMPcreator(Values, NumValues);
        // Memória felszabadítása
        free(Values);
    }

    /*// ### TEST ### ///
    printf("# ReceiveViaSocket vege.\n");
    /// ### TEST ### //*/
}
