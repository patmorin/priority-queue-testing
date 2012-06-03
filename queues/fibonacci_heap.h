#ifndef FIBONACCI_HEAP
#define FIBONACCI_HEAP

//==============================================================================
// DEFINES AND INCLUDES
//==============================================================================

#include "queue_common.h"

//==============================================================================
// STRUCTS
//==============================================================================

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node is contained in a doubly-linked circular list
 * of its siblings.  Additionally, each node has a pointer to its parent
 * and its first child.
 */
struct fibonacci_node_t
{
    //! Parent of this node
    struct fibonacci_node_t *parent;
    //! "First" child of this node
    struct fibonacci_node_t *first_child;
    //! Next node in the list of this node's siblings
    struct fibonacci_node_t *next_sibling;
    //! Previous node in the list of this node's siblings
    struct fibonacci_node_t *prev_sibling;

    //! The "height" of a node, i.e. bound on log of subtree size
    uint32_t rank;
    //! Denotes if a non-root node has lost a child or not
    bool marked;
    
    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct fibonacci_node_t fibonacci_node;

/**
 * A mutable, meldable, Fibonacci heap.  Maintains a forest of (partial)
 * binomial trees, resulting from rank-wise paired merging.  Uses an array to
 * index trees by rank.  Imposes standard queue variant, as well as requiring
 * that there remains at most one tree of any given rank after any deletion
 * (managed by merging).  Furthermore, a series of ("cascading") cuts is
 * performed after a key decrease if the parent loses it's second child.
 */
struct fibonacci_heap_t
{
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    fibonacci_node *minimum;
    //! An array of roots of the queue, indexed by rank
    fibonacci_node *roots[MAXRANK];
    //! Current largest rank in queue
    uint32_t largest_rank;
} __attribute__ ((aligned(4)));

typedef struct fibonacci_heap_t fibonacci_heap;

typedef fibonacci_heap* pq_ptr;
typedef fibonacci_node it_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param capacity  Maximum number of nodes the queue is expected to hold
 * @return          Pointer to the new queue
 */
fibonacci_heap* pq_create( uint32_t capacity );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( fibonacci_heap *queue );

/**
 * Deletes all nodes in the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( fibonacci_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( fibonacci_heap *queue, fibonacci_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( fibonacci_heap *queue, fibonacci_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( fibonacci_heap *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  inserts the node as a new root.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
fibonacci_node* pq_insert( fibonacci_heap *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue without modifying the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
fibonacci_node* pq_find_min( fibonacci_heap *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the node.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( fibonacci_heap *queue );

/**
 * Removes an arbitrary item from the queue.  Requires that the location
 * of the item's corresponding node is known.  After removing the node,
 * makes its children new roots in the queue.  Iteratively merges trees
 * of the same rank such that no two of the same rank remain afterward.
 * May initiate sequence of cascading cuts from node's parent.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( fibonacci_heap *queue, fibonacci_node *node );

/**
 * If the item in the queue is modified in such a way to decrease the
 * key, then this function will restructure the queue to repair any
 * potential structural violations.  Cuts the node from its parent and
 * makes it a new root, and potentially performs a series of cascading
 * cuts.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( fibonacci_heap *queue, fibonacci_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( fibonacci_heap *queue );

#endif
