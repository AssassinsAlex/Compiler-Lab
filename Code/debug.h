#ifndef DEBUG_H
#define DEBUG_H
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ASSERT_ON

#ifdef ASSERT_ON
#define TODO() printf("need to do\n"); //Assert(0)
#define Assert(expr) \
        do{\
            assert(expr);\
        }while(0)       // ?
#else
#define Assert(expr) if(expr) {};
    #define TODO() printf("need to do\n")
#endif

#define false 0
#define true 1
#endif