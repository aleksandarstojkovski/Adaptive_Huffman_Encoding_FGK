#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>
#include "constants.h"


/*
 * adh_node_t struct
 */
typedef struct bit_array {
    unsigned int    lenght;
    uint8_t         bits[MAX_CODE_SIZE];
} bit_array_t;

//
// public methods
//
FILE*   bin_open_read(const char *filename);
FILE*   bin_open_create(const char *filename);
FILE*   bin_open_update(const char *filename);
int     bin_read_file(const char *filename, void (*fn_process_char)(uint8_t));

void    log_info(const char *msg, ...);
void    log_trace(const char *msg, ...);
void    log_trace_char_bin(uint8_t ch);
void    log_trace_char_bin_msg(const char *msg, uint8_t ch);

char    bit_check(uint8_t ch, int bit_pos);
void    bit_set_one(uint8_t * ch, int bit_pos);
void    bit_set_zero(uint8_t * ch, int bit_pos);
void    bit_copy(uint8_t *byte_to, uint8_t byte_from, int read_pos, int write_pos, int size);


#endif //ALGO_BIN_IO_H
