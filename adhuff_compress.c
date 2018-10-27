#include "adhuff_compress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

//
// modules variables
//
static uint8_t output_buffer[BUFFER_SIZE] = {0};
static unsigned short buffer_bit_idx;

static FILE * outputFilePtr;
static char firstByteWritten;

//
// private methods
//
void    encode_symbol(uint8_t ch);
void    output_bit_array(const uint8_t bit_array[], int num_bit);
void    output_symbol(uint8_t ch);
int     flush_data();
int     flush_header();
int     get_bits_to_ignore();

/*
 * Compress file
 */
int adh_compress_file(const char *input_file, const char *output_file) {
    log_info("adh_compress_file: %s ...\n", input_file);
    buffer_bit_idx = HEADER_BITS;

    FILE * inputFilePtr = bin_open_read(input_file);
    if (inputFilePtr == NULL) {
        return RC_FAIL;
    }

    outputFilePtr = bin_open_create(output_file);
    if (outputFilePtr == NULL) {
        return RC_FAIL;
    }

    uint8_t buffer[BUFFER_SIZE] = {0};

    int rc = adh_init_tree();
    if (rc == 0) {

        size_t bytesRead = 0;
        // read up to sizeof(buffer) bytes
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFilePtr)) > 0)
        {
            for(int i=0;i<bytesRead;i++)
                encode_symbol(buffer[i]);
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
        fclose(outputFilePtr);

        outputFilePtr = bin_open_update(output_file);
        if (outputFilePtr == NULL) {
            return RC_FAIL;
        }
        rc = flush_header();
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
}

/*
 * encode char
 */
void encode_symbol(uint8_t ch) {
    log_trace_char_bin_msg("encode_symbol: ", ch);
    uint8_t bit_array[MAX_CODE_SIZE] = {0};

    adh_node_t* node = adh_search_symbol_in_tree(ch);

    if(node == NULL) {
        // symbol not present in tree

        // write NYT code
        int num_bit = adh_get_NYT_encoding(bit_array);
        output_bit_array(bit_array, num_bit);

        // write symbol code
        output_symbol(ch);
        node = adh_create_node_and_append(ch);
        adh_update_tree(node, true);
    } else {
        // char already present in tree

        // increase weight
        node->weight++;

        // write symbol code
        int num_bit = adh_get_symbol_encoding(ch, bit_array);
        output_bit_array(bit_array, num_bit);
        adh_update_tree(node, false);
    }

}

/*
 * copy data to output buffer as char
 */
void output_symbol(uint8_t ch) {
    log_trace_char_bin_msg("output_symbol: ", ch);

    uint8_t bit_array[CHAR_BIT] = { 0 };
    for (int bitPos = CHAR_BIT-1; bitPos >= 0; --bitPos) {
        char val = bit_check(ch, bitPos);
        bit_array[bitPos] = val;
    }
    output_bit_array(bit_array, CHAR_BIT);
}


/*
 * copy data to output buffer as bit array
 */
void output_bit_array(const uint8_t *bit_array, int num_bit) {
    log_trace("output_bit_array: %d\n", num_bit);

    for(int i = num_bit-1; i>=0; i--) {

        // calculate the current position (in byte) of the output_buffer
        unsigned int buffer_byte_idx = buffer_bit_idx / CHAR_BIT;

        // calculate which bit to change in the byte
        unsigned int bit_to_change = CHAR_BIT - 1 - (buffer_bit_idx % CHAR_BIT);

        if(bit_array[i] == BIT_1)
            bit_set_one(&output_buffer[buffer_byte_idx], bit_to_change);
        else
            bit_set_zero(&output_buffer[buffer_byte_idx], bit_to_change);

        buffer_bit_idx++;

        // buffer full, flush data to file
        if(buffer_bit_idx == BUFFER_SIZE * CHAR_BIT) {
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

    unsigned int bytesToWrite = buffer_bit_idx / CHAR_BIT;

    unsigned short bitsToIgnore = get_bits_to_ignore();
    if(bitsToIgnore > 0)
        bytesToWrite++;

    for(int i=0; i<bytesToWrite; i++)
        log_trace_char_bin_msg("", output_buffer[i]);

    size_t bytesWritten = fwrite(output_buffer, sizeof(uint8_t), bytesToWrite, outputFilePtr);
    if(bytesWritten != bytesToWrite) {
        perror("failed to write compressed file");
        return RC_FAIL;
    }

    if(isFirstByte) {
        firstByteWritten = output_buffer[0];
        isFirstByte = false;
    }
    return RC_OK;
}

int get_bits_to_ignore() { return CHAR_BIT - (buffer_bit_idx % CHAR_BIT); }

/*
 * flush header to file
 */
int flush_header() {
    log_trace_char_bin_msg("flush_header ori: ", firstByteWritten);

    first_byte_union first_byte;
    first_byte.raw = firstByteWritten;
    first_byte.split.header = get_bits_to_ignore();

    log_trace_char_bin_msg("flush_header hdr: ", first_byte.raw);
    fputc(first_byte.raw, outputFilePtr);

    if ( fseek(outputFilePtr, 0L, SEEK_SET) != 0 ) {
        perror("error moving to beginning");
        return RC_FAIL;
    }

    size_t bytesWritten = fwrite(&first_byte.raw, 1, 1, outputFilePtr);
    if(bytesWritten == 0) {
        perror("failed to overwrite first byte");
        return RC_FAIL;
    }

    return RC_OK;
}
