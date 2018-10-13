#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>

enum { BLOCK_SIZE = 1024 };

FILE* openReadBinary(const char * filename);
FILE* openWriteBinary(const char * filename);

int readBinaryFile(const char *filename, void (*processChar)(char));

#endif //ALGO_BIN_IO_H
