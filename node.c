#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

/*
 * module variables
 */
static const short ADH_NYT_CODE = 256;
static Node * adh_root_node = NULL;
static Node * adh_nyt_node = NULL;

/*
 * Module variables
 */
static unsigned short _nextOrder = MAX_ORDER;

/*
 * Initialize the tree with a single NYT node
 */
int initializeTree() {
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
    destroyNode(adh_root_node);
    adh_root_node = NULL;
    adh_nyt_node = NULL;
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
void addNewNode(short value) {
    // old NYT should be increased to weight 1
    adh_nyt_node->weight = 1;

    // create right leaf note with passed symbol and weight 1
    // createNode decrease the order each time
    // therefore right node must be created before left node
    adh_nyt_node->right = createNode(value);
    adh_nyt_node->right->weight = 1;
    adh_nyt_node->right->parent = adh_nyt_node;

    // create left leaf node with no symbol
    adh_nyt_node->left = createNYT();
    adh_nyt_node->left->parent = adh_nyt_node;

    // the new left node is the new NYT node
    adh_nyt_node = adh_nyt_node->left;
}

/*
 * Create a new Node in the heap.
 */
Node *createNYT() {
    return createNode(ADH_NYT_CODE);
}

/*
 * Create a new Node in the heap.
 */
Node *createNode(short value) {
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
Node * searchCharFromNode(Node *node, short ch) {
    // TODO ALEX
    //while (node->value != NYT_CODE){
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
    // }
    return NULL;
}

/*
 * Search Char in Tree
 */
Node * searchCharInTree(short ch) {
    return searchCharFromNode(adh_root_node, ch);
}