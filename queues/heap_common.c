#include "heap_common.h"

void print_usage_stats( heap_stats *stats ) {
    uint64_t total = 0;
    total += stats->count_insert;
    total += stats->count_find_min;
    total += stats->count_delete_min;
    total += stats->count_meld;
    total += stats->count_decrease_key;
    total += ( stats->count_delete - stats->count_delete_min );

    float percent_insert =
        100.0*((float)stats->count_insert)/((float)total);
    float percent_find_min =
        100.0*((float)stats->count_find_min)/((float)total);
    float percent_delete_min =
        100.0*((float)stats->count_delete_min)/((float)total);
    float percent_meld =
        100.0*((float)stats->count_meld)/((float)total);
    float percent_decrease_key =
        100.0*((float)stats->count_decrease_key)/((float)total);
    float percent_delete =
        100.0*((float)(stats->count_delete-stats->count_delete_min))/((float)total);
    
    printf( "Usage Stats\n" );
    printf( "===========\n" );
    printf( "\tinsert: %llu (%f)\n", (long long unsigned int) stats->count_insert, percent_insert );
    printf( "\tfind_min: %llu (%f)\n", (long long unsigned int) stats->count_find_min, percent_find_min );
    printf( "\tdelete_min: %llu (%f)\n", (long long unsigned int) stats->count_delete_min, percent_delete_min );
    printf( "\tmeld: %llu (%f)\n", (long long unsigned int) stats->count_meld, percent_meld );
    printf( "\tdecrease_key: %llu (%f)\n", (long long unsigned int) stats->count_decrease_key, percent_decrease_key );
    printf( "\tdelete: %llu (%f)\n", (long long unsigned int) stats->count_delete - stats->count_delete_min, percent_delete );
    printf("\n" );
    printf( "Asymptotics\n" );
    printf( "===========\n" );
    printf( "\tTraversals: %llu\n", (long long unsigned int) stats->count_traversals );
    printf( "\tUpdates: %llu\n", (long long unsigned int) stats->count_updates );
    printf( "\tAllocs: %llu\n", (long long unsigned int) stats->count_allocs );
    printf( "\tMax Size: %llu\n", (long long unsigned int) stats->max_size );
    printf( "\tMax Nodes: %lu\n", (long unsigned int) stats->max_nodes );
    printf( "\n" );
}
