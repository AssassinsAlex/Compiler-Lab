//
// Created by asassin on 22-4-24.
//
#include "symbol.h"

static Type INT_Type_const = NULL;
static Type FLOAT_Type_const = NULL;
Type Error_Type = NULL;

symbol symbol_list[HASH_SIZE] = {0};
EnvNode envs = NULL;

void semantic_error(int errorno, int lineno, char* str){
    printf("Error type %d at Line %d: %s.\n", errorno, lineno, str);
}

Type type_malloc(int kind1, int kind2){
    if(kind1 == BASIC){
        if(kind2 == TINT){
            return INT_Type_const;
        }else if(kind2 == TFLOAT){
            return FLOAT_Type_const;
        }else{
            Assert(0);
        }
    }else{
        Type ret = (Type)malloc(sizeof(struct Type_));
        ret->kind = kind1;
        ret->locked = false;
        ret->func_locked = false;
        return  ret;
    }
}

void type_free(Type type){
    if(type->func_locked) return;
    if(type->locked) return;
    switch (type->kind) {
        case BASIC:
            // BASIC shouldn't free;
            Assert(0);break;
        case ARRAY:
            type_free(type->u.array.elem);
            free(type);
            break;
        case STRUCTURE:
            field_free(type->u.structure);
            free(type);
            break;
        default:
            Assert(0);
    }
}

int type_com(Type dst, Type src){
    Assert( dst != NULL && src != NULL);
    if(dst == Error_Type || src == Error_Type) return true;
    if(dst->kind != src->kind) return false;
    switch (dst->kind) {
        case BASIC:
            return dst->u.basic == src->u.basic;
        case ARRAY:
            return array_com(dst, src);
        case STRUCTURE:
            return field_com(dst->u.structure, src->u.structure);
        default:
            Assert(0);
    }
}

int array_com(Type dst, Type src){
    //[2][3]  [3][2]
    int dim1 = 0, dim2 = 0;
    while (dst->kind == ARRAY){
        dim1++;
        dst = dst->u.array.elem;
    }
    while(src->kind == ARRAY){
        dim2++;
        src = src->u.array.elem;
    }
    if(dim1 != dim2) return false;
    return type_com(dst, src);
}

FieldList field_malloc(char *name, Type inh){
    FieldList cur_field = (FieldList)malloc(sizeof(struct FieldList_));
    strncpy(cur_field->name, name, NAME_SIZE);
    cur_field->type = inh;
    cur_field->tail = NULL;
    return cur_field;
}

void field_free(FieldList field){
    if(field == NULL) return;
    field_free(field->tail);
    type_free(field->type);
    free(field);
}

Type field_find(char *name, FieldList field){
    while(field != NULL){
        if(!strncmp(name, field->name, NAME_SIZE))
            return field->type;
        field = field->tail;
    }return NULL;
}

int field_com(FieldList dst, FieldList src){
    if(dst == NULL || src == NULL) return dst == src;
    else{
        if(type_com(dst->type, src->type))
            return field_com(dst->tail, src->tail);
        return false;
    }
}

symbol symbol_add(node_t *node, Type inh, int sym_kind){
    if(inh == NULL || inh == Error_Type) return NULL;
    symbol OldSym = hash_find(node->str);
    if(OldSym && OldSym->belong == (void *)envs){
        switch (sym_kind) {
            case FUNCTION:
                Assert(0);break;
            case VARIABLE:
                semantic_error(3, node->lineno, "variable redefined");
                break;
            case STRUCT_TAG:
                semantic_error(16, node->lineno, "struct name has been used");
                break;
            default:
                Assert(0);break;
        }return NULL;
    }
    symbol sym = (symbol)malloc(sizeof(struct symbol_));
    strncpy(sym->name, node->str, NAME_SIZE);
    sym->kind = sym_kind;
    sym->first_lineno  = node->lineno;
    switch (sym_kind) {
        case FUNCTION:
            sym->u.func.defined = false;
            sym->u.func.ret = inh;
            sym->u.func.parameter = NULL;
            //sym->u.func.func_used = NULL;
            break;
        case VARIABLE:
            sym->u.variable = inh;
            break;
        case STRUCT_TAG:
            sym->u.struct_tag = inh;
            break;
        default:
            Assert(0);
    }
    env_insert_sym(sym);
    sym->belong = envs;
    hash_insert(sym);
    return sym;
}

void fun_unlock(FieldList field){
    if(field == NULL)
        return;
    else{
        field->type->func_locked = false;
        fun_unlock(field->tail);
    }
}

void symbol_free(symbol sym){
    switch (sym->kind) {
        case VARIABLE:
            type_free(sym->u.variable);
            break;
        case FUNCTION:
        {
            sym->u.func.ret->func_locked = false;
            type_free(sym->u.func.ret);
            fun_unlock(sym->u.func.parameter);
            field_free(sym->u.func.parameter);
            //struct list_int *cur = sym->u.func.func_used;
            //while(cur){
            //    cur = cur->nxt;
            //    free(sym->u.func.func_used);
            //    sym->u.func.func_used = cur;
            //}
            break;
        }
        case STRUCT_TAG:
            sym->u.struct_tag->locked = false;
            type_free(sym->u.struct_tag);
            break;
        default:
            Assert(0);
    }free(sym);
}

unsigned int hash_pjw(char *name){
    unsigned int val = 0, i;
    for(; *name; ++name){
        val = (val << 2) + *name;
        if((i = val) & ~HASH_SIZE) val = (val ^ (i >> 12)) & HASH_SIZE;
    }
    return val;
}

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

EnvNode env_create(){
    EnvNode new = (EnvNode)malloc(sizeof(struct EnvNode_));
    new->sym = NULL;
    new->nxt = NULL;
    return new;
}

void env_insert(EnvNode node){
    node->nxt = envs;
    envs = node;
}

void env_insert_sym(symbol sym){
    if(envs == NULL){
        envs = env_create();
    }sym->list_nxt = envs->sym;
    envs->sym = sym;
}

void env_pop(){
    while(envs->sym != NULL){
        symbol tmp = envs->sym;
        envs->sym = tmp->list_nxt;
        hash_delete(tmp);
        symbol_free(tmp);
    }
    EnvNode tmp = envs;
    envs = envs->nxt;
    free(tmp);
    //Assert(envs);
}

void env_init(){
    INT_Type_const = (Type)malloc(sizeof(struct Type_));
    INT_Type_const->kind = BASIC;
    INT_Type_const->locked = true;
    INT_Type_const->func_locked = false;
    INT_Type_const->u.basic = TINT;

    FLOAT_Type_const = (Type)malloc(sizeof(struct Type_));
    FLOAT_Type_const->kind = BASIC;
    FLOAT_Type_const->locked = true;
    FLOAT_Type_const->func_locked = false;
    FLOAT_Type_const->u.basic= TFLOAT;

    Error_Type = malloc(sizeof (struct Type_));
    Error_Type->kind = BASIC;
    Error_Type->locked = true;
    Error_Type->func_locked = false;
    Error_Type->u.basic = -1;

    node_t *node = create_token_node("FunDec", "read");
    symbol sym = symbol_add(node, INT_Type_const, FUNCTION);
    sym->u.func.parameter = NULL;
    sym->u.func.defined = true;
    free(node);

    node = create_token_node("FunDec", "write");
    sym = symbol_add(node, INT_Type_const, FUNCTION);
    sym->u.func.parameter = field_malloc("x", INT_Type_const);
    sym->u.func.defined = true;
    env_insert(env_create());
}
