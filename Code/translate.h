#ifndef TRANSLATE_H
#define TRANSLATE_H
#include "debug.h"
#include "symbol.h"

typedef struct Operand_* Operand;
struct Operand_ {
    enum  {VARIABLE_O, CONSTANT_O, ADDRESS_O, TEMPORARY_O} kind;
    union {
        int var_no;
        int value;
    } u;
};

typedef struct Operands_* Operands;
struct Operands_{
    Operand op;
    Operands nxt;
};

typedef struct InterCode{
    enum {LABEL, FUNCDEF, ASSIGN, ADD, SUB, MUL, DIVI, GOTO, CJP, RETURN, ARG, CALL, READ, PARAM, WRITE} kind;
    union {
        int label_no;
        char func_name[NAME_SIZE];
        struct {Operand right, left; enum{NORMAL, GET_ADDRESS, RIGHT, LEFT}kind;} assign;
        struct {Operand result, op1, op2;} binop;
        struct {Operand x, y; char relop[16];int label_no;} cjp;
        struct {Operand x; char name[NAME_SIZE];} call;
        Operand op_x;
    }u;
}InterCode;

typedef struct InterCodes_* InterCodes;
struct InterCodes_{
    InterCode code;
    InterCodes prev, next, tail;
};

InterCodes  code_malloc             ();
Operand     new_temp                ();
Operand     get_variable            (char* name);
InterCodes  MergeCodes              (InterCodes code1, InterCodes code2);

InterCodes  TransProgram            (node_t *node);
InterCodes  TransExtDefList         (node_t *node);
InterCodes  TransExtDef             (node_t *node);
void        TransExtDecList         (node_t *node, Type inh);
Type        TransSpecifier          (node_t *node);
Type        TransStructSpecifier    (node_t *node);
void        TransVarDec             (node_t *node, Type inh, Type structure);
InterCodes  TransFunDef             (node_t *node, Type inh);
void        TransFunDec             (node_t *node, Type inh);
void        TransVarList            (node_t *node);
void        TransParamDec           (node_t *node);
InterCodes  TransDefList            (node_t *node, Type inh);
InterCodes  TransDef                (node_t *node, Type structure);
InterCodes  TransDecList            (node_t *node, Type inh, Type structure);
InterCodes  TransDec                (node_t *node, Type inh, Type structure);
InterCodes  TransCompSt             (node_t *node, int isFun);
InterCodes  TransStmtList           (node_t *node);
InterCodes  TransStmt               (node_t *node);
InterCodes  TransExp                (node_t *node, Operand place);
InterCodes  TransArgs               (node_t *node, Operands ArgList);
InterCodes  TransId                 (node_t *node, Operand place);
Type        TransType               (node_t *node);
Type        TransArray              (Type inh, int size);
InterCodes  TransReturn             (node_t *node);
InterCodes  TransWhile              (node_t *node);
InterCodes  TransIf                 (node_t *node);
InterCodes  TransIfEl               (node_t *node);

InterCodes  TransExpFun             (node_t *node, Operand place);
InterCodes  TransExpArray           (node_t *node, Operand place, Type *ret);
InterCodes  TransExpStruct          (node_t *node, Operand place, Type *ret);


InterCodes  TransCond               (node_t *node, int LabelTrue, int LabelFalse);
InterCodes  TransAssign             (node_t *node, Operand place);
InterCodes  TransArith              (node_t *node, Operand place, int kind);
InterCodes  TransMinus              (node_t *node, Operand place);
InterCodes  TransLogic              (node_t *node, Operand place);
InterCodes  TransInt                (node_t *node, Operand place);
InterCodes  TransFloat              (node_t *node, Operand place);

void        PrintCodes              (InterCodes codes);
#endif