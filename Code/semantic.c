#include "semantic.h"
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

static Type INT_Type_const = NULL;
static Type FLOAT_Type_const = NULL;

symbol symbol_list[HASH_SIZE] = {0};
static list_node list_head = NULL;

Type type_malloc(int kind1, int kind2){
    if(kind1 == BASIC){
        if(kind2 == TINT){
            if(!INT_Type_const){
                INT_Type_const = malloc(sizeof(struct Type_));
                INT_Type_const->kind = BASIC;
                INT_Type_const->locked = true;
                INT_Type_const->func_locked = false;
                INT_Type_const->u.basic = TINT;
            }return INT_Type_const;
        }else if(kind2 == TFLOAT){
            if(!FLOAT_Type_const){
                FLOAT_Type_const = malloc(sizeof(struct Type_));
                FLOAT_Type_const->kind = BASIC;
                FLOAT_Type_const->locked = true;
                FLOAT_Type_const->func_locked = false;
                FLOAT_Type_const->u.basic= TFLOAT;
            }return FLOAT_Type_const;
        }else{
            Assert(0);
        }
    }else{
        Type ret = malloc(sizeof(struct Type_));
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
    int dim1 = 0, size1 = 1, dim2 = 0, size2 = 1;
    while (dst->kind == ARRAY){
        dim1++;
        size1 = size1 * dst->u.array.size;
        dst = dst->u.array.elem;
    }
    while(src->kind == ARRAY){
        dim2++;
        size2 = size2 * src->u.array.size;
        src = src->u.array.elem;
    }
    if(size1 != size2) return false;
    if(dim1 != dim2) return false;
    return type_com(dst, src);
}

FieldList field_malloc(char *name, Type inh){
    FieldList cur_field = malloc(sizeof(struct FieldList_));
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
    if(inh == NULL) return NULL;
    symbol OldSym = hash_find(node->str);
    if(OldSym && OldSym->belong == list_head){
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
    symbol sym = malloc(sizeof(struct symbol_));
    strncpy(sym->name, node->str, NAME_SIZE);
    sym->kind = sym_kind;
    sym->first_lineno  = node->lineno;
    switch (sym_kind) {
      case FUNCTION:
          sym->u.func.defined = false;
          sym->u.func.ret = inh;
          sym->u.func.parameter = NULL;
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
    list_insert_sym(sym);
    sym->belong = list_head;
    hash_insert(sym);
    return sym;
}

static void fun_unlock(FieldList field){
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
            sym->u.func.ret->func_locked = false;
            type_free(sym->u.func.ret);
            fun_unlock(sym->u.func.parameter);
            field_free(sym->u.func.parameter);
            break;
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
        symbol_free(tmp);
    }
    list_node tmp = list_head;
    list_head = list_head->nxt;
    free(tmp);
    //Assert(list_head);
}

/* ============================================ */

void SddProgram(node_t* node){
    Assert(node->token_val == Program);
    /* 在此初始化一些变量 */
    list_insert(list_create());
    SddExtDefList(CHILD(1, node));
    CheckFun();
    list_pop();
}

void SddExtDefList(node_t *node){
    if(node == NULL) return;
    Assert(node->token_val == ExtDefList);
    SddExtDef(CHILD(1, node));
    SddExtDefList(CHILD(2, node));
}

void SddExtDef(node_t *node){ 
    Assert(node->token_val == ExtDef);

    Type syn = NULL;
    switch (node->production_id)
    {
    case 0:
        syn = SddSpecifier(CHILD(1, node));
        SddExtDecList(CHILD(2, node), syn);
        break;
    case 1:
        SddSpecifier(CHILD(1, node));
        break;
    case 2:
        syn = SddSpecifier(CHILD(1, node));
        SddFunDef(CHILD(2, node), syn);
        break;
    case 3:
        syn = SddSpecifier(CHILD(1, node));
        SddFunDec(CHILD(2, node), syn);
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
        return NULL;
    }
}

Type SddStructSpecifier(node_t *node){
    Assert(node->token_val == StructSpecifier);
    switch (node->production_id)
    {
    case 0:
    {
        Type syn;
        if(CHILD(5, node) == NULL){
            syn = type_malloc(STRUCTURE, 0);
            syn->u.structure = NULL;
            SddDefList(CHILD(3, node), syn);
        }
        else{
            syn = type_malloc(STRUCTURE, 0);
            syn->locked = true;
            syn->u.structure = NULL;
            SddDefList(CHILD(4, node), syn);
            if(!symbol_add(CHILD(1, CHILD(2, node)), syn, STRUCT_TAG)){
                type_free(syn);
                syn = NULL;
            }
        }
        return syn;
    }
    case 1:
    {
        symbol syn = hash_find(CHILD(1, CHILD(2, node))->str);
        if(syn == NULL || syn->kind != STRUCT_TAG){
            semantic_error(17, CHILD(1, CHILD(2, node))->lineno, "struct no defined");
            return NULL;
        }
        return syn->u.struct_tag;
    }
    default:
        Assert(0);
        return NULL;
    }
}

void SddVarDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == VarDec);
    switch (node->production_id)
    {
    case 0:
        Assert(!strcmp(CHILD(1, node)->name, "ID"));
        if(structure == NULL){
            symbol_add(CHILD(1, node), inh, VARIABLE);
        }else{
            Assert(structure->kind == STRUCTURE);
            if(field_find(CHILD(1, node)->str, structure->u.structure)){
                semantic_error(15, CHILD(1, node)->lineno, "redefined the variable in one field");
            }else{
                FieldList cur_field = field_malloc(CHILD(1, node)->str, inh);
                cur_field->tail = structure->u.structure;
                structure->u.structure = cur_field;
            }
        }
        break;
    case 1:
    {
        Type syn = SddArray(inh, str2int(CHILD(3, node)->str));
        SddVarDec(CHILD(1, node), syn, structure);
        break;
    }
    default:
        Assert(0);
    }
}

