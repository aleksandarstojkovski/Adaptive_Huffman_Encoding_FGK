#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char value;
    unsigned int weight;
    unsigned short position;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
};
typedef struct Node Node;

Node* createNode(const char value);
void destroyNode(Node *node);
int initializeTree();
void destroyTree();

int readBinaryFile(const char *filename, void (*processChar)(char));
int compressFile(const char *filename);
int decompressFile(const char *filename);

void printCharBin(char ch);
int testReadBinaryFile(const char *filename);

void printUsage();

const unsigned short BLOCK_SIZE = 1024;
const unsigned short MAX_POS = 512;

Node *_root = NULL, *_NYT = NULL;
unsigned short _nextPos = 512;

const unsigned short NUM_TEST_FILES = 7;
char *TEST_FILES[NUM_TEST_FILES] = { "../test-res/32k_ff", "../test-res/32k_random", "../test-res/alice.txt",
                                     "../test-res/empty", "../test-res/ff_ff_ff", "../test-res/immagine.tiff",
                                     "a-bad-filename"};
/*
 * Main function
 */
int main(int argc, char* argv[])
{
    int rc = 0;
    if (argc < 3) {
        fprintf(stderr, "Not enough parameters.\n");
        printUsage();
        rc = 1;

        for(int i=0; i<NUM_TEST_FILES; i++) {
            testReadBinaryFile(TEST_FILES[i]);
        }
    }
    else if (strcmp(argv[1], "-c") == 0) {
        rc = compressFile(argv[2]);
    }
    else if (strcmp(argv[1], "-d") == 0) {
        rc = decompressFile(argv[2]);
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
    puts("\tto compress a file: algo -c <filename_to_compress>");
    puts("\tto decompress a file: algo -d <filename_to_decompress>");
}

/*
 * Compress Callback
 */
void compressCallback(char ch) {
    // TODO
    // encode(ch)
    // updateTree(ch)
}

/*
 * Compress file
 */
int compressFile(const char *filename) {
    printf("START compressing: %s ...\n", filename);
    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(filename, compressCallback);
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
int decompressFile(const char *filename) {
    printf("START decompressing: %s ...\n", filename);
    int rc = initializeTree();
    if (rc == 0) {
        rc = readBinaryFile(filename, decompressCallback);
        destroyTree();
    }
    return rc;
}

/*
 * Initialize the tree with a single NYT node
 */
int initializeTree() {
    if(_root != NULL) {
        perror("root already initialized");
        return 1;
    }

    _NYT = _root = createNode(0);
    return 0;
}

/*
 * Destroy Tree and reset pointers
 */
void destroyTree() {
    destroyNode(_root);
    _root = NULL;
    _NYT = NULL;
}

/*
 * Recursively destroy nodes
 */
void destroyNode(Node *node) {
    if(node == NULL)
        return;

    if(node->left != NULL) {
        destroyNode(node->left);
        node->left = NULL;
    }

    if(node->right != NULL) {
        destroyNode(node->right);
        node->right = NULL;
    }

    free(node);
}

/*
 * Append a new node to the NYT
 */
void addNewNode(const char value) {
    _NYT->weight = 1;

    _NYT->right = createNode(value);
    _NYT->right->weight = 1;
    _NYT->right->parent = _NYT;

    _NYT->left = createNode(0);
    _NYT->left->parent = _NYT;

    _NYT = _NYT->left;
}

/*
 * Create a new Node in the heap.
 */
Node *createNode(char value) {
    Node* node = malloc (sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->position = _nextPos;
    node->weight = 0;
    node->value = value;
    _nextPos--;
    return node;
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
