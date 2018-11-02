#include <string.h>
#include "adhuff_compress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"
#include "log.h"

//
// modules variables
//
static int      out_bit_idx;
static byte_t   first_byte_written;
static bool     is_first_byte = true;

//
// private methods
//
void    process_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
void    output_bit_array(const byte_t bit_array[], int size, byte_t *output_buffer, FILE* output_file_ptr);
void    output_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
void    output_new_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
int     flush_data(byte_t *output_buffer, FILE* output_file_ptr);
int     flush_header(FILE* output_file_ptr);
void    output_existing_symbol(byte_t symbol, adh_node_t *node, byte_t *output_buffer, FILE* output_file_ptr);
void    output_nyt(byte_t *output_buffer, FILE *output_file_ptr);

/*
 * Compress file
 */
int adh_compress_file(const char input_file_name[], const char output_file_name[]) {
    log_info("adh_compress_file", "%-40s %s\n", input_file_name, output_file_name);

    int rc = RC_OK;
    FILE* input_file_ptr = bin_open_read(input_file_name);
    if (input_file_ptr == NULL) {
        rc = RC_FAIL;
    }

    FILE* output_file_ptr = bin_open_create(output_file_name);
    if (output_file_ptr == NULL) {
        rc = RC_FAIL;
    }

    if(rc == RC_OK)
        rc = adh_init_tree();

    if(rc == RC_OK) {

        byte_t output_buffer[BUFFER_SIZE] = {0};
        byte_t input_buffer[BUFFER_SIZE] = {0};

        // reserve first 3 bits for header
        out_bit_idx = HEADER_BITS;
        is_first_byte = true;

        size_t bytesRead = 0;
        while ((bytesRead = fread(input_buffer, sizeof(byte_t), BUFFER_SIZE, input_file_ptr)) > 0)
        {
            for(int i=0;i<bytesRead;i++)
                process_symbol(input_buffer[i], output_buffer, output_file_ptr);
        }

        // flush remaining data to file
        rc = flush_data(output_buffer, output_file_ptr);
        if (rc == RC_FAIL) {
            release_resources(output_file_ptr, input_file_ptr);
            return rc;
        }

        adh_destroy_tree();

        // close and reopen in update mode
        fclose(output_file_ptr);

        output_file_ptr = bin_open_update(output_file_name);
        if (output_file_ptr == NULL) {
            release_resources(output_file_ptr, input_file_ptr);
            return RC_FAIL;
        }
        rc = flush_header(output_file_ptr);
    }

    release_resources(output_file_ptr, input_file_ptr);

    return rc;
}

/*
 * encode char
 */
void process_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    char symbol_str[40] = {0};
    log_info("process_symbol", "%s\n", fmt_symbol(symbol, symbol_str, sizeof(symbol_str)));

    adh_node_t* node = adh_search_symbol_in_tree(symbol);

    if(node == NULL) {
        // symbol not present in tree
        output_nyt(output_buffer, output_file_ptr);
        output_new_symbol(symbol, output_buffer, output_file_ptr);
    } else {
        // char already present in tree
        output_existing_symbol(symbol, node, output_buffer, output_file_ptr);
    }

}

void output_existing_symbol(byte_t symbol, adh_node_t *node, byte_t *output_buffer, FILE* output_file_ptr) {
    char symbol_str[40] = {0};
    log_debug("output_existing_symbol", "out_bit_idx=%-8d %s\n", out_bit_idx, fmt_symbol(symbol, symbol_str, sizeof(symbol_str)));

    byte_t bit_array[MAX_CODE_BITS] = {0};
    // increase weight
    node->weight++;

    // write symbol code
    int num_bit = adh_get_symbol_encoding(symbol, bit_array);
    output_bit_array(bit_array, num_bit, output_buffer, output_file_ptr);
    adh_update_tree(node, false);
}

void output_new_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    char symbol_str[40] = {0};
    log_debug("output_new_symbol", "out_bit_idx=%-8d %s\n", out_bit_idx, fmt_symbol(symbol, symbol_str, sizeof(symbol_str)));

    // write symbol code
    output_symbol(symbol, output_buffer, output_file_ptr);
    adh_node_t* new_node = adh_create_node_and_append(symbol);
    adh_update_tree(new_node, true);
}

