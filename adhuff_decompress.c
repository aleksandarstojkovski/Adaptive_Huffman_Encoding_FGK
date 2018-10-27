#include <stdio.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static FILE *           output_file_ptr;
static uint8_t          output_buffer[BUFFER_SIZE] = {0};
static unsigned short   output_buffer_bit_idx;
static unsigned short   input_buffer_bit_idx;
static unsigned int     bits_to_ignore;

/*
 * Private methods
 */
void    read_header(FILE *inputFilePtr);
bool    compare_bit_array(uint8_t input_buffer[], uint8_t node_bit_array[], int num_bits);

/*
 * decompress file
 */
int adh_decompress_file(const char input_file_name[], const char output_file_name[]) {
    log_info("adh_decompress_file: %s ...\n", input_file_name);
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
        int byteToRead = 1;

        uint8_t input_buffer[BUFFER_SIZE] = { 0 };
        uint8_t node_bit_array[MAX_CODE_SIZE] = { 0 };

        // read up to sizeof(buffer) bytes
        size_t bytesRead = 0;
        while ((bytesRead = fread(input_buffer, byteToRead, 1, input_file_ptr)) > 0)
        {
            if(bytesRead != byteToRead)
                perror("bytesRead != byteToRead");

            uint16_t input_byte_idx = bit_idx_to_byte_idx(input_buffer_bit_idx);
            uint16_t output_byte_idx = bit_idx_to_byte_idx(output_buffer_bit_idx);

            if(firstChar == true) {
                // not coded byte

                bit_copy(&output_buffer[output_byte_idx], input_buffer[input_byte_idx], HEADER_DATA_BITS, 0, HEADER_BITS);
                input_buffer_bit_idx += SYMBOL_BITS;
                output_buffer_bit_idx += SYMBOL_BITS;

                firstChar = false;

                adh_node_t * node = adh_create_node_and_append(output_buffer[0]);
                adh_update_tree(node, true);
            } else {
                int num_bits = adh_get_NYT_encoding(node_bit_array);
                int num_bytes = bits_to_bytes(num_bits);
                if(num_bytes > bytesRead) {
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

        unsigned int out_byte_idx = bit_idx_to_byte_idx(output_buffer_bit_idx);
        size_t bytesWritten = fwrite(output_buffer, out_byte_idx, 1, output_file_ptr);
        if(bytesWritten != out_byte_idx)
            perror("bytesWritten != out_byte_idx");

        adh_destroy_tree();
    }

    fclose(output_file_ptr);
    fclose(input_file_ptr);

    return rc;
}

bool compare_bit_array(uint8_t input_buffer[], uint8_t node_bit_array[], int num_bits) {
    bool haveSameBits = true;
    for(int bit_idx=0; bit_idx<num_bits; bit_idx++) {
        int byte_idx = bit_idx_to_byte_idx(bit_idx);
        uint8_t input_byte = input_buffer[byte_idx];

        uint8_t value = bit_check(input_byte, bit_idx);
        if(value != node_bit_array[bit_idx]) {
            haveSameBits = false;
        }
    }
    return haveSameBits;
}

/*
 * read header
 */
void read_header(FILE *inputFilePtr) {
    uint8_t header;
    fread(&header, sizeof(uint8_t), 1, inputFilePtr);

    first_byte_union first_byte;
    first_byte.raw = header;

    bits_to_ignore = first_byte.split.header;

    output_buffer[0] = first_byte.split.data << HEADER_BITS;
}