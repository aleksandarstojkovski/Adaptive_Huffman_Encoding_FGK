#include "adhuff_compress.h"
#include "constants.h"
#include "bin_io.h"
#include "node.h"

/*
 * modules variables
 */
static unsigned char output_buffer[BLOCK_SIZE];
static unsigned short buffer_byte_idx = 0;
static unsigned short buffer_bit_idx = 0;

static FILE * outputFilePtr;
static FILE * inputFilePtr;

/*
 * Compress file
 */
int compressFile(const char * input_file, const char * output_file) {
    trace("compressFile: %s ...\n", input_file);

    inputFilePtr = openReadBinary(input_file);
    if (inputFilePtr == NULL) {
        return 1;
    }

    outputFilePtr = openWriteBinary(output_file);
    if (outputFilePtr == NULL) {
        return 1;
    }

    unsigned char buffer[BLOCK_SIZE];
    int rc = initializeTree();
    if (rc == 0) {

        size_t bytesRead = 0;
        // read up to sizeof(buffer) bytes
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), inputFilePtr)) > 0)
        {
            for(int i=0;i<bytesRead;i++)
                processChar(buffer[i]);
        }


        destroyTree();

        if(buffer_byte_idx < BLOCK_SIZE) {
            // flush remaining data to file
            flushData();
        }
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
}

/*
 * Process char
 */
void processChar(unsigned char ch) {
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
    } else {
        // char already present in tree

        // increase weight
        node->weight++;

        // write symbol code
        int num_bit = getSymbolCode(ch, bit_array);
        outputBitArray(bit_array, num_bit);
    }
    updateTree(node);
}

/*
 * copy data to output buffer as char
 */
void outputChar(unsigned char ch) {
    traceCharBinMsg("outputChar: ", ch);

    unsigned char bit_array[CHAR_SIZE];
    for (int bitPos = CHAR_SIZE-1; bitPos >= 0; --bitPos) {
        char val = bit_check(ch, bitPos);
        bit_array[bitPos] = val;
    }
    outputBitArray(bit_array, CHAR_SIZE);
}


/*
 * copy data to output buffer as bit array
 */
void outputBitArray(unsigned char bit_array[], int num_bit) {
    trace("outputBitArray: %d\n", num_bit);

    for(int i = 0; i<num_bit; i++) {

        buffer_byte_idx = buffer_bit_idx / CHAR_SIZE;

        int bit_pos = buffer_bit_idx % CHAR_SIZE;
        if(bit_array[i] == BIT_1)
            bit_set_one(&output_buffer[buffer_byte_idx], bit_pos);
        else
            bit_set_zero(&output_buffer[buffer_byte_idx], bit_pos);

        buffer_bit_idx++;

        // buffer full, flush data to file
        if(buffer_byte_idx == BLOCK_SIZE) {
            flushData();
        }
    }
}


/*
 * flush data to file
 */
void flushData() {
    int bit_pos = buffer_bit_idx % CHAR_SIZE;
    if(bit_pos > 1)
        buffer_byte_idx++;

    trace("flushData: %d byte\n", buffer_byte_idx);

    // TODO check error code
    size_t bytesWritten = fwrite(output_buffer, buffer_byte_idx, 1, outputFilePtr);

    // reset buffer indexes
    buffer_byte_idx = 0;
    buffer_bit_idx = 0;
}