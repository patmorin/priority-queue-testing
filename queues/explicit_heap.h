#ifndef EXPLICIT_HEAP
#define EXPLICIT_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#ifdef BRANCH_16
    #define BRANCHING_FACTOR 16
    #define BRANCHING_POWER 4
#elif defined BRANCH_8
    #define BRANCHING_FACTOR 8
    #define BRANCHING_POWER 3
#elif defined BRANCH_4
    #define BRANCHING_FACTOR 4
    #define BRANCHING_POWER 2
#else
    #define BRANCHING_FACTOR 2
    #define BRANCHING_POWER 1
#endif

#include "queue_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node has a pointer to its parent and its left and
 * right siblings.
 */
struct explicit_node_t
{
    //! Pointer to parent node
    struct explicit_node_t *parent;
    //! Pointers to children
    struct explicit_node_t *children[BRANCHING_FACTOR];

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct explicit_node_t explicit_node;
typedef explicit_node pq_node_type;

#include "../memory_management.h"

/**
 * A mutable, meldable, node-based d-ary heap.  Maintains a single, complete
 * d-ary tree.  Imposes the standard queue invariant.
 */
struct explicit_heap_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The root of the d-ary tree representing the queue
    explicit_node *root;
    //! The number of items held in the queue
    uint32_t size;
} __attribute__ ((aligned(4)));

typedef struct explicit_heap_t explicit_heap;
typedef explicit_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty priority queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
explicit_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( explicit_heap *queue );

/**
 * Deletes all nodes from the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( explicit_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( explicit_heap *queue, explicit_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( explicit_heap *queue, explicit_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( explicit_heap *queue );

/**
 * Takes a item-key pair to insert into the queue and creates a new
 * corresponding node.  Inserts the node at the base of the tree in the
 * next open spot and reorders to preserve the heap invariant.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
explicit_node* pq_insert( explicit_heap *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue without modifying anything.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
explicit_node* pq_find_min( explicit_heap *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the root node of the tree, containing the
 * minimum element.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( explicit_heap *queue ) ;

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
key_type pq_delete( explicit_heap *queue, explicit_node* node );

/**
 * If an item in the queue is modified in such a way to decrease the
 * key, then this function will update the queue to preserve invariants
 * given a pointer to the corresponding node.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( explicit_heap *queue, explicit_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( explicit_heap *queue );

#endif
