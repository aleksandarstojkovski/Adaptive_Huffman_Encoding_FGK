#include "adhuff_compress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

//
// modules variables
//
static unsigned char output_buffer[BUFFER_SIZE] = {0};
static unsigned short buffer_bit_idx;

static FILE * outputFilePtr;
static char firstByteWritten;

//
// private methods
//
void encodeChar(unsigned char ch);
void outputBitArray(const unsigned char bit_array[], int num_bit);
void outputChar(unsigned char ch);
int flushData();
int flushHeader();
int getBitsToIgnore();

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

    unsigned char buffer[BUFFER_SIZE] = {0};

    int rc = adh_init_tree();
    if (rc == 0) {

        size_t bytesRead = 0;
        // read up to sizeof(buffer) bytes
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFilePtr)) > 0)
        {
            for(int i=0;i<bytesRead;i++)
                encodeChar(buffer[i]);
        }


        adh_destroy_tree();

        if(buffer_bit_idx > 0) {
            // flush remaining data to file
            rc = flushData();
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
        rc = flushHeader();
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
}

/*
 * encode char
 */
void encodeChar(unsigned char ch) {
    log_trace_char_bin_msg("processChar: ", ch);
    unsigned char bit_array[MAX_CODE_SIZE] = {0};

    adh_node_t* node = adh_search_symbol_in_tree(ch);

    if(node == NULL) {
        // symbol not present in tree

        // write NYT code
        int num_bit = adh_get_NYT_encoding(bit_array);
        outputBitArray(bit_array, num_bit);

        // write symbol code
        outputChar(ch);
        node = adh_create_node_and_append(ch);
        adh_update_tree(node, true);
    } else {
        // char already present in tree

        // increase weight
        node->weight++;

        // write symbol code
        int num_bit = adh_get_symbol_encoding(ch, bit_array);
        outputBitArray(bit_array, num_bit);
        adh_update_tree(node, false);
    }

}

/*
 * copy data to output buffer as char
 */
void outputChar(unsigned char ch) {
    log_trace_char_bin_msg("outputChar: ", ch);

    unsigned char bit_array[CHAR_BIT] = { 0 };
    for (int bitPos = CHAR_BIT-1; bitPos >= 0; --bitPos) {
        char val = bit_check(ch, bitPos);
        bit_array[bitPos] = val;
    }
    outputBitArray(bit_array, CHAR_BIT);
}


/*
 * copy data to output buffer as bit array
 */
void outputBitArray(const unsigned char bit_array[], int num_bit) {
    log_trace("outputBitArray: %d\n", num_bit);

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
            flushData();

            // reset buffer index
            buffer_bit_idx = bit_to_change;
        }
    }
}


/*
 * flush data to file
 */
int flushData() {
    static bool isFirstByte = true;
    log_trace("flushData: %d bits\n", buffer_bit_idx);

    unsigned int bytesToWrite = buffer_bit_idx / CHAR_BIT;

    unsigned short bitsToIgnore = getBitsToIgnore();
    if(bitsToIgnore > 0)
        bytesToWrite++;

    for(int i=0; i<bytesToWrite; i++)
        log_trace_char_bin_msg("", output_buffer[i]);

    size_t bytesWritten = fwrite(output_buffer, sizeof(unsigned char), bytesToWrite, outputFilePtr);
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

int getBitsToIgnore() { return CHAR_BIT - (buffer_bit_idx % CHAR_BIT); }

/*
 * flush header to file
 */
int flushHeader() {
    log_trace_char_bin_msg("flushHeader ori: ", firstByteWritten);

    first_byte_union first_byte;
    first_byte.raw = firstByteWritten;
    first_byte.split.header = getBitsToIgnore();

    log_trace_char_bin_msg("flushHeader hdr: ", first_byte.raw);
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
