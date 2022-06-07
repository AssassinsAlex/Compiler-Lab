#include <stdio.h>
//#define BISON_DEBUG
extern int yylineno;
extern int yydebug;
extern void yyrestart(FILE *input_file);
extern int yyparse (void);
#include "multitree.h"
#include "translate.h"

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 0x1;
    }
    yylineno = 1;
    error_lineno = 0;
    #ifdef BISON_DEBUG
        yydebug = 1;
    #endif
    yyrestart(f);
    yyparse();
    if(!get_syn_error){
        //print_tree(CST, 0);
        SddProgram(CST);
    }
    if(!is_semantic_error) {
        InterCodes codes = TransProgram(CST);
        //PrintInterCodes(codes, "out1.ir");
        DividingBlock(codes, argv[2]);
    }
    return 0;
}
