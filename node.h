//
// Created by massi on 12.10.2018.
//

#ifndef ALGO_NODE_H
#define ALGO_NODE_H

#include <stdlib.h>


#define MAX_ORDER 512
//const unsigned short MAX_ORDER = 512;

struct Node {
    char value;
    unsigned int weight;
    unsigned short order;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
};
typedef struct Node Node;

Node* createNode(const char value);
void destroyNode(Node *node);
int initializeTree();
void destroyTree();
Node *searchCharInTree(char ch);
void addNewNode(char ch);

#endif //ALGO_NODE_H
