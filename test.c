#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "bin_io.h"

enum { NUM_TEST_FILES = 7 };
static const char * TEST_FILES[NUM_TEST_FILES] = {
        "../test-res/ff_ff_ff",
        "../test-res/alice.txt",
        "../test-res/empty",
        "../test-res/32k_random",
        "../test-res/32k_ff",
        "../test-res/immagine.tiff",
        "a-bad-filename" };

int testReadAllBinaryFiles() {
    int rc = 0;
    for(int i=0; i<NUM_TEST_FILES; i++) {
        rc = testReadBinaryFile(TEST_FILES[i]);
        if(rc != 0)
            break;
    }

    return 0;
}

/*
 * Test Read Binary file
 */
int testReadBinaryFile(const char *filename) {
    trace("Start testReadBinaryFile: [%s]\n", filename);

    int rc = readBinaryFile(filename, traceCharBin);

    trace("\nRead completed, return code: %d\n", rc);
    trace("press a key to continue\n");
    getchar();

    return rc;
}