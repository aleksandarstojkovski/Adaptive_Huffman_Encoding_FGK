#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "test.h"

#include "adhuff_compress.h"
#include "adhuff_decompress.h"

/*
 * Print usage
 */
void printUsage() {
    puts("usage:");
    puts("\tto compress a file: algo -c <filename_to_compress> <file");
    puts("\tto decompress a file: algo -d <filename_to_decompress>");
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
    int rc = 0;
    if (argc < 4) {
        fprintf(stderr, "Not enough parameters.\n");
        printUsage();
        rc = 1;

        for(int i=0; i<NUM_TEST_FILES; i++) {
            testReadBinaryFile(TEST_FILES[i]);
        }
    }
    else if (strcmp(argv[1], "-c") == 0) {
        rc = compressFile(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "-d") == 0) {
        rc = decompressFile(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "-t") == 0) {
        rc = testReadBinaryFile(argv[2]);
    }
    else {
        fprintf(stderr, "Unexpected argument\n");
        printUsage();
        rc = 2;
    }

    return rc;
}