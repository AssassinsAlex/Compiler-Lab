%option yylineno

%{
    #include "syntax.tab.h"
    #include "multitree.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
    node_t *create_token_node(char *name, char *str);
    extern int get_syn_error;
    extern int error_lineno;
%}

/* regular definitions */
digit [0-9]
letter [a-zA-Z]
dec [1-9]{digit}*
oct 0+[0-7]*
hec 0[xX][0-9A-Fa-f]+


INT {dec}|{oct}|{hec}
FLOAT (([1-9]{digit}*)|0)(\.{digit}+)?([eE][+-]?{digit}+)?
ID ({letter}|_)({letter}|{digit}|_)*
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS \+
MINUS -
STAR \*
DIV \\
AND &&
OR \|\|
DOT \.
NOT !
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
NOTELINE "//"
NOTEALL \/\*(\/)*(\*)*(([^\*\/](\/)*(\*)*))*(\*\/)
CRLF (\r\n)|(\n)
DELIM [ \t\n]+


%%
{INT} {yylval.type_node = create_token_node("INT", yytext); return INT;}
{FLOAT} {yylval.type_node = create_token_node("FLOAT", yytext); return FLOAT;}
{SEMI} {yylval.type_node = create_token_node("SEMI", yytext); return SEMI;}
{COMMA} {yylval.type_node = create_token_node("COMMA", yytext); return COMMA;}
{ASSIGNOP} {yylval.type_node = create_token_node("ASSIGNOP", yytext); return ASSIGNOP;}
{RELOP} {yylval.type_node = create_token_node("RELOP", yytext); return RELOP;}
{PLUS} {yylval.type_node = create_token_node("PLUS", yytext); return PLUS;}
{MINUS} {yylval.type_node = create_token_node("MINUS", yytext); return MINUS;}
{STAR} {yylval.type_node = create_token_node("STAR", yytext); return STAR;}
{DIV} {yylval.type_node = create_token_node("DIV", yytext); return DIV;}
{AND} {yylval.type_node = create_token_node("AND", yytext); return AND;}
{OR} {yylval.type_node = create_token_node("OR", yytext); return OR;}
{DOT} {yylval.type_node = create_token_node("DOT", yytext); return DOT;}
{NOT} {yylval.type_node = create_token_node("NOT", yytext); return NOT;}
{TYPE} {yylval.type_node = create_token_node("TYPE", yytext); return TYPE;}
{LP} {yylval.type_node = create_token_node("LP", yytext); return LP;}
{RP} {yylval.type_node = create_token_node("RP", yytext); return RP;}
{LB} {yylval.type_node = create_token_node("LB", yytext); return LB;}
{RB} {yylval.type_node = create_token_node("RB", yytext); return RB;}
{LC} {yylval.type_node = create_token_node("LC", yytext); return LC;}
{RC} {yylval.type_node = create_token_node("RC", yytext); return RC;}
{STRUCT} {yylval.type_node = create_token_node("STRUCT", yytext); return STRUCT;}
{RETURN} {yylval.type_node = create_token_node("RETURN", yytext); return RETURN;}
{IF} {yylval.type_node = create_token_node("IF", yytext); return IF;}
{ELSE} {yylval.type_node = create_token_node("ELSE", yytext); return ELSE;}
{WHILE} {yylval.type_node = create_token_node("WHILE", yytext); return WHILE;}
{ID} {yylval.type_node = create_token_node("ID", yytext); return ID;}
{NOTELINE} {yylval.type_node = NULL; char c = input(); while(c != '\n') c=input();}
{NOTEALL} {yylval.type_node = NULL; }
{CRLF} {yylval.type_node = NULL; yycolumn = 1;}
{DELIM} {yylval.type_node = NULL;}
. {yylval.type_node = NULL; get_syn_error = 1; 
    if(error_lineno != yylineno){error_lineno=yylineno;
    printf("Error type A at line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}};
%%
