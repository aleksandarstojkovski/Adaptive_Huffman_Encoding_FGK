#include <stdio.h>
#include <stdlib.h>

#include "adhuff_common.h"
#include "bin_io.h"

//
// module variables
//

static unsigned short _nextOrder = MAX_ORDER;
static Node * adh_root_node = NULL;
static Node * adh_nyt_node = NULL;

//
// private methods
//
Node * createNYT();
Node * createNode(unsigned short value);
void destroyNode(Node * node);
Node * searchCharFromNode(Node * node, unsigned short ch);
int getNodeCode(Node *node, unsigned char *bit_array);

/*
 * Initialize the tree with a single NYT node
 */
int initializeTree() {
    trace("initializeTree\n");

    if(adh_root_node != NULL) {
        perror("root already initialized");
        return RC_FAIL;
    }

    adh_nyt_node = adh_root_node = createNYT();
    return RC_OK;
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
Node * createNodeAndAppend(unsigned short value) {
    trace("createNodeAndAppend: value=%c,\n", value);

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
    // reset old NYT value, since is not a NYT anymore
    newNYT->parent->value = ADH_OLD_NYT_CODE;

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
Node * createNode(unsigned short value) {
    trace("createNode: order=%d, value=%c\n", _nextOrder, value);

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
Node * searchCharFromNode(Node * node, unsigned short ch) {
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

Node * searchNodeWithSameWeightAndHigherOrder(Node * node, unsigned int weight, unsigned int order) {
    trace("searchNodeWithSameWeightAndHigherOrder: weight=%u, order=%u\n",weight,order);

    // if current node has same weight and higher order of input node, return it
    if ((node->weight == weight) && (node->order>order) && node != adh_root_node){
        return node;
    }

    if(node->left != NULL){
        Node * leftRes = searchNodeWithSameWeightAndHigherOrder(node->left, weight, order);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        Node * rightRes = searchNodeWithSameWeightAndHigherOrder(node->right, weight, order);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

/*
 * Search Char in Tree
 */
Node * searchCharInTree(unsigned short ch) {
    trace("searchCharInTree\n");

    return searchCharFromNode(adh_root_node, ch);
}

/*
 * Swap Nodes
 */
void swapNodes(Node * node1, Node * node2){

    // check if node1 is left or right child
    if (node1->parent->left == node1){
        //node1 is left child
        node1->parent->left=node2;
    } else {
        //node1 is right child
        node1->parent->right=node2;
    }

    // check if node2 is left or right child
    if (node2->parent->left == node2){
        //node2 is left child
        node2->parent->left=node1;
    } else {
        //node2 is right child
        node2->parent->right=node1;
    }

    // fix their fathers
    Node *tempNode = node1->parent;
    node1->parent = node2->parent;
    node2->parent = tempNode;

    // revert original order, since doesn't need to be swapped
    int tempOrder = node1->order;
    node1->order=node2->order;
    node2->order=tempOrder;
}

/*
 * Update Tree, fix sibling property
 */
void updateTree(Node * node, bool isNewNode) {
    trace("updateTre: isNewNode=%d\n",isNewNode);

    Node * nodeToCheck;

    // if node is new it's father is NYT, therefore we don't need to check it
    // if node is not new, his father needs also to be checked
    if(isNewNode == true) {
        nodeToCheck = node->parent->parent;
    } else {
        nodeToCheck = node->parent;
    }

    while(nodeToCheck != NULL && nodeToCheck != adh_root_node) {
        // search in tree node with same weight and higher order
        Node * nodeToBeSwappedWith = searchNodeWithSameWeightAndHigherOrder(adh_root_node,nodeToCheck->weight,nodeToCheck->order);
        // if nodeToBeSwappedWith == NULL, then no swap is needed
        if (nodeToBeSwappedWith != NULL) {
            swapNodes(nodeToCheck, nodeToBeSwappedWith);
        }
        // continue ascending the tree
        nodeToCheck=nodeToCheck->parent;
    }
}

/*
 * return the encoded version of passed symbol
 * fill bit_array from left to right
 * the number of bit to read is returned
 * 0 = left node, 1 = right node
 */
int getSymbolCode(unsigned short ch, unsigned char bit_array[]) {
    Node * node = searchCharInTree(ch);

    return getNodeCode(node, bit_array);
}

int getNodeCode(Node *node, unsigned char *bit_array) {
    int bit_size = 0;
    if(node != NULL) {
        Node * parent = node->parent;
        while(parent != NULL) {
            // 0 = left node, 1 = right node
            bit_array[bit_size] = parent->right == node ? BIT_1 : BIT_0;
            bit_size++;
            node = parent;
            parent = parent->parent;
        }
    }
    return bit_size;
}

/*
 * getSymbolCode for NYT
 */
int getNYTCode(unsigned char bit_array[]) {
    return getNodeCode(adh_nyt_node, bit_array);
}