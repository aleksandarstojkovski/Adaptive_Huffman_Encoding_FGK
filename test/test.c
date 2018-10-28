#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../bin_io.h"
#include "../adhuff_compress.h"
#include "../adhuff_decompress.h"

void    test_uncompress_all_files();
void    test_compress_all_files();
void    test_bit_helpers();
void    test_bit_check(byte_t source, unsigned int bit_pos, byte_t expected);
void    test_bit_set_zero(byte_t source, unsigned int bit_pos, byte_t expected);
void    test_bit_set_one(byte_t source, unsigned int bit_pos, byte_t expected);
void    test_bit_copy(byte_t source, byte_t destination, unsigned int read_pos, unsigned int write_pos, int size, byte_t expected);

#define MAX_FILE_NAME  80
#define NUM_TEST_FILES  11  // skip immagine.tiff for the moment
static const char * TEST_FILES[] = {
        "../../test/res/a-bad-filename",
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
        "../../test/res/immagine.tiff" };

/*
 * Main function
 */
int main(int argc, char* argv[]) {
    test_bit_helpers();
    //test_compress_all_files();
    test_uncompress_all_files();
}

void test_uncompress_all_files() {
    log_info("test_uncompress_all_files\n");
    char input_filename[MAX_FILE_NAME];
    char output_filename[MAX_FILE_NAME];

    for(int i=0; i<NUM_TEST_FILES; i++) {
        strcpy(input_filename, TEST_FILES[i]);
        strcat(input_filename, ".compressed");

        char * filename = strrchr(TEST_FILES[i], '/') + 1;
        strcpy(output_filename, filename);

        adh_decompress_file(input_filename, output_filename);
    }
}

void test_compress_all_files() {
    log_info("test_compress_all_files\n");
    char output_filename[MAX_FILE_NAME];

    for(int i=0; i<NUM_TEST_FILES; i++) {

        char * filename = strrchr(TEST_FILES[i], '/') + 1;
        strcpy(output_filename, filename);
        strcat(output_filename, ".compressed");

        adh_compress_file(TEST_FILES[i], output_filename);
    }
}

/*
 * test bit manipulation functions
 */
void test_bit_helpers() {
    log_info("test_bit_helpers\n");
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

    // source 2 = 0000 0010
    // dest   1 = 0000 0001
    // res    9 = 0000 1001
    // copy 10 from source to dest in pos 3
    test_bit_copy(0x02, 0x01, 1, 3, 2, 0x09);

    // source 2 = 0000 1000
    // dest   1 = 0000 0001
    // res    9 = 0100 0001
    // copy 0100 from source to dest in pos 7
    test_bit_copy(0x08, 0x01, 4, 7, 4, 0x41);
}

void test_bit_set_one(byte_t source, unsigned int bit_pos, byte_t expected) {
    bit_set_one(&source, bit_pos);
    test_bit_check(source, bit_pos, expected);
}

void test_bit_set_zero(byte_t source, unsigned int bit_pos, byte_t expected) {
    bit_set_zero(&source, bit_pos);
    test_bit_check(source, bit_pos, expected);
}

void test_bit_check(byte_t source, unsigned int bit_pos, byte_t expected) {
    char val = bit_check(source, bit_pos);
    if(val != expected)
        perror("error checking bit");
}

void test_bit_copy(byte_t source, byte_t destination, unsigned int read_pos, unsigned int write_pos, int size, byte_t expected) {
    bit_copy(source, &destination, read_pos, write_pos, size);
    if(destination != expected)
        fprintf(stderr, "error copying bits: expected=0x%02X received=0x%02X\n", expected, destination);
}
