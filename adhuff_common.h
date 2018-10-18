#ifndef ALGO_ADHUFF_COMMON_H
#define ALGO_ADHUFF_COMMON_H

#include "constants.h"

/*
 * Header of compressed file
 */
#pragma pack(1)
typedef struct
{
    unsigned int data: HEADER_DATA_BITS;    // rest of the bits for data
    unsigned int header: HEADER_BITS;       // bits for header
} first_byte_struct;

typedef union
{
    first_byte_struct split;
    unsigned char raw;
} first_byte_union;
#pragma pack()

/*
 * Node struct
 */
struct Node {
    unsigned short value;
    unsigned int weight;
    unsigned short order;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
};
typedef struct Node Node;

//
// public methods
//
int     initializeTree();
void    destroyTree();

void    updateTree(Node * node, bool isNewNode);

Node *  searchCharInTree(unsigned short ch);

Node *  createNodeAndAppend(unsigned short ch);

int     getNYTCode(unsigned char bit_array[]);
int     getSymbolCode(unsigned short ch, unsigned char bit_array[]);


#endif //ALGO_ADHUFF_COMMON_H
