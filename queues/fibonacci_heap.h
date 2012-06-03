#ifndef FIBONACCI_HEAP
#define FIBONACCI_HEAP

//==============================================================================
// DEFINES AND INCLUDES
//==============================================================================

#include "heap_common.h"

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
typedef struct fibonacci_node_t
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
} fibonacci_node;

/**
 * A mutable, meldable, Fibonacci heap.  Maintains a forest of (partial)
 * binomial trees, resulting from rank-wise paired merging.  Uses an array to
 * index trees by rank.  Imposes standard heap variant, as well as requiring
 * that there remains at most one tree of any given rank after any deletion
 * (managed by merging).  Furthermore, a series of ("cascading") cuts is
 * performed after a key decrease if the parent loses it's second child.
 */
typedef struct fibonacci_heap_t
{
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    fibonacci_node *minimum;
    //! An array of roots of the heap, indexed by rank
    fibonacci_node *roots[MAXRANK];
    //! Current largest rank in heap
    uint32_t largest_rank;
} fibonacci_heap;

typedef fibonacci_heap* pq_ptr;
typedef fibonacci_node it_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty heap.
 *
 * @param capacity  Maximum number of nodes the heap is expected to hold
 * @return          Pointer to the new heap
 */
fibonacci_heap* pq_create( uint32_t capacity );

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void pq_destroy( fibonacci_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void pq_clear( fibonacci_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( fibonacci_heap *heap, fibonacci_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( fibonacci_heap *heap, fibonacci_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t pq_get_size( fibonacci_heap *heap );

/**
 * Takes an item-key pair to insert it into the heap and creates a new
 * corresponding node.  inserts the node as a new root.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
fibonacci_node* pq_insert( fibonacci_heap *heap, item_type item, key_type key );

/**
 * Returns the minimum item from the heap without modifying the heap.
 *
 * @param heap  Heap to query
 * @return      Node with minimum key
 */
fibonacci_node* pq_find_min( fibonacci_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <pq_delete> to remove the node.
 *
 * @param heap  Heap to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( fibonacci_heap *heap );

/**
 * Removes an arbitrary item from the heap.  Requires that the location
 * of the item's corresponding node is known.  After removing the node,
 * makes its children new roots in the heap.  Iteratively merges trees
 * of the same rank such that no two of the same rank remain afterward.
 * May initiate sequence of cascading cuts from node's parent.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( fibonacci_heap *heap, fibonacci_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will restructure the heap to repair any
 * potential structural violations.  Cuts the node from its parent and
 * makes it a new root, and potentially performs a series of cascading
 * cuts.
 *
 * @param heap      Heap in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( fibonacci_heap *heap, fibonacci_node *node,
    key_type new_key );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool pq_empty( fibonacci_heap *heap );

#endif
