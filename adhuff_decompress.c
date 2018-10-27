#include <stdio.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static FILE *           output_file_ptr;
static byte_t           output_buffer[BUFFER_SIZE] = {0};
static unsigned short   output_buffer_bit_idx;
static unsigned short   input_buffer_bit_idx;
static unsigned int     bits_to_ignore;

/*
 * Private methods
 */
void    read_header(FILE *inputFilePtr);

/*
 * decompress file
 */
int adh_decompress_file(const char input_file_name[], const char output_file_name[]) {
    log_info("%-30s %s\n", "adh_decompress_file:", input_file_name);
    input_buffer_bit_idx = HEADER_BITS;

    FILE * input_file_ptr = bin_open_read(input_file_name);
    if (input_file_ptr == NULL) {
        return RC_FAIL;
    }

    output_file_ptr = bin_open_create(output_file_name);
    if (output_file_ptr == NULL) {
        return RC_FAIL;
    }

    int rc = adh_init_tree();
    if (rc == 0) {
        read_header(input_file_ptr);

        bool firstChar = true;
        int bytes_to_read = 1;

        byte_t input_buffer[BUFFER_SIZE] = { 0 };
        byte_t node_bit_array[MAX_CODE_SIZE] = { 0 };

        // read up to sizeof(buffer) bytes
        size_t bytes_read = 0;
        while ((bytes_read = fread(input_buffer, bytes_to_read, 1, input_file_ptr)) > 0)
        {
            if(bytes_read != bytes_to_read)
                perror("bytes_read != bytes_to_read");

            int input_byte_idx = bit_idx_to_byte_idx(input_buffer_bit_idx);
            int output_byte_idx = bit_idx_to_byte_idx(output_buffer_bit_idx);

            if(firstChar == true) {
                // not coded byte
                bit_copy(input_buffer[input_byte_idx], &output_buffer[output_byte_idx], HEADER_DATA_BITS, 0, HEADER_BITS);
                input_buffer_bit_idx += SYMBOL_BITS;
                output_buffer_bit_idx += SYMBOL_BITS;

                firstChar = false;

                adh_node_t * node = adh_create_node_and_append(output_buffer[0]);
                adh_update_tree(node, true);
            } else {
                int num_bits = adh_get_NYT_encoding(node_bit_array);
                int num_bytes = bits_to_bytes(num_bits);
                if(num_bytes > bytes_read) {
                    //TODO read missing bytes
                }

                bool haveSameBits = compare_bit_array(input_buffer, node_bit_array, num_bits);
                if(haveSameBits) {
                    //TODO: read new char and append
                    ;
                } else {
                    //TODO: find node with same code
                    ;
                }
            }
        }

        int out_byte_idx = bit_idx_to_byte_idx(output_buffer_bit_idx);
        size_t bytes_written = fwrite(output_buffer, sizeof(byte_t), out_byte_idx, output_file_ptr);
        if(bytes_written != out_byte_idx)
            fprintf(stderr, "bytes_written (%d) != out_byte_idx (%d)\n", bytes_written, out_byte_idx);

        adh_destroy_tree();
    }

    fclose(output_file_ptr);
    fclose(input_file_ptr);

    return rc;
}

/*
 * read header
 */
void read_header(FILE *inputFilePtr) {
    byte_t header;
    fread(&header, sizeof(byte_t), 1, inputFilePtr);

    first_byte_union first_byte;
    first_byte.raw = header;

    bits_to_ignore = first_byte.split.header;

    output_buffer[0] = (byte_t)(first_byte.split.data << (byte_t)HEADER_BITS);
}