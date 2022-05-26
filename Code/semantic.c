#include "semantic.h"

extern Type Error_Type;

void SddProgram(node_t *node){
    Assert(node->token_val == Program);
    env_init();

    SddExtDefList(CHILD(1, node));
    CheckFun();
    env_pop();
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
        if(CHILD(5, node) == NULL){     //5?
            syn = type_malloc(STRUCTURE, 0);
            syn->u.structure = NULL;
            SddDefList(CHILD(3, node), syn);
        }
        else{
            syn = type_malloc(STRUCTURE, 0);
            syn->locked = true;
            syn->u.structure = NULL;
            SddDefList(CHILD(4, node), syn);    //4?
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
            semantic_error(17, CHILD(1, CHILD(2, node))->lineno, "struct not defined");
            return Error_Type;
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
    env_push();
    FieldList varlist = NULL;
    if(node->production_id == 0) {
        SddVarList(CHILD(3, node));
        symbol cur = envs->sym;
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
            env_pop();
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
        env_pop();
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
        env_push();
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
    env_pop();
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
            CheckInt1(SddExp(CHILD(3, node), false), node->lineno);
            SddStmt(CHILD(5, node), ret);
            break;
        case 4:
            CheckInt1(SddExp(CHILD(3, node), false), node->lineno);
            SddStmt(CHILD(5, node), ret);
            SddStmt(CHILD(7, node), ret);
            break;
        case 5:
            CheckInt1(SddExp(CHILD(3, node), false), node->lineno);
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
            case 13:
            case 14:
            case 15:
                break;
            default:
                semantic_error(6, node->lineno, "The left-hand side of an assignment must be a variable.");
                return Error_Type;
        }
    }
    switch (node->production_id) {
        case 0:
        {
            Type type1 = SddExp(CHILD(1, node), true);
            Type type2 = SddExp(CHILD(3, node), false);
            return CheckAssign(type1, type2, node->lineno);
        }
        case 1:
        case 2:
        {
            Type type1 = SddExp(CHILD(1, node), false);
            Type type2 = SddExp(CHILD(3, node), false);
            return CheckInt2(type1, type2, node->lineno);
        }
        case 3:
        {
            Type type1 = SddExp(CHILD(1, node), false);
            Type type2 = SddExp(CHILD(3, node), false);
            CheckArithm2(type1, type2, node->lineno);
            return type_malloc(BASIC, TINT);
        }
        case 4:
        case 5:
        case 6:
        case 7:
        {
            Type type1 = SddExp(CHILD(1, node), false);
            Type type2 = SddExp(CHILD(3, node), false);
            return CheckArithm2(type1, type2, node->lineno);
        }
        case 8:
            return SddExp(CHILD(2, node), isLeft);
        case 9:
            return CheckArithm1(SddExp(CHILD(2, node), false), node->lineno);
        case 10:
            return CheckInt1(SddExp(CHILD(2, node), false), node->lineno);
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

void SddArgs(node_t *node, FieldList ArgList){
    // error 9
    Assert(node->token_val == Args);
    Assert(node->production_id == 0 || node->production_id == 1);
    Type syn = SddExp(CHILD(1, node), false);
    if(ArgList == NULL || !type_com(ArgList->type, syn))
        semantic_error(9, node->lineno, "the type of arg conflict");
    if(node->production_id == 0)
        SddArgs(CHILD(3, node), ArgList->tail);
    else if(ArgList->tail != NULL)
        semantic_error(9, node->lineno, "the type of arg conflict");

}

Type SddExpFun(node_t *node, int isLeft){
    // 检测error 2;
    // error 11
    symbol sym = hash_find(CHILD(1, node)->str);
    if(sym == NULL) {
        semantic_error(2, node->lineno, "function used but not declare");
        return Error_Type;
    }
    if(sym->kind != FUNCTION) {
        semantic_error(11, node->lineno, "variable can't use (..)");
        return Error_Type;
    }
    if(!sym->u.func.defined) {
        //struct list_int* tmp = malloc(sizeof(struct list_int));
        //tmp->nxt = sym->u.func.func_used;
        //sym->u.func.func_used = tmp;
        //tmp->lineno = node->lineno;
    }
    Assert(node->production_id == 11 || node->production_id == 12);
    if(node->production_id == 11)
        SddArgs(CHILD(3, node), sym->u.func.parameter);
    return sym->u.func.ret;
}

Type SddExpArray(node_t *node, int isLeft){
    Type syn1 = SddExp(CHILD(1, node), isLeft);
    if(syn1->kind != ARRAY){
        semantic_error(10, CHILD(1, node)->lineno, "use LB RB but not ARRAY variable");
        return Error_Type;
    }Type syn2 = SddExp(CHILD(3, node), false);
    if(syn2->kind != BASIC || syn2->u.basic != TINT) {
        semantic_error(12, CHILD(3, node)->lineno, "[...] use not int variable");
        return Error_Type;
    }
    return syn1->u.array.elem;
}

Type SddExpStruct(node_t *node, int isLeft){
    // 检测13 14
    Type syn1 =  SddExp(CHILD(1, node), isLeft);
    if(syn1->kind != STRUCTURE) {
        semantic_error(13, CHILD(1, node)->lineno, "use dot but not structure");
        return Error_Type;
    }Type syn2 = field_find(CHILD(3, node)->str, syn1->u.structure)->type;
    if(syn2 == NULL){
        semantic_error(14,CHILD(3, node)->lineno, "the field has no match name");
        return Error_Type;
    }
    return syn2;
}

Type SddId(node_t *node){
    // 检测 error 1;
    symbol sym = hash_find(node->str);
    if(sym == NULL){
        semantic_error(1, node->lineno, "variable used but not define");
        return Error_Type;
    }
    if(sym->kind == FUNCTION || sym->kind == STRUCT_TAG){
        semantic_error(1, node->lineno, "not a variable");
        return Error_Type;
    }
    return sym->u.variable;
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
    // shi fou jian ce han shu you wu fan hui zhi
    Type type = SddExp(node, false);
    if (!type_com(ret, type))
        semantic_error(8, node->lineno, "function return type not match");
}

Type CheckInt2(Type type1, Type type2, int lineno){
    if(CheckInt1(type1, lineno) == Error_Type) return Error_Type;
    if(CheckInt1(type2, lineno) == Error_Type) return Error_Type;
    if (type1->u.basic != TINT || type2->u.basic != TINT) {
        semantic_error(7, lineno, "not integer");
        return Error_Type;
    }
    return type_malloc(BASIC, TINT);
}

Type CheckInt1(Type type, int lineno){
    Assert(type != NULL);
    if(type == Error_Type)
        return Error_Type;

    if(type->u.basic != TINT){
        semantic_error(7, lineno, "not integer");
        return Error_Type;
    }
    return type_malloc(BASIC, TINT);
}
Type CheckArithm2(Type type1, Type type2, int lineno){
    // 检测类型 7
    Assert(type1 != NULL && type2 != NULL);
    if (!type_com(type1, type2))
        semantic_error(7, lineno, "operand type mismatch");
    else if(type1->kind != BASIC)
        semantic_error(7, lineno, "operand type does not match operator");
    else if(type1 != Error_Type && type2 != Error_Type)
        return type1;
    return Error_Type;
}
Type CheckArithm1(Type type, int lineno){
    Assert(type != NULL);
    if(type == Error_Type) return Error_Type;
    if (type->kind != BASIC) {
        semantic_error(7, lineno, "operand type does not match operator");
        return Error_Type;
    }
    return type;
}
Type CheckAssign(Type type1, Type type2, int lineno){
    // 检测类型5
    Assert(type1 != NULL && type2 != NULL);
    if(type1 == Error_Type || type2 == Error_Type) return Error_Type;
    if (!type_com(type1, type2)) {
        semantic_error(5, lineno, "Type mismatched for assignment.");
        return Error_Type;
    }
    return type1;
}

void CheckFun(){
    Assert(envs != NULL);
    symbol cur = envs->sym;
    while(cur){
        if(cur->kind == FUNCTION){
            if(!cur->u.func.defined) {
                semantic_error(18, cur->first_lineno, "declared but not defined");
                //struct list_int* tmp = cur->u.func.func_used;
                //while(tmp){
                //    semantic_error(2, tmp->lineno, "function used but not define");
                //    tmp = tmp->nxt;
                //}
            }
        }cur = cur->list_nxt;
    }
}