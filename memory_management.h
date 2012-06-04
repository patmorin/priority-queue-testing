#ifndef PQ_MEMORY_MANAGEMENT
#define PQ_MEMORY_MANAGEMENT

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Basic memory pool to use for node allocation.  Requires the maximum number of
 * simultaneously live nodes to be known ahead of time.  Memory mapss can be
 * shared between multiple queues for the purpose of melding.  After the initial
 * allocation through malloc, all node alloc/free operations should be O(1) with
 * a small constant.
 */
 
typedef struct mem_map_t
{
    //! size of a single node
    uint32_t size;
    //! current capacity
    uint32_t capacity;
    //! index of first as-of-yet-unused node
    uint32_t unused;
    //! pointer to first node
    void *head;
    //! pointer to list of freed nodes
    void **free_list;
    //! number of items currently in the freed list
    uint32_t free_size;
} mem_map;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new memory map with capacity for the specified number of nodes.
 *
 * @param size      Size of a single node
 * @param capacity  Maximum number of supported live nodes
 * @return          Pointer to the new memory map
 */
mem_map* mm_create( uint32_t size, uint32_t capacity );

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
 * @return      Pointer to allocated node
 */
void* pq_alloc_node( mem_map *map );

/**
 * Takes a previously allocated node and adds it to the free list to be
 * recycled with further allocation requests.
 * 
 * @param map   Map to which the node belongs
 * @param node  Node to free
 */
void pq_free_node( mem_map *map, void *node );

#endif
