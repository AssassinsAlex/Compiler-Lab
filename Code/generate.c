#include "translate.h"
FILE *output_file;

enum {t0 = 0, t1, t2, t3, t4, t5, t6, t7, t8, t9, s0, s1, s2, s3, s4, s5, s6, s7, a0, a1, a2, a3, fp, v0, v1,  z0, at, k0, k1, gp, sp, ra} reg_no;
Operand reg[32];
char* reg_name[] = {"$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9",
                    "$s0","$s1", "$s2","$s3", "$s4", "$s5", "$s6", "$s7", "$a0", "$a1", "$a2", "$a3",
                    "$fp", "$v0", "$v1", "$0", "$at", "$k0", "$k1", "$gp", "$sp", "$ra"};

#define FRAMESIZE  4*10

struct stack_node{
    Operand op[8];
    int sp_offset;
    int arg_count;
    Operands arg_list;
    int param_count;
    int st_lk;
}func_status;


void init(){
    for(int i =0 ; i < 32;i++)
        reg[i] = NULL;
    char *init_code =
            ".data\n"
            "_ret: .asciiz \"\\n\"\n"
            ".globl main\n"
            ".text\n"
            "read:\n"
            "  li $v0, 5\n"
            "  syscall\n"
            "  jr $ra\n"
            "write:\n"
            "  li $v0, 1\n"
            "  syscall\n"
            "  li $v0, 4\n"
            "  la $a0, _ret\n"
            "  syscall\n"
            "  move $v0, $0\n"
            "  jr $ra\n\n"
    ;
    fprintf(output_file,"%s", init_code);
}

void _allocate(Operand x, int r){
    x->reg_no = r;
    reg[r] = x;
}

void Save(Operand x){
    if(x->offset < 0){
        if(func_status.st_lk)
            return;
        fprintf(output_file, "addi $sp, $sp, -4\n");
        fprintf(output_file, "sw %s, 0($sp)\n", reg_name[x->reg_no]);
        func_status.sp_offset += 4;
        x->offset = func_status.sp_offset;
    }else
        fprintf(output_file, "sw %s, -%d($fp)\n", reg_name[x->reg_no], x->offset);
}

void _free(Operand x){
    reg[x->reg_no] = NULL;
    x->reg_no = -1;
}

void SaveFree(int r){
    if(reg[r] != NULL) {
        Save(reg[r]);
        _free(reg[r]);
    }
}

void Free(Operand x){
    if(x->nxt_code > 0) return;
    if(x->kind == VARIABLE_O){
        Save(x);
    }
    _free(x);
}

int allocate(Operand x){
    if(x->reg_no >= 0)
        return x->reg_no;
    for(int i = 0; i <= s7; i++){
        if(reg[i] == NULL) {
            _allocate(x, i);
            return i;
        }
    }
    for(int i = 0; i <= s7; i++){
        if(reg[i]->nxt_code < 0){
            Free(reg[i]);
            _allocate(x, i);
            return i;
        }
    }
    int max = -1;
    int r = 0;
    for(int i = 0; i <= s7; i++){
        if(reg[i]->nxt_code > max){
            r = i;
            max = reg[i]->nxt_code;
        }
    } Save(reg[r]);
    _allocate(x, r);
    return r;
}

int ensure(Operand x){
    if(x->reg_no >= 0)
        return x->reg_no;
    else{
        int r = allocate(x);
        if(x->kind == CONSTANT_O)
            fprintf(output_file, "li %s, %d\n", reg_name[r], x->u.value);
        else {
            Assert(x->offset != -1);
            fprintf(output_file, "lw %s, -%d($fp)\n", reg_name[r], x->offset);
        }
        return r;
    }
}

void allocate_stack(Operand op, int size){
    if(op->kind != VARIABLE_O || op->offset >= 0)
        return;
    func_status.sp_offset += size;
    op->offset = func_status.sp_offset;

}

