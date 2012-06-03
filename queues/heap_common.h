#ifndef HEAP_COMMON
#define HEAP_COMMON

//==============================================================================
// DEFINES AND INCLUDES
//==============================================================================

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#ifndef DEF_VALUES
    #define DEF_VALUES
    #define FALSE       0
    #define TRUE        1
    #define LEFT        0
    #define MAXRANK     50
    #define ALPHA       0.75
    #define INFINITY    0xFFFFFFFF
#endif

typedef uint32_t bool;
typedef double key_type, pr_type;
typedef char str16[16];
typedef str16 item_type, in_type;
#define ITEM_ASSIGN(a,b) strncpy(a,b,16)

#define MAX_KEY DBL_MAX

#endif
