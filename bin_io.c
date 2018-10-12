#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bin_io.h"

enum { BLOCK_SIZE = 1024 };

/*
 *  Read a binary file char by char
 *  and call a callback function after each char read
 */
int readBinaryFile(const char *filename, void (*processChar)(char)) {
    FILE *file = NULL;
    unsigned char buffer[BLOCK_SIZE];
    size_t bytesRead = 0;

    file = fopen(filename, "rb");
    if(file == NULL) {
        perror("cannot open file in [rb] mode");
        return 1;
    }

    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        for(int i=0;i<bytesRead;i++)
            processChar(buffer[i]);
    }
    fclose(file);
    return 0;
}

/*
 *  Write a binary file char by char
 *  and call a callback function after each char read
 */
int writeBinaryFile(const char *filename, char symbol) {
    FILE *file = NULL;
    size_t bytesWritten = 0;

    file = fopen(filename, "wb");
    if(file == NULL) {
        perror("cannot open file in [wb] mode");
        return 1;
    }

    bytesWritten = fwrite(&symbol, sizeof(symbol), 1, file);

    fclose(file);
    return 0;

}