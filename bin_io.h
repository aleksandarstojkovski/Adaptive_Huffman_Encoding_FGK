#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>
#include "constants.h"


/*
 * bit_array struct
 */
typedef struct bit_array {
    unsigned int    lenght;
    byte_t         bits[MAX_CODE_SIZE];
} bit_array_t;

//
// public methods
//
FILE*       bin_open_read(const char *filename);
FILE*       bin_open_create(const char *filename);
FILE*       bin_open_update(const char *filename);
int         bin_read_file(const char *filename, void (*fn_process_char)(byte_t));

void        log_info(const char *msg, ...);
void        log_trace(const char *msg, ...);
void        log_trace_char_bin(byte_t symbol);
void        log_trace_char_bin_msg(const char *msg, byte_t symbol);

byte_t      bit_check(byte_t symbol, unsigned int bit_pos);
void        bit_set_one(byte_t * symbol, unsigned int bit_pos);
void        bit_set_zero(byte_t * symbol, unsigned int bit_pos);
void        bit_copy(byte_t *byte_to, byte_t byte_from, unsigned int read_pos, unsigned int write_pos, int size);

int         bit_idx_to_byte_idx(int bit_idx);
int         bits_to_bytes(int num_bits);
bool        compare_bit_array(const byte_t input_buffer[], const byte_t node_bit_array[], int num_bits);


#endif //ALGO_BIN_IO_H