static FieldList SddFunVar_(node_t *node){
    Assert(node->token_val == FunDec);
    Assert(node->production_id == 0 || node->production_id == 1);
    list_insert(list_create());
    FieldList varlist = NULL;
    if(node->production_id == 0) {
        SddVarList(CHILD(3, node));
        symbol cur = list_head->sym;
        while(cur){
            Assert(cur->kind != FUNCTION);
            cur->u.variable->func_locked = true;
            if(cur->kind == VARIABLE) {
                FieldList field = field_malloc(cur->name, cur->u.variable);
                field->tail = varlist;
                varlist = field;
            }
            cur = cur->list_nxt;
        }
    }return varlist;
}

static symbol SddFun_(node_t *node, Type inh){
    Assert(node->token_val == FunDec);
    symbol OldSym = hash_find(CHILD(1, node)->str);
    symbol funsym = NULL;
    if(OldSym != NULL){
        if(!type_com(OldSym->u.func.ret, inh)) {
            semantic_error(19, node->lineno, "function conflict");
            return NULL;
        }type_free(inh);
        FieldList field = SddFunVar_(node);
        if(!field_com(OldSym->u.func.parameter, field)) {
            semantic_error(19, node->lineno, "function conflict");
            list_pop();
            fun_unlock(field);
            field_free(field);
            return NULL;
        }else{
            fun_unlock(OldSym->u.func.parameter);
            field_free(OldSym->u.func.parameter);
            OldSym->u.func.parameter = field;
            return OldSym;
        }
    }else{
        funsym = symbol_add(CHILD(1, node), inh, FUNCTION);
        inh->func_locked = true;
        FieldList field = SddFunVar_(node);
        funsym->u.func.parameter = field;
        return funsym;
    }
}

void SddFunDef(node_t *node, Type inh){
    Assert(node->token_val == FunDec);
    symbol OldSym = hash_find(CHILD(1, node)->str);
    if(OldSym && (OldSym->kind != FUNCTION ||  OldSym->u.func.defined)) {
        semantic_error(4, node->lineno, "redefined function");
        return;
    }symbol funsym =  SddFun_(node, inh);
    if(funsym){
        funsym->u.func.defined = true;
        SddCompSt(node->brother, true, inh);
    }

}

void SddFunDec(node_t *node, Type inh){
    Assert(node->token_val == FunDec);
    symbol OldSym = hash_find(CHILD(1, node)->str);
    if(OldSym && OldSym->kind != FUNCTION) {
        semantic_error(4, node->lineno, "redefined function");
        return;
    }if(SddFun_(node, inh)){
        list_pop();
    }
}

void SddVarList(node_t* node){
    Assert(node->token_val == VarList);
    Assert(node->production_id == 0 || node->production_id == 1);
    SddParamDec(CHILD(1, node));
    if(node->production_id == 0)
        SddVarList(CHILD(3, node));
}

void SddParamDec(node_t *node){
    Assert(node->token_val == ParamDec);
    Type syn = SddSpecifier(CHILD(1, node));
    SddVarDec(CHILD(2, node), syn, NULL);
}

void SddDefList(node_t *node, Type structure){
    if(node == NULL) return;
    Assert(node->token_val == DefList);
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
    }
}

void SddDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == Dec);
    if(structure != NULL && node->production_id == 1){
        semantic_error(15, node->lineno, "can't initial the variable in struct");
        SddVarDec(CHILD(1, node), inh, structure);
        return;
    }
    switch (node->production_id)
    {
    case 0:
        SddVarDec(CHILD(1, node), inh, structure);
        break;
    case 1:
        SddVarDec(CHILD(1, node), inh, structure);
        Type syn = SddExp(CHILD(3, node), false);
        if(!type_com(inh, syn))
            semantic_error(5, node->lineno, "type conflict when define");
        break;
    default:
        Assert(0);
        break;
    }
}

void SddCompSt(node_t *node, int isFun, Type ret){
    Assert(node->token_val == CompSt);
    if(!isFun)
        list_insert(list_create());
    if(CHILD(3, node) != NULL){
        if(CHILD(4, node) != NULL){
            SddDefList(CHILD(2, node), NULL);
            SddStmtList(CHILD(3, node), ret);
        }else{
            if(CHILD(2,node)->token_val == DefList)
                SddDefList(CHILD(2, node), NULL);
            else
                SddStmtList(CHILD(2, node), ret);
        }
    }
    list_pop();
}

