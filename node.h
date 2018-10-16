#ifndef ALGO_NODE_H
#define ALGO_NODE_H

#include "constants.h"

struct Node {
    unsigned short value;
    unsigned int weight;
    unsigned short order;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
};
typedef struct Node Node;

Node * createNYT();

Node * createNode(unsigned short value);

void destroyNode(Node * node);

int initializeTree();

void destroyTree();

void updateTree(Node * node, bool isNewNode);

Node * searchCharInTree(unsigned short ch);

Node * searchCharFromNode(Node * node, unsigned short ch);

Node * createNodeAndAppend(unsigned short ch);

int getNYTCode(unsigned char bit_array[]);
int getSymbolCode(unsigned short ch, unsigned char bit_array[]);

#endif //ALGO_NODE_H