void output_nyt(byte_t *output_buffer, FILE *output_file_ptr) {
    log_debug("output_nyt", "out_bit_idx=%-8d\n", out_bit_idx);

    byte_t bit_array[MAX_CODE_BITS] = {0};
    // write NYT code
    int nyt_size = adh_get_NYT_encoding(bit_array);
    output_bit_array(bit_array, nyt_size, output_buffer, output_file_ptr);

    log_debug("output_nyt", "out_bit_idx=%-8d nyt_size=%d\n", out_bit_idx, nyt_size);
}

/*
 * copy data to output buffer as char
 */
void output_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    byte_t bit_array[SYMBOL_BITS] = { 0 };
    symbol_to_bits(symbol, bit_array);

    char symbol_str[40] = {0};
    char bit_array_str[9] = {0};
    log_debug("output_symbol", "%s bin=%s\n",
            fmt_symbol(symbol, symbol_str, sizeof(symbol_str)),
            fmt_bit_array(bit_array, SYMBOL_BITS, bit_array_str, sizeof(bit_array_str)));

    output_bit_array(bit_array, SYMBOL_BITS, output_buffer, output_file_ptr);
}

/*
 * copy data to output buffer as bit array
 */
void output_bit_array(const byte_t bit_array[], int size, byte_t *output_buffer, FILE* output_file_ptr) {
    char bit_array_str[256] = {0};
    log_debug("output_bit_array", "size=%-3d bin=%s\n", size,
              fmt_bit_array(bit_array, size, bit_array_str, sizeof(bit_array_str)));

    for(int i = size-1; i>=0; i--) {
        // calculate the current position (in byte) of the output_buffer
        int buffer_byte_idx = bit_idx_to_byte_idx(out_bit_idx);

        // calculate which bit to change in the byte 11100000
        int bit_pos = bit_to_change(out_bit_idx);

        if(bit_array[i] == BIT_1)
            bit_set_one(&output_buffer[buffer_byte_idx], bit_pos);
        else
            bit_set_zero(&output_buffer[buffer_byte_idx], bit_pos);

        // buffer full, flush data to file
        if(out_bit_idx+1 == BUFFER_SIZE * SYMBOL_BITS) {
            flush_data(output_buffer, output_file_ptr);

            // reset buffer index
            out_bit_idx = 0;
            memset(output_buffer, 0, BUFFER_SIZE);
        } else {
            out_bit_idx++;
        }

    }
}


/*
 * flush data to file
 */
int flush_data(byte_t *output_buffer, FILE* output_file_ptr) {
    if(out_bit_idx > 0) {
        int num_bytes_to_write = bit_idx_to_byte_idx(out_bit_idx);
        if (get_available_bits(out_bit_idx) > 0)
            num_bytes_to_write++;

        log_debug("flush_data", "out_bit_idx=%-8d num_bytes_to_write=%d\n", out_bit_idx, num_bytes_to_write);

        for (int i = 0; i < num_bytes_to_write; i++)
            log_trace_char_bin(output_buffer[i]);

        size_t bytesWritten = fwrite(output_buffer, sizeof(byte_t), num_bytes_to_write, output_file_ptr);
        if (bytesWritten != num_bytes_to_write) {
            perror("failed to write compressed file");
            return RC_FAIL;
        }

        if (is_first_byte) {
            first_byte_written = output_buffer[0];
            is_first_byte = false;
        }
    }
    return RC_OK;
}

/*
 * flush header to file
 */
int flush_header(FILE* output_file_ptr) {
    log_trace("flush_header", "old_bits=");
    log_trace_char_bin(first_byte_written);

    first_byte_union first_byte;
    first_byte.raw = first_byte_written;
    first_byte.split.header = (byte_t)get_available_bits(out_bit_idx);

    log_trace("flush_header", "new_bits=");
    log_trace_char_bin(first_byte.raw);

    fputc(first_byte.raw, output_file_ptr);

    if ( fseek(output_file_ptr, 0L, SEEK_SET) != 0 ) {
        perror("error moving file ptr to beginning");
        return RC_FAIL;
    }

    size_t bytesWritten = fwrite(&first_byte.raw, sizeof(byte_t), 1, output_file_ptr);
    if(bytesWritten == 0) {
        perror("failed to overwrite first byte");
        return RC_FAIL;
    }

    return RC_OK;
}
