#include "adhuff_compress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

//
// modules variables
//
static uint8_t  output_buffer[BUFFER_SIZE] = {0};
static uint16_t buffer_bit_idx;
static FILE *   output_file_ptr;
static uint8_t  first_byte_written;

//
// private methods
//
void    process_symbol(uint8_t symbol);
void    output_bit_array(const uint8_t bit_array[], int num_bit);
void    output_symbol(uint8_t symbol);
int     flush_data();
int     flush_header();
uint8_t get_bits_to_ignore();

/*
 * Compress file
 */
int adh_compress_file(const char input_file_name[], const char output_file_name[]) {
    log_info("adh_compress_file: %s ...\n", input_file_name);
    buffer_bit_idx = HEADER_BITS;

    FILE * input_file_ptr = bin_open_read(input_file_name);
    if (input_file_ptr == NULL) {
        return RC_FAIL;
    }

    output_file_ptr = bin_open_create(output_file_name);
    if (output_file_ptr == NULL) {
        return RC_FAIL;
    }

    uint8_t buffer[BUFFER_SIZE] = {0};

    int rc = adh_init_tree();
    if (rc == 0) {

        size_t bytesRead = 0;
        // read up to sizeof(buffer) bytes
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), input_file_ptr)) > 0)
        {
            for(int i=0;i<bytesRead;i++)
                process_symbol(buffer[i]);
        }


        adh_destroy_tree();

        if(buffer_bit_idx > 0) {
            // flush remaining data to file
            rc = flush_data();
            if (rc == RC_FAIL) {
                return rc;
            }
        }

        // close and reopen in update mode
        fclose(output_file_ptr);

        output_file_ptr = bin_open_update(output_file_name);
        if (output_file_ptr == NULL) {
            return RC_FAIL;
        }
        rc = flush_header();
    }

    fclose(output_file_ptr);
    fclose(input_file_ptr);

    return rc;
}

/*
 * encode char
 */
void process_symbol(uint8_t symbol) {
    log_trace_char_bin_msg("process_symbol: ", symbol);
    uint8_t bit_array[MAX_CODE_SIZE] = {0};

    adh_node_t* node = adh_search_symbol_in_tree(symbol);

    if(node == NULL) {
        // symbol not present in tree

        // write NYT code
        int num_bit = adh_get_NYT_encoding(bit_array);
        output_bit_array(bit_array, num_bit);

        // write symbol code
        output_symbol(symbol);
        node = adh_create_node_and_append(symbol);
        adh_update_tree(node, true);
    } else {
        // char already present in tree

        // increase weight
        node->weight++;

        // write symbol code
        int num_bit = adh_get_symbol_encoding(symbol, bit_array);
        output_bit_array(bit_array, num_bit);
        adh_update_tree(node, false);
    }

}

/*
 * copy data to output buffer as char
 */
void output_symbol(uint8_t symbol) {
    log_trace_char_bin_msg("output_symbol: ", symbol);

    uint8_t bit_array[SYMBOL_BITS] = { 0 };
    for (int bitPos = SYMBOL_BITS-1; bitPos >= 0; --bitPos) {
        char val = bit_check(symbol, bitPos);
        bit_array[bitPos] = val;
    }
    output_bit_array(bit_array, SYMBOL_BITS);
}


/*
 * copy data to output buffer as bit array
 */
void output_bit_array(const uint8_t bit_array[], int num_bit) {
    log_trace("output_bit_array: %d\n", num_bit);

    for(int i = num_bit-1; i>=0; i--) {

        // calculate the current position (in byte) of the output_buffer
        uint16_t buffer_byte_idx = bit_idx_to_byte_idx(buffer_bit_idx);

        // calculate which bit to change in the byte
        uint16_t bit_to_change = SYMBOL_BITS - 1 - (buffer_bit_idx % SYMBOL_BITS);

        if(bit_array[i] == BIT_1)
            bit_set_one(&output_buffer[buffer_byte_idx], bit_to_change);
        else
            bit_set_zero(&output_buffer[buffer_byte_idx], bit_to_change);

        buffer_bit_idx++;

        // buffer full, flush data to file
        if(buffer_bit_idx == BUFFER_SIZE * SYMBOL_BITS) {
            flush_data();

            // reset buffer index
            buffer_bit_idx = bit_to_change;
        }
    }
}


/*
 * flush data to file
 */
int flush_data() {
    static bool isFirstByte = true;
    log_trace("flush_data: %d bits\n", buffer_bit_idx);

    uint16_t bytesToWrite = bit_idx_to_byte_idx(buffer_bit_idx);

    uint8_t  bitsToIgnore = get_bits_to_ignore();
    if(bitsToIgnore > 0)
        bytesToWrite++;

    for(int i=0; i<bytesToWrite; i++)
        log_trace_char_bin_msg("", output_buffer[i]);

    size_t bytesWritten = fwrite(output_buffer, sizeof(uint8_t), bytesToWrite, output_file_ptr);
    if(bytesWritten != bytesToWrite) {
        perror("failed to write compressed file");
        return RC_FAIL;
    }

    if(isFirstByte) {
        first_byte_written = output_buffer[0];
        isFirstByte = false;
    }
    return RC_OK;
}

uint8_t get_bits_to_ignore() {
    return SYMBOL_BITS - (buffer_bit_idx % SYMBOL_BITS);
}

/*
 * flush header to file
 */
int flush_header() {
    log_trace_char_bin_msg("flush_header ori: ", first_byte_written);

    first_byte_union first_byte;
    first_byte.raw = first_byte_written;
    first_byte.split.header = get_bits_to_ignore();

    log_trace_char_bin_msg("flush_header hdr: ", first_byte.raw);
    fputc(first_byte.raw, output_file_ptr);

    if ( fseek(output_file_ptr, 0L, SEEK_SET) != 0 ) {
        perror("error moving to beginning");
        return RC_FAIL;
    }

    size_t bytesWritten = fwrite(&first_byte.raw, 1, 1, output_file_ptr);
    if(bytesWritten == 0) {
        perror("failed to overwrite first byte");
        return RC_FAIL;
    }

    return RC_OK;
}
