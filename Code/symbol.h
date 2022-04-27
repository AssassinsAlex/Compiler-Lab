//
// Created by asassin on 22-4-24.
//

#ifndef SYMBOL_H
#define SYMBOL_H

#include "multitree.h"

#define CHILD_1(node) node->child
#define CHILD_2(node) CHILD_1(node)->brother
#define CHILD_3(node) CHILD_2(node)->brother
#define CHILD_4(node) CHILD_3(node)->brother
#define CHILD_5(node) CHILD_4(node)->brother
#define CHILD_6(node) CHILD_5(node)->brother
#define CHILD_7(node) CHILD_6(node)->brother

#define CHILD(id, node) CHILD_##id(node)

#define NAME_SIZE 40
/* symbol table */

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
struct Type_{
    // Type 的放规则:
    // 对于BASIC的变量以及函数参数,返回值不会被释放
    // ARRAY 可以被释放
    // struct 匿名variable或者OptTag会被释放
    // locked 将basic 和 struct tag 锁住不会被释放
    // func_locked 将函数调用的参数和返回值锁住,这些都不会被释放
    enum { BASIC, ARRAY, STRUCTURE} kind;
    int locked;
    int func_locked;
    union{
        // 基本类型
        enum{TINT, TFLOAT} basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct { Type elem; int size; } array;
        // 结构体类型信息是一个链表
        FieldList structure;
    } u;
};

struct FieldList_{
    char name[NAME_SIZE]; // 域的名字
    Type type; // 域的类型
    int offset;
    FieldList tail;
};
/*
struct list_int{
    int lineno;
    struct list_int *nxt;
};
*/

typedef struct symbol_* symbol;
struct symbol_
{
    enum {VARIABLE, FUNCTION, STRUCT_TAG} kind;
    union {
        Type variable;
        Type struct_tag;
        struct {
            int defined;
            Type ret;
            FieldList parameter;
            //struct list_int *func_used;
        } func;
    }u;
    int no;
    int offset;
    char name[NAME_SIZE];
    int first_lineno;
    void *belong;
    symbol hash_nxt;
    symbol list_nxt;
};

/* hash table */
#define HASH_SIZE 0x3fff
extern symbol symbol_list[HASH_SIZE];

/* orthogonal list */
typedef struct EnvNode_* EnvNode;
struct EnvNode_{
    symbol sym;
    EnvNode nxt;
    int offset;

};
extern EnvNode envs;
extern int is_semantic_error;

Type            type_malloc     (int kind1, int kind2);
void            type_free       (Type type);
int             type_com        (Type dst, Type src);
int             type_size       (Type type);
int             array_com       (Type dst, Type src);

FieldList       field_malloc    (char *name, Type inh);
void            field_free      (FieldList field);
Type            field_find      (char *name, FieldList field);
int             field_com       (FieldList dst, FieldList src);

symbol          symbol_add      (node_t *node, Type inh, int sym_kind);
void            symbol_free     (symbol sym);

unsigned int    hash_pjw        (char *name);
void            hash_insert     (symbol sym);
symbol          hash_find       (char* name);
int             hash_delete     (symbol sym);

void            env_push        ();
void            env_insert_sym (symbol sym);
void            env_pop        ();
void            env_init       ();

void            fun_unlock      (FieldList field);
void            semantic_error  (int errorno, int lineno, char* str);

#endif //SYMBOL_H
