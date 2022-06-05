#include "translate.h"
static int temp_num = 0;
static int label_num = 0;

InterCodes code_malloc(){
    InterCodes code = malloc(sizeof(struct InterCodes_));
    code->next = NULL;
    code->prev = NULL;
    code->tail = code;
    code->code.op_num = 0;
    code->code.nxt_code[0] = -1;
    code->code.nxt_code[1] = -1;
    code->code.nxt_code[2] = -1;
    return code;
}

InterCodes gen_assign_code(Operand right, Operand left, int kind){
    if(left == NULL) return NULL;
    InterCodes code = code_malloc();
    code->code.op_num = 2;
    code->code.kind = ASSIGN;
    code->code.u.assign.kind = kind;
    code->code.u.assign.left = left;
    code->code.u.assign.right = right;
    return code;
}

InterCodes gen_arith_code(Operand result, Operand op1, Operand op2, int kind){
    if(result == NULL) return NULL;
    InterCodes code = code_malloc();
    code->code.kind = kind;
    code->code.op_num = 3;
    code->code.u.binop.result = result;
    code->code.u.binop.op1 = op1;
    code->code.u.binop.op2 = op2;
    return code;
}

InterCodes gen_cjp_code(Operand x, Operand y, char *relop, int label){
    InterCodes code = code_malloc();
    code->code.kind = CJP;
    code->code.op_num = 2;
    code->code.u.cjp.label_no = label;
    code->code.u.cjp.x = x;
    code->code.u.cjp.y = y;
    strncpy(code->code.u.cjp.relop, relop, 16);
    return code;
}

InterCodes gen_goto_code(int label){
    InterCodes code = code_malloc();
    code->code.kind = GOTO;
    code->code.u.label_no = label;
    return code;
}

InterCodes gen_opx_code(Operand x, int kind){
    InterCodes code = code_malloc();
    code->code.kind = kind;
    code->code.op_num = 1;
    code->code.u.op_x = x;
    return code;
}

InterCodes gen_dec_code(Operand x, int size){
    InterCodes code = code_malloc();
    code->code.kind = DEC;
    code->code.op_num = 1;
    code->code.u.dec.x = x;
    code->code.u.dec.size = size;
    return code;
}

InterCodes gen_call_code(char *name, Operand x){
    if(x == NULL) x = new_temp();
    InterCodes code = code_malloc();
    code->code.kind = CALL;
    code->code.op_num = 1;
    code->code.u.call.x = x;
    strncpy(code->code.u.call.name, name, NAME_SIZE);
    return code;
}

InterCodes gen_func_code(char *name){
    InterCodes code = code_malloc();
    code->code.kind = FUNCDEF;
    strncpy(code->code.u.func_name, name, NAME_SIZE);
    return code;
}

InterCodes new_label(){
    InterCodes codes = code_malloc();
    codes->code.kind = LABEL;
    codes->code.u.label_no = label_num++;
    return codes;
}

Operand operand_malloc(int kind, int num){
    Operand op = malloc(sizeof(struct Operand_));
    op->kind = kind;
    op->nxt_code = -1;
    op->reg_no = -1;
    op->offset = -1;
    switch (kind) {
        case VARIABLE_O:
        case TEMPORARY_O:
            op->u.var_no = num;
            break;
        case CONSTANT_O:
            op->u.value = num;
            break;
        default:
            break;
    }
    return op;
}

Operand new_temp(){
    return operand_malloc(TEMPORARY_O, temp_num++);
}

Operand get_variable(char *name){
    symbol sym = hash_find(name);
    Assert(sym->kind == VARIABLE || sym->kind == PARAM_V);
    return sym->op;
}


