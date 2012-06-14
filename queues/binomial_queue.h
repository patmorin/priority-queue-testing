#ifndef BINOMIAL_QUEUE
#define BINOMIAL_QUEUE

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include "queue_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Keeps a pointer to the parent, first child, and next sibiling.
 * A root is denoted by a null parent.
 */
struct binomial_node_t
{
    //! Parent node
    struct binomial_node_t *parent;
    //! First child
    struct binomial_node_t *first_child;
    //! Right child, or next root if this node is a root
    struct binomial_node_t *next_sibling;

    //! Rank of binomial tree
    uint32_t rank;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct binomial_node_t binomial_node;
typedef binomial_node pq_node_type;

/**
 * A mutable, meldable, binomial queue.  Maintains a forest of uniquely-sized
 * perfect binomial trees using a single binary tree representation.
 */
struct binomial_queue_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    binomial_node *minimum;
    //! An array of roots of the queue, indexed by rank
    binomial_node *roots[MAXRANK];
} __attribute__ ((aligned(4)));

typedef struct binomial_queue_t binomial_queue;
typedef binomial_queue pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
binomial_queue* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( binomial_queue *queue );

/**
 * Deletes all nodes from the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( binomial_queue *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( binomial_queue *queue, binomial_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( binomial_queue *queue, binomial_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( binomial_queue *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  Makes the new node a root, and performs the usual
 * merging of trees.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
binomial_node* pq_insert( binomial_queue *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
binomial_node* pq_find_min( binomial_queue *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the minimum item.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( binomial_queue *queue );

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
key_type pq_delete( binomial_queue *queue, binomial_node *node );

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
void pq_decrease_key( binomial_queue *queue, binomial_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( binomial_queue *queue );

#endif

