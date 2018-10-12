#ifndef ALGO_ADHUFF_COMPRESS_H
#define ALGO_ADHUFF_COMPRESS_H

int compressFile(const char *input_file, const char *output_file);
void compressCallback(char ch);
void encode(char ch);

#endif //ALGO_ADHUFF_COMPRESS_H
