#include <stdio.h>
#include <stdlib.h>

#include "adhuff_common.h"
#include "bin_io.h"

//
// module variables
//

static unsigned short       adh_next_order;
static adh_node_t *         adh_root_node = NULL;
static adh_node_t *         adh_nyt_node = NULL;

//
// private methods
//
adh_node_t*     create_NYT();
adh_node_t*     create_node(unsigned short value);
void            destroy_node(adh_node_t *node);
adh_node_t*     search_char_from_node(adh_node_t *node, unsigned short ch);
int             get_node_encoding(adh_node_t *node, unsigned char *bit_array);
int             get_node_level(adh_node_t *node);

/*
 * Initialize the tree with a single NYT node
 */
int adh_init_tree() {
    adh_next_order = MAX_ORDER;
    log_trace("adh_init_tree\n");

    if(adh_root_node != NULL) {
        perror("root already initialized");
        return RC_FAIL;
    }

    adh_nyt_node = adh_root_node = create_NYT();
    return RC_OK;
}

/*
 * Destroy Tree and reset pointers
 */
void adh_destroy_tree() {
    log_trace("adh_destroy_tree\n");

    destroy_node(adh_root_node);
    adh_root_node = NULL;
    adh_nyt_node = NULL;
}

/*
 * Recursively destroy nodes
 */
void destroy_node(adh_node_t *node) {
    log_trace("destroy_node\n");

    if(node == NULL)
        return;

    if(node->left != NULL) {
        destroy_node(node->left);
        node->left = NULL;
    }

    if(node->right != NULL) {
        destroy_node(node->right);
        node->right = NULL;
    }

    free(node);
}

/*
 * Append a new node to the NYT
 * Must be used only for new for symbols (not present in the tree)
 */
adh_node_t * adh_create_node_and_append(adh_symbol_t value) {
    log_trace("adh_create_node_and_append: symbol=%c,\n", value);

    // IMPORTANT: right node must be created before left node because
    //            create_node() decrease adh_next_order each time it's called

    // create right leaf node with passed symbol (and weight 1)
    adh_node_t * newNode = create_node(value);
    newNode->weight = 1;
    newNode->parent = adh_nyt_node;
    adh_nyt_node->right = newNode;

    // create left leaf node with no symbol
    adh_node_t * newNYT = create_NYT();
    newNYT->parent = adh_nyt_node;
    adh_nyt_node->left = newNYT;

    // the new left node is the new NYT node
    adh_nyt_node = newNYT;
    // reset old NYT symbol, since is not a NYT anymore
    newNYT->parent->symbol = ADH_OLD_NYT_CODE;

    return newNode;
}

/*
 * Create a new adh_node_t in the heap.
 */
adh_node_t * create_NYT() {
    log_trace("create_NYT\n");

    return create_node(ADH_NYT_CODE);
}

/*
 * Create a new adh_node_t in the heap.
 */
adh_node_t * create_node(adh_symbol_t value) {
    if(adh_next_order == 0)
        fprintf(stderr, "!!!! unexpected new node creation, adh_next_order = 0, symbol = %d \n", value);

    log_trace("create_node: order=%d, symbol=%d\n", adh_next_order, value);

    adh_node_t* node = malloc (sizeof(adh_node_t));
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->order = adh_next_order;
    node->weight = 0;
    node->symbol = value;

    adh_next_order--;
    return node;
}

/*
 * Search Char in Tree
 */
adh_node_t * search_char_from_node(adh_node_t *node, adh_symbol_t ch) {
    log_trace("search_char_from_node\n");

    if (node->symbol == ch){
        return node;
    }

    if(node->left != NULL){
        adh_node_t * leftRes = search_char_from_node(node->left, ch);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        adh_node_t * rightRes = search_char_from_node(node->right, ch);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

adh_node_t * searchNodeWithSameWeightAndHigherOrder(adh_node_t * node, unsigned int weight, unsigned int order) {
    log_trace("searchNodeWithSameWeightAndHigherOrder: weight=%u, order=%u\n", weight, order);

    // if current node has same weight and higher order of input node, return it
    if ((node->weight == weight) && (node->order > order) && node != adh_root_node){
        return node;
    }

    if(node->left != NULL){
        adh_node_t * leftRes = searchNodeWithSameWeightAndHigherOrder(node->left, weight, order);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        adh_node_t * rightRes = searchNodeWithSameWeightAndHigherOrder(node->right, weight, order);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

/*
 * Search Char in Tree
 */
adh_node_t * adh_search_symbol_in_tree(adh_symbol_t ch) {
    log_trace("adh_search_symbol_in_tree\n");

    return search_char_from_node(adh_root_node, ch);
}

/*
 * Swap Nodes
 */
void swapNodes(adh_node_t * node1, adh_node_t * node2){

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
    adh_node_t *tempNode = node1->parent;
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
void adh_update_tree(adh_node_t *node, bool isNewNode) {
    log_trace("updateTre: isNewNode=%d\n", isNewNode);

    // update parents' weight
    adh_node_t * parent = node->parent;
    while(parent != NULL) {
        parent->weight++;
        parent = parent->parent;
    }

    adh_node_t * nodeToCheck;

    // if node is new it's father is NYT, therefore we don't need to check it
    // if node is not new, his father needs also to be checked
    if(isNewNode == true) {
        nodeToCheck = node->parent->parent;
    } else {
        nodeToCheck = node->parent;
    }

    while(nodeToCheck != NULL && nodeToCheck != adh_root_node) {
        // search in tree node with same weight and higher order
        adh_node_t * nodeToBeSwappedWith = searchNodeWithSameWeightAndHigherOrder(adh_root_node,nodeToCheck->weight,nodeToCheck->order);
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
int adh_get_symbol_encoding(adh_symbol_t ch, unsigned char *bit_array) {
    adh_node_t * node = adh_search_symbol_in_tree(ch);

    return get_node_encoding(node, bit_array);
}

int get_node_encoding(adh_node_t *node, unsigned char bit_array[]) {
    int bit_size = 0;
    if(node != NULL) {
        bit_size = get_node_level(node);

        int bit_idx = bit_size - 1;
        adh_node_t * parent= node->parent;
        while(parent != NULL) {
            // 0 = left node, 1 = right node
            bit_array[bit_idx] = (parent->right == node) ? BIT_1 : BIT_0;
            bit_idx--;
            node = parent;
            parent = parent->parent;
        }
    }
    return bit_size;
}

int get_node_level(adh_node_t *node) {
    int level = 0;
    adh_node_t * parent = node->parent;
    while(parent != NULL) {
        level++;
        parent = parent->parent;
    }
    return level;
}

/*
 * adh_get_symbol_encoding for NYT
 */
int adh_get_NYT_encoding(unsigned char *bit_array) {
    return get_node_encoding(adh_nyt_node, bit_array);
}