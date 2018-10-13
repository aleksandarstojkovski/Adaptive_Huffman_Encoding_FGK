#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bin_io.h"

FILE* openReadBinary(const char * filename) {
    FILE * filePtr = fopen(filename, "rb");
    if(filePtr == NULL) {
        perror("cannot open file in [rb] mode");
    }
    return filePtr;
}

FILE* openWriteBinary(const char * filename) {
    FILE * filePtr = fopen(filename, "wb");
    if(filePtr == NULL) {
        perror("cannot open file in [wb] mode");
    }
    return filePtr;
}


/*
 *  Read a binary file in chunk
 *  for each char read call a callback function
 */
int readBinaryFile(const char *filename, void (*processChar)(char)) {
    FILE * filePtr = openReadBinary(filename);

    unsigned char buffer[BLOCK_SIZE];
    size_t bytesRead = 0;
    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), filePtr)) > 0)
    {
        for(int i=0;i<bytesRead;i++)
            processChar(buffer[i]);
    }
    fclose(filePtr);
    return 0;
}