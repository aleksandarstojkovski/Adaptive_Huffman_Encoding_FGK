#include <stdio.h>
#include <math.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static FILE * outputFilePtr;
static unsigned char output_buffer[BUFFER_SIZE] = {0};
static unsigned short output_buffer_bit_idx;
static unsigned short input_buffer_bit_idx;
static unsigned int bitsToIgnore;

/*
 * Private methods
 */
void readHeader(FILE *inputFilePtr);
bool compareBitArray(unsigned char *input_buffer, unsigned char *node_bit_array, int num_bits);
int getByteIdxFromBitIdx(int bit_idx);
int getNumBytesFromBits(int num_bits);

/*
 * decompress file
 */
int decompressFile(const char *input_file, const char *output_file) {
    trace("decompressFile: %s ...\n", input_file);
    input_buffer_bit_idx = HEADER_BITS;

    FILE * inputFilePtr = openReadBinary(input_file);
    if (inputFilePtr == NULL) {
        return RC_FAIL;
    }

    outputFilePtr = openCreateBinary(output_file);
    if (outputFilePtr == NULL) {
        return RC_FAIL;
    }

    int rc = initializeTree();
    if (rc == 0) {
        readHeader(inputFilePtr);

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

            int inputByteIdx = getByteIdxFromBitIdx(input_buffer_bit_idx);
            int outputByteIdx = getByteIdxFromBitIdx(output_buffer_bit_idx);

            if(firstChar == true) {
                // not coded byte

                bit_copy(&output_buffer[outputByteIdx], input_buffer[inputByteIdx], HEADER_DATA_BITS, 0, HEADER_BITS);
                input_buffer_bit_idx += CHAR_BIT;
                output_buffer_bit_idx += CHAR_BIT;

                firstChar = false;

                Node * node = createNodeAndAppend(output_buffer[0]);
                updateTree(node, true);
            } else {
                int num_bits = getNYTCode(node_bit_array);
                int num_bytes = getNumBytesFromBits(num_bits);
                if(num_bytes > bytesRead) {
                    //TODO read missing bytes
                }

                bool haveSameBits = compareBitArray(input_buffer, node_bit_array, num_bits);
                if(haveSameBits) {
                    //TODO: read new char and append
                    ;
                } else {
                    //TODO: find node with same code
                    ;
                }

                // get the nyt code:  nytCode = getNYTCode();
                // get the length of nytCode: nytCodeLength =  nytCode.length();
                // read as many bits as nytCodeLength from compressed file
                // if (nytCode == bitsReadFromCompressedFile), then
                // next round we need ro read 1 byte
                // else,


            }
        }

        unsigned int output_buffer_byte_idx = getByteIdxFromBitIdx(output_buffer_bit_idx);
        size_t bytesWritten = fwrite(output_buffer, output_buffer_byte_idx, 1, outputFilePtr);
        if(bytesWritten != output_buffer_byte_idx)
            perror("bytesWritten != output_buffer_byte_idx");


        destroyTree();
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
}

bool compareBitArray(unsigned char *input_buffer, unsigned char *node_bit_array, int num_bits) {
    bool haveSameBits = true;
    for(int bit_idx=0; bit_idx<num_bits; bit_idx++) {
        int byte_idx = getByteIdxFromBitIdx(bit_idx);
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
void readHeader(FILE *inputFilePtr) {
    unsigned char header;
    fread(&header, 1, 1, inputFilePtr);

    first_byte_union first_byte;
    first_byte.raw = header;

    bitsToIgnore = first_byte.split.header;

    output_buffer[0] = first_byte.split.data << HEADER_BITS;
}

int getNumBytesFromBits(int num_bits) { return ceil(1.0 * num_bits / CHAR_BIT); }

int getByteIdxFromBitIdx(int bit_idx) { return bit_idx / CHAR_BIT; }
