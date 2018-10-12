#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

#define NUM_TEST_FILES 7
#define BLOCK_SIZE 1024
//const unsigned short NUM_TEST_FILES = 7;

int readBinaryFile(const char *filename, void (*processChar)(char));
int compressFile(const char *input_file, const char *output_file);
int decompressFile(const char *input_file, const char *output_file);

void printCharBin(char ch);
int testReadBinaryFile(const char *filename);

void printUsage();


void encode(char ch);

Node *searchCharInTree(char ch);

char *TEST_FILES[NUM_TEST_FILES] = {"../test-res/32k_ff", "../test-res/32k_random", "../test-res/alice.txt",
                                    "../test-res/empty", "../test-res/ff_ff_ff", "../test-res/immagine.tiff",
                                    "a-bad-filename"};
/*
 * Main function
 */
int main(int argc, char* argv[])
{
    int rc = 0;
    if (argc < 4) {
        fprintf(stderr, "Not enough parameters.\n");
        printUsage();
        rc = 1;

        for(int i=0; i<NUM_TEST_FILES; i++) {
            testReadBinaryFile(TEST_FILES[i]);
        }
    }
    else if (strcmp(argv[1], "-c") == 0) {
        rc = compressFile(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "-d") == 0) {
        rc = decompressFile(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "-t") == 0) {
        rc = testReadBinaryFile(argv[2]);
    }
    else {
        fprintf(stderr, "Unexpected argument\n");
        printUsage();
        rc = 2;
    }

    return rc;
}

/*
 * Print usage
 */
void printUsage() {
    puts("usage:");
    puts("\tto compress a file: algo -c <filename_to_compress> <file");
    puts("\tto decompress a file: algo -d <filename_to_decompress>");
}

/*
 * Compress Callback
 */
void compressCallback(char ch) {
    // TODO
    Node* node = searchCharInTree(ch);
    if(node == NULL) {
        // Node not present in tree
        addNewNode(ch);

    }

    encode(ch);
    // updateTree(ch)
}


void encode(char ch) {

}

/*
 * Compress file
 */
int compressFile(const char *input_file, const char *output_file) {
    printf("START compressing: %s ...\n", input_file);
    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(input_file, compressCallback);
        destroyTree();
    }
    return rc;
}

/*
 * Decompress Callback
 */
void decompressCallback(char ch) {
    // TODO
    // decode(ch)
    // updateTree(ch)
}

/*
 * decompress file
 */
int decompressFile(const char *input_file, const char *output_file) {
    printf("START decompressing: %s ...\n", input_file);
    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(input_file, decompressCallback);
        destroyTree();
    }
    return rc;
}



/*
 *  Read a binary file and call a callback function
 */
int readBinaryFile(const char *filename, void (*processChar)(char)) {
    FILE *file = NULL;
    unsigned char buffer[BLOCK_SIZE];
    size_t bytesRead = 0;

    file = fopen(filename, "rb");
    if(file == NULL) {
        perror("cannot open file");
        return 1;
    }

    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        for(int i=0;i<bytesRead;i++)
            processChar(buffer[i]);
    }
    fclose(file);
    return 0;
}

/*
 * Test Read Binary file
 */
int testReadBinaryFile(const char *filename) {
    printf("Start testReadBinaryFile: [%s]\n", filename);

    int rc = readBinaryFile(filename, printCharBin);

    printf("\nRead completed, return code: %d\n", rc);
    printf("press a key to continue\n");
    getchar();

    return rc;
}

/*
 * print char in binary format
 */
void printCharBin(char ch) {
    for (int bitPos = 7; bitPos >= 0; --bitPos) {
        char val = (ch & (1 << bitPos));
        putchar(val ? '1' : '0');
    }
}
