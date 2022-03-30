#include "semantic.h"
#include "multitree.h"
/* symbol table */
#define CHILD_1(node) node->child
#define CHILD_2(node) CHILD_1(node)->brother
#define CHILD_3(node) CHILD_2(node)->brother
#define CHILD_4(node) CHILD_3(node)->brother
#define CHILD_5(node) CHILD_4(node)->brother
#define CHILD_6(node) CHILD_5(node)->brother
#define CHILD_7(node) CHILD_6(node)->brother
#ifdef DEBUG
#endif

#define CHILD(id, node) CHILD_##id(node)

symbol add_var_symbol(char* name, Type inh){
  symbol sym = malloc(sizeof(struct symbol_));
  strncpy(sym->name, name, NAME_SIZE);
  sym->kind = VARIBLE;
  sym->u.varible = inh;

  list_insert_sym(sym);
  hash_insert(sym);
  return sym;
}

symbol add_func_symbol(node_t *node, Type inh){
    symbol sym = malloc(sizeof(struct symbol_));
    strncpy(sym->name, node->name, NAME_SIZE);
    sym->kind = FUNCTION;
    sym->u.func.ret = inh;
    sym->u.func.parameter = NULL;

    list_insert_sym(sym);
    hash_insert(sym);
    return sym;
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
    Assert(list_head);
}

void SddProgram         (node_t *node);
void SddExtDefList      (node_t *node);
void SddExtDef          (node_t *node);
void SddExtDecList      (node_t *node, Type inh);
Type SddSpecifier       (node_t *node);
Type SddStructSpecifier (node_t *node);
void SddVarDec          (node_t *node, Type inh, Type structure);
Type SddFunDec          (node_t *node, Type inh);
void SddVarList         (node_t *node, Type structure);
void SddParamDec        (node_t *node, Type structure);
void SddDefList         (node_t *node, Type structure);
void SddDef             (node_t *node, Type structure);
void SddDecList         (node_t *node, Type inh, Type structure);
void SddDec             (node_t *node, Type inh, Type structure);
void SddCompSt          (node_t *node, Type structure);
void SddStmtList        (node_t *node);
Type SddExp             (node_t *node, Type inh);

Type SddType            (node_t *node);

Type Array              (Type inh, int size);

void SddProgram(node_t* node){
    Assert(node->token_val == Program);
    /* 在此初始化一些变量 */
    list_insert(list_create());
    SddExtDefList(CHILD(1, node));
}

// ExtDefList.syn = ExtDef.syn
void SddExtDefList(node_t *node){
    if(node == NULL) return;
    Assert(node->token_val == ExtDefList);
    SddExtDef(CHILD(1, node));
    SddExtDefList(CHILD(2, node));
}

// ExtDef.syn = Specifier.syn
void SddExtDef(node_t *node){ 
    Assert(node->token_val == ExtDef);
    Type syn;
    switch (node->production_id)
    {
    case 0:
        syn = SddSpecifier(CHILD(1, node));
        SddExtDecList(CHILD(2, node), syn);
        break;
    case 1:
        syn = SddSpecifier(CHILD(1, node));
        break;
    case 2:
        syn = SddSpecifier(CHILD(1, node));
        syn = SddFunDec(CHILD(2, node), syn);
        SddCompSt(CHILD(3, node), syn);
        break;
    default:
        Assert(0);
        break;
    }
}

void SddExtDecList(node_t *node, Type inh){
    Assert(node->token_val == ExtDecList);
    switch (node->production_id)
    {
    case 0:
        SddVarDec(CHILD(1, node), inh, NULL);
        break;
    case 1:
        SddVarDec(CHILD(1, node), inh, NULL);
        SddExtDecList(CHILD(3, node), inh);
        break;
    default:
        Assert(0);
        break;
    }
}

Type SddSpecifier(node_t *node){
    Assert(node->token_val == Specifier);
    switch (node->production_id)
    {
    case 0:
        return SddType(CHILD(1, node));
    case 1:
        return SddStructSpecifier(CHILD(1, node));
    default:
        Assert(0);
        break;
    }
}

Type SddStructSpecifier(node_t *node){
    Assert(node->token_val == StructSpecifier);
    switch (node->production_id)
    {
    case 0:
    {
        Type syn = malloc(sizeof(struct Type_));
        syn->kind = STRUCTURE;
        syn->u.structure = NULL;
        if(CHILD(5, node) == NULL)
            SddDefList(CHILD(3, node), syn);
        else{
            SddDefList(CHILD(4, node), syn);
            add_var_symbol(CHILD(1, CHILD(2, node))->str, syn)->kind = STRUCT_TAG;
        }
        return syn;
    }
    case 1:
    {
        symbol syn = hash_find(CHILD(1, CHILD(2, node))->name);

        /* some error func need to do */

        return syn->u.sturct_tag;
    }
    default:
        Assert(0);
        break;
    }
}

void SddVarDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == VarDec);
    switch (node->production_id)
    {
    case 0:
    {
        Assert(!strcmp(CHILD(1, node)->name, "ID"));
        if(structure == NULL){
            symbol sym = hash_find(CHILD(1, node)->str);
            if(sym != NULL){
                printf("error: %s, redefined\n", CHILD(1, node)->name);
            }else{
                add_var_symbol(CHILD(1, node)->str, inh);
            }
        }else{
            Assert(structure->kind == STRUCTURE);
            FieldList cur_field = malloc(sizeof(struct FieldList_));
            strncpy(cur_field->name, CHILD(1,node)->name, NAME_SIZE);
            cur_field->type = inh;
            cur_field->tail = structure->u.structure->tail;
            structure->u.structure->tail = cur_field;
        }
        break;
    }   
    case 1:
    {
        Type syn = Array(inh, str2int(CHILD(3, node)->str));
        SddVarDec(CHILD(1, node), syn, structure);
        break;
    }
    default:
        Assert(0);
        break;
    }
}

Type SddFunDec(node_t *node, Type inh){
    Assert(node->token_val == FunDec);
    symbol sym = add_func_symbol(CHILD(1, node), inh);
    /* 函数暂时借助 Type Structure 的list来存放函数表 */
    switch (node->production_id)
    {
    case 0:
    {   
        Type parlist = malloc(sizeof(struct Type_));
        SddVarList(CHILD(3, node), parlist);
        sym->u.func.parameter = parlist->u.structure;
        return parlist;
    }   
    case 1:
        sym->u.func.parameter = NULL;
        return NULL;
    default:
        Assert(0);
        break;
    }
}

void SddVarList(node_t* node, Type structure){
    Assert(node->token_val == VarList);
    SddParamDec(CHILD(1, node), structure);
    switch (node->production_id)
    {
    case 0:
        SddVarList(CHILD(3, node), structure);
        break;
    case 1:
        break;
    default:
        Assert(0);
        break;
    }
}

void SddParamDec(node_t *node, Type structure){
    Assert(node->token_val == ParamDec);
    Type syn = SddSpecifier(CHILD(1, node));
    SddVarDec(CHILD(2, node), syn, structure);
}

void SddDefList(node_t *node, Type structure){
    if(node == NULL) return;
    Assert(node->token_val == DefList);
    if(node == NULL) return;
    SddDef(CHILD(1, node), structure);
    SddDefList(CHILD(2, node), structure);
}

void SddDef(node_t *node, Type structure){
    Assert(node->token_val == Def);
    Type syn = SddSpecifier(CHILD(1, node));
    SddDecList(CHILD(2, node), syn, structure);
}

void SddDecList(node_t *node, Type inh, Type structure){
    Assert(node->token_val == DecList);
    switch (node->production_id)
    {
    case 0:
        SddDec(CHILD(1, node), inh, structure);
        break;
    case 1:
    {
        SddDec(CHILD(1, node), inh, structure);
        SddDecList(CHILD(3, node), inh , structure);
        break;
    }
    default:
        Assert(0);
        break;
    }
}

void SddDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == Dec);
    switch (node->production_id)
    {
    case 0:
        SddVarDec(CHILD(1, node), inh, structure);
        break;
    case 1:
        SddVarDec(CHILD(1, node), inh, structure);
        SddExp(CHILD(3, node), inh);

        /* 做类型检查 */

        break;
    default:
        Assert(0);
        break;
    }
}

void SddCompSt(node_t *node, Type structure){
    Assert(node->token_val == CompSt);
    list_insert(list_create());
    if(structure){
        FieldList cur = structure->u.structure;
        while(cur){
            add_var_symbol(cur->name, cur->type);
            cur = cur->tail;
        }
    }
    SddDefList(CHILD(2, node), NULL);
    SddStmtList(CHILD(3, node));
    list_pop();
}

void SddStmtList(node_t *node){
    Assert(node->token_val == StmtList);
}

Type SddExp(node_t *node, Type inh){
    Assert(node->token_val == Exp);
    return NULL;
}

static Type INT_Type_const = NULL;
static Type FLOAT_Type_const = NULL;

Type SddType(node_t *node){
    Assert(!strcmp(node->name, "TYPE"));
    if(!strcmp(node->str, "int")){
        if(!INT_Type_const){
            INT_Type_const = malloc(sizeof(struct Type_));
            INT_Type_const->kind = BASIC;
            INT_Type_const->u.basic.type = TINT;
            INT_Type_const->u.basic.val.intval = 0;
        }return INT_Type_const;
    }else if(!strcmp(node->str, "float")){
        if(!FLOAT_Type_const){
            FLOAT_Type_const = malloc(sizeof(struct Type_));
            FLOAT_Type_const->kind = BASIC;
            FLOAT_Type_const->u.basic.type = TFLOAT;
            FLOAT_Type_const->u.basic.val.floatval = 0;
        }return FLOAT_Type_const;
    }else{
        Assert(0);
        return NULL;
    }
}

Type Array(Type inh, int size){
    Type arr = malloc(sizeof(struct Type_));
    arr->kind = ARRAY;
    arr->u.array.elem = inh;
    arr->u.array.size = size;
    return arr;
}

