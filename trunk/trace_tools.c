#include "trace_tools.h"

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

// some internal implementation details
#define PQ_OP_BUFFER_LEN    131072
#define MASK_PRIO 0xFFFFFFFF00000000
#define MASK_NAME 0x00000000FFFFFFFF
#define PQ_MAX(a,b) ( (a >= b) ? a : b )
#define PQ_MIN(a,b) ( (a <= b) ? a : b )

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

static size_t pq_op_buffer_pos = 0;
static uint8_t pq_op_buffer[PQ_OP_BUFFER_LEN];

static int buffered_write( int file, uint8_t* data, size_t length );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

int pq_trace_write_header( int file, pq_trace_header header )
{
    int flush = pq_trace_flush_buffer( file );
    if( flush == -1 )
        return -1;
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
    ssize_t bytes = buffered_write( file, op, length );
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

int pq_trace_flush_buffer( int file )
{
    size_t to_write = pq_op_buffer_pos;
    int bytes = write( file, pq_op_buffer, to_write );
    if( bytes != to_write )
        return -1;

    pq_op_buffer_pos = 0;

    return bytes;
}

//==============================================================================
// STATIC METHODS
//==============================================================================

static int buffered_write( int file, uint8_t* data, size_t length )
{
    int status;

    if( PQ_OP_BUFFER_LEN - pq_op_buffer_pos - 1 < length )
    {
        status = pq_trace_flush_buffer( file );
        if( status == -1 )
            return status;
    }

    memcpy( pq_op_buffer + pq_op_buffer_pos, data, length );
    pq_op_buffer_pos += length;

    return length;
}
