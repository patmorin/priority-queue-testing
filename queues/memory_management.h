#ifndef MEMORY_MANAGEMENT
#define MEMORY_MANAGEMENT

/**
 * Basic memory pool to use for node allocation.  Requires maximum queue
 * size to be known ahead of time.  After the initial allocation through
 * malloc, all node alloc/free operations should be O(1) with a small
 * constant.
 */

typedef struct mem_map_t
{
    //! current capacity
    uint32_t capacity;
    //! index of first as-of-yet-unused node
    uint32_t unused;
    //! pointer to first node
    it_type *head;
    //! pointer to list of freed nodes
    it_type **free_list;
    //! number of items currently in the freed list
    uint32_t free_size;
} mem_map;

mem_map* mm_create( uint32_t capacity )
{
    mem_map *map = (mem_map*) malloc( sizeof( mem_map ) );
    map->capacity = capacity;
    map->unused = 0;
    map->head = (it_type*) malloc( capacity * sizeof( it_type ) );
    map->free_list = (it_type**) malloc( capacity * sizeof( it_type* ) );
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

it_type* pq_alloc_node( mem_map *map )
{
    it_type *node;
    if ( map->free_size == 0 )
        node = ( map->head + (map->unused++) );
    else
        node = ( map->free_list[--map->free_size] );

    memset( node, 0, sizeof( it_type ) );

    return node;
}

void pq_free_node( mem_map *map, it_type *node )
{
    map->free_list[map->free_size++] = node;
}

#endif
