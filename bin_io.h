#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>


enum { BLOCK_SIZE = 1024 };

const char BIT_1;
const char BIT_0;


FILE* openReadBinary(const char * filename);
FILE* openWriteBinary(const char * filename);

int readBinaryFile(const char *filename, void (*processChar)(char));

void trace(const char * msg, ...);
void traceCharBin(char ch);
void traceCharBinMsg(const char *msg, char ch);

char bit_check(char ch, int bit_pos);
void bit_set_one(unsigned char * ch, int bit_pos);
void bit_set_zero(unsigned char * ch, int bit_pos);

#endif //ALGO_BIN_IO_H
