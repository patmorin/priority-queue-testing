#ifndef RANKPAIRING_HEAP
#define RANKPAIRING_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include "queue_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Keeps track of rank, as well as pointers to parent and
 * left and right children.  In the case of a root, the right child
 * pointer points to the next root.
 */
struct rank_pairing_node_t
{
    //! Parent node
    struct rank_pairing_node_t *parent;
    //! Left child
    struct rank_pairing_node_t *left;
    //! Right child, or next root if this node is a root
    struct rank_pairing_node_t *right;

    //! A proxy for tree size
    uint32_t rank;
    
    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct rank_pairing_node_t rank_pairing_node;
typedef rank_pairing_node pq_node_type;

#include "../memory_management.h"

/**
 * A mutable, meldable, rank-pairing heap.  Maintains a forest of half-trees
 * managed by rank.  Obeys the type-1 rank rule and utilizes restricted
 * multi-pass linking.
 */
struct rank_pairing_heap_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    rank_pairing_node *minimum;
    //! An array of roots of the queue, indexed by rank
    rank_pairing_node *roots[MAXRANK];
    //! Current largest rank in queue
    uint32_t largest_rank;
} __attribute__ ((aligned(4)));

typedef struct rank_pairing_heap_t rank_pairing_heap;
typedef rank_pairing_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
rank_pairing_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( rank_pairing_heap *queue );

/**
 * Deletes all nodes from the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( rank_pairing_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( rank_pairing_heap *queue, rank_pairing_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( rank_pairing_heap *queue, rank_pairing_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( rank_pairing_heap *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  Makes the new node a root.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
rank_pairing_node* pq_insert( rank_pairing_heap *queue, item_type item,
    uint32_t key );

/**
 * Returns the minimum item from the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
rank_pairing_node* pq_find_min( rank_pairing_heap *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the minimum item.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( rank_pairing_heap *queue );

/**
 * Removes an arbitrary item from the queue and modifies queue structure
 * to preserve heap properties.  Requires that the location of the
 * item's corresponding node is known.  Severs the right spine of both
 * the left and right children of the node in order to make each of
 * these nodes a new halftree root.  Merges all these new roots into
 * the list and then performs a one-pass linking step to reduce the
 * number of trees.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( rank_pairing_heap *queue, rank_pairing_node *node );

/**
 * If the item in the queue is modified in such a way to decrease the
 * key, then this function will update the queue to preserve heap
 * properties given a pointer to the corresponding node.  Cuts the node
 * from its parent and traverses the tree upwards to correct the rank
 * values.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( rank_pairing_heap *queue, rank_pairing_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( rank_pairing_heap *queue );

#endif

