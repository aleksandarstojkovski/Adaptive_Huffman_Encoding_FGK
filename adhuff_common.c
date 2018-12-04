#include <stdio.h>
#include <string.h>

#include "adhuff_common.h"
#include "bin_io.h"
#include "log.h"


//
// module variables
//

static adh_order_t          adh_next_order;
static adh_node_t *         adh_root_node = NULL;
static adh_node_t *         adh_nyt_node = NULL;
static adh_node_t *         adh_node_array[MAX_ORDER];
static adh_node_t *         adh_symbol_node_array[MAX_CODE_BITS];
static int                  last_index_of_node_array;
static int                  last_index_of_symbol_node_array;
//
// private methods
//
adh_node_t*     create_nyt();
adh_node_t*     create_node(adh_symbol_t symbol);
void            destroy_node(adh_node_t *node);
void            update_node_encoding(adh_node_t *node);
void            sort_node_array();


adh_node_t*     get_nyt() {
    return adh_nyt_node;
}

/*
 * Initialize the tree with a single NYT node
 */
int adh_init_tree() {
#ifdef _DEBUG
    log_trace("adh_init_tree", "\n");
#endif
    last_index_of_symbol_node_array = 0;
    last_index_of_node_array = 0;
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
#ifdef _DEBUG
    log_trace("adh_destroy_tree", "\n");
#endif

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

#ifdef _DEBUG
    log_trace(" destroy_node", "%s\n", fmt_node(node));
#endif

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
 * Must be used only for new symbols (not present in the tree)
 */
adh_node_t * adh_create_node_and_append(adh_symbol_t symbol) {
#ifdef _DEBUG
    log_trace("    adh_create_node_and_append", "%s (1,%d)\n", fmt_symbol(symbol), adh_next_order);
#endif

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

        update_node_encoding(newNode);  // update bit_array
        update_node_encoding(newNYT);    // update bit_array
    }
    return newNode;
}

/*
 * Create a new adh_node_t in the heap.
 */
adh_node_t * create_nyt() {
#ifdef _DEBUG
    log_trace("    create_nyt", "%s (0,%d)\n", fmt_symbol(ADH_NYT_CODE), adh_next_order);
#endif

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

#ifdef _DEBUG
    log_trace("     create_node", "%s (0,%d)\n", fmt_symbol(symbol), adh_next_order);
#endif

    adh_node_t* node = malloc (sizeof(adh_node_t));

    // add node to node array
    adh_node_array[last_index_of_node_array++] = node;
    if(symbol > ADH_NYT_CODE)
        adh_symbol_node_array[last_index_of_symbol_node_array++] = node;

    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->order = adh_next_order;
    node->weight = 0;
    node->symbol = symbol;
    memset(&(node->bit_array), 0, sizeof(bit_array_t));

    adh_next_order--;

    sort_node_array();

    return node;
}

void sort_node_array() {
    for (int i = 1; i < last_index_of_node_array; i++) {
        int j = i-1;
        adh_node_t* node1 = adh_node_array[j];
        adh_node_t* node2 = adh_node_array[i];

        // sort by weight so the find_higher_order_same_weight method will be faster
        while (j >= 0 && node1->weight > node2->weight) {
            adh_node_array[j+1] = adh_node_array[j];
            j = j-1;
        }
        adh_node_array[j+1] = node2;
    }
}

adh_node_t * find_higher_order_same_weight(adh_weight_t weight, adh_order_t order) {
    //TODO: ordinando adh_node_array saremmo piu' veloci nella ricerca.
    //      da valutare il costo dell'ordinamento rispettto a una ricerca completa
    //      83% del costo della compressione di immagine.tiff e' speso in questo metodo
    adh_node_t *node_to_be_returned=NULL;
    adh_node_t *current_node;

    for (int i=0; i<last_index_of_node_array;i++){
        current_node = adh_node_array[i];
        if ((current_node->weight == weight) &&
            (current_node->order > order) &&
            (current_node != adh_root_node) &&
            (node_to_be_returned == NULL || current_node->order > node_to_be_returned->order)) {
                node_to_be_returned = current_node;
        }
    }

    return node_to_be_returned;
}

/*
 * Search symbol in tree
 */
adh_node_t * adh_search_symbol_in_tree(adh_symbol_t symbol) {
#ifdef _DEBUG
    log_trace("  adh_search_symbol_in_tree", "%s\n", fmt_symbol(symbol));
#endif

    // search only in adh_symbol_node_array
    for (int i=0; i<last_index_of_symbol_node_array; i++){
        if (adh_symbol_node_array[i]->symbol == symbol){
            return adh_symbol_node_array[i];
        }
    }
    return NULL;
}

/*
 * Swap Nodes
 */
