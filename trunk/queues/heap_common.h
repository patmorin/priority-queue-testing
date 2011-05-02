#ifndef HEAP_COMMON
#define HEAP_COMMON

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

//! Operation counters for usage statistics
typedef struct heap_stats_t {
    uint64_t count_insert;
    uint64_t count_find_min;
    uint64_t count_delete_min;
    uint64_t count_meld;
    uint64_t count_decrease_key;
    uint64_t count_delete;
    uint32_t max_size;
} heap_stats;

/**
 * Prints operation statistics by percent usage.
 */
void print_usage_stats( heap_stats *stats );

#endif
