#include "multitree.h"

int error_lineno = 0;
int get_syn_error = 0;
node_t *CST;

node_t *trans_tree(char *name, int lineno, int num, ...){
    node_t *root = create_other_node(name, lineno);
    va_list valist;
    va_start(valist, num);

    for (int i = 0; i < num; i++){
        node_t *node = va_arg(valist, node_t*);
        int cur_lineno = va_arg(valist, int);
        if(node != NULL) {
          node->lineno = cur_lineno;
          root = insert_node(root, node);
        }
    }

    va_end(valist);
    return root;
}

node_t *create_token_node(char *name, char *str){
    node_t *node = malloc(sizeof(node_t));
    node->brother = NULL;
    node->child = NULL;
    strncpy(node->name, name, 32);
    if(str != NULL)
        strncpy(node->str, str, 64);
    node->is_token = 1;
    node->lineno = 0;
    return node;
}
node_t *create_other_node(char *name, int lineno){
    node_t *node = malloc(sizeof(node_t));
    node->brother = NULL;
    node->child = NULL;
    strncpy(node->name, name, 32);
    memset(node->str, 0, sizeof(node->str));
    node->is_token = 0;
    node->lineno = lineno;
    return node;
}

node_t *insert_node(node_t *root, node_t *node){
    if(node)
    if(!root->child){
        root->child = node;
    }else{
        node_t *cur = root->child;
        while(cur->brother != NULL){
        cur = cur->brother;
        }cur->brother = node;
    };
    return root;
}

void print_str(char *str){
    if(str[0] == '0'){
        if(strlen(str) > 1 && (str[1] == 'x' || str[1] == 'X'))
            printf("%ld\n", strtol(str, NULL, 16));
        else
            printf("%ld\n", strtol(str, NULL, 8));
    }else
        printf("%d\n", atoi(str));
}

void print_tree(node_t *root, int depth){
    for(int i = 0; i < depth; i++){
        putchar(' ');putchar(' ');
    }
    if(!root->is_token)
        printf("%s (%d)\n", root->name, root->lineno);
    else{
        if(!strcmp(root->name, "TYPE") | !strcmp(root->name, "ID"))
            printf("%s: %s\n",root->name, root->str);
        else if(!strcmp(root->name, "INT")){
            printf("%s: ", root->name);
            print_str(root->str);
        }
        else if(!strcmp(root->name, "FLOAT"))
            printf("%s: %f\n",root->name, atof(root->str));
        else
            printf("%s\n", root->name);
    }
    if(root->child)
        print_tree(root->child, depth+1);
    node_t *cur = root->brother;
    if(cur != NULL)
        print_tree(cur, depth);
}

