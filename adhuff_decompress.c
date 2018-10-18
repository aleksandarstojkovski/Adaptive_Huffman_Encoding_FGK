#include <stdio.h>

#include "adhuff_decompress.h"
#include "adhuff_common.h"
#include "constants.h"
#include "bin_io.h"

/*
 * modules variables
 */
static FILE * outputFilePtr;
static unsigned char output_buffer[BUFFER_SIZE];
static unsigned short buffer_bit_idx;
static unsigned int oddBits;

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
    oddBits = 0;

    FILE * inputFilePtr = openReadBinary(input_file);
    if (inputFilePtr == NULL) {
        return RC_FAIL;
    }

    outputFilePtr = openWriteBinary(output_file);
    if (outputFilePtr == NULL) {
        return RC_FAIL;
    }

    int rc = initializeTree();
    if (rc == 0) {
        size_t bytesRead = 0;

        readHeader(inputFilePtr);

        bool firstChar = true;
        int byteToRead = 1;

        unsigned char buffer[BUFFER_SIZE];

        // read up to sizeof(buffer) bytes
        while ((bytesRead = fread(buffer, byteToRead, 1, inputFilePtr)) > 0)
        {
            if(firstChar == true) {
                // not coded byte
                bit_copy(&output_buffer[0], buffer[0], HEADER_BITS, 0, HEADER_DATA_BITS);
                buffer_bit_idx = CHAR_BIT;
                firstChar = false;
            } else {
                // TODO handle next bytes
            }
        }

        unsigned int buffer_byte_idx = buffer_bit_idx / CHAR_BIT;
        size_t bytesWritten = fwrite(output_buffer, buffer_byte_idx, 1, outputFilePtr);


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
    unsigned char buffer[1];
    fread(buffer, 1, 1, outputFilePtr);

    first_byte_union first_byte;
    first_byte.raw = buffer[0];

    oddBits = first_byte.split.header;
    output_buffer[0] = first_byte.split.data;
}
