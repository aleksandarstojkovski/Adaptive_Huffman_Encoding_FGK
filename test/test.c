#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../bin_io.h"
#include "../adhuff_compress.h"


int     test_read_all_test_files();
int     test_read_bin_file(const char *filename);
void    test_compress_all_test_files();

void    test_bit();
void    test_bit_check(uint8_t toTest, int bit_pos, uint8_t expected);
void    test_bit_set_zero(uint8_t toTest, int bit_pos, uint8_t expected);
void    test_bit_set_one(uint8_t toTest, int bit_pos, uint8_t expected);
void    test_bit_copy(uint8_t byte_from, uint8_t byte_to, int read_pos, int write_pos, int size, uint8_t expected);

#define NUM_TEST_FILES  12
static const char * TEST_FILES[NUM_TEST_FILES] = {
        "../../test/res/A.txt",
        "../../test/res/AB.txt",
        "../../test/res/ABA.txt",
        "../../test/res/ABAB.txt",
        "../../test/res/ff_ff_ff",
        "../../test/res/empty",
        "../../test/res/32k_ff",
        "../../test/res/alice_small.txt",
        "../../test/res/alice.txt",
        "../../test/res/32k_random",
        "../../test/res/immagine.tiff",
        "../../test/res/a-bad-filename" };

/*
 * Main function
 */
int main(int argc, char* argv[]) {
    test_bit();
    test_compress_all_test_files();
    //test_read_all_test_files();
}

void test_compress_all_test_files() {
    char outputFile[30];

    for(int i=0; i<NUM_TEST_FILES; i++) {

        char * filename = strrchr(TEST_FILES[i], '/') + 1;
        strcpy(outputFile, filename);
        strcat(outputFile, ".compressed");

        adh_compress_file(TEST_FILES[i], outputFile);
    }
}

/*
 * test all sample files
 */
int test_read_all_test_files() {
    int rc = RC_OK;
    for(int i=0; i<NUM_TEST_FILES; i++) {
        rc = test_read_bin_file(TEST_FILES[i]);
        if(rc != 0)
            break;
    }

    return rc;
}

/*
 * test Read binary file
 */
int test_read_bin_file(const char *filename) {
    log_info("test read: %s\n", filename);
    return bin_read_file(filename, log_trace_char_bin);
}

/*
 * test bit manipulation functions
 */
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

void test_bit_set_one(uint8_t toTest, int bit_pos, uint8_t expected) {
    bit_set_one(&toTest, bit_pos);
    test_bit_check(toTest, bit_pos, expected);
}

void test_bit_set_zero(uint8_t toTest, int bit_pos, uint8_t expected) {
    bit_set_zero(&toTest, bit_pos);
    test_bit_check(toTest, bit_pos, expected);
}

void test_bit_check(uint8_t toTest, int bit_pos, uint8_t expected) {
    char val = bit_check(toTest, bit_pos);
    if(val != expected)
        perror("error checking bit");
}

void test_bit_copy(uint8_t byte_from, uint8_t byte_to, int read_pos, int write_pos, int size, uint8_t expected) {
    bit_copy(&byte_to, byte_from, read_pos, write_pos, size);
    if(byte_to != expected)
        perror("error copying bits");
}
