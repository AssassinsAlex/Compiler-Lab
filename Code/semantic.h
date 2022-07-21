#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "debug.h"
#include "symbol.h"

void    SddProgram          (node_t *node);
void    SddExtDefList       (node_t *node);
void    SddExtDef           (node_t *node);
void    SddExtDecList       (node_t *node, Type inh);
Type    SddSpecifier        (node_t *node);
Type    SddStructSpecifier  (node_t *node);
void    SddVarDec           (node_t *node, Type inh, Type structure);
void    SddFunDef           (node_t *node, Type inh);
void    SddFunDec           (node_t *node, Type inh);
void    SddVarList          (node_t *node);
void    SddParamDec         (node_t *node);
void    SddDefList          (node_t *node, Type structure);
void    SddDef              (node_t *node, Type structure);
void    SddDecList          (node_t *node, Type inh, Type structure);
void    SddDec              (node_t *node, Type inh, Type structure);
void    SddCompSt           (node_t *node, int isFun, Type ret);
void    SddStmtList         (node_t *node, Type ret);
void    SddStmt             (node_t *node, Type ret);
Type    SddExp              (node_t *node, int isLeft);
void    SddArgs             (node_t *node, FieldList ArgList);
Type    SddId               (node_t *node);
Type    SddType             (node_t *node);
Type    SddArray            (Type inh, int size);
void    SddReturn           (node_t *node, Type ret);
Type    SddExpFun           (node_t *node, int isLeft);
Type    SddExpArray         (node_t *node, int isLeft);
Type    SddExpStruct        (node_t *node, int isLeft);

Type    CheckInt2           (Type type1, Type type2, int lineno);
Type    CheckInt1           (Type type, int lineno);
Type    CheckAssign         (Type type1, Type type2, int lineno);
Type    CheckArithm2        (Type type1, Type type2, int lineno);
Type    CheckArithm1        (Type type, int lineno);
void    CheckFun            ();


#endif
