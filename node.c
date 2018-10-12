//
// Created by massi on 12.10.2018.
//

#include "node.h"

/*
 * global variables
 */
Node *_root = NULL, *_NYT = NULL;
unsigned short _nextPos = MAX_POS;

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