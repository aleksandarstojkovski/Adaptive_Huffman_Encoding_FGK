#ifndef ALGO_NODE_H
#define ALGO_NODE_H

//const unsigned short MAX_ORDER = 512;
#define MAX_ORDER 512

struct Node {
    short value;
    unsigned int weight;
    unsigned short order;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
};
typedef struct Node Node;

static Node * adh_root_node;
static Node * adh_nyt_node;

Node * createNYT();

Node * createNode(short value);

void destroyNode(Node *node);

int initializeTree();

void destroyTree();

Node * searchCharInTree(short ch);
Node * searchCharFromNode(Node * node, short ch);

void addNewNode(short ch);

#endif //ALGO_NODE_H
