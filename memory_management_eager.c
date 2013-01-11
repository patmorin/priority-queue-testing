#include "memory_management_eager.h"
#include <stdio.h>

//==============================================================================
// PUBLIC METHODS
//==============================================================================

mem_map* mm_create( uint32_t types, uint32_t *sizes, uint32_t *capacities )
{
    int i;

    mem_map *map = malloc( sizeof( mem_map ) );
    map->types = types;
    map->sizes = malloc( types * sizeof( uint32_t ) );
    map->capacities = malloc( types * sizeof( uint32_t ) );
    map->data = malloc( types * sizeof( uint8_t* ) );
    map->free = malloc( types * sizeof( uint8_t** ) );
    map->index_data = calloc( types, sizeof( uint32_t ) );
    map->index_free = calloc( types, sizeof( uint32_t ) );


    for( i = 0; i < types; i++ )
    {
        map->sizes[i] = sizes[i];
        map->capacities[i] = capacities[i];

        map->data[i] = calloc( PQ_MEM_WIDTH, sizeof( uint8_t* ) );
        map->free[i] = calloc( PQ_MEM_WIDTH, sizeof( uint8_t** ) );

        map->data[i] = malloc( map->sizes[i] * map->capacities[i] );
        map->free[i] = malloc( sizeof( uint8_t* ) * map->capacities[i] );
    }

    return map;
}

void mm_destroy( mem_map *map )
{
    int i;
    for( i = 0; i < map->types; i++ )
    {
        free( map->data[i] );
        free( map->free[i] );
    }

    free( map->data );
    free( map->free );
    free( map->capacities );
    free( map->sizes );

    free( map );
}

void mm_clear( mem_map *map )
{
    int i;
    for( i = 0; i < map->types; i++ )
    {
        map->index_data[i] = 0;
        map->index_free[i] = 0;
    }
}

void* pq_alloc_node( mem_map *map, uint32_t type )
{
    void *node;
    if ( map->index_free[type] == 0 )
        node = ( map->data[type] + ( map->sizes[type] *
            (map->index_data[type])++ ) );
    else
        node = map->free[type][--(map->index_free[type])];

    memset( node, 0, map->sizes[type] );

    return node;
}

void pq_free_node( mem_map *map, uint32_t type, void *node )
{
    map->free[type][(map->index_free[type])++] = node;
}
