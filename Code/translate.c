#include "translate.h"
static int temp_num = 0;
static int label_num = 0;

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

InterCodes new_label(){
    InterCodes codes = code_malloc();
    codes->code.kind = LABEL;
    codes->code.u.label_no = label_num++;
    return codes;
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
    env_push();
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
    strncpy(code1->code.u.func_name,CHILD(1, node)->str, NAME_SIZE);
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
            return NULL;
    }
}

InterCodes TransCompSt(node_t *node, int isFun, Type ret){
    Assert(node->token_val == CompSt);
    if(!isFun)
        env_push();
    InterCodes code = NULL;
    if(CHILD(3, node) != NULL){
        if(CHILD(4, node) != NULL){
            InterCodes code1 = TransDefList(CHILD(2, node), NULL);
            InterCodes code2 = TransStmtList(CHILD(3, node), ret);
            code = MergeCodes(code1, code2);
        }else{
            if(CHILD(2,node)->token_val == DefList)
                code = TransDefList(CHILD(2, node), NULL);
            else
                code = TransStmtList(CHILD(2, node), ret);
        }
    }
    env_pop();
}

InterCodes SddStmtList(node_t *node, Type ret){
    if(node == NULL) return NULL;
    Assert(node->token_val == StmtList);
    InterCodes code1 = TransStmt(CHILD(1, node), ret);
    InterCodes code2 = TransStmtList(CHILD(2, node), ret);
    return MergeCodes(code1, code2);
}

InterCodes TransStmt(node_t *node, Type ret){
    Assert(node->token_val == Stmt);
    switch (node->production_id) {
        case 0:
            return TransExp(CHILD(1, node), NULL);
        case 1:
            return TransCompSt(CHILD(1, node), false, ret);
        case 2:
            return TransReturn(node);
        case 3:
            return TransIf(node);
        case 4:
            return TransIfEl(node);
        case 5:
            return TransWhile(node);
        default:
            Assert(0);
            return NULL;
    }

}

InterCodes TransExp(node_t *node, Operand op){
    Assert(node->token_val == Exp);
    // 检测类型6

    switch (node->production_id) {
        case 0:
        {
            Type type1 = TransExp(CHILD(1, node), op);
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
            return TransExpFun(node, op);
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

InterCodes TransArgs(node_t *node, FieldList ArgList){
    // error 9
    Assert(node->token_val == Args);
    Assert(node->production_id == 0 || node->production_id == 1);

}

InterCodes TransExpFun(node_t *node, Operand place){
    // 检测error 2;
    // error 11
    symbol sym = hash_find(CHILD(1, node)->str);

    Assert(node->production_id == 11 || node->production_id == 12);
    if(node->production_id == 11)
        TransArgs(CHILD(3, node), sym->u.func.parameter);
    return NULL;
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

InterCodes TransReturn(node_t *node){
    Operand t1 = new_temp();
    InterCodes code1 = TransExp(CHILD(2, node), t1);
    InterCodes code2 = code_malloc();
    code2->code.kind = RETURN;
    code2->code.u.ret.val = t1;
    MergeCodes(code1, code2);
}

InterCodes TransIf(node_t *node){

}

InterCodes TransIfEl(node_t *node){

}

InterCodes TransWhile(node_t *node){

}

InterCodes TransCond(node_t *node, InterCode* LabelTrue, InterCode* LabelFalse){

}
