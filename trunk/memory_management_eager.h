#ifndef PQ_MEMORY_MANAGEMENT
#define PQ_MEMORY_MANAGEMENT

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PQ_MEM_WIDTH 32

/**
 * Basic memory pool to use for node allocation.  Memory maps can be shared
 * between multiple queues for the purpose of melding.  The size of the pool is
 * doubled when the current capacity is exceeded.
 */

typedef struct mem_map_t
{
    //! number of different node types
    uint32_t types;
    //! sizes of single nodes
    uint32_t *sizes;
    //! number of each type of node
    uint32_t *capacities;

    uint8_t **data;
    uint8_t ***free;

    uint32_t *index_data;
    uint32_t *index_free;
} mem_map;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new memory map for the specified node sizes
 *
 * @param types         The number of different types of nodes to manage
 * @param size          Sizes of a single node of each type
 * @param capacities    The number of nodes of each type to allocate
 * @return              Pointer to the new memory map
 */
mem_map* mm_create( uint32_t types, uint32_t *sizes, uint32_t *capacities );

/**
 * Releases all allocated memory associated with the map.
 *
 * @param map   Map to deallocate
 */
void mm_destroy( mem_map *map );

/**
 * Resets map to initial state.  Does not deallocate memory.
 *
 * @param map   Map to reset
 */
void mm_clear( mem_map *map );

/**
 * Allocates a single node from the memory pool.  First attempts to recycle old
 * data from the free list.  If there is nothing to recycle, then it takes a new
 * node off the allocated list.  Zeroes the memory of the allocated node.
 *
 * @param map   Map from which to allocate
 * @param type  Type of node to allocate
 * @return      Pointer to allocated node
 */
void* pq_alloc_node( mem_map *map, uint32_t type );

/**
 * Takes a previously allocated node and adds it to the free list to be
 * recycled with further allocation requests.
 *
 * @param map   Map to which the node belongs
 * @param type  Type of node to free
 * @param node  Node to free
 */
void pq_free_node( mem_map *map, uint32_t type, void *node );

#endif
