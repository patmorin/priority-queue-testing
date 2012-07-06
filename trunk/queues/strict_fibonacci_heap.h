#ifndef STRICT_FIBONACCI_HEAP
#define STRICT_FIBONACCI_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

// passive, active non-root with 0 loss, active root, positive loss
#define STRICT_TYPE_PASSIVE 0
#define STRICT_TYPE_ACTIVE  1
#define STRICT_TYPE_ROOT    2
#define STRICT_TYPE_LOSS    3

// node types for memory map allocation
#define STRICT_NODE_FIB     0
#define STRICT_NODE_FIX     1
#define STRICT_NODE_ACTIVE  2
#define STRICT_NODE_RANK    3

// modes for fix list operations
#define STRICT_FIX_ROOT     0
#define STRICT_FIX_LOSS     1

// directions for rank moves
#define STRICT_DIR_DEMOTE   0
#define STRICT_DIR_PROMOTE  1

#include "queue_common.h"

// forward declares for pointer resolution
struct strict_fibonacci_node_t;
struct fix_node_t;

typedef struct strict_item_t strict_item;

/**
 * All active nodes in a single heap will point to the same active record.
 * Setting the flag to passive will render an entire heap passive for the
 * purpose of melding.
 */
struct active_record_t
{
    //! 1 if active, 0 otherwise
    uint32_t flag;
    //! Number of nodes currently pointing to it.  If 0, free record.
    uint32_t ref_count;
} __attribute__ ((aligned(4)));

typedef struct active_record_t active_record;

/**
 * List of nodes representing rank.  Used to group active nodes for
 * restructuring.
 */
struct rank_record_t
{
    uint32_t rank;
    //! rank one higher if exists
    struct rank_record_t *inc;
    //! rank one lower if exists
    struct rank_record_t *dec;
    //! flags of last known transformability status
    int transformable[2];
    //! pointers to fix nodes of the current rank
    struct fix_node_t *head[2];
    struct fix_node_t *tail[2];
    //! number of nodes pointing to it, free record if 0
    uint32_t ref_count;
} __attribute__ ((aligned(4)));

typedef struct rank_record_t rank_record;

/**
 * A node in a doubly-linked circular list.  Holds a pointer to an active node
 * and a corresponding rank.
 */
struct fix_node_t
{
    struct strict_fibonacci_node_t *node;
    struct fix_node_t *left;
    struct fix_node_t *right;
    rank_record *rank;
} __attribute__ ((aligned(4)));

typedef struct fix_node_t fix_node;

/**
 * The main structural element of the heap.  Each node stores an item-key pair
 * and  is contained in a doubly-linked circular list of its siblings.
 * Additionally, each node has a pointer to its parent and its leftmost child.
 * The node has pointers to the next and previous nodes in the queue as well as
 * to rank and active records if it is active.  The last known type of the node
 * is stored so that a change in type can easily be detected and unnecessary
 * restructuring can be avoided.  Finally, active nodes may also have positive
 * loss and a reference to a node in the fix list.
 */
struct strict_fibonacci_node_t
{
    item_type item;
    key_type key;

    struct strict_fibonacci_node_t *parent;
    struct strict_fibonacci_node_t *left;
    struct strict_fibonacci_node_t *right;
    struct strict_fibonacci_node_t *left_child;

    struct strict_fibonacci_node_t *q_prev;
    struct strict_fibonacci_node_t *q_next;

    uint32_t type;
    active_record *active;
    rank_record *rank;
    fix_node *fix;
    uint32_t loss;
} __attribute__ ((aligned(4)));

typedef struct strict_fibonacci_node_t strict_fibonacci_node;
typedef strict_fibonacci_node pq_node_type;

/**
 * A mutable, meldable, strict Fibonacci heap.  Maintains a single tree with
 * an auxiliary queue and a fix list.  Entirely pointer-based.
 */
struct strict_fibonacci_heap_t
{
    mem_map *map;
    uint32_t size;

    strict_fibonacci_node *root;
    strict_fibonacci_node *q_head;

    active_record *active;
    rank_record *rank_list;
    fix_node *fix_list[2];

    fix_node *garbage_fix;
} __attribute__ ((aligned(4)));

typedef struct strict_fibonacci_heap_t strict_fibonacci_heap;
typedef strict_fibonacci_heap pq_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
strict_fibonacci_heap* pq_create( mem_map *map );

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
void pq_destroy( strict_fibonacci_heap *queue );

/**
 * Deletes all nodes in the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
void pq_clear( strict_fibonacci_heap *queue );

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
uint32_t pq_get_size( strict_fibonacci_heap *queue );

/**
 * Takes an item-key pair to insert it into the queue and creates a new
 * corresponding node.  inserts the node as a new root.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
strict_fibonacci_node* pq_insert( strict_fibonacci_heap *queue, item_type item,
    key_type key );

/**
 * Returns the minimum item from the queue without modifying the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
strict_fibonacci_node* pq_find_min( strict_fibonacci_heap *queue );

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the node.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( strict_fibonacci_heap *queue );

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
key_type pq_delete( strict_fibonacci_heap *queue, strict_fibonacci_node *node );

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
void pq_decrease_key( strict_fibonacci_heap *queue, strict_fibonacci_node *node,
    key_type new_key );

/**
 * Combines two different item-disjoint queues which share a memory map.
 * Returns a pointer to the resulting queue.
 *
 * @param a First queue
 * @param b Second queue
 * @return  Resulting merged queue
 */
strict_fibonacci_heap* pq_meld( strict_fibonacci_heap *a,
    strict_fibonacci_heap *b );

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
bool pq_empty( strict_fibonacci_heap *queue );

#endif
