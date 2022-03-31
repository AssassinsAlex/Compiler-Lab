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
#define Assert(expr) if(expr) {};
#endif

#define false 0
#define true 1


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

Type malloc_type(int kind1, int kind2);
void free_type(Type type);
int array_com(Type dst, Type src);
FieldList creat_field(char *name, Type inh);
void free_field(FieldList field);
int field_com(FieldList dst, FieldList src);
typedef struct symbol_* symbol;
struct symbol_
{
    enum {VARIABLE, FUNCTION, STRUCT_TAG} kind;
    union {
        Type variable;
        Type struct_tag;
        struct {int defined; Type ret; FieldList parameter;} func;
    }u;
    char name[NAME_SIZE];
    symbol hash_nxt;
    symbol list_nxt;
};

symbol add_symbol(char* name, Type inh, int sym_kind);
void free_symbol(symbol sym);

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


void semanitic_error(int errorno, int lineno, char* str);
#endif
