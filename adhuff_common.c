#include <stdio.h>
#include <string.h>

#include "adhuff_common.h"
#include "bin_io.h"
#include "log.h"

/**
 * constants
 */
enum {
    MAX_ORDER = MAX_CODE_BITS*2+1, //513
    HASH_BUCKETS = 7919 //TODO: tune it. prime number or power of 2 ? see https://en.wikipedia.org/wiki/Hash_table
};

/**
 * hash entry for hash table
 */
typedef struct hash_entry {
    adh_node_t*             value;
    struct hash_entry*      next;
    struct hash_entry*      prev;
} hash_entry_t;

/**
 * hash table for quick search of nodes
 */
typedef struct {
    int             length;
    //hash_entry_t    **buckets;            //heap
    hash_entry_t    *buckets[HASH_BUCKETS]; //stack
} hash_table_t;


//
// module variables
//
static adh_order_t          adh_next_order;
static adh_node_t *         adh_root_node = NULL;
static adh_node_t *         adh_nyt_node = NULL;
static adh_node_t *         symbol_node_array[MAX_CODE_BITS] = {0};
static hash_table_t         map_weight_nodes = {0};

#ifdef _DEBUG
static long                 collision = 0;
#endif

//
// private methods
//
int             init_tree();
void            destroy_tree();
adh_node_t*     create_nyt();
adh_node_t*     create_node(adh_symbol_t symbol);
void            destroy_node(adh_node_t *node);
void            update_node_encoding(adh_node_t *node);
void            increase_weight(adh_node_t *node);

void            hash_init();
void            hash_release();
void            hash_add(adh_node_t* node, hash_entry_t* entry);
hash_entry_t*   hash_detach_entry(adh_node_t *node);
unsigned int    hash_get_index(adh_weight_t weight);
adh_node_t*     hash_get_value(adh_weight_t weight, adh_order_t order);
void            hash_check_collision(adh_weight_t weight, int hash_index, const adh_node_t *node);

/**
 * get NYT node
 */
adh_node_t*     get_nyt() {
    return adh_nyt_node;
}

/**
 * Initialize the structure for Adaptive Huffman algorithm
 * @param input_file_name
 * @param output_file_name
 * @param output_file_ptr
 * @param input_file_ptr
 * @return RC_OK / RC_FAIL
 */
int adh_init(const char input_file_name[], const char output_file_name[],
             FILE **output_file_ptr, FILE **input_file_ptr) {
    int rc = init_tree();
    if(rc == RC_OK) {
        hash_init();
    }

    *input_file_ptr = bin_open_read(input_file_name);
    if ((*input_file_ptr) == NULL) {
        rc = RC_FAIL;
    }

    if(rc == RC_OK) {
        (*output_file_ptr) = bin_open_create(output_file_name);
        if ((*output_file_ptr) == NULL) {
            rc = RC_FAIL;
        }
    }

    return rc;
}

/**
 * Release allocated resources
 * @param output_file_ptr
 * @param input_file_ptr
 */
void adh_release(FILE *output_file_ptr, FILE *input_file_ptr) {
    if(output_file_ptr) {
        fclose(output_file_ptr);
    }

    if(input_file_ptr) {
        fclose(input_file_ptr);
    }

    destroy_tree();
    hash_release();
}

/**
 * Initialize the tree with a single node: the NYT
 */
int init_tree() {
#ifdef _DEBUG
    log_trace("adh_init_tree", "\n");
#endif

    for (int i = 0; i < MAX_CODE_BITS; ++i) {
        symbol_node_array[i] = NULL;
    }

    adh_next_order = MAX_ORDER;
    if(adh_root_node != NULL) {
        perror("init_tree: root already initialized");
        return RC_FAIL;
    }

    adh_nyt_node = adh_root_node = create_nyt();
    return RC_OK;
}

/**
 * Destroy Tree and reset pointers
 */
void destroy_tree() {
#ifdef _DEBUG
    log_trace("adh_destroy_tree", "\n");
#endif

    destroy_node(adh_root_node);
    adh_root_node = NULL;
    adh_nyt_node = NULL;
}

