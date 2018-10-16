#ifndef ALGO_ADHUFF_COMPRESS_H
#define ALGO_ADHUFF_COMPRESS_H

#include "node.h"

int compressFile(const char *input_file, const char *output_file);
void processChar(unsigned char ch);
void outputBitArray(unsigned char bit_array[], int num_bit);
void outputChar(unsigned char ch);
void flushData();


#define HEADER_BITS 3;
#define HEADER_DATA_BITS 5;

#pragma pack(1)
typedef struct
{
    unsigned int header: HEADER_BITS;       // bits for header
    unsigned int data: HEADER_DATA_BITS;    // rest of the bits for data
} first_byte_struct;

typedef union
{
    first_byte_struct split;
    unsigned char raw;
} first_byte_union;
#pragma pack()

#endif //ALGO_ADHUFF_COMPRESS_H
