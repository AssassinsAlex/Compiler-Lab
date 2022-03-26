#ifndef STRUCT_NODE
#define STRUCT_NODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

typedef struct node_t{
    struct node_t* child;
    struct node_t* brother;
    char name[32];
    char str[64];
    int is_token;
    int lineno;
}node_t;

node_t *create_token_node(char *name, char *str);
node_t *create_other_node(char *name, int lineno);
node_t *trans_tree(char *name, int lineno, int num, ...);
node_t *insert_node(node_t *root, node_t *node);
void print_tree(node_t *root, int depth);

extern int get_error;
extern int error_lineno;
#endif