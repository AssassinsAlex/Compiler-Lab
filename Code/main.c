#include <stdio.h>
//#define DEBUG
extern int yylineno;
extern int yydebug;
extern int error_lineno;
extern void yyrestart(FILE *input_file);
extern int yyparse (void);
int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 0x1;
    }
    yylineno = 1;
    error_lineno = 0;
    #ifdef DEBUG
        yydebug = 1;
    #endif
    yyrestart(f);
    yyparse();
    return 0;
}
