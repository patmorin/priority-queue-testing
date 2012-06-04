#ifndef VIOLATION_HEAP
#define VIOLATION_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#include "queue_common.h"

/**
* Holds an inserted element, as well as pointers to maintain tree
* structure.  Acts as a handle to clients for the purpose of
* mutability.  Tracks rank of node as well as pointers to this node's
* first child and the next and previous nodes in the list of siblings.
* The last node in the list of siblings will have a null prev pointer
* and the first node's next pointer will point to their parent.
*/
struct violation_node_t
{
    //! Last child of this node
    struct violation_node_t *child;
    //! Next node in the list of this node's siblings
    struct violation_node_t *next;
    //! Previous node in the list of this node's siblings
    struct violation_node_t *prev;

    //! The number of children this node has
    int32_t rank;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct violation_node_t violation_node;
typedef violation_node pq_node_type;

/**
 * A mutable, meldable, violation queue.  Maintains a forest of trees indexed by
 * rank.  At most two trees of each rank remain after a @ref <pq_delete> or @ref
 * <pq_delete_min> operation.
 */
struct violation_heap_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    violation_node* minimum;
    //! An array of roots of the queue, indexed by rank
    violation_node* roots[MAXRANK][2];
    //! Current largest rank in queue
    uint32_t largest_rank;
} __attribute__ ((aligned(4)));

typedef struct violation_heap_t violation_heap;
typedef violation_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
violation_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( violation_heap *queue );

/**
 * Deletes all items in the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( violation_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( violation_heap *queue, violation_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( violation_heap *queue, violation_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( violation_heap *queue );

/**
 * Takes an item-key pair to insert into the queue and creates a new
 * corresponding node.  Makes the new node a root.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
violation_node* pq_insert( violation_heap *queue, item_type item, key_type key );

/**
 * Returns the minimum item from the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
violation_node* pq_find_min( violation_heap *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the minimum item.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( violation_heap *queue );

/**
 * Removes an arbitrary item from the queue and modifies queue structure
 * to preserve queue properties.  Requires that the location of the
 * item's corresponding node is known.  Merges the node's children with
 * the root list.  Merges roots such that no more than two roots have
 * the same rank.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( violation_heap *queue, violation_node *node );

/**
 * If the item in the queue is modified in such a way to decrease the
 * key, then this function will update the queue to preserve queue
 * properties given a pointer to the corresponding node.  Replaces
 * the subtree rooted at the given node with its active subtree of
 * larger rank and then relocates the rest of the tree as a new root.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( violation_heap *queue, violation_node *node,
    key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( violation_heap *queue );

#endif
