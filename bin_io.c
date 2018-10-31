#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>

#include "bin_io.h"

//
// private variables
//
static const byte_t     SINGLE_BIT_1 = 0x01u;
static bool             TRACE_ACTIVE = false;

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
// Diagnostic functions
//
void        set_trace_active(bool is_active) { TRACE_ACTIVE = is_active; }
bool        get_trace_active() { return TRACE_ACTIVE; }

void log_trace_char_bin(byte_t symbol) {
    if(!get_trace_active())
        return;

    byte_t bit_array[SYMBOL_BITS] = { 0 };
    symbol_to_bits(symbol, bit_array);
    for (int i = SYMBOL_BITS -1; i >= 0; --i) {
        printf("%c", bit_array[i]);
    }
    puts("");
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
    if(!get_trace_active())
        return;

    va_list args;

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
}

//
// bit manipulation functions
//

/*
 * return '1' if the bit at bit_pos is 1, otherwise '0'
 */
byte_t bit_check(byte_t symbol, unsigned int bit_pos) {
    byte_t val = (symbol & (byte_t)(SINGLE_BIT_1 << bit_pos));
    return val ? BIT_1 : BIT_0;
}

void bit_set_one(byte_t * symbol, unsigned int bit_pos) {
    *symbol |= (byte_t) (SINGLE_BIT_1 << bit_pos);
}

void bit_set_zero(byte_t * symbol, unsigned int bit_pos) {
    *symbol  &= ~((byte_t)(SINGLE_BIT_1 << bit_pos));
}

/*
 * copy bits from most significant bit to least significant
 * e.g. from 5, size 4 -> 5,4,3,2
 */
void bit_copy(byte_t byte_from, byte_t * byte_to, unsigned int read_pos, unsigned int write_pos, int size) {
    for(int offset=0; offset < size; offset++) {

        unsigned int from = read_pos - offset;
        unsigned int to = write_pos - offset;

        byte_t bit = (byte_t) (byte_from >> from) & SINGLE_BIT_1;            /* Get the source bit as 0/1 symbol */
        *byte_to &= ~((byte_t)(SINGLE_BIT_1 << to));                  /* clear destination bit */
        *byte_to |= (byte_t) (bit << to);  /* set destination bit */
    }
}

int bits_to_bytes(int num_bits) {
    // round up
    return (int)ceil(1.0 * num_bits / SYMBOL_BITS);
}

int bit_idx_to_byte_idx(int bit_idx) {
    // truncate
    return bit_idx / SYMBOL_BITS;
}

int bit_to_change(int buffer_idx) {
    return get_available_bits(buffer_idx) - 1;
}

int get_available_bits(int buffer_bit_idx) {
    return SYMBOL_BITS - (buffer_bit_idx % SYMBOL_BITS);
}

bool compare_bit_arrays(const byte_t *bit_array1, int size1, const byte_t *bit_array2, int size2) {
    if(size1 != size2)
        return false;

    for (int i = 0; i < size1; ++i) {
        if(bit_array1[i] != bit_array2[i])
            return false;
    }
    return true;
}

bool compare_input_and_bit_array(const byte_t *input_buffer, int input_buffer_bit_idx, const byte_t *node_bit_array,
                                 int num_bits) {
    log_trace("%-40s num_bit=%d \n", "compare_input_and_bit_array", num_bits);

    bool have_same_bits = true;
    for(int bit_idx=0; bit_idx<num_bits; bit_idx++) {
        int byte_idx = bit_idx_to_byte_idx(input_buffer_bit_idx + bit_idx);
        byte_t input_byte = input_buffer[byte_idx];

        int input_byte_bit_idx = bit_to_change(input_buffer_bit_idx + bit_idx);
        byte_t value = bit_check(input_byte, (unsigned int)input_byte_bit_idx);
        if(value != node_bit_array[bit_idx]) {
            have_same_bits = false;
            break;
        }
    }
    return have_same_bits;
}

void symbol_to_bits(byte_t symbol, byte_t bit_array[]) {
    for (int bit_pos = SYMBOL_BITS - 1; bit_pos >= 0; --bit_pos) {
        byte_t val = bit_check(symbol, (unsigned int)bit_pos);
        bit_array[bit_pos] = val;
    }

    log_trace("%-40s symbol=%-3d char=%c bits=", "symbol_to_bits", symbol, symbol);
    for(int i = SYMBOL_BITS-1; i>=0; i--) {
        log_trace("%c", bit_array[i]);
    }
    log_trace("\n");
}