#ifndef EXPLICIT_HEAP
#define EXPLICIT_HEAP

//==============================================================================
// DEFINES AND INCLUDES
//==============================================================================

#include "heap_common.h"

#ifndef BRANCHING_FACTOR
    //! 2^k for some k
    #define BRANCHING_FACTOR 2
    //! matching k
    #define BRANCHING_POWER 1
#endif

//==============================================================================
// STRUCTS
//==============================================================================

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node has a pointer to its parent and its left and
 * right siblings.
 */
typedef struct explicit_node_t
{
    //! Pointer to parent node
    struct explicit_node_t *parent;
    //! Pointers to children
    struct explicit_node_t *children[BRANCHING_FACTOR];

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} explicit_node;

/**
 * A mutable, meldable, node-based binary heap.  Maintains a single, complete
 * binary tree.  Imposes the standard heap invariant.
 */
typedef struct explicit_heap_t
{
    //! The root of the binary tree representing the heap
    explicit_node *root;
    //! The number of items held in the heap
    uint32_t size;
} explicit_heap;

typedef explicit_heap* pq_ptr;
typedef explicit_node it_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty heap.
 *
 * @param capacity  Maximum number of nodes the heap is expected to hold
 * @return          Pointer to the new heap
 */
explicit_heap* pq_create( uint32_t capacity );

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void pq_destroy( explicit_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void pq_clear( explicit_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( explicit_heap *heap, explicit_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( explicit_heap *heap, explicit_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t pq_get_size( explicit_heap *heap );

/**
 * Takes a item-key pair to insert into the heap and creates a new
 * corresponding node.  inserts the node at the base of the tree in the
 * next open spot and reorders to preserve the heap property.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
explicit_node* pq_insert( explicit_heap *heap, item_type item, key_type key );

/**
 * Returns the minimum item from the heap without modifying the heap.
 *
 * @param heap  Heap to query
 * @return      Node with minimum key
 */
explicit_node* pq_find_min( explicit_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <pq_delete> to remove the root node of the tree, containing the
 * minimum element.
 *
 * @param heap  Heap to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( explicit_heap *heap ) ;

/**
 * Removes an arbitrary item from the heap.  Requires that the location
 * of the item's corresponding node is known.  First swaps target node
 * with last item in the tree, removes target item node, then pushes
 * down the swapped node (previously last) to it's proper place in
 * the tree to maintain heap properties.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the target item
 * @return      Key of item removed
 */
key_type pq_delete( explicit_heap *heap, explicit_node* node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.
 *
 * @param heap      Heap in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( explicit_heap *heap, explicit_node *node,
    key_type new_key );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool pq_empty( explicit_heap *heap );

#endif
