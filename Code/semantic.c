#include "semantic.h"
/* symbol table */

void create_symbol(int kind, char *name){
  symbol sym = malloc(sizeof(struct symbol_));
  strncpy(sym->name, name, NAME_SIZE);
  sym->kind = TYPE;
  Type t = malloc(sizeof(struct Type_));
  t->kind = kind;
  t->u.basic = 1;
  sym->u.varible = t;

  list_insert_sym(sym);
  hash_insert(sym);
}

unsigned int hash_pjw(char *name){
    unsigned int val = 0, i;
    for(; *name; ++name){
        val = (val << 2) + *name;
        if((i = val) & ~HASH_SIZE) val = (val ^ (i >> 12));
    }
    return val;
}

symbol symbol_list[HASH_SIZE];

void hash_insert(symbol sym){
    unsigned int key = hash_pjw(sym->name);
    sym->hash_nxt = symbol_list[key];
    symbol_list[key] = sym;
}

symbol hash_find(char *name){
    unsigned int key = hash_pjw(name);
    symbol cur = symbol_list[key];
    while(cur != NULL){
        if(!strncmp(cur->name, name, NAME_SIZE))
            return cur;
        cur=cur->hash_nxt;
    }
    return NULL;
}

int hash_delete(symbol sym){
    unsigned int key = hash_pjw(sym->name);
    if(symbol_list[key] == sym){
        symbol_list[key] = sym->hash_nxt;
        return 1;
    }
    symbol cur = symbol_list[key];
    while(cur->hash_nxt != NULL){
        if(cur->hash_nxt == sym){
            cur->hash_nxt = sym->hash_nxt;
            return 1;
        }cur = cur->hash_nxt;
    }
    return 0;
}

/* orthogonal list */

list_node list_head = NULL;

list_node list_create(){
    list_node new = malloc(sizeof(struct list_node_));
    new->sym = NULL;
    new->nxt = NULL;
    return new;
}

void list_insert(list_node node){
    node->nxt = list_head;
    list_head = node;
}

void list_insert_sym(symbol sym){
    if(list_head == NULL){
        list_head = list_create();
    }sym->list_nxt = list_head->sym;
    list_head->sym = sym;
}

void list_pop(){
    while(list_head->sym != NULL){
        symbol tmp = list_head->sym;
        list_head->sym = tmp->list_nxt;
        hash_delete(tmp);
        free(tmp);
    }
    list_node tmp = list_head;
    list_head = list_head->nxt;
    free(tmp);
    assert(list_head);
}
