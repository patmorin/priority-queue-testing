#include "memory_management.h"
#include <stdio.h>

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static const uint32_t mm_sizes[32] =
{
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000
};

static void mm_grow_data( mem_map *map );
static void mm_grow_free( mem_map *map );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

mem_map* mm_create( uint32_t size )
{
    mem_map *map = malloc( sizeof( mem_map ) );
    map->size = size;

    memset( map->data, 0, 32 * sizeof( uint8_t* ) );
    memset( map->free, 0, 32 * sizeof( uint8_t** ) );

    map->data[0] = malloc( map->size );
    map->free[0] = malloc( sizeof( uint8_t* ) );

    return map;
}

void mm_destroy( mem_map *map )
{
    int i;
    for( i = 0; i < 32; i++ )
    {
        if( map->data[i] != NULL )
            free( map->data[i] );
        if( map->free[i] != NULL )
            free( map->free[i] );
    }

    free( map );
}

void mm_clear( mem_map *map )
{
    map->chunk_data = 0;
    map->chunk_free = 0;
    map->index_data = 0;
    map->index_free = 0;
}

void* pq_alloc_node( mem_map *map )
{
    void *node;    
    if ( map->chunk_free == 0 && map->index_free == 0 )
    {
        if( map->index_data == mm_sizes[map->chunk_data] )
            mm_grow_data( map );
        
        node = ( map->data[map->chunk_data] + ( map->size *
            (map->index_data)++ ) );
    }
    else
    {
        if( map->index_free == 0 )
            map->index_free = mm_sizes[--(map->chunk_free)];

        node = map->free[map->chunk_free][--(map->index_free)];
    }

    memset( node, 0, map->size );

    return node;
}

void pq_free_node( mem_map *map, void *node )
{
    if( map->index_free == mm_sizes[map->chunk_free] )
        mm_grow_free( map );
        
    map->free[map->chunk_free][(map->index_free)++] = node;
}

//==============================================================================
// STATIC METHODS
//==============================================================================

static void mm_grow_data( mem_map *map )
{
    uint32_t chunk = ++(map->chunk_data);
    map->index_data = 0;

    if( map->data[chunk] == NULL )
        map->data[chunk] = malloc( map->size * mm_sizes[chunk] );
}

static void mm_grow_free( mem_map *map )
{
    uint32_t chunk = ++(map->chunk_free);
    map->index_free = 0;

    if( map->free[chunk] == NULL )
        map->free[chunk] = malloc( map->size * mm_sizes[chunk] );
}

