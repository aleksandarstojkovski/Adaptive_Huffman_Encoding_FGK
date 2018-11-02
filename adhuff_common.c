#include <stdio.h>

#include "adhuff_common.h"
#include "bin_io.h"
#include "log.h"


//
// module variables
//

static adh_order_t          adh_next_order;
static adh_node_t *         adh_root_node = NULL;
static adh_node_t *         adh_nyt_node = NULL;

//
// private methods
//
adh_node_t*     create_nyt();
adh_node_t*     create_node(adh_symbol_t symbol);
void            destroy_node(adh_node_t *node);
adh_node_t*     find_node_by_symbol(adh_node_t *node, adh_symbol_t symbol);
adh_node_t*     find_node_by_encoding(adh_node_t *node, const byte_t bit_array[], int num_bits);
int             get_node_encoding(const adh_node_t *node, byte_t bit_array[]);
int             get_node_level(const adh_node_t *node);

/*
 * Initialize the tree with a single NYT node
 */
int adh_init_tree() {
    log_trace("adh_init_tree", "\n");

    adh_next_order = MAX_ORDER;
    if(adh_root_node != NULL) {
        perror("adh_init_tree: root already initialized");
        return RC_FAIL;
    }

    adh_nyt_node = adh_root_node = create_nyt();
    return RC_OK;
}

/*
 * Destroy Tree and reset pointers
 */
void adh_destroy_tree() {
    log_trace("adh_destroy_tree", "\n");

    destroy_node(adh_root_node);
    adh_root_node = NULL;
    adh_nyt_node = NULL;
}

/*
 * Recursively destroy nodes
 */
