#ifndef ALGO_ADHUFF_COMMON_H
#define ALGO_ADHUFF_COMMON_H

#include "constants.h"

/*
 * Header of compressed file
 */
#pragma pack(1)
typedef struct
{
    byte_t     data: HEADER_DATA_BITS;    // rest of the bits for data
    byte_t     header: HEADER_BITS;       // bits for header
} first_byte_struct;

typedef union
{
    first_byte_struct   split;
    byte_t              raw;
} first_byte_union;
#pragma pack()

/*
 * A symbol in adh:
 * - BYTE     = [0..255]
 * - NYT      = 256        // Not Yet Transmitted
 * - OLD_NYT  = 257
 */
typedef uint16_t    adh_symbol_t;
typedef uint16_t    adh_order_t;
typedef uint32_t    adh_weight_t;

/*
 * adh_node_t struct
 */
typedef struct adh_node {
    adh_symbol_t        symbol;
    adh_weight_t        weight;
    adh_order_t         order;
    struct adh_node *   left;
    struct adh_node *   right;
    struct adh_node *   parent;
} adh_node_t;

static const adh_symbol_t   ADH_NYT_CODE = MAX_CODE_BITS + 1;
static const adh_symbol_t   ADH_OLD_NYT_CODE = ADH_NYT_CODE + 1;

int             adh_init_tree();
void            adh_destroy_tree();
void            adh_update_tree(adh_node_t *node, bool is_new_node);
adh_node_t *    adh_search_symbol_in_tree(adh_symbol_t symbol);
adh_node_t *    adh_create_node_and_append(adh_symbol_t symbol);
int             adh_get_NYT_encoding(byte_t bit_array[]);
int             adh_get_symbol_encoding(adh_symbol_t symbol, byte_t bit_array[]);

#endif //ALGO_ADHUFF_COMMON_H
