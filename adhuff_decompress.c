#include <stdio.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static byte_t           output_buffer[BUFFER_SIZE] = {0};
static unsigned short   output_buffer_byte_idx;
static unsigned short   input_buffer_bit_idx;
static unsigned int     bits_to_ignore;
static long             input_size;

/*
 * Private methods
 */
void    read_header(FILE *inputFilePtr);
long    get_filesize(FILE *input_file_ptr);
int     read_data_cross_bytes(const byte_t input_buffer[], int num_bits_to_read, byte_t sub_buffer[]);
void    decode_new_symbol(const byte_t input_buffer[]);
void    decode_existing_symbol(const byte_t input_buffer[]);
void    flush_uncompressed(FILE *output_file_ptr);

/*
 * decompress file
 */
int adh_decompress_file(const char input_file_name[], const char output_file_name[]) {
    int rc = RC_OK;
    log_info("%-30s %s\t%s\n", "adh_decompress_file:", input_file_name, output_file_name);

    FILE * output_file_ptr = NULL;
    FILE * input_file_ptr = bin_open_read(input_file_name);
    if (input_file_ptr == NULL) {
        rc = RC_FAIL;
    }

    if(rc == RC_OK) {
        output_file_ptr = bin_open_create(output_file_name);
        if (output_file_ptr == NULL) {
            rc = RC_FAIL;
        }
    }

    if (rc == RC_OK)
        rc = adh_init_tree();

    if (rc == RC_OK) {
        read_header(input_file_ptr);

        // TODO: handle big files, don't read entire file in memory
        input_size = get_filesize(input_file_ptr);
        int bytes_to_read = input_size;

        //byte_t input_buffer[BUFFER_SIZE] = { 0 };
        byte_t input_buffer[input_size];
        byte_t node_bit_array[MAX_CODE_BITS] = { 0 };

        // read up to sizeof(buffer) bytes
        size_t bytes_read = fread(input_buffer, sizeof(byte_t), bytes_to_read, input_file_ptr);
        //while ((bytes_read = fread(input_buffer, sizeof(byte_t), bytes_to_read, input_file_ptr)) > 0)
        {
            if(bytes_read != bytes_to_read) {
                fprintf(stderr, "bytes_read (%zu) != bytes_to_read (%d)\n", bytes_read, bytes_to_read);
                exit(RC_FAIL);
            }

            while(input_size != (input_buffer_bit_idx + bits_to_ignore) / SYMBOL_BITS) {
                int num_bits = adh_get_NYT_encoding(node_bit_array);
                bool is_nyt_code = compare_input_and_bit_array(input_buffer, input_buffer_bit_idx, node_bit_array,
                                                               num_bits);
                if(is_nyt_code) {
                    input_buffer_bit_idx += num_bits;
                    // not coded byte
                    decode_new_symbol(input_buffer);
                } else {
                    decode_existing_symbol(input_buffer);
                }

                if(output_buffer_byte_idx == BUFFER_SIZE -1)
                    flush_uncompressed(output_file_ptr);
            }
        }

        flush_uncompressed(output_file_ptr);

        adh_destroy_tree();
    }

    if(output_file_ptr) {
        fclose(output_file_ptr);
    }

    if(input_file_ptr) {
        fclose(input_file_ptr);
    }

    return rc;
}

void flush_uncompressed(FILE *output_file_ptr) {
    log_trace("%-40s input_buffer_bit_idx=%d output_buffer_byte_idx=%d\n", "flush_uncompressed:", input_buffer_bit_idx, output_buffer_byte_idx);

    size_t bytes_written = fwrite(output_buffer, sizeof(byte_t), output_buffer_byte_idx, output_file_ptr);
    if(bytes_written != output_buffer_byte_idx) {
        fprintf(stderr, "bytes_written (%zu) != out_byte_idx (%d)\n", bytes_written, output_buffer_byte_idx);
        exit(RC_FAIL);
    }

    output_buffer_byte_idx = 0;
}