InterCodes MergeCodes(InterCodes code1, InterCodes code2){
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
    InterCodes code2 = TransExtDefList(CHILD(2, node));
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

InterCodes TransVarDec(node_t *node, Type inh, Type structure){
    Assert(node->token_val == VarDec);
    switch (node->production_id)
    {
        case 0:
            Assert(!strcmp(CHILD(1, node)->name, "ID"));
            if(structure == NULL){
                symbol sym = symbol_add(CHILD(1, node), inh, VARIABLE);
                if(inh->kind == ARRAY || inh->kind == STRUCTURE)
                    return gen_dec_code(sym->op, type_size(inh));
            }else{
                Assert(structure->kind == STRUCTURE);
                FieldList cur_field = field_malloc(CHILD(1, node)->str, inh);
                if(structure->u.structure != NULL){
                    cur_field->offset = structure->u.structure->offset + type_size(structure->u.structure->type);
                }
                cur_field->tail = structure->u.structure;
                structure->u.structure = cur_field;
            }
            return NULL;
        case 1:
        {
            Type syn = TransArray(inh, str2int(CHILD(3, node)->str));
            return TransVarDec(CHILD(1, node), syn, structure);
        }
        default:
            Assert(0);
            return NULL;
    }
}

static InterCodes TransFunVar_(node_t *node){
    Assert(node->token_val == FunDec);
    Assert(node->production_id == 0 || node->production_id == 1);
    env_push();
    InterCodes code1 = NULL;
    if(node->production_id == 0) {
        TransVarList(CHILD(3, node));
        symbol cur = envs->sym;
        while(cur){
            Assert(cur->kind != FUNCTION);
            cur->u.variable->func_locked = true;
            if(cur->kind == VARIABLE) {
                cur->kind = PARAM_V;
                InterCodes code2 = gen_opx_code(cur->op, PARAM);
                code1 = MergeCodes(code2, code1);
            }
            cur = cur->list_nxt;
        }
    }return code1;
}

InterCodes TransFunDef(node_t *node, Type inh){
    Assert(node->token_val == FunDec);
    Assert(hash_find(CHILD(1, node)->str) == NULL);
    symbol funsym = symbol_add(CHILD(1, node), inh, FUNCTION);
    Assert(funsym != NULL);
    InterCodes code1 = gen_func_code(CHILD(1, node)->str);
    inh->func_locked = true;
    InterCodes code2 = TransFunVar_(node);
    funsym->u.func.parameter = NULL;
    funsym->u.func.defined = true;

    InterCodes code3 = TransCompSt(node->brother, true);
    code1 = MergeCodes(code1, code2);
    return MergeCodes(code1, code3);
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
            return TransVarDec(CHILD(1, node), inh, structure);
        case 1:
        {
            InterCodes code1 = TransVarDec(CHILD(1, node), inh, structure);
            Operand t1 = new_temp();
            InterCodes code2 = TransExp(CHILD(3, node), t1, NULL);
            InterCodes code3 = gen_assign_code(t1, get_variable(CHILD(1, CHILD(1, node))->str), NORMAL);
            code1 = MergeCodes(code1, code2);
            return MergeCodes(code1, code3);
        }
        default:
            Assert(0);
            return NULL;
    }
}

InterCodes TransCompSt(node_t *node, int isFun){
    Assert(node->token_val == CompSt);
    if(!isFun)
        env_push();
    InterCodes code = NULL;
    if(CHILD(3, node) != NULL){
        if(CHILD(4, node) != NULL){
            InterCodes code1 = TransDefList(CHILD(2, node), NULL);
            InterCodes code2 = TransStmtList(CHILD(3, node));
            code = MergeCodes(code1, code2);
        }else{
            if(CHILD(2,node)->token_val == DefList)
                code = TransDefList(CHILD(2, node), NULL);
            else
                code = TransStmtList(CHILD(2, node));
        }
    }
    env_pop();
    return code;
}

InterCodes TransStmtList(node_t *node){
    if(node == NULL) return NULL;
    Assert(node->token_val == StmtList);
    InterCodes code1 = TransStmt(CHILD(1, node));
    InterCodes code2 = TransStmtList(CHILD(2, node));
    return MergeCodes(code1, code2);
}

