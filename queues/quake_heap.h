#ifndef QUAKE_HEAP
#define QUAKE_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include "queue_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Keeps track of the height of the node as well as pointer
 * to the node's parent, left child (duplicate), and right child.
 */
struct quake_node_t
{
    //! Parent node
    struct quake_node_t *parent;
    //! Left child
    struct quake_node_t *left;
    //! Right child, or next root if this node is a root
    struct quake_node_t *right;

    //! The height of this node
    uint8_t height;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct quake_node_t quake_node;
typedef quake_node pq_node_type;

/**
 * A mutable, meldable, Quake heap.  Maintains a forest of (binary) tournament
 * trees of unique height.  Maintains standard heap invariant and guarantees
 * exponential decay in node height.
 */
struct quake_heap_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    quake_node *minimum;
    //! An array of roots of the queue, indexed by height
    quake_node *roots[MAXRANK];
    //! An array of counters corresponding to the number of nodes at height
    //! equal to the index
    uint32_t nodes[MAXRANK];
    //! Current height of highest node in queue
    uint32_t highest_node;
    //! Index at which first decay violation occurs, MAXRANK if none
    uint32_t violation;
} __attribute__ ((aligned(4)));

typedef struct quake_heap_t quake_heap;
typedef quake_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
quake_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( quake_heap *queue );

/**
 * Deletes all nodes, leaving the queue empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( quake_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( quake_heap *queue, quake_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( quake_heap *queue, quake_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( quake_heap *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  Inserts the node as a new root in the queue.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
quake_node* pq_insert( quake_heap *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
quake_node* pq_find_min( quake_heap *queue );

/**
 * Removes the minimum item from the queue and returns it, restructuring
 * the queue along the way to maintain the queue property.  Relies on
 * @ref <pq_delete> to extract the minimum.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( quake_heap *queue );

/**
 * Removes an arbitrary item from the queue and modifies queue structure
 * to preserve heap properties.  Requires that the location of the
 * item's corresponding node is known.  Merges roots so that no two
 * roots of the same height remain.  Checks to make sure that the
 * exponential decay invariant is maintained, and corrects if not.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( quake_heap *queue, quake_node *node );

/**
 * If the item in the queue is modified in such a way to decrease the
 * key, then this function will update the queue to preserve queue
 * properties given a pointer to the corresponding node.  Removes the
 * subtree rooted at the given node and makes it a new tree in the queue.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( quake_heap *queue, quake_node *node, key_type new_key );

/**
 * Combines two different item-disjoint queues which share a memory map.
 * Merges node lists and adds the rank lists.  Returns a pointer to the
 * resulting queue.
 *
 * @param a First queue
 * @param b Second queue
 * @return  Resulting merged queue
 */
quake_heap* pq_meld( quake_heap *a, quake_heap *b );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( quake_heap *queue );

#endif

