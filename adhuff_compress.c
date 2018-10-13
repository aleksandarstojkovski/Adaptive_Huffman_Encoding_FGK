#include <stdbool.h>

#include "adhuff_compress.h"
#include "bin_io.h"
#include "node.h"

static const int CHAR_SIZE = 8;
static const char *_compress_outputp_file;
static unsigned char output_buffer[BLOCK_SIZE];
static int buffer_idx = 0;
static FILE * outputFilePtr;

/*
 * Compress file
 */
int compressFile(const char * input_file, const char * output_file) {
    outputFilePtr = openWriteBinary(output_file);
    if (outputFilePtr == NULL) {
        return 1;
    }

    _compress_outputp_file = output_file;
    printf("START compressing: %s ...\n", input_file);

    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(input_file, compressCallback);
        destroyTree();

        if(buffer_idx < BLOCK_SIZE) {
            // flush remaining data to file
            flushData();
        }
    }

    fclose(outputFilePtr);

    return rc;
}

/*
 * Compress Callback
 */
void compressCallback(char ch) {
    Node* node = searchCharInTree(_root, ch);
    if(node == NULL) {
        // Node not present in tree
        writeOutput(ch, CHAR_SIZE);
        addNewNode(ch);
    } else {
        // TODO
        node->weight++;
        encode(ch);
        // updateTree
    }
}

void encode(char ch) {

}

void writeOutput(char ch, int numBit) {

    if(numBit == CHAR_SIZE) {
        output_buffer[buffer_idx] = ch;
        buffer_idx++;
    } else {
        // TODO MAX
        // handle bit output
    }

    if(buffer_idx == BLOCK_SIZE) {
        // buffer full, flush data to file
        flushData();
    }
}

void flushData() {
    size_t bytesWritten = fwrite(output_buffer, buffer_idx, 1, outputFilePtr);
    buffer_idx = 0;
}