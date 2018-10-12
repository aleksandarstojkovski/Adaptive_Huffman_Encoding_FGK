#include <stdio.h>

#include "adhuff_compress.h"
#include "bin_io.h"
#include "node.h"

/*
 * global variables
 */
const char *_compress_outputp_file;

/*
 * Compress file
 */
int compressFile(const char *input_file, const char *output_file) {
    _compress_outputp_file = output_file;
    printf("START compressing: %s ...\n", input_file);

    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(input_file, compressCallback);
        destroyTree();
    }
    return rc;
}

/*
 * Compress Callback
 */
void compressCallback(char ch) {
    Node* node = searchCharInTree(ch);
    if(node == NULL) {
        // Node not present in tree
        writeBinaryFile(_compress_outputp_file, ch);
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
