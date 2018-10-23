#include <stdio.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static FILE * outputFilePtr;
static unsigned char output_buffer[BUFFER_SIZE] = {0};
static unsigned short buffer_bit_idx;
static unsigned int bitsToIgnore;

/*
 * Private methods
 */
void readHeader(FILE *inputFilePtr);

/*
 * decompress file
 */
int decompressFile(const char *input_file, const char *output_file) {
    trace("decompressFile: %s ...\n", input_file);
    buffer_bit_idx = HEADER_BITS;

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
        unsigned char bit_array[MAX_CODE_SIZE] = { 0 };

        // read up to sizeof(buffer) bytes
        size_t bytesRead = 0;
        while ((bytesRead = fread(input_buffer, byteToRead, 1, inputFilePtr)) > 0)
        {
            if(bytesRead != byteToRead)
                perror("bytesRead != byteToRead");

            if(firstChar == true) {
                // not coded byte
                bit_copy(&output_buffer[0], input_buffer[0], HEADER_DATA_BITS, 0, HEADER_BITS);
                buffer_bit_idx = CHAR_BIT;
                firstChar = false;

                Node * node = createNodeAndAppend(output_buffer[0]);
                updateTree(node, true);
            } else {
                // TODO handle next bytes

                int num_bit = getNYTCode(bit_array);
                // get the nyt code:  nytCode = getNYTCode();
                // get the length of nytCode: nytCodeLength =  nytCode.length();
                // read as many bits as nytCodeLength from compressed file
                // if (nytCode == bitsReadFromCompressedFile), then
                // next round we need ro read 1 byte
                // else,


            }
        }

        unsigned int buffer_byte_idx = buffer_bit_idx / CHAR_BIT;
        size_t bytesWritten = fwrite(output_buffer, buffer_byte_idx, 1, outputFilePtr);
        if(bytesWritten != buffer_byte_idx)
            perror("bytesWritten != buffer_byte_idx");


        destroyTree();
    }

    fclose(outputFilePtr);
    fclose(inputFilePtr);

    return rc;
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

    //bit_copy(&output_buffer[0], first_byte.split.data, 0, HEADER_BITS, HEADER_DATA_BITS);
}