void destroy_node(adh_node_t *node) {
    if(node == NULL)
        return;

    char symbol_str[40] = {0};
    log_trace("destroy_node", "%s order=%-8d\n", fmt_symbol(node->symbol, symbol_str, sizeof(symbol_str)), node->order);

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
adh_node_t * adh_create_node_and_append(adh_symbol_t symbol) {
    log_debug("adh_create_node_and_append", "char=%-3c code=%-4d\n", symbol, symbol);

    // IMPORTANT: right node must be created before left node because
    //            create_node() decrease adh_next_order each time it's called

    // create right leaf node with passed symbol (and weight 1)
    adh_node_t * newNode = create_node(symbol);
    if(newNode) {
        newNode->weight = 1;
        newNode->parent = adh_nyt_node;
        adh_nyt_node->right = newNode;

        // create left leaf node with no symbol
        adh_node_t * newNYT = create_nyt();
        newNYT->parent = adh_nyt_node;
        adh_nyt_node->left = newNYT;

        // the new left node is the new NYT node
        adh_nyt_node = newNYT;
        // reset old NYT symbol, since is not a NYT anymore
        newNYT->parent->symbol = ADH_OLD_NYT_CODE;
    }
    return newNode;
}

/*
 * Create a new adh_node_t in the heap.
 */
adh_node_t * create_nyt() {
    log_debug("create_nyt", "\n");

    return create_node(ADH_NYT_CODE);
}

/*
 * Create a new adh_node_t in the heap.
 */
adh_node_t * create_node(adh_symbol_t symbol) {
    if(adh_next_order == 0) {
        log_error("create_node", "unexpected new node creation, adh_next_order = 0, symbol = %d \n", symbol);
        return NULL;
    }

    char symbol_str[40] = {0};
    log_trace("create_node", "%s order=%-8d\n", fmt_symbol(symbol, symbol_str, sizeof(symbol_str)), adh_next_order);

    adh_node_t* node = malloc (sizeof(adh_node_t));
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->order = adh_next_order;
    node->weight = 0;
    node->symbol = symbol;

    adh_next_order--;
    return node;
}

/*
 * Search Char in Tree
 */
adh_node_t * find_node_by_symbol(adh_node_t *node, adh_symbol_t symbol) {
    log_trace("find_node_by_symbol", "char=%-3c code=%-4d\n", symbol, symbol);

    if (node->symbol == symbol){
        return node;
    }

    if(node->left != NULL){
        adh_node_t * leftRes = find_node_by_symbol(node->left, symbol);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        adh_node_t * rightRes = find_node_by_symbol(node->right, symbol);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

adh_node_t * find_node_same_weight_hi_order(adh_node_t *node, adh_weight_t weight, adh_order_t order) {
    log_trace("find_node_same_weight_hi_order", "weight=%-8d order=%-8d\n", weight, order);
    if(node == NULL)
        return NULL;

    // if current node has same weight and higher order of input node, return it
    if ((node->weight == weight) && (node->order > order) && node != adh_root_node){
        return node;
    }

    if(node->left != NULL){
        adh_node_t * leftRes = find_node_same_weight_hi_order(node->left, weight, order);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        adh_node_t * rightRes = find_node_same_weight_hi_order(node->right, weight, order);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

/*
 * Search symbol in tree
 */
adh_node_t * adh_search_symbol_in_tree(adh_symbol_t symbol) {
    char symbol_str[40] = {0};
    log_debug("adh_search_symbol_in_tree", "%s\n", fmt_symbol(symbol, symbol_str, sizeof(symbol_str)));

    return find_node_by_symbol(adh_root_node, symbol);
}

/*
 * Swap Nodes
 */
void swap_nodes(adh_node_t *node1, adh_node_t *node2){
    char symbol1_str[40];
    char symbol2_str[40];

    fmt_symbol(node1->symbol, symbol1_str, sizeof(symbol1_str));
    fmt_symbol(node2->symbol, symbol2_str, sizeof(symbol2_str));
    log_debug("swap_nodes", "(%s order=%d weight=%d) <-> (%s order=%d weight=%d)\n",
            symbol1_str, node1->order, node1->weight,
            symbol2_str, node2->order, node2->weight);

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
    adh_node_t *temp_node = node1->parent;
    node1->parent = node2->parent;
    node2->parent = temp_node;

    // revert original order, since doesn't need to be swapped
    adh_order_t temp_order = node1->order;
    node1->order = node2->order;
    node2->order = temp_order;
}

/*
 * Update Tree, fix sibling property
 */
void adh_update_tree(adh_node_t *node, bool is_new_node) {
    log_debug("adh_update_tree", "char=%-3c code=%-4d order=%-8d weight=%-8d is_new=%d\n", node->symbol, node->symbol, node->order, node->weight, is_new_node);

    // update parents' weight
    adh_node_t * parent = node->parent;
    while(parent != NULL) {
        parent->weight++;
        parent = parent->parent;
    }

    adh_node_t * node_to_check;

    // if node is new it's father is NYT, therefore we don't need to check it
    // if node is not new, his father needs also to be checked
    if(is_new_node == true) {
        node_to_check = node->parent->parent;
    } else {
        node_to_check = node->parent;
    }

    while(node_to_check != NULL && node_to_check != adh_root_node) {
        // search in tree node with same weight and higher order
        adh_node_t * node_to_swap = find_node_same_weight_hi_order(adh_root_node,
                                                                   node_to_check->weight,
                                                                   node_to_check->order);

        // if node_to_swap == NULL, then no swap is needed
        if (node_to_swap != NULL) {
            swap_nodes(node_to_check, node_to_swap);
        }
        // continue ascending the tree
        node_to_check = node_to_check->parent;
    }
}

/*
 * calculate the encoded version of passed symbol
 * fill bit_array from left (MSB) to right (LSB)
 * 0 = left node, 1 = right node
 * return the length of the array
 */
int adh_get_symbol_encoding(adh_symbol_t symbol, byte_t bit_array[]) {
    char symbol_str[40] = {0};
    log_trace("adh_get_symbol_encoding", "%s\n", fmt_symbol(symbol, symbol_str, sizeof(symbol_str)));
    adh_node_t * node = adh_search_symbol_in_tree(symbol);

    return get_node_encoding(node, bit_array);
}

/*
 * calculate the encoded symbol of passed node
 * fill bit_array from left (MSB) to right (LSB)
 * 0 = left node, 1 = right node
 * return the length of the array
 */
int get_node_encoding(const adh_node_t *node, byte_t bit_array[]) {
    int bit_size = 0;
    if(node != NULL) {
        bit_size = get_node_level(node);
        int bit_idx = 0;
        adh_node_t * parent= node->parent;
        while(parent != NULL) {
            // 0 = left node, 1 = right node
            bit_array[bit_idx] = (parent->right == node) ? BIT_1 : BIT_0;
            bit_idx++;
            node = parent;
            parent = parent->parent;
        }

        char symbol_str[40] = {0};
        char bit_array_str[256] = {0};
        log_trace("get_node_encoding", "%s order=%-8d weight=%-8d node_encoding=%s\n",
                fmt_symbol(node->symbol, symbol_str, sizeof(symbol_str)), node->order, node->weight,
                fmt_bit_array(bit_array, bit_idx, bit_array_str, sizeof(bit_array_str)));
    }

    return bit_size;
}

int get_node_level(const adh_node_t *node) {
    char symbol_str[40] = {0};
    log_trace("get_node_level", "%s order=%-8d weight=%-8d \n", fmt_symbol(node->symbol, symbol_str, sizeof(symbol_str)), node->order, node->weight);

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
int adh_get_NYT_encoding(byte_t bit_array[]) {
    log_trace("adh_get_NYT_encoding", "\n");

    return get_node_encoding(adh_nyt_node, bit_array);
}

adh_node_t* adh_search_encoding_in_tree(const byte_t bit_array[], int num_bits) {
    char bit_array_str[256] = {0};
    log_debug("adh_search_encoding_in_tree", "encoding=%s\n",
              fmt_bit_array(bit_array, num_bits, bit_array_str, sizeof(bit_array_str)));

    return find_node_by_encoding(adh_root_node, bit_array, num_bits);
}

adh_node_t* find_node_by_encoding(adh_node_t *node, const byte_t bit_array[], int num_bits) {
    char symbol_str[40] = {0};
    char bit_array_str[256] = {0};
    log_trace("find_node_by_encoding", "%s order=%-8d weight=%-8d encoding=%s\n",
              fmt_symbol(node->symbol, symbol_str, sizeof(symbol_str)), node->order, node->weight,
              node->order, node->weight,
              fmt_bit_array(bit_array, num_bits, bit_array_str, sizeof(bit_array_str)));


    if (node->symbol > ADH_NYT_CODE){
        byte_t node_bit_array[MAX_CODE_BITS] = {0};
        int node_num_bits = get_node_encoding(node, node_bit_array);
        if(compare_bit_arrays(bit_array, num_bits, node_bit_array, node_num_bits))
            return node;
    }

    if(node->left != NULL){
        adh_node_t * leftRes = find_node_by_encoding(node->left, bit_array, num_bits);
        if(leftRes != NULL)
            return leftRes;
    }

    if(node->right != NULL){
        adh_node_t * rightRes = find_node_by_encoding(node->right, bit_array, num_bits);
        if(rightRes != NULL)
            return rightRes;
    }
    return NULL;
}

