#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adhuff_compress.h"
#include "adhuff_decompress.h"
#include "log.h"

/**
 * Print usage
 */
void printUsage() {
    puts("usage:");
    puts("\tto compress a file: algo -c <filename_to_compress> <file");
    puts("\tto decompress a file: algo -d <filename_to_decompress>");
}

/**
 * Main function of the application
 * @param argc
 * @param argv
 * @return 0 OK, otherwise FAILURE
 */
int main(int argc, char* argv[])
{
    int rc = 0;
    if (argc < 4) {
        log_error("main", "Not enough parameters.\n");
        printUsage();
        rc = 1;
    }
    else if (strcmp(argv[1], "-c") == 0) {
        rc = adh_compress_file(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "-d") == 0) {
        rc = adh_decompress_file(argv[2], argv[3]);
    }
    else {
        log_error("main", "Unexpected argument\n");
        printUsage();
        rc = 2;
    }

    return rc;
}