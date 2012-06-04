#include "trace_tools.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static const size_t pq_op_lengths[13] =
{
    sizeof( pq_op_create ),
    sizeof( pq_op_destroy ),
    sizeof( pq_op_clear ),
    sizeof( pq_op_get_key ),
    sizeof( pq_op_get_item ),
    sizeof( pq_op_get_size ),
    sizeof( pq_op_insert ),
    sizeof( pq_op_find_min ),
    sizeof( pq_op_delete ),
    sizeof( pq_op_delete_min ),
    sizeof( pq_op_decrease_key ),
    sizeof( pq_op_meld ),
    sizeof( pq_op_empty )
};

//==============================================================================
// PUBLIC METHODS
//==============================================================================

int pq_trace_write_header( FILE *file, pq_trace_header header )
{
    rewind( file );
    size_t items = fwrite( &header, sizeof( pq_trace_header), 1, file );
    if( items != 1 )
        return -1;

    return 0;
}

int pq_trace_read_header( FILE *file, pq_trace_header *header )
{
    size_t items = fread( header, sizeof( pq_trace_header ), 1, file );
    if( items != 1 )
        return -1;

    return 0;
}

int pq_trace_write_op( FILE *file, void *op )
{
    uint32_t code = *((uint32_t*) op);
    size_t length = pq_op_lengths[code];
    size_t items = fwrite( op, length, 1, file );
    if( items != 1 )
        return -1;

    return 0;
}

uint32_t pq_trace_read_op( FILE *file, void *op )
{
    size_t items = fread( op, sizeof( uint32_t ), 1, file );
    if( items != 1 )
        return -1;

    uint32_t code = *((uint32_t*) op);
    size_t length = pq_op_lengths[code] - sizeof( uint32_t );
    items = fread( op + sizeof( uint32_t ), length, 1, file );
    if( items != 1 )
        return -1;

    return code;
}
