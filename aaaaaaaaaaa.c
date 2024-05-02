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



void print_bits(unsigned int num) {
    for (int i = sizeof(num) * 8 - 1; i >= 0; --i) {
        printf("%d", (num >> i) & 1);
    }
    printf("\n");
}

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




    for (int i = 0; i <4; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");

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


    for (int i = 0; i <4; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");

    return bytes;
}


int main() {
    unsigned int x = 42;
    print_bits(x);
    printf("\n\nLit:  ");
    unsigned char* b = u_int_bytes(x);
    free(b);
    printf("\n\nBig:  ");
    b = u_int_bytes_bigendian(x);
    free(b);

    return 0;
}
