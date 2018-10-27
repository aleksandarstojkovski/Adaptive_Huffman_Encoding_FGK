#include <stdio.h>
#include <math.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static FILE *           outputFilePtr;
static unsigned char    output_buffer[BUFFER_SIZE] = {0};
static unsigned short   output_buffer_bit_idx;
static unsigned short   input_buffer_bit_idx;
static unsigned int     bits_to_ignore;

/*
 * Private methods
 */
void    read_header(FILE *inputFilePtr);
bool    compare_bit_array(unsigned char *input_buffer, unsigned char *node_bit_array, int num_bits);
int     get_byte_idx_from_bit_idx(int bit_idx);
int     get_num_bytes_from_bits(int num_bits);

/*
 * decompress file
 */
int adh_decompress_file(const char *input_file, const char *output_file) {
    log_info("adh_decompress_file: %s ...\n", input_file);
    input_buffer_bit_idx = HEADER_BITS;

    FILE * inputFilePtr = bin_open_read(input_file);
    if (inputFilePtr == NULL) {
        return RC_FAIL;
    }

    outputFilePtr = bin_open_create(output_file);
    if (outputFilePtr == NULL) {
        return RC_FAIL;
    }

    int rc = adh_init_tree();
    if (rc == 0) {
        read_header(inputFilePtr);

        bool firstChar = true;
        int byteToRead = 1;

        unsigned char input_buffer[BUFFER_SIZE] = { 0 };
        unsigned char node_bit_array[MAX_CODE_SIZE] = { 0 };

        // read up to sizeof(buffer) bytes
        size_t bytesRead = 0;
        while ((bytesRead = fread(input_buffer, byteToRead, 1, inputFilePtr)) > 0)
        {
            if(bytesRead != byteToRead)
                perror("bytesRead != byteToRead");

            int inputByteIdx = get_byte_idx_from_bit_idx(input_buffer_bit_idx);
            int outputByteIdx = get_byte_idx_from_bit_idx(output_buffer_bit_idx);

            if(firstChar == true) {
                // not coded byte

                bit_copy(&output_buffer[outputByteIdx], input_buffer[inputByteIdx], HEADER_DATA_BITS, 0, HEADER_BITS);
                input_buffer_bit_idx += CHAR_BIT;
                output_buffer_bit_idx += CHAR_BIT;

                firstChar = false;

                adh_node_t * node = adh_create_node_and_append(output_buffer[0]);
                adh_update_tree(node, true);
            } else {
                int num_bits = adh_get_NYT_encoding(node_bit_array);
                int num_bytes = get_num_bytes_from_bits(num_bits);
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

        unsigned int out_byte_idx = get_byte_idx_from_bit_idx(output_buffer_bit_idx);
        size_t bytesWritten = fwrite(output_buffer, out_byte_idx, 1, outputFilePtr);
        if(bytesWritten != out_byte_idx)
            perror("bytesWritten != out_byte_idx");

        adh_destroy_tree();
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
}

bool compare_bit_array(unsigned char *input_buffer, unsigned char *node_bit_array, int num_bits) {
    bool haveSameBits = true;
    for(int bit_idx=0; bit_idx<num_bits; bit_idx++) {
        int byte_idx = get_byte_idx_from_bit_idx(bit_idx);
        unsigned char input_byte = input_buffer[byte_idx];

        unsigned char value = bit_check(input_byte, bit_idx);
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
    unsigned char header;
    fread(&header, 1, 1, inputFilePtr);

    first_byte_union first_byte;
    first_byte.raw = header;

    bits_to_ignore = first_byte.split.header;

    output_buffer[0] = first_byte.split.data << HEADER_BITS;
}

int get_num_bytes_from_bits(int num_bits) { return ceil(1.0 * num_bits / CHAR_BIT); }

int get_byte_idx_from_bit_idx(int bit_idx) { return bit_idx / CHAR_BIT; }
