#ifndef RANKPAIRING_HEAP
#define RANKPAIRING_HEAP

#include "heap_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Keeps track of rank, as well as pointers to parent and
 * left and right children.  In the case of a root, the right child
 * pointer points to the next root.
 */
typedef struct rank_pairing_node_t {
    //! Parent node
    struct rank_pairing_node_t *parent;
    //! Left child
    struct rank_pairing_node_t *left;
    //! Right child, or next root if this node is a root
    struct rank_pairing_node_t *right;

    //! The number of children this node has
    uint32_t rank;
    
    //! Pointer to a piece of client data
    void *item;
    //! Key for the item
    PRIORITY_T key;
} rank_pairing_node;

/**
 * A mutable, meldable, rank pairing heap.  Maintains a forest of half-trees
 * managed by rank.  Obeys the type-1 rank rule and utilizes restricted
 * multi-pass linking.
 */
typedef struct rank_pairing_heap_t {
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    rank_pairing_node *minimum;
    //! An array of roots of the heap, indexed by rank
    rank_pairing_node *roots[MAXRANK];
    //! A collection of operation counters
    STAT_STRUCTURE
} rank_pairing_heap;

/**
 * Creates a new, empty heap.
 *
 * @return  Pointer to the new heap
 */
rank_pairing_heap* create_heap();

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void destroy_heap( rank_pairing_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void clear_heap( rank_pairing_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's key
 */
uint32_t get_key( rank_pairing_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's item
 */
void* get_item( rank_pairing_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t get_size( rank_pairing_heap *heap );

/**
 * Takes an item-key pair to insert it into the heap and creates a new
 * corresponding node.  Makes the new node a root.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
rank_pairing_node* insert( rank_pairing_heap *heap, void* item, uint32_t key );

/**
 * Returns the minimum item from the heap.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* find_min( rank_pairing_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <delete> to remove the minimum item.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* delete_min( rank_pairing_heap *heap );

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
 * @return      Item removed
 */
void* delete( rank_pairing_heap *heap, rank_pairing_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.  Cuts the node
 * from its parent and traverses the tree upwards to correct the rank
 * values.
 *
 * @param heap  Heap in which the node resides
 * @param node  Node to change
 * @param delta Amount by which to decrease the requested key
 */
void decrease_key( rank_pairing_heap *heap, rank_pairing_node *node, uint32_t delta );

/**
 * Moves all elements from the secondary heap into this one.  Leaves the
 * secondary heap empty.  Merges root lists.
 *
 * @param heap          Primary heap to be melded - target
 * @param other_heap    Secondary heap to be melded - source
 */
void meld( rank_pairing_heap *heap, rank_pairing_heap *other_heap );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool empty( rank_pairing_heap *heap );

/**
 * Merges two node lists into one and makes the minimum the root of the
 * current tree.
 *
 * @param heap  Heap the two lists belong to
 * @param a     First node list
 * @param b     Second node list
 */
void merge_roots( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b );

/**
 * Merges two node lists into one and returns a pointer into the new
 * list
 *
 * @param a First node list
 * @param b Second node list
 * @return  A node in the list
 */
rank_pairing_node* merge_lists( rank_pairing_node *a, rank_pairing_node *b );

/**
 * Picks and returns the minimum between two nodes.
 *
 * @param a First node
 * @param b Second node
 * @return  Minimum of the two nodes
 */
rank_pairing_node* pick_min( rank_pairing_node *a, rank_pairing_node *b );

/**
 * Links two trees, making the larger-key tree the child of the lesser.
 *
 * @param a First node
 * @param b Second node
 * @return  Returns the resulting tree
 */
rank_pairing_node* join( rank_pairing_node *a, rank_pairing_node *b );

/**
 * Performs a one-pass linking run through the list of roots.  Links
 * trees of equal ranks.
 *
 * @param heap  Heap whose roots to fix
 */
void fix_roots( rank_pairing_heap *heap );

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param heap  Heap to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
bool attempt_insert( rank_pairing_heap *heap, rank_pairing_node *node );

/**
 * Scans through the roots list starting from the current, potentially
 * inaccurate, minimum to find the tree with the minimum-value
 * root.
 * 
 * @param heap  Heap to fix
 */
void fix_min( rank_pairing_heap *heap );

/**
 * Propagates rank corrections upward from the initial node.
 *
 * @param node  Initial node to begin updating from.
 */
void propagate_ranks( rank_pairing_node *node );

/**
 * Converts the given node and its right spine to a singly-linked circular list
 * of roots.
 *
 * @param node  Root of the spine
 */
rank_pairing_node* sever_spine( rank_pairing_node *node );

#endif

