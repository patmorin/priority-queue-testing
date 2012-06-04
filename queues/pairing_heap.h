#ifndef PAIRING_HEAP
#define PAIRING_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include "queue_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node is contained in a doubly linked list of
 * siblings and has a pointer to it's first child.  If a node is the
 * first of its siblings, then its prev pointer points to their
 * collective parent.  The last child is marked by a null next pointer.
 */
struct pairing_node_t
{
    //! First child of this node
    struct pairing_node_t *child;
    //! Next node in the list of this node's siblings
    struct pairing_node_t *next;
    //! Previous node in the list of this node's siblings
    struct pairing_node_t *prev;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct pairing_node_t pairing_node;
typedef pairing_node pq_node_type;
    
/**
 * A mutable, meldable, two-pass Pairing heap.  Maintains a single multiary tree
 * with no structural constraints other than the standard heap invariant.
 * Handles most operations through cutting and pairwise merging.  Primarily uses
 * iteration for merging rather than the standard recursion methods (due to
 * concerns for stackframe overhead).
 */
struct pairing_heap_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    pairing_node *root;
} __attribute__ ((aligned(4)));

typedef struct pairing_heap_t pairing_heap;
typedef pairing_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
pairing_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( pairing_heap *queue );

/**
 * Deletes all nodes, leaving the queue empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( pairing_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( pairing_heap *queue, pairing_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( pairing_heap *queue, pairing_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( pairing_heap *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  Merges the new node with the root of the queue.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
pairing_node* pq_insert( pairing_heap *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue without modifying any data.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
pairing_node* pq_find_min( pairing_heap *queue );

/**
 * Deletes the minimum item from the queue and returns it, restructuring
 * the queue along the way to maintain the heap property.  Relies on the
 * @ref <pq_delete> method to delete the root of the tree.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( pairing_heap *queue );

/**
 * Deletes an arbitrary item from the queue and modifies queue structure
 * to preserve the heap invariant.  Requires that the location of the
 * item's corresponding node is known.  Removes the node from its list
 * of siblings, then merges all its children into a new tree and
 * subsequently merges that tree with the root.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the item to delete
 * @return      Key of item deleted
 */
key_type pq_delete( pairing_heap *queue, pairing_node *node );

/**
 * If the item in the queue is modified in such a way to decrease the
 * key, then this function will update the queue to preserve queue
 * properties given a pointer to the corresponding node.  Cuts the node
 * from its list of siblings and merges it with the root.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( pairing_heap *queue, pairing_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( pairing_heap *queue );

#endif
