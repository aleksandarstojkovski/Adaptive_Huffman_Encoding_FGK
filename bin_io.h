#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#define BLOCK_SIZE 1024

int readBinaryFile(const char *filename, void (*processChar)(char));
int writeBinaryFile(const char *filename, char symbol);

#endif //ALGO_BIN_IO_H
