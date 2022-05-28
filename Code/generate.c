#include "translate.h"
FILE *output_file;
char* reg(Operand x){
    return "v1";
}

void PrintAssign(InterCodes codes){
    Operand left = codes->code.u.assign.left;
    Operand right = codes->code.u.assign.right;
    switch (codes->code.u.assign.kind) {
        case NORMAL:
            if (right->kind == CONSTANT_O)
                fprintf(output_file, "%s := %d", reg(left), right->u.value);
            else
                fprintf(output_file, "%s := %s", reg(left), reg(right));
            break;
        case LEFT:
            fprintf(output_file, "sw %s, 0(%s)", reg(left), reg(right));
            break;
        case GET_ADDRESS:
            fprintf(output_file, " := &");
            break;
        case RIGHT:
            fprintf(output_file, "lw %s, 0(%s)", reg(left), reg(right));
            break;
        default:
            Assert(0);
    }
}

void PrintAdd(InterCodes codes){
    Operand r = codes->code.u.binop.result;
    Operand op1 = codes->code.u.binop.op1;
    Operand op2 = codes->code.u.binop.op2;
    if(op2->kind == CONSTANT_O)
        fprintf(output_file, "addi %s, %s, %d", reg(r), reg(op1), op2->u.value);
    else if(op1->kind == CONSTANT_O)
        fprintf(output_file, "addi %s, %s, %d", reg(r), reg(op2), op1->u.value);
    else
        fprintf(output_file, "add %s, %s, %s", reg(r), reg(op1), reg(op2));

}
void PrintSub(InterCodes codes){
    Operand r = codes->code.u.binop.result;
    Operand op1 = codes->code.u.binop.op1;
    Operand op2 = codes->code.u.binop.op2;
    if(op2->kind == CONSTANT_O)
        fprintf(output_file, "addi %s, %s, -%d", reg(r), reg(op1), op2->u.value);
    else
        fprintf(output_file, "sub %s, %s, %s", reg(r), reg(op1), reg(op2));
}


void PrintCodes(InterCodes codes, char *filename){
    FILE *file = fopen(filename, "w");
    output_file = file;
    while(codes != NULL){
        switch (codes->code.kind) {
            case ASSIGN:
                PrintAssign(codes);
                break;
            case ADD:
                PrintAdd(codes);
                break;
            case SUB:
                PrintSub(codes);
                break;
            case MUL:
                fprintf(file, "mul %s, %s, %s", reg(codes->code.u.binop.result),
                        reg(codes->code.u.binop.op1), reg(codes->code.u.binop.op2));
                break;
            case DIVI:
                fprintf(file, "div %s, %s, %s", reg(codes->code.u.binop.result),
                        reg(codes->code.u.binop.op1), reg(codes->code.u.binop.op2));
                break;
            case FUNCDEF:
                fprintf(file, "%s:", codes->code.u.func_name);
                break;
            case RETURN:
                fprintf(file, "move $v0, %s\n", reg(codes->code.u.op_x));
                fprintf(file, "jr $ra");
                break;
            case LABEL:
                fprintf(file, "label%d:", codes->code.u.label_no);
                break;
            case GOTO:
                fprintf(file, "GOTO label%d", codes->code.u.label_no);
                break;
            case CJP:
                fprintf(file, "IF %s %s %s GOTO label%d", reg(codes->code.u.cjp.x),
                        codes->code.u.cjp.relop, reg(codes->code.u.cjp.y), codes->code.u.cjp.label_no);
                break;
            case CALL:
                fprintf(file, "jal %s", codes->code.u.call.name);
                fprintf(file, "move %s, $v0", reg(codes->code.u.call.x));
                break;
            case DEC:
                fprintf(file, "DEC ");
                //PrintOperand(codes->code.u.dec.x, file);
                fprintf(file, " %d", codes->code.u.dec.size);
                break;
            case READ:
                fprintf(file, "READ ");
                //PrintOperand(codes->code.u.op_x, file);
                break;
            case WRITE:
                fprintf(file, "WRITE ");
                //PrintOperand(codes->code.u.op_x, file);
                break;
            case ARG:
                fprintf(file, "ARG ");
                //PrintOperand(codes->code.u.op_x, file);
                break;
            case PARAM:
                fprintf(file, "PARAM ");
                //PrintOperand(codes->code.u.op_x, file);
                break;
            default:
                break;
        }
        fprintf(file, "\n");
        codes = codes->next;
    }
}