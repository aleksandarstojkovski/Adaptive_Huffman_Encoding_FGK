#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "bin_io.h"

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
        fprintf(stderr, "cannot open file [%s] in [%s] mode\n", filename, mode);
    }
    return file_ptr;
}

/*
 *  Read a binary file in chunk
 *  for each byte read it calls fn_process_char callback
 */
int bin_read_file(const char *filename, void (*fn_process_char)(uint8_t)) {
    FILE * filePtr = bin_open_read(filename);
    if(filePtr == NULL)
        return RC_FAIL;

    uint8_t buffer[BUFFER_SIZE] = { 0 };
    size_t bytesRead = 0;
    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), filePtr)) > 0)
    {
        for(int i=0;i<bytesRead;i++)
            fn_process_char(buffer[i]);
    }
    fclose(filePtr);
    return RC_OK;
}


//
// Diagnostic functions
//

void log_trace_char_bin(uint8_t symbol) {
    if(TRACE_OFF)
        return;

    for (int bitPos = SYMBOL_BITS-1; bitPos >= 0; --bitPos) {
        char val = bit_check(symbol, bitPos);
        putchar(val);
    }
    printf("\n");
}

void log_trace_char_bin_msg(const char *msg, uint8_t symbol) {
    if(TRACE_OFF)
        return;

    log_trace(msg);
    log_trace_char_bin(symbol);
}

void print_time() {
    time_t rawtime;
    time (&rawtime);
    struct tm * timeinfo = localtime (&rawtime);

    char time_string[10];
    strftime(time_string, 10, "%T",timeinfo);

    printf("%s ", time_string);
}

void log_info(const char *msg, ...) {
    print_time();

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
}

void log_trace(const char *msg, ...) {
    if(TRACE_OFF)
        return;

    va_list args;

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
}


/*
 * return '1' if the bit at bit_pos is 1, otherwise '0'
 */
char bit_check(uint8_t symbol, int bit_pos) {
    uint8_t val = (symbol & (0x01 << bit_pos));
    return val ? BIT_1 : BIT_0;
}

void bit_set_one(uint8_t * symbol, int bit_pos) {
    *symbol |= (0x01 << bit_pos);
}

void bit_set_zero(uint8_t * symbol, int bit_pos) {
    *symbol  &= ~(0x01 << bit_pos);
}

void bit_copy(uint8_t * byte_to, uint8_t byte_from, int read_pos, int write_pos, int size) {
    for(unsigned int offset=0; offset < size; offset++) {

        unsigned int from = read_pos + offset;
        unsigned int to = write_pos + offset;

        unsigned int bit;
        bit = (byte_from >> from) & 0x01;            /* Get the source bit as 0/1 symbol */
        *byte_to &= ~(0x01 << to);                  /* clear destination bit */
        *byte_to |= (bit << to);  /* set destination bit */
    }
}

uint16_t bits_to_bytes(uint16_t num_bits) {
    // round up
    return ceil(1.0 * num_bits / SYMBOL_BITS);
}

uint16_t bit_idx_to_byte_idx(uint16_t bit_idx) {
    // truncate
    return bit_idx / SYMBOL_BITS;
}
