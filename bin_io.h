#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

int readBinaryFile(const char *filename, void (*processChar)(char));
int writeBinaryFile(const char *filename, char symbol);

#endif //ALGO_BIN_IO_H
