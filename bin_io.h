#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>
#include "constants.h"

//
// binary file helpers
//
FILE*       bin_open_read(const char *filename);
FILE*       bin_open_create(const char *filename);
FILE*       bin_open_update(const char *filename);
int         bin_read_file(const char *filename, void (*fn_process_char)(byte_t));

//
// logging
//
void        log_info(const char *msg, ...);
void        log_trace(const char *msg, ...);
void        log_trace_char_bin(byte_t symbol);
void        set_trace_active(bool is_off);
bool        get_trace_active();

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
bool        compare_bit_arrays(const byte_t *bit_array1, int size1, const byte_t *bit_array2, int size2);
bool        compare_input_and_bit_array(const byte_t *input_buffer, int input_buffer_bit_idx,
                                        const byte_t *node_bit_array, int num_bits);
void        symbol_to_bits(byte_t symbol, byte_t bit_array[]);

/*
 * bit_array_t 256 bit (64 * 4)
 */
typedef uint8_t bit_idx_t;
typedef struct {
    bit_idx_t    length;
    uint64_t     buffer[4];
} bit_array_t;



#endif //ALGO_BIN_IO_H