void swap_nodes(adh_node_t *node1, adh_node_t *node2){
    if (node1->parent == node2 || node2->parent == node1) {
        //log_info("swap_nodes", " TRYING TO SWAP NODE WITH IT'S PARENT\n");
        return;
    }

#ifdef _DEBUG
    char str1[MAX_SYMBOL_STR];
    strcpy (str1,fmt_node(node1));

    log_debug("    swap_nodes", "%s <-> %s\n", str1, fmt_node(node2));
#endif

    bool is_node1_left = node1->parent->left == node1;
    bool is_node2_left = node2->parent->left == node2;

    // check if node1 is left or right child
    if (is_node1_left){
        //node1 is left child
        node1->parent->left = node2;
    } else {
        //node1 is right child
        node1->parent->right = node2;
    }

    // check if node2 is left or right child
    if (is_node2_left){
        //node2 is left child
        node2->parent->left = node1;
    } else {
        //node2 is right child
        node2->parent->right = node1;
    }

    // fix their fathers
    adh_node_t *temp_node = node1->parent;
    node1->parent = node2->parent;
    node2->parent = temp_node;

    // revert original order, since doesn't need to be swapped
    adh_order_t temp_order = node1->order;
    node1->order = node2->order;
    node2->order = temp_order;

    update_node_encoding(node1);  // update bit_array
    update_node_encoding(node2);  // update bit_array
}

/*
 * Update Tree, fix sibling property
 * @param1: node to be updated.
 * @param2: true if node is new, false if node is not new.
 */
void adh_update_tree(adh_node_t *node, bool is_new_node) {
#ifdef _DEBUG
    log_debug("  adh_update_tree", "%s is_new=%d\n",
              fmt_node(node), is_new_node);
#endif

    // create node_to_check
    adh_node_t * node_to_check = is_new_node ? node->parent : node;
    while(node_to_check != NULL && node_to_check != adh_root_node) {
        // search in tree node with same weight and higher order
        adh_node_t * node_to_swap = find_higher_order_same_weight(node_to_check->weight,
                                                                  node_to_check->order);


        // if node_to_swap == NULL, then no swap is needed
        if (node_to_swap != NULL) {
#ifdef _DEBUG
            log_tree();
#endif
            swap_nodes(node_to_check, node_to_swap);
        }
        // now we can safely update the weight of the node
        node_to_check->weight++;

        // continue ascending the tree
        node_to_check = node_to_check->parent;
    }
    if(node_to_check != NULL) {
        node_to_check->weight++;
    }

#ifdef _DEBUG
    log_tree();
#endif
}

/*
 * calculate the encoded symbol of passed node
 * fill bit_array from left (MSB) to right (LSB)
 * 0 = left node, 1 = right node
 * return the length of the array
 */
void update_node_encoding(adh_node_t *node) {
    if(node != NULL) {
        update_node_encoding(node->left);
        update_node_encoding(node->right);

        bit_array_t* bit_array = &(node->bit_array);
        bit_array->length = 0;

        adh_node_t * parent = node->parent;
        while(parent != NULL) {
            if(bit_array->length == MAX_CODE_BITS) {
                log_error("update_node_encoding", "bit_array->length == MAX_CODE_BITS");

                //TODO: exit properly, need to release resources
                exit(1);
            }

            // 0 = left node, 1 = right node
            bit_array->buffer[bit_array->length] = (parent->right == node) ? BIT_1 : BIT_0;
            bit_array->length++;

            node = parent;
            parent = node->parent;
        }

#ifdef _DEBUG
        log_trace("  update_node_encoding", "%s bin=%s\n",
                fmt_node(node),
                fmt_bit_array(bit_array));
#endif

    }
}

int get_node_level(const adh_node_t *node) {
#ifdef _DEBUG
    log_trace("  get_node_level", "%s \n", fmt_node(node));
#endif

    int level = 0;
    adh_node_t * parent = node->parent;
    while(parent != NULL) {
        level++;
        parent = parent->parent;
    }
    return level;
}


adh_node_t* adh_search_encoding_in_tree(const bit_array_t* bit_array) {
    // search only in adh_symbol_node_array
    for (int i=0; i<last_index_of_symbol_node_array; i++){
        adh_node_t* node = adh_symbol_node_array[i];

        // skip NYT and OLD NYT
        if (node->symbol > ADH_NYT_CODE) {
            if(compare_bit_arrays(bit_array, &(node->bit_array))) {
#ifdef _DEBUG
                log_debug("  adh_search_encoding_in_tree", "bin=%s found=%s\n",
              fmt_bit_array(bit_array), fmt_node(node));
#endif

                return node;
            }
        }
    }

    return NULL;
}

void print_sub_tree(const adh_node_t *node, int depth)
{
    if(node==NULL)
        return;

    static int nodes[MAX_ORDER];
    printf("  ");

    // unicode chars for box drawing (doesn't work well under windows CLion)
    // https://en.wikipedia.org/wiki/Box_Drawing
    for(int i=0;i<depth;i++) {
        if(i == depth-1)
            printf("%s------ ", nodes[depth-1] ? "+" : "\\");
        else
            printf("%s       ", nodes[i] ? "|" : " ");
    }

    printf("%s  %s\n", fmt_node(node), fmt_bit_array(&(node->bit_array)));

    nodes[depth]=1;
    print_sub_tree(node->left, depth + 1);
    nodes[depth]=0;
    print_sub_tree(node->right, depth + 1);
}

void print_tree() {
    print_sub_tree(adh_root_node, 0);
    fprintf(stdout, "\n");
}

void print_node_array() {
    log_debug("print_node_array", "\n");
    for (int i=0; i<last_index_of_node_array;i++){
        log_debug("", "%3i  %s \n", i, fmt_node(adh_node_array[i]));
    }
}