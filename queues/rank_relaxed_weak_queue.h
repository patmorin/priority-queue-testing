#ifndef RANK_RELAXED_WEAK_QUEUE
#define RANK_RELAXED_WEAK_QUEUE

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#define ROOTS 0
#define MARKS 1

#include "queue_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Keeps track of rank, as well as pointers to parent and
 * left and right children.  In the case of a root, the right child
 * pointer points to the next root.
 */
struct rank_relaxed_weak_node_t
{
    //! Parent node
    struct rank_relaxed_weak_node_t *parent;
    //! Left child
    struct rank_relaxed_weak_node_t *left;
    //! Right child, or next root if this node is a root
    struct rank_relaxed_weak_node_t *right;

    //! A proxy for tree size
    uint32_t rank;
    //! Mark status
    uint32_t marked;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} __attribute__ ((aligned(4)));

typedef struct rank_relaxed_weak_node_t rank_relaxed_weak_node;
typedef rank_relaxed_weak_node pq_node_type;

/**
 * A mutable, meldable, rank-relaxed weak queue.  Maintains a forest of
 * half-trees managed by rank.  Some operations will mark nodes.  After any
 * operation, there will be no two roots of the same rank, no two marked nodes
 * of the same rank, no marked node with a parent which is also marked, and no
 * marked node which is a left child.
 */
struct rank_relaxed_weak_queue_t
{
    //! Memory map to use for node allocation
    mem_map *map;
    //! The number of items held in the queue
    uint32_t size;
    //! Pointer to the minimum node in the queue
    rank_relaxed_weak_node *minimum;
    //! Arrays of roots and marked nodes in the queue, indexed by rank
    rank_relaxed_weak_node *nodes[2][MAXRANK];
    //! Bit vectors indicating which pointers
    uint64_t registry[2];
} __attribute__ ((aligned(4)));

typedef struct rank_relaxed_weak_queue_t rank_relaxed_weak_queue;
typedef rank_relaxed_weak_queue pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
rank_relaxed_weak_queue* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( rank_relaxed_weak_queue *queue );

/**
 * Deletes all nodes from the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( rank_relaxed_weak_queue *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( rank_relaxed_weak_queue *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  Makes the new node a root, and proceeds to restore
 * structural invariants.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
rank_relaxed_weak_node* pq_insert( rank_relaxed_weak_queue *queue,
    item_type item, key_type key );

/**
 * Returns the minimum item from the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
rank_relaxed_weak_node* pq_find_min( rank_relaxed_weak_queue *queue );

/**
 * Removes the minimum item from the queue and modifies queue structure to
 * preserve queue properties.  Sifts node all the way to the root of its tree.
 * Severs the left spine of the tree and makes all resulting trees new roots.
 * Merges all these new roots into the list and restores invariants.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( rank_relaxed_weak_queue *queue );

/**
 * Removes an arbitrary item from the queue and returns it.  Relies on
 * @ref <pq_decrease_key> to make the item the minimum in the queue and then
 * uses @ref <pq_delete_min> to remove it.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );

/**
 * If the item in the queue is modified in such a way to decrease the
 * key, then this function will mark the nodes and begin an upwards mark-removal
 * process.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node, key_type new_key );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( rank_relaxed_weak_queue *queue );

#endif

