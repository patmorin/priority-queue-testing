#ifndef IMPLICIT_HEAP
#define IMPLICIT_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#ifdef BRANCH_16
    #define BRANCHING_FACTOR 16
#elif defined BRANCH_8
    #define BRANCHING_FACTOR 8
#elif defined BRANCH_4
    #define BRANCHING_FACTOR 4
#else
    #define BRANCHING_FACTOR 2
#endif

#include "queue_common.h"

/**
 * Holds an inserted element, as well as the current index in the node array.
 * Acts as a handle to clients for the purpose of mutability.
 */
struct implicit_node_t
{
    //! Index for the item in the "tree" array
    uint32_t index;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct implicit_node_t implicit_node;
typedef implicit_node pq_node_type;

/**
 * A mutable, meldable, array-based d-ary heap.  Maintains a single, complete
 * d-ary tree.  Imposes the standard heap invariant.
 */
struct implicit_heap_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The array of node pointers encoding the tree structure
    implicit_node **nodes;
    //! The number of items held in the queue
    uint32_t size;
    //! The maximum number of items the queue can currently hold
    uint32_t capacity;
} __attribute__ ((aligned(4)));

typedef struct implicit_heap_t implicit_heap;
typedef implicit_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
implicit_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( implicit_heap *queue );

/**
 * Removes all items from the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( implicit_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( implicit_heap *queue, implicit_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( implicit_heap *queue, implicit_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( implicit_heap *queue );

/**
 * Takes an item-key pair to insert into the queue and creates a new
 * corresponding node.  Inserts the node at the base of the tree in the
 * next open spot and reorders to preserve the heap invariant.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
implicit_node* pq_insert( implicit_heap *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue without modifying the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
implicit_node* pq_find_min( implicit_heap *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the root node of the tree, containing the
 * minimum element.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( implicit_heap *queue ) ;

/**
 * Removes an arbitrary item from the queue.  Requires that the location
 * of the item's corresponding node is known.  First swaps target node
 * with last item in the tree, removes target item node, then pushes
 * down the swapped node (previously last) to it's proper place in
 * the tree to maintain queue properties.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the target item
 * @return      Key of item removed
 */
key_type pq_delete( implicit_heap *queue, implicit_node* node );

/**
 * If the item in the queue is modified in such a way as to decrease the
 * key, then this function will update the queue to preserve the heap invariant
 * given a pointer to the corresponding node.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( implicit_heap *queue, implicit_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( implicit_heap *queue );

#endif
