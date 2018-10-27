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
long    get_filesize(FILE *input_file_ptr);
int     read_data_cross_bytes(const byte_t input_buffer[], int num_bits_to_read, byte_t temp_buffer[]);

void decode_first_symbol(const byte_t *input_buffer);

/*
 * decompress file
 */
int adh_decompress_file(const char input_file_name[], const char output_file_name[]) {
    int rc = RC_OK;
    log_info("%-30s %s\n", "adh_decompress_file:", input_file_name);

    FILE * input_file_ptr = bin_open_read(input_file_name);
    if (input_file_ptr == NULL) {
        rc = RC_FAIL;
    }

    if(rc == RC_OK)
        output_file_ptr = bin_open_create(output_file_name);

    if (output_file_ptr == NULL) {
        rc = RC_FAIL;
    }

    if (rc == RC_OK)
        rc = adh_init_tree();

    if (rc == RC_OK) {
        read_header(input_file_ptr);

        bool first_char = true;
        long file_size = get_filesize(input_file_ptr);
        int bytes_to_read = file_size;

        //byte_t input_buffer[BUFFER_SIZE] = { 0 };
        byte_t input_buffer[file_size];
        byte_t node_bit_array[MAX_CODE_SIZE] = { 0 };

        // read up to sizeof(buffer) bytes
        size_t bytes_read = fread(input_buffer, sizeof(byte_t), bytes_to_read, input_file_ptr);
        //while ((bytes_read = fread(input_buffer, sizeof(byte_t), bytes_to_read, input_file_ptr)) > 0)
        {
            if(bytes_read != bytes_to_read)
                fprintf(stderr, "bytes_read (%zu) != bytes_to_read (%d)\n", bytes_read, bytes_to_read);

            // not coded byte
            decode_first_symbol(input_buffer);

            int num_bits = adh_get_NYT_encoding(node_bit_array);
            int num_bytes = bits_to_bytes(num_bits);

            bool have_same_bits = compare_bit_array(input_buffer, input_buffer_bit_idx, node_bit_array, num_bits);
            if(have_same_bits) {
                decode_first_symbol(input_buffer);
            } else {
                //TODO: find node with same code
               ;
            }
        }

        int out_byte_idx = bit_idx_to_byte_idx(output_buffer_bit_idx);
        size_t bytes_written = fwrite(output_buffer, sizeof(byte_t), out_byte_idx, output_file_ptr);
        if(bytes_written != out_byte_idx)
            fprintf(stderr, "bytes_written (%zu) != out_byte_idx (%d)\n", bytes_written, out_byte_idx);

        adh_destroy_tree();
    }

free_res:
    if(output_file_ptr)
        fclose(output_file_ptr);
    if(input_file_ptr)
        fclose(input_file_ptr);

    return rc;
}

void decode_first_symbol(const byte_t *input_buffer) {
    int num_bits_to_read = SYMBOL_BITS;
    byte_t   temp_buffer[BUFFER_SIZE] = {0};
    int num_bytes = read_data_cross_bytes(input_buffer, num_bits_to_read, temp_buffer);
    int output_buffer_byte_idx = bit_idx_to_byte_idx(output_buffer_bit_idx);
    output_buffer[output_buffer_byte_idx] = temp_buffer[0];
    output_buffer_bit_idx +=  SYMBOL_BITS;

    adh_node_t * node = adh_create_node_and_append(output_buffer[output_buffer_byte_idx]);
    adh_update_tree(node, true);
}

int read_data_cross_bytes(const byte_t input_buffer[], int num_bits_to_read, byte_t temp_buffer[]) {
    int temp_buffer_bit_idx = 0;
    int temp_byte_idx = 0;
    while(num_bits_to_read > 0) {
        int available_bits = get_available_bits(input_buffer_bit_idx);
        int input_byte_idx = bit_idx_to_byte_idx(input_buffer_bit_idx);
        temp_byte_idx = bit_idx_to_byte_idx(temp_buffer_bit_idx);
        int read_bit_idx = bit_to_change(input_buffer_bit_idx);
        int write_bit_idx = bit_to_change(temp_buffer_bit_idx);
        int bits_to_copy = available_bits > num_bits_to_read ? num_bits_to_read : available_bits;

        bit_copy(input_buffer[input_byte_idx], &temp_buffer[temp_byte_idx], read_bit_idx, write_bit_idx, bits_to_copy);
        input_buffer_bit_idx += bits_to_copy;
        temp_buffer_bit_idx += bits_to_copy;
        num_bits_to_read -= bits_to_copy;
    }
    return temp_byte_idx + 1;
}

long get_filesize(FILE *input_file_ptr) {
    long file_size;
    fseek(input_file_ptr, 0 , SEEK_END);
    file_size = ftell(input_file_ptr);
    fseek(input_file_ptr, 0 , SEEK_SET);
    return file_size;
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
    input_buffer_bit_idx = HEADER_BITS;

    //output_buffer[0] = (byte_t)(first_byte.split.data << (byte_t)HEADER_BITS);
    //output_buffer_bit_idx = HEADER_DATA_BITS;
 }