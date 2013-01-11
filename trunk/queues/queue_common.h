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

#ifdef USE_EAGER
    #include "../memory_management_eager.h"
#elif USE_LAZY
    #include "../memory_management_lazy.h"
#else
    #include "../memory_management_dumb.h"
#endif

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

#define OCCUPIED(a,b)       ( a & ( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_SET(a,b)   ( a |= ( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_UNSET(a,b) ( a &= ~( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_LEADER(a)  ( (uint32_t) __builtin_ctzll( a ) )

#endif
