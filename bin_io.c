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

/*
 *  open file in read binary mode.
 */
FILE* bin_open_read(const char *filename) {
    return bin_open_file(filename, "rb");
}

/*
 *  create a file in write binary mode. overwrite if existing
 */
FILE* bin_open_create(const char *filename) {
    return bin_open_file(filename, "wb");
}

/*
 *  open a file in read and update mode.
 */
FILE* bin_open_update(const char *filename) {
    return bin_open_file(filename, "rb+");
}

/*
 *  wrapper function to open a file.
 *  in case of error return NULL
 */
FILE* bin_open_file(const char *filename, const char *mode) {
    FILE* file_ptr = fopen(filename, mode);
    if(file_ptr == NULL) {
        log_error("bin_open_file", "cannot open file [%s] in [%s] mode\n", filename, mode);
    }
    return file_ptr;
}

/*
 *  Read a binary file in chunk of BUFFER_SIZE.
 *  for each byte read it calls fn_process_char callback
 */
int bin_read_file(const char *filename, void (*fn_process_char)(byte_t)) {
    FILE * file_ptr = bin_open_read(filename);
    if(file_ptr == NULL)
        return RC_FAIL;

    byte_t buffer[BUFFER_SIZE] = { 0 };
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer, sizeof(byte_t), sizeof(buffer), file_ptr)) > 0)
    {
        for(int i=0;i<bytes_read;i++)
            fn_process_char(buffer[i]);
    }
    fclose(file_ptr);
    return RC_OK;
}



//
// bit manipulation functions
//

/*
 * return '1' if the bit at bit_pos is 1, otherwise '0'
 */
inline byte_t bit_check(byte_t symbol, unsigned int bit_pos) {
    byte_t val = (symbol & (byte_t)(SINGLE_BIT_1 << bit_pos));
    return val ? BIT_1 : BIT_0;
}

inline void bit_set_one(byte_t * symbol, unsigned int bit_pos) {
    *symbol |= (byte_t) (SINGLE_BIT_1 << bit_pos);
}

inline void bit_set_zero(byte_t * symbol, unsigned int bit_pos) {
    *symbol  &= ~((byte_t)(SINGLE_BIT_1 << bit_pos));
}

/*
 * copy bits from most significant bit to least significant
 * e.g. from 5, size 4 -> 5,4,3,2
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

inline int bit_idx_to_byte_idx(int bit_idx) {
    // truncate
    return bit_idx / SYMBOL_BITS;
}

inline int bit_to_change(int buffer_idx) {
    return SYMBOL_BITS - (buffer_idx % SYMBOL_BITS) - 1;
}

inline int get_available_bits(int buffer_bit_idx) {
    return SYMBOL_BITS - (buffer_bit_idx % SYMBOL_BITS);
}

bool compare_bit_arrays(const bit_array_t *bit_array1, const bit_array_t *bit_array2) {
    if(bit_array1->length != bit_array2->length)
        return false;

    for (int i = 0; i < bit_array1->length; ++i) {
        if(bit_array1->buffer[i] != bit_array2->buffer[i])
            return false;
    }
    return true;
}

bool compare_input_and_nyt(const byte_t *input_buffer, int in_bit_idx, int last_bit_idx, const bit_array_t *bit_array_nyt) {
    int size = bit_array_nyt->length;
    if(last_bit_idx - in_bit_idx < size)
        return false;

#ifdef _DEBUG
    log_debug("compare_input_and_nyt", "in_bit_idx=%-8d NYT=%s\n",
              in_bit_idx,
              fmt_bit_array(bit_array_nyt));
#endif

    bool have_same_bits = true;
    for(int offset=0; offset<size; offset++) {
        int byte_idx = bit_idx_to_byte_idx(in_bit_idx + offset);
        byte_t input_byte = input_buffer[byte_idx];

        int input_byte_bit_idx = bit_to_change(in_bit_idx + offset);
        byte_t value = bit_check(input_byte, (unsigned int)input_byte_bit_idx);
        if(value != bit_array_nyt->buffer[size-offset-1]) {
            have_same_bits = false;
            break;
        }
    }
    return have_same_bits;
}

void symbol_to_bits(byte_t symbol, bit_array_t *bit_array) {
    bit_array->length = SYMBOL_BITS;
    for (int bit_pos = SYMBOL_BITS - 1; bit_pos >= 0; --bit_pos) {
        byte_t val = bit_check(symbol, (unsigned int)bit_pos);
        bit_array->buffer[bit_pos] = val;
    }
}

void release_resources(FILE *output_file_ptr, FILE *input_file_ptr) {
    //print_node_array();

    if(output_file_ptr) {
        fclose(output_file_ptr);
    }

    if(input_file_ptr) {
        fclose(input_file_ptr);
    }
    adh_destroy_tree();
}


void print_final_stats(FILE * input_file_ptr, FILE * output_file_ptr) {
    long inSize = ftell(input_file_ptr);
    long outSize = ftell(output_file_ptr);
    double ratio = 100.0 * (inSize - outSize) / inSize;
    log_info(" print_final_stats", "rate= %.2f%% [%ld -> %ld] (bytes)\n", ratio, inSize, outSize);
}
