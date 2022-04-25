#include "translate.h"
static int temp_num = 0;

InterCodes Code_malloc(){
    InterCodes code = malloc(sizeof(struct InterCodes_));
    code->next = NULL;
    code->prev = NULL;
    code->tail = code;
    return code;
}

Operand new_temp(){
    Operand tmp = malloc(sizeof(struct Operand_));
    tmp->kind = TEMPORARY_O;
    tmp->u.var_no = temp_num++;
    return tmp;
}

Operand get_variable(char *name){
    symbol sym = hash_find(name);
    Assert(sym->kind == VARIABLE);
    Operand var = malloc(sizeof(struct Operand_));
    var->kind = VARIABLE_O;
    var->u.var_no = sym->no;
    return var;
}

InterCodes  MergeCodes(InterCodes code1, InterCodes code2){
    if(code1 == NULL) return code2;
    else if(code2 == NULL) return code1;
    code1->tail->next = code2;
    code2->prev = code1->tail;
    code1->tail = code2->tail;
    return code1;
}

InterCodes TransProgram(node_t *node){
    Assert(node->token_val == Program);
    env_init();
    InterCodes code = TransExtDefList(CHILD(1, node));
    env_pop();
    return code;
}

InterCodes TransExtDefList(node_t *node){
    if(node == NULL) return NULL;
    Assert(node->token_val == ExtDefList);
    InterCodes code1 = TransExtDef(CHILD(1, node));
    InterCodes code2 = TransDefList(CHILD(2, node));
    return MergeCodes(code1, code2);
}

InterCodes TransExtDef(node_t *node){
    Assert(node->token_val == ExtDef);

    Type syn = NULL;
    switch (node->production_id)
    {
        case 0:
            syn = TransSpecifier(CHILD(1, node));
            TransExtDecList(CHILD(2, node), syn);
            return NULL;
        case 1:
            TransSpecifier(CHILD(1, node));
            return NULL;
        case 2:
            syn = TransSpecifier(CHILD(1, node));
            return TransFunDef(CHILD(2, node), syn);
        case 3:
            syn = TransSpecifier(CHILD(1, node));
            TransFunDec(CHILD(2, node), syn);
            return NULL;
        default:
            Assert(0);
            return NULL;
    }
}

void TransExtDecList(node_t *node, Type inh){
    Assert(node->token_val == ExtDecList);
    switch (node->production_id)
    {
        case 0:
            TransVarDec(CHILD(1, node), inh, NULL);
            break;
        case 1:
            TransVarDec(CHILD(1, node), inh, NULL);
            TransExtDecList(CHILD(3, node), inh);
            break;
        default:
            Assert(0);
            break;
    }
}

Type TransSpecifier(node_t *node){
    Assert(node->token_val == Specifier);
    switch (node->production_id)
    {
        case 0:
            return TransType(CHILD(1, node));
        case 1:
            return TransStructSpecifier(CHILD(1, node));
        default:
            Assert(0);
            return NULL;
    }
}

Type TransStructSpecifier(node_t *node){
    Assert(node->token_val == StructSpecifier);
    switch (node->production_id)
    {
        case 0:
        {
            Type syn;
            if(CHILD(5, node) == NULL){     //5?
                syn = type_malloc(STRUCTURE, 0);
                syn->u.structure = NULL;
                TransDefList(CHILD(3, node), syn);
            }
            else{
                syn = type_malloc(STRUCTURE, 0);
                syn->locked = true;
                syn->u.structure = NULL;
                TransDefList(CHILD(4, node), syn);    //4?
                symbol_add(CHILD(1, CHILD(2, node)), syn, STRUCT_TAG);
            }
            return syn;
        }
        case 1:
        {
            symbol syn = hash_find(CHILD(1, CHILD(2, node))->str);
            return syn->u.struct_tag;
        }
        default:
            Assert(0);
            return NULL;
    }
}

void TransVarDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == VarDec);
    switch (node->production_id)
    {
        case 0:
            Assert(!strcmp(CHILD(1, node)->name, "ID"));
            if(structure == NULL){
                symbol_add(CHILD(1, node), inh, VARIABLE);
            }else{
                Assert(structure->kind == STRUCTURE);
                FieldList cur_field = field_malloc(CHILD(1, node)->str, inh);
                cur_field->tail = structure->u.structure;
                structure->u.structure = cur_field;
            }
            break;
        case 1:
        {
            Type syn = TransArray(inh, str2int(CHILD(3, node)->str));
            TransVarDec(CHILD(1, node), syn, structure);
            break;
        }
        default:
            Assert(0);
    }
}

