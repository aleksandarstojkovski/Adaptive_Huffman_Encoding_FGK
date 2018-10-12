#include <stdio.h>

#include "adhuff_decompress.h"
#include "bin_io.h"
#include "node.h"

/*
 * Decompress Callback
 */
void decompressCallback(char ch) {
    // TODO
    // decode(ch)
    // updateTree(ch)
}

/*
 * decompress file
 */
int decompressFile(const char *input_file, const char *output_file) {
    printf("START decompressing: %s ...\n", input_file);
    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(input_file, decompressCallback);
        destroyTree();
    }
    return rc;
}