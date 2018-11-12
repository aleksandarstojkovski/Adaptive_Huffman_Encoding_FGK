#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>
#include "constants.h"

/*
 * bit_array_t 256 bit (64 * 4)
 */
typedef uint16_t bit_idx_t;
typedef struct {
    bit_idx_t   length;
    byte_t      buffer[MAX_CODE_BITS];
    //uint64_t     buffer[4];
} bit_array_t;


//
// binary file helpers
//
FILE*       bin_open_read(const char *filename);
FILE*       bin_open_create(const char *filename);
FILE*       bin_open_update(const char *filename);
int         bin_read_file(const char *filename, void (*fn_process_char)(byte_t));

//
// bit manipulation
//
byte_t      bit_check(byte_t symbol, unsigned int bit_pos);
void        bit_set_one(byte_t * symbol, unsigned int bit_pos);
void        bit_set_zero(byte_t * symbol, unsigned int bit_pos);
void        bit_copy(byte_t byte_from, byte_t *byte_to, unsigned int read_pos, unsigned int write_pos, int size);

int         get_available_bits(int buffer_bit_idx);
int         bit_to_change(int buffer_idx);
int         bit_idx_to_byte_idx(int bit_idx);
int         bits_to_bytes(int num_bits);
bool        compare_bit_arrays(const bit_array_t *bit_array1, const bit_array_t *bit_array2);
bool        compare_input_and_nyt(const byte_t *input_buffer, int in_bit_idx,
                                  const bit_array_t *node_bit_array);
void        symbol_to_bits(byte_t symbol, bit_array_t *bit_array);

void        release_resources(FILE *output_file_ptr, FILE *input_file_ptr);

#endif //ALGO_BIN_IO_H
