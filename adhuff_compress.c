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
int     process_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
int     output_bit_array(const bit_array_t * bit_array, byte_t *output_buffer, FILE* output_file_ptr);
int     output_new_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr);
int     flush_data(byte_t *output_buffer, FILE* output_file_ptr);
int     flush_header(FILE* output_file_ptr);
int     output_existing_symbol(byte_t symbol, adh_node_t *node, byte_t *output_buffer, FILE* output_file_ptr);
int     output_nyt(byte_t *output_buffer, FILE *output_file_ptr);

/**
 * the main method for compression
 * @param input_file_name
 * @param output_file_name
 * @return RC_OK / RC_FAIL
 */
int adh_compress_file(const char input_file_name[], const char output_file_name[]) {
    log_info("adh_compress_file", "%-40s %s\n", input_file_name, output_file_name);

    FILE *output_file_ptr, *input_file_ptr;
    int rc = adh_init(input_file_name, output_file_name, &output_file_ptr, &input_file_ptr);
    if (rc != RC_OK) goto error_handling;

    byte_t output_buffer[BUFFER_SIZE] = {0};
    byte_t input_buffer[BUFFER_SIZE] = {0};

    // reserve first 3 bits for header
    out_bit_idx = HEADER_BITS;
    is_first_byte = true;

    size_t bytesRead = 0;
    while ((bytesRead = fread(input_buffer, sizeof(byte_t), BUFFER_SIZE, input_file_ptr)) > 0)
    {
        for(int i=0;i<bytesRead;i++) {
            rc = process_symbol(input_buffer[i], output_buffer, output_file_ptr);
            if (rc != RC_OK) goto error_handling;
        }
    }

    // flush remaining data to file
    rc = flush_data(output_buffer, output_file_ptr);
    if (rc != RC_OK) goto error_handling;

    print_final_stats(input_file_ptr, output_file_ptr);

    // close and reopen in update mode
    fclose(output_file_ptr);
    output_file_ptr = bin_open_update(output_file_name);
    if (output_file_ptr == NULL) goto error_handling;

    rc = flush_header(output_file_ptr);

error_handling:
    adh_release(output_file_ptr, input_file_ptr);

    return rc;
}

/**
 * process the given symbol
 * @param symbol
 * @param output_buffer
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int process_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
#ifdef _DEBUG
    log_debug(" process_symbol", "%s out_bit_idx=%-8d\n",
            fmt_symbol(symbol),
            out_bit_idx);
#endif
    int rc;
    adh_node_t* node = adh_search_symbol_in_tree(symbol);
    if(node == NULL) {
        // symbol not present in tree
        rc = output_nyt(output_buffer, output_file_ptr);
        if(rc == RC_OK) {
            rc = output_new_symbol(symbol, output_buffer, output_file_ptr);
        }
    } else {
        // symbol already present in tree
        rc = output_existing_symbol(symbol, node, output_buffer, output_file_ptr);
    }
    return rc;
}

/**
 * write to output the encoding of an existing symbol. then update tree
 * @param symbol
 * @param node
 * @param output_buffer
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int output_existing_symbol(byte_t symbol, adh_node_t *node, byte_t *output_buffer, FILE* output_file_ptr) {
    // write symbol code
    adh_node_t* nodeSymbol = adh_search_symbol_in_tree(symbol);

#ifdef _DEBUG
    log_debug("  output_existing_symbol", "%s out_bit_idx=%-8d bin=%s\n",
             fmt_symbol(symbol),
             out_bit_idx,
             fmt_bit_array(&nodeSymbol->bit_array));
#endif

    int rc = output_bit_array(&(nodeSymbol->bit_array), output_buffer, output_file_ptr);
    if(rc != RC_OK)
        return rc;

    adh_update_tree(node, false);
    return RC_OK;
}

/**
 * write to output the binary version of the symbol. then update tree
 * @param symbol
 * @param output_buffer
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int output_new_symbol(byte_t symbol, byte_t *output_buffer, FILE* output_file_ptr) {
    // write symbol code
    bit_array_t bit_array = {0};
    symbol_to_bits(symbol, &bit_array);

#ifdef _DEBUG
    log_debug("  output_new_symbol", "%s out_bit_idx=%-8d bin=%s\n",
              fmt_symbol(symbol), out_bit_idx,
              fmt_bit_array(&bit_array));
#endif
    int rc = output_bit_array(&bit_array, output_buffer, output_file_ptr);
    if(rc != RC_OK)
        return rc;

    adh_node_t* new_node = adh_create_node_and_append(symbol);
    adh_update_tree(new_node, true);
    return rc;
}

/**
 * write to output the encoding of the NYT node
 * @param output_buffer
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int output_nyt(byte_t *output_buffer, FILE *output_file_ptr) {
    // write NYT code
    adh_node_t* nyt = get_nyt();

#ifdef _DEBUG
    log_debug("  output_nyt", "%3s out_bit_idx=%-8d NYT=%s\n", "",
             out_bit_idx,
             fmt_bit_array(&nyt->bit_array));
#endif

    return output_bit_array(&(nyt->bit_array), output_buffer, output_file_ptr);
}

/**
 * write to output buffer the bit array. if the buffer is full, flush data to file
 * @param bit_array
 * @param output_buffer
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int output_bit_array(const bit_array_t* bit_array, byte_t *output_buffer, FILE* output_file_ptr) {
    for(int i = bit_array->length-1; i>=0; i--) {
        // calculate the current position (in byte) of the output_buffer
        long buffer_byte_idx = bit_idx_to_byte_idx(out_bit_idx);

        // calculate which bit to change in the byte 11100000
        int bit_pos = bit_pos_in_current_byte(out_bit_idx);

        if(bit_array->buffer[i] == BIT_1)
            bit_set_one(&output_buffer[buffer_byte_idx], bit_pos);
        else
            bit_set_zero(&output_buffer[buffer_byte_idx], bit_pos);

        // buffer full, flush data to file
        if(out_bit_idx+1 == BUFFER_SIZE * SYMBOL_BITS) {
            int rc = flush_data(output_buffer, output_file_ptr);
            if(rc != RC_OK)
                return rc;

            // reset buffer index
            out_bit_idx = 0;
            memset(output_buffer, 0, BUFFER_SIZE);
        } else {
            out_bit_idx++;
        }
    }

    return RC_OK;
}

/**
 * flush data to file
 * @param output_buffer
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int flush_data(byte_t *output_buffer, FILE* output_file_ptr) {
    if(out_bit_idx > 0) {
        long num_bytes_to_write = bit_idx_to_byte_idx(out_bit_idx);

        if (get_available_bits(out_bit_idx) < SYMBOL_BITS)
            num_bytes_to_write++;   // reserve the space for odd bits

#ifdef _DEBUG
        log_debug("flush_data", "out_bit_idx=%-8d num_bytes_to_write=%d\n", out_bit_idx, num_bytes_to_write);

        for (int i = 0; i < num_bytes_to_write; i++)
            log_trace_char_bin(output_buffer[i]);
#endif

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

/*!
 * flush header to file
 * @param output_file_ptr
 * @return RC_OK / RC_FAIL
 */
int flush_header(FILE* output_file_ptr) {
#ifdef _DEBUG
    log_trace("flush_header", "old_bits=");
    log_trace_char_bin(first_byte_written);
#endif

    first_byte_union first_byte;
    first_byte.raw = first_byte_written;
    first_byte.split.header = (byte_t)get_available_bits(out_bit_idx);

#ifdef _DEBUG
    log_trace("flush_header", "new_bits=");
    log_trace_char_bin(first_byte.raw);
#endif

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
