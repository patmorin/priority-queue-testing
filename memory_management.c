#include "memory_management.h"

//==============================================================================
// PUBLIC METHODS
//==============================================================================

mem_map* mm_create( uint32_t size, uint32_t capacity )
{
    mem_map *map = malloc( sizeof( mem_map ) );
    map->size = size;
    map->capacity = capacity;
    map->unused = 0;
    map->head = malloc( size * capacity );
    map->free_list = malloc( capacity * sizeof( void* ) );
    map->free_size = 0;

    return map;
}

void mm_destroy( mem_map *map )
{
    free( map->head );
    free( map->free_list );
    free( map );
}

void mm_clear( mem_map *map )
{
    map->unused = map->capacity;
    map->free_size = 0;
}

void* pq_alloc_node( mem_map *map )
{
    void *node;
    if ( map->free_size == 0 )
        node = ( map->head + (map->unused++) );
    else
        node = ( map->free_list[--map->free_size] );

    memset( node, 0, map->size );

    return node;
}

void pq_free_node( mem_map *map, void *node )
{
    map->free_list[map->free_size++] = node;
}
