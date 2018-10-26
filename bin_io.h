#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>

//
// public methods
//
FILE* openReadBinary(const char * filename);
FILE* openCreateBinary(const char *filename);
FILE* openUpdateBinary(const char * filename);

int readBinaryFile(const char *filename, void (*processChar)(unsigned char));

void info(const char *msg, ...);
void trace(const char * msg, ...);
void traceCharBin(unsigned char ch);
void traceCharBinMsg(const char *msg, unsigned char ch);

char bit_check(unsigned char ch, int bit_pos);
void bit_set_one(unsigned char * ch, int bit_pos);
void bit_set_zero(unsigned char * ch, int bit_pos);
void bit_copy(unsigned char *byte_to, unsigned char byte_from, int read_pos, int write_pos, int size);

#endif //ALGO_BIN_IO_H
