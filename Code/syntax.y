%locations
%{
    extern int yylex (void);
    #include "multitree.h"
    int prompt(char *msg);
    int yyerror(char *msg);

%}

/* declared types */
%union {
    node_t *type_node;
}

%token <type_node> INT
%token <type_node> FLOAT
%token <type_node> ID
%token <type_node> SEMI
%token <type_node> COMMA
%token <type_node> ASSIGNOP
%token <type_node> RELOP
%token <type_node> PLUS
%token <type_node> MINUS
%token <type_node> STAR
%token <type_node> DIV
%token <type_node> AND
%token <type_node> OR
%token <type_node> DOT
%token <type_node> NOT
%token <type_node> TYPE
%token <type_node> LP
%token <type_node> RP
%token <type_node> LB
%token <type_node> RB
%token <type_node> LC
%token <type_node> RC
%token <type_node> STRUCT 
%token <type_node> RETURN
%token <type_node> IF
%token <type_node> ELSE
%token <type_node> WHILE

%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS SUB
%left STAR DIV
%right MINUS NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/* High-level Definitions */
Program : ExtDefList {$$ = trans_tree("Program", @$.first_line, 1, $1, @1.first_line);CST = $$;}
    ;
ExtDefList : ExtDef ExtDefList {$$ = trans_tree("ExtDefList", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | /* empty */ {$$ = NULL;}
    ;
ExtDef : Specifier ExtDecList SEMI {$$ = trans_tree("ExtDef", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Specifier SEMI {$$ = trans_tree("ExtDef", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | Specifier FunDec CompSt {$$ = trans_tree("ExtDef", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    
    | Specifier error SEMI {$$ = NULL;prompt("wrong extdef");}
    | error SEMI {$$ = NULL; prompt("wrong extdef");}
    | Specifier error{$$ = NULL; prompt("need ;");};
    ;
ExtDecList : VarDec {$$ = trans_tree("ExtDecList", @$.first_line, 1, $1, @1.first_line);}
    | VarDec COMMA ExtDecList {$$ = trans_tree("ExtDecList", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    ;

/* Specifiers */
Specifier : TYPE {$$ = trans_tree("Specifier", @$.first_line, 1, $1, @1.first_line);}
    | StructSpecifier {$$ = trans_tree("Specifier", @$.first_line, 1, $1, @1.first_line);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = trans_tree("StructSpecifier", @$.first_line, 5, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line, $5, @5.first_line);}
    | STRUCT Tag {$$ = trans_tree("StructSpecifier", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | STRUCT error RC {$$ = NULL;prompt("wrong struct");}
    ;
OptTag : ID {$$ = trans_tree("OptTag", @$.first_line, 1, $1, @1.first_line);}
    | /* empty */ {$$ = NULL;}
    ;
Tag : ID {$$ = trans_tree("Tag", @$.first_line, 1, $1, @1.first_line);}
    ;

/* Declarations */
VarDec : ID {$$ = trans_tree("VarDec", @$.first_line, 1, $1, @1.first_line);}
    | VarDec LB INT RB {$$ = trans_tree("VarDec", @$.first_line, 4, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line);}                                          
    ;
FunDec : ID LP VarList RP {$$ = trans_tree("FunDec",@$.first_line, 4, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line);}
    | ID LP RP {$$ = trans_tree("FunDec", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | ID error RP {$$ = NULL; prompt("wrong funcdec");}
    ;
VarList : ParamDec COMMA VarList {$$ = trans_tree("VarList", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | ParamDec {$$ = trans_tree("VarList", @$.first_line, 1, $1, @1.first_line);}
    ;
ParamDec : Specifier VarDec {$$ = trans_tree("ParamDec", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    ;

/* Statements */
CompSt : LC DefList StmtList RC {$$ = trans_tree("CompSt", @$.first_line, 4, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line);}
    | error RC {$$ = NULL; prompt("wrong compst");}
    ;
StmtList : Stmt StmtList {$$ = trans_tree("StmtList", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | /* empty */ {$$ = NULL;}
    ;
Stmt : Exp SEMI {$$ = trans_tree("Stmt", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | CompSt {$$ = trans_tree("Stmt", @$.first_line, 1, $1, @1.first_line);}
    | RETURN Exp SEMI {$$ = trans_tree("Stmt", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {$$ = trans_tree("Stmt", @$.first_line, 5, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line, $5, @5.first_line);}
    | IF LP Exp RP Stmt ELSE Stmt {$$ = trans_tree("Stmt", @$.first_line, 7, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line, $5, @5.first_line, $6, @6.first_line, $7, @7.first_line);}
    | WHILE LP Exp RP Stmt {$$ = trans_tree("Stmt", @$.first_line, 5, $1, @1.first_line, $2, @2.first_line, 
                                                $3, @3.first_line, $4, @4.first_line, $5, @5.first_line);}
    | error SEMI {$$ = NULL;prompt("wrong stmt");}
    | Exp error SEMI {$$ = NULL; prompt("wrong stmt");}
    | IF LP error RP Stmt {$$ = NULL; prompt("wrong stmt");} 
    | IF LP error RP Stmt ELSE Stmt {$$ = NULL; prompt("wrong stmt");}
    | WHILE LP error RP Stmt {$$ = NULL; prompt("wrong stmt");}
    ;

/* Local Deifinitons */
DefList : Def DefList {$$ = trans_tree("DefList", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | /* empty */ {$$ = NULL;}
    ;
Def : Specifier DecList SEMI {$$ = trans_tree("Def", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Specifier error SEMI {$$ = NULL;prompt("wrong define");yyerrok;}
    ;
DecList : Dec {$$ = trans_tree("DecList", @$.first_line, 1, $1, @1.first_line);}
    | Dec COMMA DecList {$$ = trans_tree("DecList", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    ;
Dec : VarDec {$$ = trans_tree("Dec", @$.first_line, 1, $1, @1.first_line);}
    | VarDec ASSIGNOP Exp {$$ = trans_tree("Dec", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    ;

/* Expressions */
Exp : Exp ASSIGNOP Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp AND Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp OR Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp RELOP Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp PLUS Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp MINUS Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp STAR Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp DIV Exp {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | LP Exp RP {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | MINUS Exp {$$ = trans_tree("Exp", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | NOT Exp {$$ = trans_tree("DefList", @$.first_line, 2, $1, @1.first_line, $2, @2.first_line);}
    | ID LP Args RP {$$ = trans_tree("Exp", @$.first_line, 4, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line, $4, @4.first_line);}
    | ID LP RP {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | Exp LB Exp RB {$$ = trans_tree("Exp", @$.first_line, 4, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line, $4, @4.first_line);}
    | Exp DOT ID {$$ = trans_tree("Exp", @$.first_line, 3, $1, @1.first_line, $2, @2.first_line, $3, @3.first_line);}
    | ID {$$ = trans_tree("Exp", @$.first_line, 1, $1, @1.first_line);}
    | INT {$$ = trans_tree("Exp", @$.first_line, 1, $1, @1.first_line);}
    | FLOAT {$$ = trans_tree("Exp", @$.first_line, 1, $1, @1.first_line);}
    ;
Args : Exp COMMA Args
    | Exp
    ;

%%
#include "lex.yy.c"

int yyerror(char* msg) {
    get_syn_error = 1;
    if(error_lineno != yylineno){
        printf("Error type B at line %d: %s\n", yylineno, msg);
        error_lineno = yylineno;
    }
}
int prompt(char *msg) {
    //fprintf(stderr, " %s\n", msg);
}
