#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../bin_io.h"

#define NUM_TEST_FILES  7

int testReadAllBinaryFiles();
int testReadBinaryFile(const char *filename);

void test_bit();
void test_bit_check(char toTest, int bit_pos, char expected);
void test_bit_set_zero(char toTest, int bit_pos, char expected);
void test_bit_set_one(char toTest, int bit_pos, char expected);
void test_bit_copy(char byte_from, char byte_to, int read_pos, int write_pos, int size, char expected);


static const char * TEST_FILES[NUM_TEST_FILES] = {
        "../../test-res/ff_ff_ff",
        "../../test-res/alice.txt",
        "../../test-res/empty",
        "../../test-res/32k_random",
        "../../test-res/32k_ff",
        "../../test-res/immagine.tiff",
        "a-bad-filename" };

/*
 * test all sample files
 */
int testReadAllBinaryFiles() {
    int rc = RC_OK;
    for(int i=0; i<NUM_TEST_FILES; i++) {
        rc = testReadBinaryFile(TEST_FILES[i]);
        if(rc != 0)
            break;
    }

    return rc;
}

/*
 * test Read binary file
 */
int testReadBinaryFile(const char *filename) {
    trace("Start testReadBinaryFile: [%s]\n", filename);

    int rc = readBinaryFile(filename, traceCharBin);
    trace("\nRead completed, return code: %d\n", rc);
    return rc;
}

/*
 * Main function
 */
int main(int argc, char* argv[]) {
    test_bit();

    testReadAllBinaryFiles();
}

void test_bit() {
    // 2 = 00000010
    test_bit_check(2, 2, BIT_0);
    test_bit_check(2, 1, BIT_1);
    test_bit_check(2, 0, BIT_0);

    // 8 = 00001000
    test_bit_check(8, 4, BIT_0);
    test_bit_check(8, 3, BIT_1);
    test_bit_check(8, 2, BIT_0);

    // 8 = 00001000
    test_bit_set_one(8, 0, BIT_1);
    test_bit_set_one(8, 7, BIT_1);

    // 9 = 00001001
    test_bit_set_zero(9, 0, BIT_0);
    test_bit_set_zero(9, 1, BIT_0);

    // source 2 = 00000010
    // dest   1 = 00000001
    // res    9 = 00001001
    test_bit_copy(2, 1, 0, 2, 2, 9);
}

void test_bit_set_one(char toTest, int bit_pos, char expected) {
    bit_set_one(&toTest, bit_pos);
    test_bit_check(toTest, bit_pos, expected);
}

void test_bit_set_zero(char toTest, int bit_pos, char expected) {
    bit_set_zero(&toTest, bit_pos);
    test_bit_check(toTest, bit_pos, expected);
}

void test_bit_check(char toTest, int bit_pos, char expected) {
    char val = bit_check(toTest, bit_pos);
    if(val != expected)
        perror("error checking bit");
}

void test_bit_copy(char byte_from, char byte_to, int read_pos, int write_pos, int size, char expected) {
    bit_copy(&byte_to, byte_from, read_pos, write_pos, size);
    if(byte_to != expected)
        perror("error copying bits");
}
