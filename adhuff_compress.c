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
void    output_bit_array(const byte_t bit_array[], int length, byte_t *output_buffer, FILE* output_file_ptr);
void    output_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
void    output_new_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
int     flush_data(byte_t *output_buffer, FILE* output_file_ptr);
int     flush_header(FILE* output_file_ptr);
void    output_existing_symbol(byte_t symbol, adh_node_t *node, byte_t *output_buffer, FILE* output_file_ptr);

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
            return rc;
        }

        adh_destroy_tree();

        // close and reopen in update mode
        fclose(output_file_ptr);

        output_file_ptr = bin_open_update(output_file_name);
        if (output_file_ptr == NULL) {
            return RC_FAIL;
        }
        rc = flush_header(output_file_ptr);
    }

    fclose(output_file_ptr);
    fclose(input_file_ptr);

    return rc;
}

/*
 * encode char
 */
void process_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    log_trace("process_symbol", "symbol=%-8d char=%-8c hex=0x%02X\n", symbol, symbol, symbol);

    adh_node_t* node = adh_search_symbol_in_tree(symbol);

    if(node == NULL) {
        // symbol not present in tree
        output_new_symbol(symbol, output_buffer, output_file_ptr);
    } else {
        // char already present in tree
        output_existing_symbol(symbol, node, output_buffer, output_file_ptr);
    }

}

void output_existing_symbol(byte_t symbol, adh_node_t *node, byte_t *output_buffer, FILE* output_file_ptr) {
    log_trace("output_existing_symbol", "symbol=%-8d char=%-8c\n", symbol, symbol);

    byte_t bit_array[MAX_CODE_BITS] = {0};
    // increase weight
    node->weight++;

    // write symbol code
    int num_bit = adh_get_symbol_encoding(symbol, bit_array);
    output_bit_array(bit_array, num_bit, output_buffer, output_file_ptr);
    adh_update_tree(node, false);
}

void output_new_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    log_trace("output_new_symbol", "symbol=%-8d char=%-8c\n", symbol, symbol);

    byte_t bit_array[MAX_CODE_BITS] = {0};
    // write NYT code
    int num_bit = adh_get_NYT_encoding(bit_array);
    output_bit_array(bit_array, num_bit, output_buffer, output_file_ptr);

    // write symbol code
    output_symbol(symbol, output_buffer, output_file_ptr);
    adh_node_t* new_node = adh_create_node_and_append(symbol);
    adh_update_tree(new_node, true);
}

/*
 * copy data to output buffer as char
 */
void output_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    log_trace("output_symbol", "symbol=%-8d char=%-8c\n", symbol, symbol);

    byte_t bit_array[SYMBOL_BITS] = { 0 };
    symbol_to_bits(symbol, bit_array);
    output_bit_array(bit_array, SYMBOL_BITS, output_buffer, output_file_ptr);
}


/*
 * copy data to output buffer as bit array
 */
void output_bit_array(const byte_t bit_array[], int length, byte_t *output_buffer, FILE* output_file_ptr) {
    log_trace("output_bit_array", "length=%-8d bits=", length);
    log_trace_bit_array(bit_array, length);

    for(int i = length-1; i>=0; i--) {
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
        log_trace("flush_data", "out_bit_idx=%d\n", out_bit_idx);

        int num_bytes_to_write = bit_idx_to_byte_idx(out_bit_idx);

        if (get_available_bits(out_bit_idx) > 0)
            num_bytes_to_write++;

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
