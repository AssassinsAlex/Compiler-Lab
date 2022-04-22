#ifndef SEMANTIC_H
#define SEMANTIC_H
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "multitree.h"



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
typedef struct list_node_* list_node;
struct list_node_{
    symbol sym;
    list_node nxt;
};

Type            type_malloc     (int kind1, int kind2);
void            type_free       (Type type);
int             type_com        (Type dst, Type src);
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

list_node       list_create     ();
void            list_insert     (list_node node);
void            list_insert_sym (symbol sym);
void            list_pop        ();

void    SddProgram          (node_t *node);
void    SddExtDefList       (node_t *node);
void    SddExtDef           (node_t *node);
void    SddExtDecList       (node_t *node, Type inh);
Type    SddSpecifier        (node_t *node);
Type    SddStructSpecifier  (node_t *node);
void    SddVarDec           (node_t *node, Type inh, Type structure);
void    SddFunDef           (node_t *node, Type inh);
void    SddFunDec           (node_t *node, Type inh);
void    SddVarList          (node_t *node);
void    SddParamDec         (node_t *node);
void    SddDefList          (node_t *node, Type structure);
void    SddDef              (node_t *node, Type structure);
void    SddDecList          (node_t *node, Type inh, Type structure);
void    SddDec              (node_t *node, Type inh, Type structure);
void    SddCompSt           (node_t *node, int isFun, Type ret);
void    SddStmtList         (node_t *node, Type ret);
void    SddStmt             (node_t *node, Type ret);
Type    SddExp              (node_t *node, int isLeft);
void    SddArgs             (node_t *node, FieldList ArgList);
Type    SddId               (node_t *node);
Type    SddType             (node_t *node);
Type    SddArray            (Type inh, int size);
void    SddReturn           (node_t *node, Type ret);
Type    SddExpFun           (node_t *node, int isLeft);
Type    SddExpArray         (node_t *node, int isLeft);
Type    SddExpStruct        (node_t *node, int isLeft);

Type    CheckInt2           (Type type1, Type type2, int lineno);
Type    CheckInt1           (Type type, int lineno);
Type    CheckAssign         (Type type1, Type type2, int lineno);
Type    CheckArithm2        (Type type1, Type type2, int lineno);
Type    CheckArithm1        (Type type, int lineno);
void    CheckFun            ();

void semantic_error(int errorno, int lineno, char* str);

#endif
