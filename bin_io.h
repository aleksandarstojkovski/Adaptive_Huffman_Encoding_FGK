#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>

#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>

/**
 * uncomment line below to turn on LOGGING
 */
//#define _DEBUG

/**
 * constants
 */
enum {
    RC_OK               = 0,
    RC_FAIL             = 1,
    SYMBOL_BITS         = 8,
    MAX_SYMBOL_STR      = 100,
    MAX_CODE_BITS       = 256
};

static const char BIT_1 = '1';
static const char BIT_0 = '0';

typedef uint8_t     byte_t;
/*
 * bit_array_t 256 bit (64 * 4)
 */
typedef struct {
    uint16_t    length;  // 256 should be enough, let's use larger number for error handling
    byte_t      buffer[MAX_CODE_BITS]; //TODO: transform to -> uint64_t     buffer[4];
} bit_array_t;


//
// binary file helpers
//
FILE*       bin_open_read(const char *filename);
FILE*       bin_open_create(const char *filename);
FILE*       bin_open_update(const char *filename);

//
// bit manipulation
//
byte_t      bit_check(byte_t symbol, unsigned int bit_pos);
void        bit_set_one(byte_t * symbol, unsigned int bit_pos);
void        bit_set_zero(byte_t * symbol, unsigned int bit_pos);
void        bit_copy(byte_t source, byte_t *destination, int read_pos, int write_pos, int size);

int         get_available_bits(long buffer_bit_idx);
int         bit_pos_in_current_byte(long buffer_idx);
long        bit_idx_to_byte_idx(long bit_idx);
void        symbol_to_bits(byte_t symbol, bit_array_t *bit_array);

void        print_final_stats(FILE *input_file_ptr, FILE *output_file_ptr);


#endif //ALGO_BIN_IO_H
