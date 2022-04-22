#ifndef TRANSLATE_H
#define TRANSLATE_H
#include "debug.h"

typedef struct Operand_* Operand;
struct Operand_ {
    enum  {VAIABLE, CONSTANT, ADDRESS} kind;
    union {
        int var_no;
        int value;
    } u;
};

struct InterCode{
    enum {ASSIGN, ADD, SUB, MUL} kind;
    union {
        struct {Operand right, left;} assign;
        struct {Operand result, op1, op2;} binop;
    }u;
};
int Trans_Program();
#endif