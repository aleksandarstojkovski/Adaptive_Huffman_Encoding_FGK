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

int testReadBinaryFile(const char *filename);
void compressFile(const char *filename);
void decompressFile(const char *filename);

const unsigned short MAX_POS = 512;
Node *_root = NULL, *_NYT = NULL;
unsigned short _nextPos = 512;

/*
 * Main function
 */
int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Not enough parameters. Usage:\n");
        printf("to compress a file: algo -c <filename_to_compress>\n");
        printf("to decompress a file: algo -d <filename_to_decompress>\n");

        printf("Start Test read binary files\n");

        int rc = 0;
        rc = testReadBinaryFile("../test-res/nofile.bo");
        rc = testReadBinaryFile("../test-res/32k_ff");
        rc = testReadBinaryFile("../test-res/32k_random");
        rc = testReadBinaryFile("../test-res/alice.txt");
        rc = testReadBinaryFile("../test-res/empty");
        rc = testReadBinaryFile("../test-res/ff_ff_ff");
        rc = testReadBinaryFile("../test-res/immagine.tiff");

        //fprintf(stderr, "Numero insufficiente di argomenti\n");
        return 1;
    }

    if (strcmp(argv[1], "-c") == 0)
        compressFile(argv[2]);
    else if (strcmp(argv[1], "-d") == 0)
        decompressFile(argv[2]);
    else {
        fprintf(stderr, "Unexpected argument\n");
        return 2;
    }

    return 0;
}


/*
 * Compress file
 */
void compressFile(const char *filename) {
    if(initializeTree())
        return;

    // TODO
    printf("START compressing: %s ...\n", filename);

    destroyTree();
}


/*
 * Decompress file
 */
void decompressFile(const char *filename) {
    if(initializeTree())
        return;

    // TODO
    printf("START decompressing: %s ...\n", filename);

    destroyTree();
}

/*
 * Initialize the tree with a single NYT node
 */
int initializeTree() {
    if(_root != NULL) {
        perror("already initialized");
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


// read binary file
int testReadBinaryFile(const char *filename) {
    printf("***************\n");
    printf("reading file %s\n", filename);
    printf("***************\n");

    FILE *file = NULL;
    unsigned char buffer[1024];  // array of bytes, not pointers-to-bytes
    size_t bytesRead = 0;

    file = fopen(filename, "rb");
    if(file == NULL) {
        perror("cannot open file");
        return 1;
    }

    // read up to sizeof(buffer) bytes
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        // process bytesRead worth of data in buffer
        for(int i=0;i<bytesRead;i++)
            ; //printf("%c", buffer[i]);
    }
    fclose(file);
    return 0;
}


