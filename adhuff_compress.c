#include "adhuff_compress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

//
// modules variables
//
static unsigned char output_buffer[BUFFER_SIZE];
static unsigned short buffer_bit_idx;

static FILE * outputFilePtr;

//
// private methods
//
void encodeChar(unsigned char ch);
void outputBitArray(const unsigned char bit_array[], int num_bit);
void outputChar(unsigned char ch);
void flushData();
void flushHeader();

/*
 * Compress file
 */
int compressFile(const char * input_file, const char * output_file) {
    trace("compressFile: %s ...\n", input_file);
    buffer_bit_idx = HEADER_BITS;

    FILE * inputFilePtr = openReadBinary(input_file);
    if (inputFilePtr == NULL) {
        return RC_FAIL;
    }

    outputFilePtr = openWriteBinary(output_file);
    if (outputFilePtr == NULL) {
        return RC_FAIL;
    }

    unsigned char buffer[BUFFER_SIZE];

    int rc = initializeTree();
    if (rc == 0) {

        size_t bytesRead = 0;
        // read up to sizeof(buffer) bytes
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFilePtr)) > 0)
        {
            for(int i=0;i<bytesRead;i++)
                encodeChar(buffer[i]);
        }


        destroyTree();

        if(buffer_bit_idx > 0) {
            // flush remaining data to file
            flushData();
        }

        flushHeader();
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
}

/*
 * encode char
 */
void encodeChar(unsigned char ch) {
    traceCharBinMsg("processChar: ", ch);
    unsigned char bit_array[MAX_CODE_SIZE];

    Node* node = searchCharInTree(ch);

    if(node == NULL) {
        // symbol not present in tree

        // write NYT code
        int num_bit = getNYTCode(bit_array);
        outputBitArray(bit_array, num_bit);

        // write symbol code
        outputChar(ch);
        node = createNodeAndAppend(ch);
        updateTree(node, true);
    } else {
        // char already present in tree

        // increase weight
        node->weight++;

        // write symbol code
        int num_bit = getSymbolCode(ch, bit_array);
        outputBitArray(bit_array, num_bit);
        updateTree(node, false);
    }

}

/*
 * copy data to output buffer as char
 */
void outputChar(unsigned char ch) {
    traceCharBinMsg("outputChar: ", ch);

    unsigned char bit_array[CHAR_BIT+1];
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
    trace("outputBitArray: %d\n", num_bit);

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
void flushData() {
    trace("flushData: %d bits\n", buffer_bit_idx);

    unsigned int buffer_byte_idx = buffer_bit_idx / CHAR_BIT;
    unsigned short spare_bits = buffer_bit_idx % CHAR_BIT;
    if(spare_bits > 1)
        buffer_byte_idx++;


    for(int i=0; i<buffer_byte_idx; i++)
        traceCharBinMsg("flushData: ", output_buffer[i]);

    // TODO check error code
    fwrite(output_buffer, buffer_byte_idx, 1, outputFilePtr);
    fflush(outputFilePtr);
}

/*
 * flush header to file
 */
void flushHeader() {
    if ( fseek(outputFilePtr, 0L, SEEK_SET) != 0 ) {
        perror("error moving to beginning");
    }

    //rewind(outputFilePtr);

    unsigned char buffer[2];
    size_t bytesRead = fread(buffer, 1, 1, outputFilePtr);
    if(bytesRead == 0)
        perror("failed to read first byte");

    traceCharBinMsg("flushHeader ori: ", buffer[0]);

    first_byte_union first_byte;
    first_byte.raw = buffer[0];
    first_byte.split.header = buffer_bit_idx % CHAR_BIT;

    traceCharBinMsg("flushHeader hdr: ", first_byte.raw);
    //fwrite(&first_byte.raw, 1, 1, outputFilePtr);

    buffer[0] = first_byte.raw;
    traceCharBinMsg("flushHeader hdr: ", buffer[0]);

    fwrite(buffer, 1, 1, outputFilePtr);
    fflush(outputFilePtr);
}
