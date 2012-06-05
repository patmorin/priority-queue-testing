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

int pq_trace_write_header( int file, pq_trace_header header )
{
    lseek( file, 0, SEEK_SET );
    ssize_t bytes = write( file, &header, sizeof( pq_trace_header) );
    if( bytes != sizeof( pq_trace_header ) )
        return -1;

    return 0;
}

int pq_trace_read_header( int file, pq_trace_header *header )
{
    ssize_t bytes = read( file, header, sizeof( pq_trace_header ) );
    if( bytes != sizeof( pq_trace_header ) )
        return -1;

    return 0;
}

int pq_trace_write_op( int file, void *op )
{
    uint32_t code = *((uint32_t*) op);
    ssize_t length = pq_op_lengths[code];
    ssize_t bytes = write( file, op, length );
    if( bytes != length )
        return -1;

    return 0;
}

int pq_trace_read_op( int file, void *op )
{
    size_t bytes = read( file, op, sizeof( uint32_t ) );
    if( bytes != sizeof( uint32_t ) )
        return -1;

    uint32_t code = *((uint32_t*) op);
    size_t length = pq_op_lengths[code] - sizeof( uint32_t );
    bytes = read( file, op + sizeof( uint32_t ), length );
    if( bytes != length )
        return -1;

    return 0;
}