void prologue(InterCodes codes){
    InterCodes cur = codes->next;
    Assert(codes->code.kind == FUNCDEF);
    for(int i = 0; i < 8; i++)
        func_status.op[i] = NULL;
    func_status.sp_offset = FRAMESIZE;
    func_status.arg_count = 0;
    func_status.param_count = 0;
    func_status.arg_list = NULL;
    func_status.st_lk = false;
    while(cur != NULL && cur->code.kind != FUNCDEF){
        switch (cur->code.kind) {
            case LABEL:case GOTO:case FUNCDEF:
                break;
            case ASSIGN:
                allocate_stack(cur->code.u.assign.left, 4);
                allocate_stack(cur->code.u.assign.right, 4);
                break;
            case ADD:case SUB:case MUL:case DIVI:
                allocate_stack(cur->code.u.binop.op1, 4);
                allocate_stack(cur->code.u.binop.op2, 4);
                allocate_stack(cur->code.u.binop.result, 4);
                break;
            case CJP:
                allocate_stack(cur->code.u.cjp.x, 4);
                allocate_stack(cur->code.u.cjp.y, 4);
                break;
            case RETURN:case WRITE:case READ:case ARG:
            case CALL:case PARAM:
                allocate_stack(cur->code.u.op_x, 4);
                break;
            case DEC:
                allocate_stack(cur->code.u.dec.x, cur->code.u.dec.size);
                break;
        }cur = cur->next;
    }
    fprintf(output_file, "addi $sp, $sp, -%d\n", func_status.sp_offset);
    fprintf(output_file, "sw $ra, %d($sp)\n", func_status.sp_offset-4);
    fprintf(output_file, "sw $fp, %d($sp)\n", func_status.sp_offset-8);
    fprintf(output_file, "addi, $fp, $sp, %d\n", func_status.sp_offset);
    for(int i = 0; i < 8; i++) {
        int r = i + s0;
        if(reg[r] != NULL) {
            func_status.op[i] = reg[r];
            fprintf(output_file, "sw, %s, -%d($fp)\n", reg_name[r], 12 + i * 4);
            func_status.op[i]->reg_no = -1;
            reg[r] = NULL;
        }else
            func_status.op[i] = NULL;

    }
}

void epilogue(){
    for(int i = t0; i < s7; i++){
        if(reg[i] != NULL){
            reg[i]->reg_no = -1;
            reg[i] = NULL;
        }
    }
    for(int i = 0; i < 8; i++) {
        if(func_status.op[i] != NULL) {
            reg[i+s0] = func_status.op[i];
            func_status.op[i]->reg_no = i+s0;
            fprintf(output_file, "lw, %s, -%d($fp)\n", reg_name[i + s0], 12 + i * 4);
        }
    }
    fprintf(output_file, "lw $ra, -4($fp)\n");
    fprintf(output_file, "move $sp, $fp\n");
    fprintf(output_file, "lw $fp, -8($sp)\n");
}

void PrintAssign(InterCodes codes){
    Operand left = codes->code.u.assign.left;
    Operand right = codes->code.u.assign.right;
    int rl = allocate(left);
    int rr = -1;
    if(right->kind != CONSTANT_O && codes->code.u.assign.kind != GET_ADDRESS)
        rr = ensure(right);
    switch (codes->code.u.assign.kind) {
        case NORMAL:
            if (right->kind == CONSTANT_O)
                fprintf(output_file, "li %s, %d\n", reg_name[rl], right->u.value);
            else
                fprintf(output_file, "move %s, %s\n", reg_name[rl], reg_name[rr]);
            break;
        case LEFT:
            fprintf(output_file, "sw %s, 0(%s)\n", reg_name[rr], reg_name[rl]);
            break;
        case GET_ADDRESS:
            fprintf(output_file, "addi %s, %s, -%d\n", reg_name[rl], reg_name[fp], right->offset);
            break;
        case RIGHT:
            fprintf(output_file, "lw %s, 0(%s)\n", reg_name[rl], reg_name[rr]);
            break;
        default:
            Assert(0);
    }
    if(right->kind != CONSTANT_O && codes->code.u.assign.kind != GET_ADDRESS)
        Free(right);
}

void PrintAdd(InterCodes codes){
    Operand op1 = codes->code.u.binop.op1;
    Operand op2 = codes->code.u.binop.op2;
    int rr = allocate(codes->code.u.binop.result);
    int rop1 = ensure(op1);
    if(op2->kind == CONSTANT_O)
        fprintf(output_file, "addi %s, %s, %d\n", reg_name[rr], reg_name[rop1], op2->u.value);
    else {
        fprintf(output_file, "add %s, %s, %s\n", reg_name[rr], reg_name[rop1], reg_name[ensure(op2)]);
        Free(op2);
    }
    Free(op1);

}
void PrintSub(InterCodes codes){
    Operand op1 = codes->code.u.binop.op1;
    Operand op2 = codes->code.u.binop.op2;
    int rr = allocate(codes->code.u.binop.result);
    int rop1 = ensure(op1);
    if(op2->kind == CONSTANT_O)
        fprintf(output_file, "addi %s, %s, -%d\n",  reg_name[rr], reg_name[rop1], op2->u.value);
    else {
        fprintf(output_file, "sub %s, %s, %s\n", reg_name[rr], reg_name[rop1], reg_name[ensure(op2)]);
        Free(op2);
    }
    Free(op1);
}

