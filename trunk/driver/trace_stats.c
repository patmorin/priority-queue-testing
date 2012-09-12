#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "../memory_management.h"
#include "../trace_tools.h"
#include "../typedefs.h"

#ifdef DUMMY
    // This measures the overhead of processing the input files, which should be
    // subtracted from all heap time measurements.  Does some silly stuff to
    // avoid compiler warnings.
    #define pq_create(m)            map
    #define pq_destroy(q)           dummy = ( q == NULL ) ? 1 : 0
    #define pq_clear(q)             dummy = 0
    #define pq_get_key(q,n)         dummy = 0
    #define pq_get_item(q,n)        dummy = 0
    #define pq_get_size(q)          dummy = 0
    #define pq_insert(q,i,k)        n
    #define pq_find_min(q)          dummy = 0
    #define pq_delete(q,n)          dummy = 0
    #define pq_delete_min(q)        dummy = 0
    #define pq_decrease_key(q,n,k)  dummy = 0
    //#define pq_meld(q,r)            dummy = ( q == r ) ? 1 : 0
    #define pq_empty(q)             dummy = 0
    typedef void pq_type;
    typedef void pq_node_type;
    static uint32_t dummy;
#else
    #ifdef USE_BINOMIAL
        #include "../queues/binomial_queue.h"
    #elif USE_EXPLICIT_2
        #include "../queues/explicit_heap.h"
    #elif defined USE_EXPLICIT_4
        #include "../queues/explicit_heap.h"
    #elif defined USE_EXPLICIT_8
        #include "../queues/explicit_heap.h"
    #elif defined USE_EXPLICIT_16
        #include "../queues/explicit_heap.h"
    #elif defined USE_FIBONACCI
        #include "../queues/fibonacci_heap.h"
    #elif defined USE_IMPLICIT_2
        #include "../queues/implicit_heap.h"
    #elif defined USE_IMPLICIT_4
        #include "../queues/implicit_heap.h"
    #elif defined USE_IMPLICIT_8
        #include "../queues/implicit_heap.h"
    #elif defined USE_IMPLICIT_16
        #include "../queues/implicit_heap.h"
    #elif defined USE_PAIRING
        #include "../queues/pairing_heap.h"
    #elif defined USE_QUAKE
        #include "../queues/quake_heap.h"
    #elif defined USE_RANK_PAIRING
        #include "../queues/rank_pairing_heap.h"
    #elif defined USE_RANK_RELAXED_WEAK
        #include "../queues/rank_relaxed_weak_queue.h"
    #elif defined USE_STRICT_FIBONACCI
        #include "../queues/strict_fibonacci_heap.h"
    #elif defined USE_VIOLATION
        #include "../queues/violation_heap.h"
    #endif
#endif

#ifdef USE_STRICT_FIBONACCI
    static uint32_t mem_types = 4;
    static uint32_t mem_sizes[4] =
    {
        sizeof( strict_fibonacci_node ),
        sizeof( fix_node ),
        sizeof( active_record ),
        sizeof( rank_record )
    };
#else
    static uint32_t mem_types = 1;
    static uint32_t mem_sizes[1] =
    {
        sizeof( pq_node_type )
    };
#endif

int main( int argc, char** argv )
{
    uint64_t i;

    // counters for collecting operation stats
    uint64_t count_create = 0;
    uint64_t count_destroy = 0;
    uint64_t count_clear = 0;
    uint64_t count_get_key = 0;
    uint64_t count_get_item = 0;
    uint64_t count_get_size = 0;
    uint64_t count_insert = 0;
    uint64_t count_find_min = 0;
    uint64_t count_delete = 0;
    uint64_t count_delete_min = 0;
    uint64_t count_decrease_key = 0;
    uint64_t count_empty = 0;

    if( argc < 2 )
        exit( -1 );

    int trace_file = open( argv[1], O_RDONLY );
    if( trace_file < 0 )
    {
        printf("Could not open file.\n");
        return -1;
    }

    pq_trace_header header;
    pq_trace_read_header( trace_file, &header );

    //printf("Header: (%llu,%lu,%lu)\n",header.op_count,header.pq_ids,
    //    header.node_ids);

    pq_op_blank *ops = calloc( header.op_count, sizeof( pq_op_blank ) );
    pq_type **pq_index = calloc( header.pq_ids, sizeof( pq_type* ) );
    pq_node_type **node_index = calloc( header.node_ids,
        sizeof( pq_node_type* ) );
    if( ops == NULL || pq_index == NULL || node_index == NULL )
    {
        printf("Calloc fail.\n");
        return -1;
    }
    mem_map *map = mm_create( mem_types, mem_sizes );

    int status;
    for( i = 0; i < header.op_count; i++ )
    {
        status = pq_trace_read_op( trace_file, ops + i );
        if( status == -1 )
        {
            printf("Invalid operation!");
            return -1;
        }
    }

    close( trace_file );
    mm_clear( map );

    for( i = 0; i < header.op_count; i++ )
    {
        switch( ops[i].code )
        {
            case PQ_OP_CREATE:
                count_create++;
                break;
            case PQ_OP_DESTROY:
                count_destroy++;
                break;
            case PQ_OP_CLEAR:
                count_clear++;
                break;
            case PQ_OP_GET_KEY:
                count_get_key++;
                break;
            case PQ_OP_GET_ITEM:
                count_get_item++;
                break;
            case PQ_OP_GET_SIZE:
                count_get_size++;
                break;
            case PQ_OP_INSERT:
                count_insert++;
                break;
            case PQ_OP_FIND_MIN:
                count_find_min++;
                break;
            case PQ_OP_DELETE:
                count_delete++;
                break;
            case PQ_OP_DELETE_MIN:
                count_delete_min++;
                break;
            case PQ_OP_DECREASE_KEY:
                count_decrease_key++;
                break;
            /*case PQ_OP_MELD:
                printf("Meld.\n");
                op_meld = (pq_op_meld*) ( ops + i );
                q = pq_index[op_meld->pq_src1_id];
                r = pq_index[op_meld->pq_src2_id];
                pq_index[op_meld->pq_dst_id] = pq_meld( q, r );
                break;*/
            case PQ_OP_EMPTY:
                count_empty++;
                break;
            default:
                break;
        }
    }

    for( i = 0; i < header.pq_ids; i++ )
    {
        if( pq_index[i] != NULL )
            pq_destroy( pq_index[i] );
    }

    mm_destroy( map );
    free( pq_index );
    free( node_index );
    free( ops );

    printf("create: %llu\n",count_create);
    printf("destroy: %llu\n",count_destroy);
    printf("clear: %llu\n",count_clear);
    printf("get_key: %llu\n",count_get_key);
    printf("get_item: %llu\n",count_get_item);
    printf("get_size: %llu\n",count_get_size);
    printf("insert: %llu\n",count_insert);
    printf("find_min: %llu\n",count_find_min);
    printf("delete: %llu\n",count_delete);
    printf("delete_min: %llu\n",count_delete_min);
    printf("decrease_key: %llu\n",count_decrease_key);
    printf("empty: %llu\n",count_empty);

    return 0;
}