void decode_existing_symbol(const byte_t input_buffer[]) {
    log_trace("%-40s input_buffer_bit_idx=%d\n", "decode_existing_symbol:", input_buffer_bit_idx);

    int original_input_buffer_bit_idx = input_buffer_bit_idx;

    adh_node_t* node = NULL;
    int     bit_array_size = 0;
    byte_t  bit_array[MAX_CODE_BITS] = {0};
    byte_t  sub_buffer[MAX_CODE_BYTES] = {0};
    int     num_bytes = read_data_cross_bytes(input_buffer, MAX_CODE_BITS, sub_buffer);

    for (int byte_idx = 0; byte_idx < num_bytes && node == NULL; ++byte_idx) {
        for (int bit_idx = 0; bit_idx < SYMBOL_BITS && node == NULL; ++bit_idx) {
            int bit_array_idx = (byte_idx * SYMBOL_BITS) + bit_idx;
            if(bit_array_idx >= MAX_CODE_BITS) {
                fprintf(stderr, "bit_array_idx (%d) >= MAX_CODE_BITS (%d)", bit_array_idx, MAX_CODE_BITS);
                exit(RC_FAIL);
            }

            bit_array[bit_array_idx] = bit_check(sub_buffer[byte_idx], SYMBOL_BITS - bit_idx -1);
            bit_array_size++;
            node = adh_search_encoding_in_tree(bit_array, bit_array_size);
        }
    }

    if(node == NULL) {
        fprintf(stderr, "decode_existing_symbol: cannot find node");
        exit(RC_FAIL);
    }

    input_buffer_bit_idx = original_input_buffer_bit_idx + bit_array_size;
    byte_t symbol = node->symbol;

    log_trace("%-40s symbol=%-3d\n", "decode_existing_symbol:", symbol);

    output_buffer[output_buffer_byte_idx] = symbol;
    output_buffer_byte_idx++;
    adh_update_tree(node, false);
}

void decode_new_symbol(const byte_t input_buffer[]) {
    log_trace("%-40s input_buffer_bit_idx=%d\n", "decode_new_symbol:", input_buffer_bit_idx);

    byte_t  new_symbol[1] = {0};
    int     num_bytes = read_data_cross_bytes(input_buffer, SYMBOL_BITS, new_symbol);
    if(num_bytes > 1) {
        fprintf(stderr, "decode_new_symbol expected 1 byte received %d", num_bytes);
        exit(RC_FAIL);
    }

    output_buffer[output_buffer_byte_idx] = new_symbol[0];
    adh_node_t * node = adh_create_node_and_append(new_symbol[0]);
    adh_update_tree(node, true);

    output_buffer_byte_idx++;
}

int read_data_cross_bytes(const byte_t input_buffer[], int num_bits_to_read, byte_t sub_buffer[]) {
    log_trace("%-40s num_bits_to_read=%3d\n", "read_data_cross_bytes:", num_bits_to_read);

    int temp_buffer_bit_idx = 0;
    int temp_byte_idx = 0;
    while(num_bits_to_read > 0) {
        int input_byte_idx = bit_idx_to_byte_idx(input_buffer_bit_idx);
        if(input_byte_idx > input_size-1)
            break;

        int available_bits = get_available_bits(input_buffer_bit_idx);
        int bits_to_copy = available_bits > num_bits_to_read ? num_bits_to_read : available_bits;

        int read_bit_idx = bit_to_change(input_buffer_bit_idx);
        int write_bit_idx = bit_to_change(temp_buffer_bit_idx);

        temp_byte_idx = bit_idx_to_byte_idx(temp_buffer_bit_idx);

        // copy bits from most significant bit to least significant
        // e.g. from 5 and size 4 -> 5,4,3,2
        bit_copy(input_buffer[input_byte_idx], &sub_buffer[temp_byte_idx], read_bit_idx, write_bit_idx, bits_to_copy);

        input_buffer_bit_idx += bits_to_copy;
        temp_buffer_bit_idx += bits_to_copy;
        num_bits_to_read -= bits_to_copy;
    }
    return temp_byte_idx + 1;
}

long get_filesize(FILE *input_file_ptr) {
    fseek(input_file_ptr, 0 , SEEK_END);
    long file_size = ftell(input_file_ptr);
    fseek(input_file_ptr, 0 , SEEK_SET);
    return file_size;
}

/*
 * read header
 */
void read_header(FILE *inputFilePtr) {
    log_trace("read_header\n");

    byte_t header;
    fread(&header, sizeof(byte_t), 1, inputFilePtr);

    first_byte_union first_byte;
    first_byte.raw = header;

    bits_to_ignore = first_byte.split.header;
    input_buffer_bit_idx = HEADER_BITS;
    output_buffer_byte_idx = 0;
 }