void SddStmtList(node_t *node, Type ret){
    if(node == NULL) return;
    Assert(node->token_val == StmtList);
    SddStmt(CHILD(1, node), ret);
    SddStmtList(CHILD(2, node), ret);
}

void SddStmt(node_t *node, Type ret){
    Assert(node->token_val == Stmt);
    switch (node->production_id) {
        case 0:
            SddExp(CHILD(1, node), false);
            break;
        case 1:
            SddCompSt(CHILD(1, node), false, ret);
            break;
        case 2:
            SddReturn(CHILD(2, node), ret);
            break;
        case 3:
            CheckInt1(SddExp(CHILD(3, node), false));
            SddStmt(CHILD(5, node), ret);
            break;
        case 4:
            CheckInt1(SddExp(CHILD(3, node), false));
            SddStmt(CHILD(5, node), ret);
            SddStmt(CHILD(7, node), ret);
            break;
        case 5:
            CheckInt1(SddExp(CHILD(3, node), false));
            SddStmt(CHILD(5, node), ret);
            break;
        default:
            Assert(0);
    }

}

Type SddExp(node_t *node, int isLeft){
    Assert(node->token_val == Exp);
    // 检测类型6
    if(isLeft) {
        switch (node->production_id) {
            case 0:
            case 8:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                break;
            default:
                semantic_error(6, node->lineno, "the left of '=' can't be right value");
        }
    }
    switch (node->production_id) {
        case 0:
        {
            Type type1 = SddExp(CHILD(1, node), true);
            Type type2 = SddExp(CHILD(3, node), false);
            return CheckAssign(type1, type2);
        }
        case 1:
        case 2:
        case 3:
        {
            Type type1 = SddExp(CHILD(1, node), false);
            Type type2 = SddExp(CHILD(3, node), false);
            return CheckInt2(type1, type2);
        }
        case 4:
        case 5:
        case 6:
        case 7:
        {
            Type type1 = SddExp(CHILD(1, node), false);
            Type type2 = SddExp(CHILD(3, node), false);
            return CheckArithm2(type1, type2);
        }
        case 8:
            return SddExp(CHILD(2, node), isLeft);
        case 9:
            return CheckArithm1(SddExp(CHILD(2, node), false));
        case 10:
            return CheckInt1(SddExp(CHILD(1, node), false));
        case 11:
        case 12:
            return SddExpFun(node, isLeft);
        case 13: /* array */
            return SddExpArray(node, isLeft);
        case 14: /* struct */
            return SddExpStruct(node, isLeft);
        case 15:
            return SddId(CHILD(1, node));
        case 16:
            return type_malloc(BASIC, TINT);
        case 17:
            return type_malloc(BASIC, TFLOAT);
        default:
            Assert(0);
            return NULL;
    }
}

Type SddExpFun(node_t *node, int isLeft){
    // 检测error 2;
    // error 9
    // error 11
    TODO();
    return NULL;
}

Type SddExpArray(node_t *node, int isLeft){
    // 检测10, 12
    TODO();
    return NULL;
}

Type SddExpStruct(node_t *node, int isLeft){
    // 检测13 14
    TODO();
    return NULL;
}

Type SddId(node_t *node){
    // 检测 error 1;

    TODO();
    return NULL;
}

Type SddType(node_t *node){
    Assert(!strcmp(node->name, "TYPE"));
    if(!strcmp(node->str, "int")){
        return type_malloc(BASIC, TINT);
    }else if(!strcmp(node->str, "float")){
        return type_malloc(BASIC, TFLOAT);
    }else{
        Assert(0);
        return NULL;
    }
}

Type SddArray(Type inh, int size){
    Type arr = type_malloc(ARRAY, 0);
    arr->u.array.elem = inh;
    arr->u.array.size = size;
    return arr;
}

void SddReturn(node_t *node, Type ret){
    // 检测类型8
    TODO();
}

Type CheckInt2(Type type1, Type type2){
    TODO();
    return NULL;
}
Type CheckInt1(Type type){
    TODO();
    return NULL;
}
Type CheckArithm2(Type type1, Type type2){
    // 检测类型 7
    TODO();
    return NULL;
}
Type CheckArithm1(Type type){
    TODO();
    return NULL;
}
Type CheckAssign(Type type1, Type type2){
    // 检测类型5
    TODO();
    return NULL;
}

void CheckFun(){
    Assert(list_head != NULL);
    symbol cur = list_head->sym;
    while(cur){
        if(cur->kind == FUNCTION){
            if(!cur->u.func.defined)
                semantic_error(18, cur->first_lineno, "declared but not defined");
        }cur = cur->list_nxt;
    }
}

void semantic_error(int errorno, int lineno, char* str){
    printf("Error type %d at Line %d: %s.\n", errorno, lineno, str);
}