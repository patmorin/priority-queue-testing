#ifndef RANKPAIRING_HEAP
#define RANKPAIRING_HEAP

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
 * mutability.  Keeps track of rank, as well as pointers to parent and
 * left and right children.  In the case of a root, the right child
 * pointer points to the next root.
 */
typedef struct rank_pairing_node_t
{
    //! Parent node
    struct rank_pairing_node_t *parent;
    //! Left child
    struct rank_pairing_node_t *left;
    //! Right child, or next root if this node is a root
    struct rank_pairing_node_t *right;

    //! The number of children this node has
    uint32_t rank;
    
    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} rank_pairing_node;

/**
 * A mutable, meldable, rank pairing heap.  Maintains a forest of half-trees
 * managed by rank.  Obeys the type-1 rank rule and utilizes restricted
 * multi-pass linking.
 */
typedef struct rank_pairing_heap_t
{
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    rank_pairing_node *minimum;
    //! An array of roots of the heap, indexed by rank
    rank_pairing_node *roots[MAXRANK];
    //! Current largest rank in heap
    uint32_t largest_rank;
} rank_pairing_heap;

typedef rank_pairing_heap* pq_ptr;
typedef rank_pairing_node it_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty heap.
 *
 * @param capacity  Maximum number of nodes the heap is expected to hold
 * @return          Pointer to the new heap
 */
rank_pairing_heap* pq_create( uint32_t capacity );

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void pq_destroy( rank_pairing_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void pq_clear( rank_pairing_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( rank_pairing_heap *heap, rank_pairing_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( rank_pairing_heap *heap, rank_pairing_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t pq_get_size( rank_pairing_heap *heap );

/**
 * Takes an item-key pair to insert it into the heap and creates a new
 * corresponding node.  Makes the new node a root.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
rank_pairing_node* pq_insert( rank_pairing_heap *heap, item_type item,
    uint32_t key );

/**
 * Returns the minimum item from the heap.
 *
 * @param heap  Heap to query
 * @return      Node with minimum key
 */
rank_pairing_node* pq_find_min( rank_pairing_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <pq_delete> to remove the minimum item.
 *
 * @param heap  Heap to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( rank_pairing_heap *heap );

/**
 * Removes an arbitrary item from the heap and modifies heap structure
 * to preserve heap properties.  Requires that the location of the
 * item's corresponding node is known.  Severs the right spine of both
 * the left and right children of the node in order to make each of
 * these nodes a new halftree root.  Merges all these new roots into
 * the list and then performs a one-pass linking step to reduce the
 * number of trees.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( rank_pairing_heap *heap, rank_pairing_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.  Cuts the node
 * from its parent and traverses the tree upwards to correct the rank
 * values.
 *
 * @param heap      Heap in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( rank_pairing_heap *heap, rank_pairing_node *node,
    key_type new_key );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool pq_empty( rank_pairing_heap *heap );

#endif

