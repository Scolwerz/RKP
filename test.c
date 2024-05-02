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

/*
int Measurement(int** Values) {
    int size = 10;

    *Values = (int*)malloc(size * sizeof(int));    // Mérések tömb lefoglalása
    if (*Values == NULL) { return -1; }

    // Kezdőérték beállítása
    (*Values)[0] = 0;

    // Többi érték generálása
    for (int i = 1; i < size; i++)
    {
        if (i%2 == 0)       (*Values)[i] = (*Values)[i-1] + 1;
        else if (i%3 == 0)  (*Values)[i] = (*Values)[i-1] - 1;
        else                (*Values)[i] = (*Values)[i-1];
    }
    return size; // Visszatér az előállított értékek számával
}
*/


// Mérések előállítása
int Measurement(int** Values) {
    // if (*Values == NULL) { return -1; }

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




int main()
{
    int* measurements;      // Mérések helyére mutató pointer
    int size;               // Mérések területének mérete

    size = Measurement(&measurements);

    printf("Measurements: (%d db szam)\n",size);
    for (int i = 0; i < size; i++)
    {
        printf("%d  ", measurements[i]);
    }

    BMPcreator(measurements, size);

    free(measurements);

    return EXIT_SUCCESS;
}
