#ifndef STRUCT_NODE
#define STRUCT_NODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "debug.h"

enum {Program, ExtDefList, ExtDef, ExtDecList,
    Specifier, StructSpecifier, OptTag, Tag,
    VarDec, FunDec, VarList, ParamDec,
    CompSt, StmtList, Stmt,
    DefList, Def, DecList, Dec,
    Exp, Args} terminal;

typedef struct node_t{
    struct node_t* child;
    struct node_t* brother;
    char name[32];
    char str[64];
    int is_token;
    int lineno;
    int token_val;
    int production_id;
}node_t;

node_t *create_token_node(char *name, char *str);
node_t *create_other_node(char *name,int lineno, int terminal_token, int production_id);
node_t *trans_tree(char *name, int terminal_token, int production_id, int lineno, int num, ...);
node_t *insert_node(node_t *root, node_t *node);
void print_tree(node_t *root, int depth);
void SddProgram         (node_t *node);

int str2int(char *str);

extern int get_syn_error;
extern int error_lineno;
extern node_t* CST;
#endif