/**
 * release resources for the passed node and its children
 * @param node: should start from the root
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

/**
 * Create a new node and append it to the NYT (Not Yet Transmitted)
 * NB: this method must be used only for new symbols (not present in the tree)
 * @param symbol
 * @return the new node, NULL in case of error
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
        increase_weight(newNode);
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

/**
 * create a new node with the NYT code
 * @return the new node
 */
adh_node_t * create_nyt() {
#ifdef _DEBUG
    log_trace("    create_nyt", "%s (0,%d)\n", fmt_symbol(ADH_NYT_CODE), adh_next_order);
#endif

    return create_node(ADH_NYT_CODE);
}

/**
 * create a new node and initialize it
 * @param symbol: the symbol that the node will store
 * @return the new node, NULL in case of error
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

    // if the new node is a symbol node
    // save its reference in the symbol_node_array to improve searches
    if(symbol > ADH_NYT_CODE)
        symbol_node_array[symbol] = node;

    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->order = adh_next_order;
    node->weight = 0;
    node->symbol = symbol;
    memset(&(node->bit_array), 0, sizeof(bit_array_t));

    adh_next_order--;

    return node;
}

/**
 * search for a node with the given weight and an higher order
 * @param weight
 * @param order
 * @return a node that respect the given criteria. NULL if not found
 */
adh_node_t * find_higher_order_same_weight(adh_weight_t weight, adh_order_t order) {
    // small optimization: only NYT and new nodes have weight 0
    // so they are already ordered, don't swap
    if(weight == 0)
        return NULL;

    return hash_get_value(weight, order);
}

/**
 * search a node that contains the given symbol
 * @param symbol
 * @return the node that respect the given criteria. NULL if not found
 */
adh_node_t * adh_search_symbol_in_tree(adh_symbol_t symbol) {
#ifdef _DEBUG
    log_trace("  adh_search_symbol_in_tree", "%s\n", fmt_symbol(symbol));
#endif
    return symbol_node_array[symbol];
}

/**
 * swap the two nodes, original order attribute is preserved
 * @param node1
 * @param node2
 */
