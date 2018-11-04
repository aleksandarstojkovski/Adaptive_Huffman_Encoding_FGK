#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../log.h"
#include "../bin_io.h"
#include "../adhuff_compress.h"
#include "../adhuff_decompress.h"

void    test_all_files();
void    test_bit_helpers();
void    test_bit_check(byte_t source, unsigned int bit_pos, byte_t expected);
void    test_bit_set_zero(byte_t source, unsigned int bit_pos, byte_t expected);
void    test_bit_set_one(byte_t source, unsigned int bit_pos, byte_t expected);
void    test_bit_copy(byte_t source, byte_t destination, unsigned int read_pos, unsigned int write_pos, int size, byte_t expected);
int     compare_files(const char *original, const char *generated);

#define MAX_FILE_NAME  80
#define NUM_TEST_FILES  10  // skip immagine.tiff for the moment
static const char * TEST_FILES[] = {
        "../../test/res/alice_small.txt",
        "../../test/res/ABAB.txt",
        "../../test/res/A.txt",
        "../../test/res/AB.txt",
        "../../test/res/ABA.txt",
        "../../test/res/empty",
        "../../test/res/ff_ff_ff",
        "../../test/res/32k_ff",
        "../../test/res/alice.txt",
        "../../test/res/32k_random",
        "../../test/res/immagine.tiff",
        "../../test/res/a-bad-filename"};

/*
 * Main function
 */
int main(int argc, char* argv[]) {
    set_log_level(LOG_DEBUG);
    test_bit_helpers();
    test_all_files();
}

void test_all_files() {
    log_info("test_all_files", "\n");
    char compressed[MAX_FILE_NAME];
    char uncompressed[MAX_FILE_NAME];

    for(int i=0; i<NUM_TEST_FILES; i++) {
        char * filename = strrchr(TEST_FILES[i], '/') + 1;

        strcpy(compressed, filename);
        strcat(compressed, ".compressed");

        strcpy(uncompressed, filename);
        strcat(uncompressed, ".uncompressed");

        int rc = adh_compress_file(TEST_FILES[i], compressed);
        if(rc == RC_FAIL)
            break;

        puts("--------");
        rc = adh_decompress_file(compressed, uncompressed);
        if(rc == RC_FAIL)
            break;

        rc = compare_files(TEST_FILES[i], uncompressed);
        if(rc == RC_FAIL)
            break;
    }
}

int compare_files(const char *original, const char *generated) {
    log_info("compare_files", "%-40s %s\n", original, generated);

    FILE* fp_original = bin_open_read(original);
    FILE* fp_generated = bin_open_read(generated);

    // obtain file size:
    fseek (fp_original, 0, SEEK_END);

    long sz_original = ftell (fp_original);
    rewind (fp_original);

    // obtain file size:
    fseek (fp_generated, 0, SEEK_END);
    long sz_generated = ftell (fp_generated);
    rewind (fp_generated);

    if (sz_original != sz_generated) {
        log_error("compare_files", "different file size.  original: %ld != generated: %ld\n", sz_original, sz_generated);
        return RC_FAIL;
    }

    char ch1, ch2;
    for (int i=0; i<sz_original; i++) {
        fread(&ch1, 1, 1, fp_original);
        fread(&ch2, 1, 1, fp_generated);
        if (ch1 != ch2) {
            log_error("compare_files", "different byte found at position (%x). original 0x%02X != generated 0x%02X\n", i , ch1, ch2);
            return RC_FAIL;
        }
    }
    return RC_OK;
}

/*
 * test bit manipulation functions
 */
void test_bit_helpers() {
    log_info("test_bit_helpers", "\n");
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
        log_error("test_bit_check", "error checking bit");
}

void test_bit_copy(byte_t source, byte_t destination, unsigned int read_pos, unsigned int write_pos, int size, byte_t expected) {
    bit_copy(source, &destination, read_pos, write_pos, size);
    if(destination != expected)
        log_error("test_bit_copy", "error copying bits: expected=0x%02X received=0x%02X\n", expected, destination);
}
