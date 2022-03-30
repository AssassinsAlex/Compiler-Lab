#ifndef SEMANTIC_H
#define SEMANTIC_H
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ASSERT_ON

#ifdef ASSERT_ON
#define Assert(expr) \
    do{\
        assert(expr);\
    }while(0)
#else
#define Assert(expr) expr
#endif



#define NAME_SIZE 40
/* symbol table */

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
struct Type_{
    enum { BASIC, ARRAY, STRUCTURE } kind;
    union{   
        // 基本类型
        struct {enum{TINT, TFLOAT} type; union {int intval; float floatval;} val;}basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct { Type elem; int size; } array;
        // 结构体类型信息是一个链表
        FieldList structure;
    } u;
};

struct FieldList_{
    char* name; // 域的名字
    Type type; // 域的类型
    FieldList tail;
};

typedef struct symbol_* symbol;

struct symbol_
{
    enum {VARIBLE, FUNCTION, STRUCT_TAG} kind;
    union {
        Type varible;
        Type sturct_tag;
        struct {Type ret; FieldList parameter;} func;
    }u;
    char name[NAME_SIZE];
    symbol hash_nxt;
    symbol list_nxt;
};
void create_symbol(int kind, char *name);

/* hash table */
#define HASH_SIZE 0x3fff
extern symbol symbol_list[HASH_SIZE];

unsigned int hash_pjw(char *name);

void hash_insert(symbol sym);

int hash_delete(symbol sym);

symbol hash_find(char* name);

/* orthogonal list */
typedef struct list_node_* list_node;

struct list_node_{
    symbol sym;
    list_node nxt;
};
extern list_node list_head;

list_node list_create();

void list_insert(list_node node);

void list_insert_sym(symbol sym);

void list_pop();

#endif
