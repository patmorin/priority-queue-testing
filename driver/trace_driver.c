#include <stdio.h>
#include <stdlib.h> 
#include <sys/time.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "../memory_management.h"
#include "../trace_tools.h"
#include "../typedefs.h"

#define PQ_MIN_USEC 2000000

#ifdef DUMMY
    // This measures the overhead of processing the input files, which should be 
    // subtracted from all heap time measurements.  Does some silly stuff to
    // avoid compiler warnings.
    #define pq_create(m)            map
    #define pq_destroy(q)           dummy = ( q == NULL ) ? 1 : 0
    #define pq_clear(q)
    #define pq_get_key(q,n)         dummy = 0
    #define pq_get_item(q,n)        dummy = 0
    #define pq_get_size(q)          dummy = 0
    #define pq_insert(q,i,k)        n
    #define pq_find_min(q)          dummy = 0
    #define pq_delete(q,n)          dummy = 0
    #define pq_delete_min(q)        dummy = 0
    #define pq_decrease_key(q,n,k)
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
    #elif defined USE_VIOLATION
        #include "../queues/violation_heap.h"
    #endif
#endif

int main( int argc, char** argv )
{
    uint64_t i;

    // pointers for casting
    pq_op_create *op_create;
    pq_op_destroy *op_destroy;
    pq_op_clear *op_clear;
    pq_op_get_key *op_get_key;
    pq_op_get_item *op_get_item;
    pq_op_get_size *op_get_size;
    pq_op_insert *op_insert;
    pq_op_find_min *op_find_min;
    pq_op_delete *op_delete;
    pq_op_delete_min *op_delete_min;
    pq_op_decrease_key *op_decrease_key;
    //pq_op_meld *op_meld;
    pq_op_empty *op_empty;

    // temp dummies for readability
    pq_type *q;//, *r;
    pq_node_type *n;

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
    mem_map *map = mm_create( sizeof( pq_node_type ) );

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

    struct timeval t0, t1;
    uint32_t iterations = 0;
    uint32_t total_time = 0;    
    while( total_time < PQ_MIN_USEC )
    {
        mm_clear( map );
        iterations++;
        gettimeofday(&t0, NULL);
        
        for( i = 0; i < header.op_count; i++ )
        {
            switch( ops[i].code )
            {
                case PQ_OP_CREATE:
                    op_create = (pq_op_create*) ( ops + i );
                    //printf("pq_create(%d)\n", op_create->pq_id);
                    pq_index[op_create->pq_id] = pq_create( map );
                    break;
                case PQ_OP_DESTROY:
                    op_destroy = (pq_op_destroy*) ( ops + i );
                    //printf("pq_destroy(%d)\n", op_destroy->pq_id);
                    q = pq_index[op_destroy->pq_id];
                    pq_destroy( q );
                    pq_index[op_destroy->pq_id] = NULL;
                    break;
                case PQ_OP_CLEAR:
                    op_clear = (pq_op_clear*) ( ops + i );
                    //printf("pq_clear(%d)\n", op_clear->pq_id );
                    q = pq_index[op_clear->pq_id];
                    pq_clear( q );
                    break;
                case PQ_OP_GET_KEY:
                    op_get_key = (pq_op_get_key*) ( ops + i );
                    //printf("pq_get_key(%d,%d)\n", op_get_key->pq_id,
                    //    op_get_key->node_id );
                    q = pq_index[op_get_key->pq_id];
                    n = node_index[op_get_key->node_id];
                    pq_get_key( q, n );
                    break;
                case PQ_OP_GET_ITEM:
                    op_get_item = (pq_op_get_item*) ( ops + i );
                    //printf("pq_get_item(%d,%d)\n", op_get_item->pq_id,
                    //    op_get_item->node_id);
                    q = pq_index[op_get_item->pq_id];
                    n = node_index[op_get_item->node_id];
                    pq_get_item( q, n );
                    break;
                case PQ_OP_GET_SIZE:
                    op_get_size = (pq_op_get_size*) ( ops + i );
                    //printf("pq_get_size(%d)\n", op_get_size->pq_id);
                    q = pq_index[op_get_size->pq_id];
                    pq_get_size( q );
                    break;
                case PQ_OP_INSERT:
                    op_insert = (pq_op_insert*) ( ops + i );
                    //printf("pq_insert(%d,%d,%llu,%d)\n", op_insert->pq_id,
                    //    op_insert->node_id, op_insert->key, op_insert->item );
                    q = pq_index[op_insert->pq_id];
                    node_index[op_insert->node_id] = pq_insert( q,
                        op_insert->item, op_insert->key );
                    break;
                case PQ_OP_FIND_MIN:
                    op_find_min = (pq_op_find_min*) ( ops + i );
                    //printf("pq_find_min(%d)\n", op_find_min->pq_id );
                    q = pq_index[op_find_min->pq_id];
                    pq_find_min( q );
                    break;
                case PQ_OP_DELETE:
                    op_delete = (pq_op_delete*) ( ops + i );
                    //printf("pq_delete(%d,%d)\n", op_delete->pq_id,
                    //    op_delete->node_id );
                    q = pq_index[op_delete->pq_id];
                    n = node_index[op_delete->node_id];
                    pq_delete( q, n );
                    break;
                case PQ_OP_DELETE_MIN:
                    op_delete_min = (pq_op_delete_min*) ( ops + i );
                    //printf("pq_delete_min(%d)\n", op_delete_min->pq_id);
                    q = pq_index[op_delete_min->pq_id];
                    pq_delete_min( q );
                    break;
                case PQ_OP_DECREASE_KEY:
                    op_decrease_key = (pq_op_decrease_key*) ( ops + i );
                    //printf("pq_decrease_key(%d,%d,%llu)\n", op_decrease_key->pq_id,
                    //    op_decrease_key->node_id, op_decrease_key->key);
                    q = pq_index[op_decrease_key->pq_id];
                    n = node_index[op_decrease_key->node_id];
                    pq_decrease_key( q, n, op_decrease_key->key );
                    break;
                /*case PQ_OP_MELD:
                    printf("Meld.\n");
                    op_meld = (pq_op_meld*) ( ops + i );
                    q = pq_index[op_meld->pq_src1_id];
                    r = pq_index[op_meld->pq_src2_id];
                    pq_index[op_meld->pq_dst_id] = pq_meld( q, r );
                    break;*/
                case PQ_OP_EMPTY:
                    op_empty = (pq_op_empty*) ( ops + i );
                    //printf("pq_empty(%d)\n", op_empty->pq_id);
                    q = pq_index[op_empty->pq_id];
                    pq_empty( q );
                    break;
                default:
                    break;
            }
        }
        
        gettimeofday(&t1, NULL);
        total_time += (t1.tv_sec - t0.tv_sec) * 1000000 +
            (t1.tv_usec - t0.tv_usec);
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
    
    printf( "%d\n", total_time / iterations );

    return 0;
}
