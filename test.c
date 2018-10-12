#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "bin_io.h"

char *TEST_FILES[NUM_TEST_FILES] = {
        "../test-res/ff_ff_ff",
        "../test-res/alice.txt",
        "../test-res/empty",
        "../test-res/32k_random",
        "../test-res/32k_ff",
        "../test-res/immagine.tiff",
        "a-bad-filename" };


/*
 * Test Read Binary file
 */
int testReadBinaryFile(const char *filename) {
    printf("Start testReadBinaryFile: [%s]\n", filename);

    int rc = readBinaryFile(filename, printCharBin);

    printf("\nRead completed, return code: %d\n", rc);
    printf("press a key to continue\n");
    getchar();

    return rc;
}

/*
 * print char in binary format
 */
void printCharBin(char ch) {
    for (unsigned short bitPos = 7; bitPos >= 0; --bitPos) {
        char val = (ch & (1 << bitPos));
        putchar(val ? '1' : '0');
    }
}
