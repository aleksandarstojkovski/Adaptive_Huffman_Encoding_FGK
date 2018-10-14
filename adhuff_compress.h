#ifndef ALGO_ADHUFF_COMPRESS_H
#define ALGO_ADHUFF_COMPRESS_H

#include "node.h"

int compressFile(const char *input_file, const char *output_file);
void processChar(char ch);
void outputBitArray(char bit_array[], int num_bit);
void outputChar(char ch);
void flushData();

#endif //ALGO_ADHUFF_COMPRESS_H
