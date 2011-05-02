#ifndef VIOLATION_HEAP
#define VIOLATION_HEAP

#include "heap_common.h"

/**
* Holds an inserted element, as well as pointers to maintain tree
* structure.  Acts as a handle to clients for the purpose of
* mutability.  Tracks rank of node as well as pointers to this node's
* first child and the next and previous nodes in the list of siblings.
* The last node in the list of siblings will have a null prev pointer
* and the first node's next pointer will point to their parent.
*/
typedef struct violation_node_t {
    //! Pointer to a piece of client data
    void* item;
    //! Key for the item
    uint32_t key;
    //! The number of children this node has
    int32_t rank;

    //! Last child of this node
    struct violation_node_t *child;
    //! Next node in the list of this node's siblings
    struct violation_node_t *next;
    //! Previous node in the list of this node's siblings
    struct violation_node_t *prev;
} violation_node;

/**
 * A mutable, meldable, violation heap.  Maintains a forest of trees indexed by
 * rank.  At most two trees of each rank remain after a @ref <delete> or @ref
 * <delete_min> operation.
 */
typedef struct violation_heap_t {
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    violation_node* minimum;
    //! An array of roots of the heap, indexed by rank
    violation_node* roots[MAXRANK][2];
    //! A collection of operation counters
    heap_stats *stats;
} violation_heap;

/**
 * Creates a new, empty heap.
 *
 * @return  Pointer to the new heap
 */
violation_heap* create_heap();

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void destroy_heap( violation_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void clear_heap( violation_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's key
 */
uint32_t get_key( violation_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's item
 */
void* get_item( violation_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t get_size( violation_heap *heap );

/**
 * Takes an item-key pair to insert into the heap and creates a new
 * corresponding node.  Makes the new node a root.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
violation_node* insert( violation_heap *heap, void *item, uint32_t key );

/**
 * Returns the minimum item from the heap.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* find_min( violation_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <delete> to remove the minimum item.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* delete_min( violation_heap *heap );

/**
 * Removes an arbitrary item from the heap and modifies heap structure
 * to preserve heap properties.  Requires that the location of the
 * item's corresponding node is known.  Merges the node's children with
 * the root list.  Merges roots such that no more than two roots have
 * the same rank.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Item removed
 */
void* delete( violation_heap *heap, violation_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.  Replaces
 * the subtree rooted at the given node with its active subtree of
 * larger rank and then relocates the rest of the tree as a new root.
 *
 * @param heap  Heap in which the node resides
 * @param node  Node to change
 * @param delta Amount by which to decrease the requested key
 */
void decrease_key( violation_heap *heap, violation_node *node, uint32_t delta );

/**
 * Moves all elements from the secondary heap into this one.  Leaves the
 * secondary heap empty.
 *
 * @param heap          Primary heap to be melded - target
 * @param other_heap    Secondary heap to be melded - source
 */
void meld( violation_heap *heap, violation_heap *other_heap );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool empty( violation_heap *heap );

/**
 * Merges a new node list into the root list.
 *
 * @param heap  Heap to merge list into
 * @param list  List to merge
 */
void merge_into_roots( violation_heap *heap, violation_node *list );

/**
 * Links three trees, making the smallest-keyed item the parent.
 *
 * @param a First node
 * @param b Second node
 * @param c Third node
 * @return  Returns the resulting tree
 */
violation_node* triple_join( violation_node *a, violation_node *b,
        violation_node *c );

/**
 * Makes two nodes the last two children of a third parent node.
 *
 * @param parent    Parent node
 * @param child1    Child of greater rank
 * @param child2    Child of lesser rank
 * @return          Root of new tree
 */
violation_node* join( violation_node *parent, violation_node *child1,
        violation_node *child2 );

/**
 * Iterates through roots and three-way joins trees of the same rank
 * until no three trees remain with the same rank.
 *
 * @param heap  Heap whose roots to fix
 */
void fix_roots( violation_heap *heap );

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param heap  Heap to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
bool attempt_insert( violation_heap *heap, violation_node *node );

/**
 * Scans through the roots array to find the tree with the minimum-value
 * root.
 *
 * @param heap  Heap to fix
 */
void set_min( violation_heap *heap );

/**
 * Loops around a singly-linked list of roots to find the root prior to
 * the specified node.
 *
 * @param node  The specified node to start from
 * @return      The node prior to the start
 */
violation_node* find_prev_root( violation_node *node );

/**
 * Propagates rank changes upward from the initial node.
 *
 * @param node  Initial node to begin updating from.
 */
void propagate_ranks( violation_node *node );

/**
 * Converts a doubly-linked list into a circular singly-linked list.
 *
 * @param node  Last node in the list
 */
void strip_list( violation_node *node );

/**
 * Determines whether this node is active, meaning it is one of
 * the last two children of its parent.
 *
 * @param node  Node to query
 * @return      True if active, false if not
 */
bool is_active( violation_node *node );

/**
 * Returns the parent of the current node.
 *
 * @param node  Node to query
 * @return      Parent of the queried node, NULL if root
 */
violation_node* get_parent( violation_node *node );

#endif
