#ifndef ALGO_ADHUFF_COMPRESS_H
#define ALGO_ADHUFF_COMPRESS_H

#include "node.h"

int compressFile(const char *input_file, const char *output_file);
void compressCallback(char ch);
void encode(Node * node, char ch);
void writeOutput(char ch, int numBit);
void flushData();

#endif //ALGO_ADHUFF_COMPRESS_H
