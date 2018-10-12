//
// Created by massi on 12.10.2018.
//

#include "node.h"

/*
 * global variables
 */
Node *_root = NULL, *_NYT = NULL;
unsigned short _nextOrder = MAX_ORDER;

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
 * Must be used only for new for symbols (not present in the tree)
 */
void addNewNode(const char value) {
    // old NYT should be increased to weight 1
    _NYT->weight = 1;

    // create right leaf note with passed symbol and weight 1
    // createNode decrease the order each time
    // therefore right node must be created before left node
    _NYT->right = createNode(value);
    _NYT->right->weight = 1;
    _NYT->right->parent = _NYT;

    //
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
    node->order = _nextOrder;
    node->weight = 0;
    node->value = value;
    _nextOrder--;
    return node;
}

/*
 * Search Char in Tree
 */
Node *searchCharInTree(char ch) {
    // TODO ALEX
    return NULL;
}
