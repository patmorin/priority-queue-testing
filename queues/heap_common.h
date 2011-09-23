#ifndef HEAP_COMMON
#define HEAP_COMMON

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

#ifdef STATS
    #define STAT_STRUCTURE      heap_stats *stats;
    #define ALLOC_STATS         heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );
    #define FREE_STATS          free( heap->stats );
    #define INCR_INSERT         heap->stats->count_insert++;
    #define INCR_FIND_MIN       heap->stats->count_find_min++;
    #define INCR_DELETE_MIN     heap->stats->count_delete_min++;
    #define INCR_DELETE         heap->stats->count_delete++;
    #define INCR_DECREASE_KEY   heap->stats->count_decrease_key++;
    #define INCR_MELD           heap->stats->count_meld++;
    #define ADD_TRAVERSALS(n)   heap->stats->count_traversals += n;
    #define ADD_UPDATES(n)      heap->stats->count_updates += n;
    #define INCR_ALLOCS         heap->stats->count_allocs++;
    #define ADD_SIZE(n)         heap->stats->current_size += n;  if ( heap->stats->current_size > heap->stats->max_size ) heap->stats->max_size = heap->stats->current_size;
    #define SUB_SIZE(n)         heap->stats->current_size -= n;
    #define FIX_MAX_NODES       if ( heap->size > heap->stats->max_nodes ) heap->stats->max_nodes = heap->size;
    #define PRINT_STATS(s)      print_usage_stats(s);
#else
    #define STAT_STRUCTURE
    #define ALLOC_STATS
    #define FREE_STATS
    #define INCR_INSERT
    #define INCR_FIND_MIN
    #define INCR_DELETE_MIN
    #define INCR_DELETE
    #define INCR_DECREASE_KEY
    #define INCR_MELD
    #define ADD_TRAVERSALS(n)
    #define ADD_UPDATES(n)
    #define INCR_ALLOCS
    #define ADD_SIZE(n)
    #define SUB_SIZE(n)
    #define FIX_MAX_NODES
    #define PRINT_STATS(a)
#endif

typedef uint32_t bool;
typedef double key_type, pr_type;
typedef char str16[16];
typedef str16 item_type, in_type;
#define ITEM_ASSIGN(a,b) strncpy(a,b,16)

#define MAX_KEY DBL_MAX

//! Operation counters for usage statistics
typedef struct heap_stats_t {
    uint64_t count_insert;
    uint64_t count_find_min;
    uint64_t count_delete_min;
    uint64_t count_meld;
    uint64_t count_decrease_key;
    uint64_t count_delete;
    uint64_t count_traversals;
    uint64_t count_updates;
    uint64_t count_allocs;
    uint64_t current_size;
    uint64_t max_size;
    uint32_t max_nodes;
} heap_stats;

/**
 * Prints operation statistics by percent usage.
 */
void print_usage_stats( heap_stats *stats );

#endif
