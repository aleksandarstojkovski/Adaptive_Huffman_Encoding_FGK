#include "adhuff_compress.h"
#include "bin_io.h"
#include "node.h"

static const int CHAR_SIZE = 8;
static unsigned char output_buffer[BLOCK_SIZE];
static unsigned short buffer_idx = 0;
static unsigned short bit_idx = 0;

static FILE * outputFilePtr;

/*
 * Compress file
 */
int compressFile(const char * input_file, const char * output_file) {
    trace("compressFile: %s ...\n", input_file);

    outputFilePtr = openWriteBinary(output_file);
    if (outputFilePtr == NULL) {
        return 1;
    }

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
    traceCharBinMsg("compressCallback: ", ch);

    Node* node = searchCharInTree(ch);
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
    traceCharBinMsg("encode: ", ch);
}

void writeOutput(char ch, int numBit) {
    traceCharBinMsg("writeOutput: ", ch);

    if(numBit == CHAR_SIZE) {
        output_buffer[buffer_idx] = ch;
        buffer_idx++;
        bit_idx = CHAR_SIZE-1;
    } else {
        // TODO MAX
        // handle bit output
        bit_idx--;
        char val = (ch & (1 << bit_idx));
        output_buffer[buffer_idx] = val;
    }

    if(buffer_idx == BLOCK_SIZE) {
        // buffer full, flush data to file
        flushData();
    }
}

void flushData() {
    trace("flushData: %d byte", buffer_idx);

    size_t bytesWritten = fwrite(output_buffer, buffer_idx, 1, outputFilePtr);
    buffer_idx = 0;
}