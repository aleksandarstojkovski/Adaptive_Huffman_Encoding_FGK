#ifndef ALGO_NODE_H
#define ALGO_NODE_H

struct Node {
    short value;
    unsigned int weight;
    unsigned short order;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
};
typedef struct Node Node;

Node * createNYT();

Node * createNode(short value);

void destroyNode(Node * node);

int initializeTree();

void destroyTree();

void updateTree(Node * node);

Node * searchCharInTree(short ch);

Node * searchCharFromNode(Node * node, short ch);

Node * createNodeAndAppend(short ch);

int getNYTCode(unsigned char bit_array[]);
int getSymbolCode(short ch, unsigned char bit_array[]);

#endif //ALGO_NODE_H
