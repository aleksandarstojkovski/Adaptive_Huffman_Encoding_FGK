#ifndef ALGO_BIN_IO_H
#define ALGO_BIN_IO_H

#include <stdio.h>

//
// public methods
//
FILE*   bin_open_read(const char *filename);
FILE*   bin_open_create(const char *filename);
FILE*   bin_open_update(const char *filename);
int     bin_read_file(const char *filename, void (*fn_process_char)(unsigned char));

void    log_info(const char *msg, ...);
void    log_trace(const char *msg, ...);
void    log_trace_char_bin(unsigned char ch);
void    log_trace_char_bin_msg(const char *msg, unsigned char ch);

char    bit_check(unsigned char ch, int bit_pos);
void    bit_set_one(unsigned char * ch, int bit_pos);
void    bit_set_zero(unsigned char * ch, int bit_pos);
void    bit_copy(unsigned char *byte_to, unsigned char byte_from, int read_pos, int write_pos, int size);


#endif //ALGO_BIN_IO_H
