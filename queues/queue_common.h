#ifndef QUEUE_COMMON
#define QUEUE_COMMON

//==============================================================================
// DEFINES AND INCLUDES
//==============================================================================

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "../memory_management.h"
#include "../typedefs.h"

#ifndef DEF_VALUES
    #define DEF_VALUES
    #define FALSE       0
    #define TRUE        1
    #define LEFT        0
    #define MAXRANK     64
    #define ALPHA       0.75
    #define INFINITY    0xFFFFFFFF
#endif

typedef uint32_t bool;
#define ITEM_ASSIGN(a,b) ( a = b )

#define MAX_KEY 0xFFFFFFFFFFFFFFFF

#endif