static FieldList TransFunVar_(node_t *node){
    Assert(node->token_val == FunDec);
    Assert(node->production_id == 0 || node->production_id == 1);
    env_insert(env_create());
    FieldList varlist = NULL;
    if(node->production_id == 0) {
        TransVarList(CHILD(3, node));
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

InterCodes TransFunDef(node_t *node, Type inh){
    Assert(node->token_val == FunDec);
    Assert(hash_find(CHILD(1, node)->str) == NULL);
    symbol funsym = symbol_add(CHILD(1, node), inh, FUNCTION);
    Assert(funsym != NULL);

    inh->func_locked = true;
    FieldList field = TransFunVar_(node);
    funsym->u.func.parameter = field;
    funsym->u.func.defined = true;
    InterCodes code1 = malloc(sizeof(struct InterCodes_));
    code1->code.kind = FUNCDEF;
    strncpy(code1->code.u.name,CHILD(1, node)->str, NAME_SIZE);
    code1->prev = NULL;
    code1->next = NULL;
    code1->tail = code1;
    InterCodes code2 = TransCompSt(node->brother, true, inh);
    return MergeCodes(code1, code2);
}

void TransFunDec(node_t *node, Type inh) {
    Assert(node->token_val == FunDec);
    //no need to run func declare
}

void TransVarList(node_t* node){
    Assert(node->token_val == VarList);
    Assert(node->production_id == 0 || node->production_id == 1);
    TransParamDec(CHILD(1, node));
    if(node->production_id == 0)
        TransVarList(CHILD(3, node));
}

void TransParamDec(node_t *node){
    Assert(node->token_val == ParamDec);
    Type syn = TransSpecifier(CHILD(1, node));
    TransVarDec(CHILD(2, node), syn, NULL);
}

InterCodes TransDefList(node_t *node, Type structure){
    if(node == NULL) return NULL;
    Assert(node->token_val == DefList);
    InterCodes code1 = TransDef(CHILD(1, node), structure);
    InterCodes code2 = TransDefList(CHILD(2, node), structure);
    return MergeCodes(code1, code2);
}

InterCodes TransDef(node_t *node, Type structure){
    Assert(node->token_val == Def);
    Type syn = TransSpecifier(CHILD(1, node));
    return TransDecList(CHILD(2, node), syn, structure);
}

InterCodes TransDecList(node_t *node, Type inh, Type structure){
    Assert(node->token_val == DecList);
    switch (node->production_id)
    {
        case 0:
            return TransDec(CHILD(1, node), inh, structure);
        case 1:
        {
            InterCodes code1 = TransDec(CHILD(1, node), inh, structure);
            InterCodes code2 = TransDecList(CHILD(3, node), inh , structure);
            return MergeCodes(code1, code2);
        }
        default:
            Assert(0);
            return NULL;
    }
}

InterCodes TransDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == Dec);
    switch (node->production_id)
    {
        case 0:
            TransVarDec(CHILD(1, node), inh, structure);
            return NULL;
        case 1:
            TransVarDec(CHILD(1, node), inh, structure);
            Operand t1 = new_temp();
            InterCodes code1 = TransExp(CHILD(3, node), t1);
            InterCodes code2 = code_malloc();
            code2->code.kind = ASSIGN;
            code2->code.u.assign.left = get_variable(CHILD(1, CHILD(1, node))->str);
            code2->code.u.assign.right = t1;
            return MergeCodes(code1, code2);
        default:
            Assert(0);
            break;
    }
}

InterCodes TransCompSt(node_t *node, int isFun, Type ret){
    Assert(node->token_val == CompSt);
    if(!isFun)
        env_insert(env_create());
    if(CHILD(3, node) != NULL){
        if(CHILD(4, node) != NULL){
            TransDefList(CHILD(2, node), NULL);
            TransStmtList(CHILD(3, node), ret);
        }else{
            if(CHILD(2,node)->token_val == DefList)
                TransDefList(CHILD(2, node), NULL);
            else
                TransStmtList(CHILD(2, node), ret);
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
            case 11:
            case 12:
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
            return CheckInt1(SddExp(CHILD(1, node), false), node->lineno);
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
    }Type syn2 = field_find(CHILD(3, node)->str, syn1->u.structure);
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