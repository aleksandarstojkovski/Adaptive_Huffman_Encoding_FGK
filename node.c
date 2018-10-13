#include <stdio.h>
#include <stdlib.h>

#include "node.h"
#include "bin_io.h"


/*
 * module variables
 */
static const unsigned short MAX_ORDER = 512;
static const short ADH_NYT_CODE = 256;

static unsigned short _nextOrder = MAX_ORDER;
static Node * adh_root_node = NULL;
static Node * adh_nyt_node = NULL;

/*
 * Initialize the tree with a single NYT node
 */
int initializeTree() {
    trace("initializeTree\n");

    if(adh_root_node != NULL) {
        perror("root already initialized");
        return 1;
    }

    adh_nyt_node = adh_root_node = createNYT();
    return 0;
}

/*
 * Destroy Tree and reset pointers
 */
void destroyTree() {
    trace("destroyTree\n");

    destroyNode(adh_root_node);
    adh_root_node = NULL;
    adh_nyt_node = NULL;
}

/*
 * Recursively destroy nodes
 */
void destroyNode(Node * node) {
    trace("destroyNode\n");

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
 * Must be used only for new for symbols (not present in the tree)
 */
Node * createNodeAndAppend(short value) {
    trace("createNodeAndAppend\n");

    // old NYT should be increased to weight 1
    adh_nyt_node->weight = 1;

    // IMPORTANT: right node must be created before left node
    // because createNode() decrease _nextOrder each time it's called

    // create right leaf node with passed symbol (and weight 1)
    Node * newNode = createNode(value);
    newNode->weight = 1;
    newNode->parent = adh_nyt_node;
    adh_nyt_node->right = newNode;

    // create left leaf node with no symbol
    Node * newNYT = createNYT();
    newNYT->parent = adh_nyt_node;
    adh_nyt_node->left = newNYT;

    // the new left node is the new NYT node
    adh_nyt_node = newNYT;

    return newNode;
}

/*
 * Create a new Node in the heap.
 */
Node * createNYT() {
    trace("createNYT\n");

    return createNode(ADH_NYT_CODE);
}

/*
 * Create a new Node in the heap.
 */
Node * createNode(short value) {
    trace("createNode: %d\n", _nextOrder);

    Node* node = malloc (sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->order = _nextOrder;
    node->weight = 0;
    node->value = value;
    _nextOrder--;
    return node;
}

/*
 * Search Char in Tree
 */
Node * searchCharFromNode(Node * node, short ch) {
    trace("searchCharFromNode\n");

    if (node->value == ch){
        return node;
    }

    if(node->left != NULL){
        Node * leftRes = searchCharFromNode(node->left, ch);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        Node * rightRes = searchCharFromNode(node->right, ch);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

/*
 * Search Char in Tree
 */
Node * searchCharInTree(short ch) {
    trace("searchCharInTree\n");

    return searchCharFromNode(adh_root_node, ch);
}

/*
 * Update Tree
 */
void updateTree(short ch) {
    trace("updateTree\n");

    // TODO ALEX

    // FIX sibling property with swapping
    // the nodes (internal and leaf) are arranged in order of increasing weights
}
