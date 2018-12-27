#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "bin_io.h"
#include "log.h"

//
// private variables
//
static const byte_t     SINGLE_BIT_1 = 0x01u;

//
// private methods
//
FILE* bin_open_file(const char *filename, const char *mode);

/**
 * open file in read binary mode.
 * @param filename
 * @return the FILE pointer
 */
FILE* bin_open_read(const char *filename) {
    return bin_open_file(filename, "rb");
}

/**
 * create a file in write binary mode. overwrite if existing
 * @param filename
 * @return the FILE pointer
 */
FILE* bin_open_create(const char *filename) {
    return bin_open_file(filename, "wb");
}

/**
 * open a file in read and update mode.
 * @param filename
 * @return the FILE pointer
 */
FILE* bin_open_update(const char *filename) {
    return bin_open_file(filename, "rb+");
}

/**
 * wrapper function to open a file.
 * @param filename
 * @param mode
 * @return the FILE pointer, NULL in case of error
 */
FILE* bin_open_file(const char *filename, const char *mode) {
    FILE* file_ptr = fopen(filename, mode);
    if(file_ptr == NULL) {
        log_error("bin_open_file", "cannot open file [%s] in [%s] mode\n", filename, mode);
    }
    return file_ptr;
}


//
// bit manipulation functions
//

/**
 * @param symbol
 * @param bit_pos
 * @return the char '1' if the bit at bit_pos is 1, otherwise '0'
 */
inline byte_t bit_check(byte_t symbol, unsigned int bit_pos) {
    byte_t val = (symbol & (byte_t)(SINGLE_BIT_1 << bit_pos));
    return val ? BIT_1 : BIT_0;
}

/**
 * set to 1 the bit at given position
 * @param symbol
 * @param bit_pos
 */
inline void bit_set_one(byte_t * symbol, unsigned int bit_pos) {
    *symbol |= (byte_t) (SINGLE_BIT_1 << bit_pos);
}

/**
 * set to 0 the bit at given position
 * @param symbol
 * @param bit_pos
 */
inline void bit_set_zero(byte_t * symbol, unsigned int bit_pos) {
    *symbol  &= ~((byte_t)(SINGLE_BIT_1 << bit_pos));
}

/**
 * copy bits from most significant bit to least significant
 * e.g. from 5, size 4 -> 5,4,3,2
 * @param byte_from: the source
 * @param byte_to: the destination (may use 2 bytes)
 * @param read_pos: bit position in input
 * @param write_pos: bit position in output
 * @param size: number of bits to copy
 */
void bit_copy(byte_t byte_from, byte_t * byte_to, unsigned int read_pos, unsigned int write_pos, int size) {
    for(int offset=0; offset < size; offset++) {
        unsigned int from = read_pos - offset;
        int to = write_pos - offset;
        if(to < 0) {
            to += SYMBOL_BITS;
        }

        byte_t bit = (byte_t) (byte_from >> from) & SINGLE_BIT_1;            /* Get the source bit as 0/1 symbol */
        *byte_to &= ~((byte_t)(SINGLE_BIT_1 << to));                  /* clear destination bit */
        *byte_to |= (byte_t) (bit << to);  /* set destination bit */

        if(to == 0) {
            byte_to++;
        }
    }
}

/**
 * @param bit_idx
 * @return the byte index from the bit index
 */
inline long bit_idx_to_byte_idx(long bit_idx) {
    return (bit_idx / SYMBOL_BITS);
}

/**
 * @param buffer_idx
 * @return bit position in current byte
 */
inline int bit_pos_in_current_byte(long buffer_idx) {
    return SYMBOL_BITS - (buffer_idx % SYMBOL_BITS) - 1;
}

/**
 * @param buffer_bit_idx
 * @return number of remaining bits for the current byte
 */
inline int get_available_bits(long buffer_bit_idx) {
    return SYMBOL_BITS - (buffer_bit_idx % SYMBOL_BITS);
}

/**
 * fill the bit_array with the binary representation of the symbol
 * @param symbol
 * @param bit_array
 */
void symbol_to_bits(byte_t symbol, bit_array_t *bit_array) {
    bit_array->length = SYMBOL_BITS;
    for (int bit_pos = SYMBOL_BITS - 1; bit_pos >= 0; --bit_pos) {
        byte_t val = bit_check(symbol, (unsigned int)bit_pos);
        bit_array->buffer[bit_pos] = val;
    }
}

/**
 * print the compression ratio between the input and output
 * @param input_file_ptr
 * @param output_file_ptr
 */
void print_final_stats(FILE * input_file_ptr, FILE * output_file_ptr) {
    long inSize = ftell(input_file_ptr);
    long outSize = ftell(output_file_ptr);
    double ratio = 100.0 * (inSize - outSize) / inSize;
    log_info(" print_final_stats", "rate= %.2f%% [%ld -> %ld] (bytes)\n", ratio, inSize, outSize);
}