void PrintBinop(InterCodes codes){
    Operand op1 = codes->code.u.binop.op1;
    Operand op2 = codes->code.u.binop.op2;
    int rr = allocate(codes->code.u.binop.result);
    char *sign;
    if(codes->code.kind == MUL)
        sign = "mul";
    else if(codes->code.kind == DIVI)
        sign = "div";
    else
        Assert(0);
    fprintf(output_file, "%s %s, %s, %s\n", sign, reg_name[rr], reg_name[ensure(op1)], reg_name[ensure(op2)]);
    Free(op1);
    Free(op2);
}

int check_in(Operands arg_list, Operand op){
    while(arg_list != NULL){
        if(arg_list->op == op)
            return true;
        arg_list = arg_list->nxt;
    }return false;
}

void PrintCodes(InterCodes codes) {
    switch (codes->code.op_num) {
        case 1:
            codes->code.u.op_x->nxt_code = codes->code.nxt_code[0];
            break;
        case 2:
            codes->code.u.assign.left->nxt_code =  codes->code.nxt_code[0];
            codes->code.u.assign.right->nxt_code = codes->code.nxt_code[1];
            break;
        case 3:
            codes->code.u.binop.result->nxt_code = codes->code.nxt_code[0];
            codes->code.u.binop.op1->nxt_code = codes->code.nxt_code[1];
            codes->code.u.binop.op2->nxt_code = codes->code.nxt_code[2];
        default:
            break;
    }
    switch (codes->code.kind) {
        case ASSIGN:
            PrintAssign(codes);break;
        case ADD:
            PrintAdd(codes);break;
        case SUB:
            PrintSub(codes);break;
        case MUL:case DIVI:
            PrintBinop(codes);break;
        case FUNCDEF:
            fprintf(output_file, "%s:\n", codes->code.u.func_name);
            prologue(codes);
            break;
        case RETURN:
            fprintf(output_file, "move $v0, %s\n", reg_name[ensure(codes->code.u.op_x)]);
            Free(codes->code.u.op_x);
            epilogue();
            fprintf(output_file, "jr $ra\n");
            break;
        case LABEL:
            fprintf(output_file, "label%d:\n", codes->code.u.label_no);
            break;
        case GOTO:
            fprintf(output_file, "j label%d\n", codes->code.u.label_no);
            break;
        case CJP: {
            for(int i = 0; i < s7; i++){
                if(reg[i] != NULL) {
                    if (reg[i] != codes->code.u.cjp.x && reg[i] != codes->code.u.cjp.y)
                        Free(reg[i]);
                }
            }
            char sign[16];
            if(!strcmp(codes->code.u.cjp.relop, "=="))
                strcpy(sign, "beq");
            else if(!strcmp(codes->code.u.cjp.relop, "!="))
                strcpy( sign, "bne");
            else if(!strcmp(codes->code.u.cjp.relop, ">"))
                strcpy(sign, "bgt");
            else if(!strcmp(codes->code.u.cjp.relop, "<"))
                strcpy(sign, "blt");
            else if(!strcmp(codes->code.u.cjp.relop, ">="))
                strcpy(sign, "bge");
            else if(!strcmp(codes->code.u.cjp.relop, "<="))
                strcpy(sign, "ble");
            else
                Assert(0);
            fprintf(output_file, "%s %s, %s, label%d\n", sign, reg_name[ensure(codes->code.u.cjp.x)],
                    reg_name[ensure(codes->code.u.cjp.y)], codes->code.u.cjp.label_no);
            Free(codes->code.u.cjp.x);
            Free(codes->code.u.cjp.y);
            break;
        }
        case CALL: {
            Operands arg_list = func_status.arg_list;
            for (int i = 0; i < s7; i++) {
                if(reg[i] != NULL) {
                    if(check_in(arg_list, reg[i])) {
                        if (reg[i]->nxt_code > 0)
                            SaveFree(i);
                    }
                    else
                        SaveFree(i);
                }
            }
            func_status.st_lk = true;
            int min = (func_status.arg_count < 4) ? func_status.arg_count : 4;
            arg_list = func_status.arg_list;
            for(int i = 0; i < min; i++){
                fprintf(output_file, "move %s, %s\n", reg_name[i+a0], reg_name[ensure( arg_list->op)]);
                arg_list = arg_list->nxt;
            }int size = func_status.arg_count - 4;
            if(size > 0) {
                fprintf(output_file, "addi $sp, $sp, -%d\n", size * 4);
                for (int i = 0; i < size; i++) {
                    fprintf(output_file, "sw %s, %d($sp)\n", reg_name[ensure(arg_list->op)], i * 4);
                    arg_list = arg_list->nxt;
                }
            }func_status.st_lk = false;
            for(int i = 0; i < t9; i++){
                if(reg[i] != NULL)
                    reg[i]->reg_no = -1;
                reg[i] = NULL;
            }
            fprintf(output_file, "jal %s\n", codes->code.u.call.name);
            fprintf(output_file, "move %s, $v0\n", reg_name[allocate(codes->code.u.call.x)]);
            func_status.arg_list = NULL;
            func_status.arg_count = 0;
            break;
        }
        case DEC:
            Assert(codes->code.u.dec.x->offset >= 0);
            break;
        case READ:
            fprintf(output_file, "jal read\n");
            fprintf(output_file, "move %s, $v0\n", reg_name[allocate(codes->code.u.call.x)]);
            break;
        case WRITE:
            fprintf(output_file, "move $a0, %s\n", reg_name[ensure(codes->code.u.call.x)]);
            fprintf(output_file, "jal write\n");
            Free(codes->code.u.call.x);
            break;
        case ARG:
            func_status.arg_count++;
            Operands arg = malloc(sizeof(struct Operands_));
            arg->op = codes->code.u.op_x;
            arg->nxt = func_status.arg_list;
            func_status.arg_list = arg;
            break;
        case PARAM:
            if(func_status.param_count < 4)
                fprintf(output_file, "move %s, %s\n", reg_name[allocate(codes->code.u.op_x)], reg_name[func_status.param_count+a0]);
            else
                fprintf(output_file, "lw %s, %d($fp)\n", reg_name[allocate(codes->code.u.op_x)], (func_status.param_count-4)*4);
            func_status.param_count++;
            break;
        default:
            break;
    }

}
void scan_mark(InterCodes codes, int num){
    switch (codes->code.op_num) {
        case 1:
            codes->code.nxt_code[0] = codes->code.u.op_x->nxt_code;
            break;
        case 2:
            codes->code.nxt_code[0] = codes->code.u.assign.left->nxt_code;
            codes->code.nxt_code[1] = codes->code.u.assign.right->nxt_code;
            break;
        case 3:
            codes->code.nxt_code[0] = codes->code.u.binop.result->nxt_code;
            codes->code.nxt_code[1] = codes->code.u.binop.op1->nxt_code;
            codes->code.nxt_code[2] = codes->code.u.binop.op2->nxt_code;
        default:
            break;
    }
    switch (codes->code.kind) {
        case LABEL:case FUNCDEF:case GOTO:
            break;
        case ASSIGN:{
            if(codes->code.u.assign.kind == NORMAL){
                codes->code.u.assign.left->nxt_code = -1;
                codes->code.u.assign.right->nxt_code = num;
            } else if(codes->code.u.assign.kind == LEFT){
                codes->code.u.assign.left->nxt_code = num;
                codes->code.u.assign.right->nxt_code = num;
            } else if(codes->code.u.assign.kind == RIGHT){
                codes->code.u.assign.left->nxt_code = -1;
                codes->code.u.assign.right->nxt_code = num;
            } else if(codes->code.u.assign.kind == GET_ADDRESS){
                codes->code.u.assign.left->nxt_code = -1;
                codes->code.u.assign.right->nxt_code = num;
            } else
                Assert(0);
            break;
        }
        case ADD:case SUB:case MUL:case DIVI:
            codes->code.u.binop.result->nxt_code = -1;
            codes->code.u.binop.op1->nxt_code = num;
            codes->code.u.binop.op2->nxt_code = num;
            break;
        case CJP:
            codes->code.u.cjp.x->nxt_code = num;
            codes->code.u.cjp.y->nxt_code = num;
        case RETURN:case WRITE:case READ:case ARG:
            codes->code.u.op_x->nxt_code = num;
            break;
        case CALL:
            codes->code.u.op_x->nxt_code = -1;
            break;
        case PARAM:
            codes->code.u.op_x->nxt_code = -1;
            break;
        case DEC:
            break;
    }
}

void DividingBlock(InterCodes codes, char *filename){
    output_file = fopen(filename, "w");
    init();
    while(codes != NULL) {
        InterCodes cur_code = codes;
        int num = 0;
        while(cur_code->next != NULL){
            InterCodes nxt_code = cur_code->next;
            if(nxt_code->code.kind == LABEL || nxt_code->code.kind == GOTO || nxt_code->code.kind == FUNCDEF)
                break;
            if(cur_code->code.kind == CJP)
                break;
            num++;
            cur_code = nxt_code;
        }

        for(int i = num; i >= 0;cur_code = cur_code->prev, i--) {
            scan_mark(cur_code, i);
        }
        for(int i = 0; i <= num; i++){
            PrintCodes(codes);
            codes = codes->next;
        }for(int i = t0; i <= s7; i++) {
            if(reg[i] != NULL)
                Free(reg[i]);
        }
    }
}