InterCodes TransStmt(node_t *node){
    Assert(node->token_val == Stmt);
    switch (node->production_id) {
        case 0:
            return TransExp(CHILD(1, node), NULL, NULL);
        case 1:
            return TransCompSt(CHILD(1, node), false);
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

InterCodes TransReturn(node_t *node){
    Operand t1 = new_temp();
    InterCodes code1 = TransExp(CHILD(2, node), t1, NULL);
    InterCodes code2 = gen_opx_code(t1, RETURN);
    return MergeCodes(code1, code2);
}

InterCodes TransIf(node_t *node){
    InterCodes label1 = new_label();
    InterCodes label2 = new_label();
    InterCodes code1 = TransCond(CHILD(3, node), label1->code.u.label_no, label2->code.u.label_no);
    InterCodes code2 = TransStmt(CHILD(5, node));
    code1 = MergeCodes(code1, label1);
    code1 = MergeCodes(code1 , code2);
    return MergeCodes(code1, label2);
}

InterCodes TransIfEl(node_t *node){
    InterCodes label1 = new_label();
    InterCodes label2 = new_label();
    InterCodes label3 = new_label();
    InterCodes code1 = TransCond(CHILD(3, node), label1->code.u.label_no, label2->code.u.label_no);
    InterCodes code2 = TransStmt(CHILD(5, node));
    InterCodes code3 = TransStmt(CHILD(7, node));
    code1 = MergeCodes(code1, label1);
    code1 = MergeCodes(code1, code2);
    code1 = MergeCodes(code1, gen_goto_code(label3->code.u.label_no));
    code1 = MergeCodes(code1, label2);
    code1 = MergeCodes(code1, code3);
    return MergeCodes(code1, label3);;
}

InterCodes TransWhile(node_t *node){
    InterCodes label1 = new_label();
    InterCodes label2 = new_label();
    InterCodes label3 = new_label();
    InterCodes code1 = TransCond(CHILD(3, node), label2->code.u.label_no, label3->code.u.label_no);
    InterCodes code2 = TransStmt(CHILD(5, node));
    label1 = MergeCodes(label1, code1);
    label1 = MergeCodes(label1, label2);
    label1 = MergeCodes(label1, code2);
    label1 = MergeCodes(label1, gen_goto_code(label1->code.u.label_no));
    return MergeCodes(label1, label3);
}

InterCodes TransExp(node_t *node, Operand place, Type *ret){
    Assert(node->token_val == Exp);
    switch (node->production_id) {
        case 0:
            return TransAssign(node, place);
        case 1:
        case 2:
        case 3:
            return TransLogic(node, place);
        case 4:
            return TransArith(node, place, ADD);
        case 5:
            return TransArith(node, place, SUB);
        case 6:
            return TransArith(node, place, MUL);
        case 7:
            return TransArith(node, place, DIVI);
        case 8:
            return TransExp(CHILD(2, node), place, NULL);
        case 9:
            return TransMinus(node, place);
        case 10:
            return TransLogic(node, place);
        case 11:
        case 12:
            return TransExpFun(node, place);
        case 13: /* array */
        case 14:
        {
            if(ret == NULL){
                ret = malloc(sizeof(Type));
            }
            Operand t1 = new_temp();
            t1->kind = ADDRESS_O;
            InterCodes code1 = NULL;
            if(node->production_id == 13)
                code1 = MergeCodes(code1,  TransExpArray(node, t1, ret));
            else
                code1 = MergeCodes(code1, TransExpStruct(node, t1, ret));
            if((*ret)->kind == BASIC)
                code1 = MergeCodes(code1, gen_assign_code(t1, place, RIGHT));
            else
                code1 = MergeCodes(code1, gen_assign_code(t1, place, NORMAL));
            return code1;
        }
        case 15:
            return TransId(CHILD(1, node), place, ret);
        case 16:
            return TransInt(CHILD(1, node), place);
        case 17:
            return TransFloat(CHILD(1, node), place);
        default:
            Assert(0);
            return NULL;
    }
}

InterCodes TransCond(node_t *node, int LabelTrue, int LabelFalse){
    Assert(node->token_val == Exp);
    switch (node->production_id) {
        case 1: // and
        {
            InterCodes label1 = new_label();
            InterCodes code1 = TransCond(CHILD(1, node), label1->code.u.label_no, LabelFalse);
            InterCodes code2 = TransCond(CHILD(3, node), LabelTrue, LabelFalse);
            code1 = MergeCodes(code1, label1);
            return MergeCodes(code1, code2);
        }
        case 2: // or
        {
            InterCodes label1 = new_label();
            InterCodes code1 = TransCond(CHILD(1, node), LabelTrue, label1->code.u.label_no);
            InterCodes code2 = TransCond(CHILD(3, node), LabelTrue, LabelFalse);
            code1 = MergeCodes(code1, label1);
            return MergeCodes(code1, code2);
        }
        case 3: // relop
        {
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            InterCodes code1 = TransExp(CHILD(1, node), t1, NULL);
            InterCodes code2 = TransExp(CHILD(3, node), t2, NULL);
            InterCodes code3 = gen_cjp_code(t1, t2, CHILD(2, node)->str, LabelTrue);
            InterCodes code4 = gen_goto_code(LabelFalse);
            code1 = MergeCodes(code1, code2);
            code1 = MergeCodes(code1, code3);
            return MergeCodes(code1, code4);
        }
        case 10: // not
            return TransCond(CHILD(2, node), LabelFalse, LabelTrue);
        default:
        {
            Operand t1 = new_temp();
            InterCodes code1 = TransExp(node, t1, NULL);
            InterCodes code2 = gen_cjp_code(t1, operand_malloc(CONSTANT_O, 0), "!=", LabelTrue);
            InterCodes code3 = gen_goto_code(LabelFalse);
            code1 = MergeCodes(code1, code2);
            return MergeCodes(code1, code3);
        }

    }
}

InterCodes TransAssign(node_t *node, Operand place){
    Operand t1 = new_temp();
    InterCodes code1 = TransExp(CHILD(3, node), t1, NULL);
    InterCodes code2 = NULL;
    node = CHILD(1, node);
    switch (node->production_id) {
        case 13:
        {
            Type ret;
            Operand a1 = new_temp();
            a1->kind = ADDRESS_O;
            InterCodes code3 = TransExpArray(node, a1, &ret);
            InterCodes code4 = gen_assign_code(t1, a1, LEFT);
            InterCodes code5 = gen_assign_code(a1, place, RIGHT);

            code2 = MergeCodes(code2, code3);
            code2 = MergeCodes(code2, code4);
            code2 = MergeCodes(code2, code5);
            break;
        }
        case 14:
        {
            Type ret;
            Operand a1 = new_temp();
            a1->kind = ADDRESS_O;
            InterCodes code3 = TransExpStruct(node, a1, &ret);
            InterCodes code4 = gen_assign_code(t1, a1, LEFT);
            InterCodes code5 = gen_assign_code(a1, place, RIGHT);

            code2 = MergeCodes(code2, code3);
            code2 = MergeCodes(code2, code4);
            code2 = MergeCodes(code2, code5);
            break;
        }

        case 15:
        {
            symbol sym = hash_find(CHILD(1, node)->str);
            if (sym->u.variable->kind == ARRAY)
                sym->kind = PARAM_V;
            InterCodes code3 = gen_assign_code(t1, sym->op, NORMAL);
            InterCodes code4 = gen_assign_code(sym->op, place, NORMAL);
            code2 = MergeCodes(code2, code3);
            code2 = MergeCodes(code2, code4);
            break;
        }
        default:
            Assert(0);
            code2 = NULL;
    }
    return MergeCodes(code1, code2);
}


InterCodes  TransLogic(node_t *node, Operand place){
    InterCodes label1 = new_label();
    InterCodes label2 = new_label();
    InterCodes code0 = gen_assign_code(operand_malloc(CONSTANT_O, 0) ,place, NORMAL);
    InterCodes code1 = TransCond(node, label1->code.u.label_no, label2->code.u.label_no);
    InterCodes code2 = MergeCodes(label1, gen_assign_code(operand_malloc(CONSTANT_O, 1), place, NORMAL));
    code0 = MergeCodes(code0, code1);
    code0 = MergeCodes(code0, code2);
    code0 = MergeCodes(code0, code2);
    return MergeCodes(code0, label2);
}

InterCodes TransArith(node_t *node, Operand place, int kind){
    Operand t1 = new_temp();
    Operand t2 = new_temp();
    InterCodes code1 = TransExp(CHILD(1, node), t1, NULL);
    InterCodes code2 = TransExp(CHILD(3, node), t2, NULL);
    InterCodes code3 = gen_arith_code(place, t1, t2, kind);
    code1 = MergeCodes(code1, code2);
    return MergeCodes(code1, code3);
}

InterCodes TransMinus(node_t *node, Operand place){
    Operand t1 = new_temp();
    InterCodes code1 = TransExp(CHILD(2, node), t1, NULL);
    InterCodes code2 = gen_arith_code(place, operand_malloc(CONSTANT_O, 0), t1, SUB);
    return MergeCodes(code1, code2);
}

InterCodes TransExpFun(node_t *node, Operand place){
    symbol sym = hash_find(CHILD(1, node)->str);
    Assert(node->production_id == 11 || node->production_id == 12);
    switch (node->production_id) {
        case 11:
        {
            Operands var_list = malloc(sizeof(struct Operands_));
            var_list->op = NULL;
            var_list->nxt = NULL;
            InterCodes code1 = TransArgs(CHILD(3, node), var_list);
            if(!strcmp(sym->name, "write")){
                InterCodes code2 = gen_opx_code(var_list->nxt->op, WRITE);
                InterCodes code3 = gen_assign_code(operand_malloc(CONSTANT_O, 0), place, NORMAL);
                code1 = MergeCodes(code1, code2);
                return MergeCodes(code1, code3);
            }
            while(var_list->nxt != NULL){
                InterCodes code2 = gen_opx_code(var_list->nxt->op, ARG);
                code1 = MergeCodes(code1, code2);
                var_list->nxt = var_list->nxt->nxt;
            }InterCodes code3 = gen_call_code(sym->name, place);
            return MergeCodes(code1, code3);
        }
        case 12:
            if(!strcmp(sym->name, "read")){
                return gen_opx_code(place, READ);
            }else{
                return gen_call_code(sym->name, place);
            }
        default:
            Assert(0);
            return NULL;
    }
}

InterCodes TransExpArray(node_t *node, Operand place, Type *ret){
    InterCodes code1 = TransExp(CHILD(1, node), place, ret);
    Operand t1 = new_temp();
    InterCodes code2 = TransExp(CHILD(3, node), t1, NULL);
    *ret = (*ret)->u.array.elem;
    InterCodes code3 = gen_arith_code(t1, t1, operand_malloc(CONSTANT_O, type_size(*ret)), MUL);
    InterCodes code4 = gen_arith_code(place, place, t1, ADD);

    code1 = MergeCodes(code1, code2);
    code1 = MergeCodes(code1, code3);
    return MergeCodes(code1, code4);
}

InterCodes TransExpStruct(node_t *node, Operand place, Type *ret){
    InterCodes code1 = TransExp(CHILD(1, node), place, ret);
    FieldList field = field_find(CHILD(3, node)->str, (*ret)->u.structure);
    Operand c1 = operand_malloc(CONSTANT_O, field->offset);
    InterCodes code2 = gen_arith_code(place, place, c1, ADD);
    *ret = field->type;
    return MergeCodes(code1, code2);
}

InterCodes TransId(node_t *node, Operand place, Type *ret){
    symbol sym = hash_find(node->str);
    if(ret != NULL)
        *ret = sym->u.variable;
    Assert(sym->kind == VARIABLE || sym->kind == PARAM_V);
    if((sym->u.variable->kind == STRUCTURE || sym->u.variable->kind == ARRAY) && sym->kind == VARIABLE)
        return gen_assign_code(sym->op, place, GET_ADDRESS);
    return gen_assign_code(sym->op, place, NORMAL);
}

Type TransType(node_t *node){
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

InterCodes TransArgs(node_t *node, Operands ArgList){
    Assert(node->token_val == Args);
    Assert(node->production_id == 0 || node->production_id == 1);
    Operand t1 = new_temp();
    InterCodes code1 = TransExp(CHILD(1, node), t1, NULL);
    Operands args = malloc(sizeof(struct Operands_));
    args->nxt = ArgList->nxt;
    args->op = t1;
    ArgList->nxt = args;

    if(node->production_id == 0){
        InterCodes code2 = TransArgs(CHILD(3, node), ArgList);
        code1 = MergeCodes(code1, code2);
    }
    return code1;
}

Type TransArray(Type inh, int size){
    Type arr = type_malloc(ARRAY, 0);
    arr->u.array.elem = inh;
    arr->u.array.size = size;
    return arr;
}

InterCodes TransInt(node_t *node, Operand place){
    return gen_assign_code(operand_malloc(CONSTANT_O, str2int(node->str)), place, NORMAL);
}

InterCodes TransFloat(node_t *node, Operand place){
    return NULL;
}