void swap_nodes(adh_node_t *node1, adh_node_t *node2){
    if (node1->parent == node2 || node2->parent == node1) {
        //log_info("swap_nodes", " TRYING TO SWAP NODE WITH ITS PARENT\n");
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

/**
 * Update Tree, fix sibling property
 * @param node: the node that has been updated
 * @param is_new_node: true if node is new, false if node is not new.
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
        increase_weight(node_to_check);

        // continue ascending the tree
        node_to_check = node_to_check->parent;
    }

    increase_weight(node_to_check);

#ifdef _DEBUG
    log_tree();
#endif
}

/**
 * calculate the encoded symbol of passed node and its children
 * fill bit_array from left (MSB) to right (LSB)
 * 0 = left node, 1 = right node
 * @param node
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

/**
 * calculate the node level (DEBUG purpose)
 * @param node
 * @return the level
 */
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

/**
 * search for a leaf node that is represented by the given bit array
 * @param bit_array
 * @return the node if found, NULL otherwise
 */
adh_node_t* adh_search_leaf_by_encoding(const bit_array_t *bit_array) {
    adh_node_t* nextNode = adh_root_node;
    for(int i = bit_array->length-1; i >= 0 && nextNode; i--) {
        if(bit_array->buffer[i] == BIT_1) {
            nextNode = nextNode->right;
        } else
            nextNode = nextNode->left;
    }

    // if it's a leaf return it
    if(nextNode && nextNode->right == NULL && nextNode->left == NULL)
        return nextNode;

    return NULL;
}

/**
 * increase the weight of the given node and update the hashing table
 * @param node
 */
void increase_weight(adh_node_t *node) {
    if(node == NULL)
        return;

    hash_entry_t* entry = hash_detach_entry(node);
    node->weight++;
    hash_add(node, entry);
}


/**
 * print in a nice way the current status of the tree (DEBUG purpose)
 */
void print_tree() {
    print_sub_tree(adh_root_node, 0);
    fprintf(stdout, "\n");
}

/**
 * recursively print the node and its children (DEBUG purpose)
 * @param node
 * @param depth
 */
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

/**
 * calculate the index of hash table associative array
 * @param weight
 * @return the index
 */
inline unsigned int hash_get_index(adh_weight_t weight) {
    return weight % HASH_BUCKETS;
}

/**
 * initialize the hash table
 */
void hash_init() {
    map_weight_nodes.length = HASH_BUCKETS;
    //moved to stack with fixed size
    //map_weight_nodes.buckets = calloc(HASH_BUCKETS, HASH_BUCKETS * sizeof(map_weight_nodes.buckets));
    memset(&map_weight_nodes.buckets, 0, sizeof(map_weight_nodes.buckets));
}

/**
 * release resources used by the hash table
 */
void hash_release() {
    for (int i = 0; i < map_weight_nodes.length; ++i) {
        hash_entry_t  *entry = map_weight_nodes.buckets[i];
        while(entry) {
            hash_entry_t  *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    //moved to stack with fixed size
    //free(map_weight_nodes.buckets);

#ifdef _DEBUG
    fprintf(stdout, "total number of collision: %ld\n", collision);
    collision = 0;
#endif
}

/**
 * detach the entry storing the given node, so we can reuse it in the next hash_add
 * @param node: a node that was previously added to the hash table
 * @return the entry or NULL if the node is not stored in the hash table.
 */
hash_entry_t* hash_detach_entry(adh_node_t *node) {
    int hash_index = hash_get_index(node->weight);

    hash_entry_t  *entry = map_weight_nodes.buckets[hash_index];
    while(entry) {
        if(entry->value == node) {
            if(entry->prev == NULL) {
                map_weight_nodes.buckets[hash_index] = entry->next;
            }
            else {
                entry->prev->next = entry->next;
            }

            if(entry->next)
                entry->next->prev = entry->prev;

            entry->prev = NULL;
            entry->next = NULL;
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

/**
 * add the node in the hash table.
 * the key will be the node weight, the value will be the node
 * @param node: the value of the hash table
 * @param entry: if NULL a new entry wil be created. if provided it will reuse it
 */
void hash_add(adh_node_t* node, hash_entry_t *entry){
    int hash_index = hash_get_index(node->weight);

    hash_entry_t  *new_entry = entry != NULL ? entry : calloc(1, sizeof(hash_entry_t));
    new_entry->value = node;

    hash_entry_t  *last = map_weight_nodes.buckets[hash_index];
    if(last == NULL) {
        map_weight_nodes.buckets[hash_index] = new_entry;
    }
    else {
        while(last->next) {
            last = last->next;
        }

        last->next = new_entry;
        new_entry->prev = last;
    }
}

/**
 * search in the hash table, a node with given weight and a higher order (skip root)
 * @param weight
 * @param order
 * @return the node that respects the criteria. otherwise NULL
 */
adh_node_t* hash_get_value(adh_weight_t weight, adh_order_t order) {
    adh_node_t* node_result = NULL;
    int hash_index = hash_get_index(weight);
    hash_entry_t* entry = map_weight_nodes.buckets[hash_index];
    while(entry) {
        adh_node_t* current_node = entry->value;

        if(current_node->weight != weight) {
#ifdef _DEBUG
            collision++;
            hash_check_collision(weight, hash_index, current_node);
#endif
        }
        else if(current_node->order > order && current_node != adh_root_node) {
            node_result = current_node;
            order = node_result->order;
        }
        entry = entry->next;
    }
    return node_result;
}

/**
 * utility function to log number of collisions (only DEBUG purpose)
 * @param weight
 * @param hash_index
 * @param node
 */
void hash_check_collision(adh_weight_t weight, int hash_index, const adh_node_t *node) {
    if(node->weight != weight) {
        int size = 0;
        hash_entry_t* he = map_weight_nodes.buckets[hash_index];
        while(he != NULL) {
            size++;
            he = he->next;
        }
        log_info("hash_check_collision", "collision, size:%d w1:%d w2:%d\n", size, weight, node->weight);
    }
}
