#include "memory_management_dumb.h"
#include <stdio.h>

//==============================================================================
// PUBLIC METHODS
//==============================================================================

mem_map* mm_create( uint32_t types, uint32_t *sizes )
{
    mem_map *map = malloc( sizeof( mem_map ) );
    map->types = types;
    map->sizes = malloc( types * sizeof( uint32_t ) );

    return map;
}

void mm_destroy( mem_map *map )
{
    free( map->sizes );
    free( map );
}

void mm_clear( mem_map *map )
{
    return;
}

void* pq_alloc_node( mem_map *map, uint32_t type )
{
    void *node = calloc( 1, map->sizes[type] );

    return node;
}

void pq_free_node( mem_map *map, uint32_t type, void *node )
{
    free( node );
}
