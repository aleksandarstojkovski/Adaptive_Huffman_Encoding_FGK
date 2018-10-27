#ifndef ALGO_ADHUFF_COMMON_H
#define ALGO_ADHUFF_COMMON_H

#include "constants.h"

/*
 * Header of compressed file
 */
#pragma pack(1)
typedef struct
{
    unsigned int        data: HEADER_DATA_BITS;    // rest of the bits for data
    unsigned int        header: HEADER_BITS;       // bits for header
} first_byte_struct;

typedef union
{
    first_byte_struct   split;
    uint8_t             raw;
} first_byte_union;
#pragma pack()

/*
 * A symbol in adh:
 * - BYTE     = [0..255]
 * - NYT      = 256        // Not Yet Transmitted
 * - OLD_NYT  = 257
 */
typedef unsigned short  adh_symbol_t;

/*
 * adh_node_t struct
 */
typedef struct adh_node {
    adh_symbol_t        symbol;
    unsigned int        weight;
    unsigned short      order;
    struct adh_node *   left;
    struct adh_node *   right;
    struct adh_node *   parent;
} adh_node_t;

//
// public methods
//

int             adh_init_tree();
void            adh_destroy_tree();
void            adh_update_tree(adh_node_t *node, bool is_new_node);
adh_node_t *    adh_search_symbol_in_tree(adh_symbol_t ch);
adh_node_t *    adh_create_node_and_append(adh_symbol_t ch);
int             adh_get_NYT_encoding(uint8_t bit_array[]);
int             adh_get_symbol_encoding(unsigned short ch, uint8_t bit_array[]);

#endif //ALGO_ADHUFF_COMMON_H
