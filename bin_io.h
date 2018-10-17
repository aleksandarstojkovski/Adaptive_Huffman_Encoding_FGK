#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>


FILE* openReadBinary(const char * filename);
FILE* openWriteBinary(const char * filename);

int readBinaryFile(const char *filename, void (*processChar)(unsigned char));

void trace(const char * msg, ...);
void traceCharBin(unsigned char ch);
void traceCharBinMsg(const char *msg, unsigned char ch);

char bit_check(unsigned char ch, unsigned int bit_pos);
void bit_set_one(unsigned char * ch, unsigned int bit_pos);
void bit_set_zero(unsigned char * ch, unsigned int bit_pos);
void bit_copy(unsigned char *byte_to, unsigned char byte_from, unsigned int read_pos, unsigned int write_pos, unsigned int size);

#endif //ALGO_BIN_IO_H
