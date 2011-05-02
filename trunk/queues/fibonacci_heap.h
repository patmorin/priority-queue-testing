#ifndef FIBONACCI_HEAP
#define FIBONACCI_HEAP

#include "heap_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node is contained in a doubly-linked circular list
 * of its siblings.  Additionally, each node has a pointer to its parent
 * and its first child.
 */
typedef struct fibonacci_node_t {
    //! Pointer to a piece of client data
    void *item;
    //! Key for the item
    uint32_t key;
    //! The "height" of a node, i.e. bound on log of subtree size
    uint32_t rank;
    //! Denotes if a non-root node has lost a child or not
    bool marked;
    
    //! Parent of this node
    struct fibonacci_node_t *parent;
    //! "First" child of this node
    struct fibonacci_node_t *first_child;
    //! Next node in the list of this node's siblings
    struct fibonacci_node_t *next_sibling;
    //! Previous node in the list of this node's siblings
    struct fibonacci_node_t *prev_sibling;
} fibonacci_node;

/**
 * A mutable, meldable, Fibonacci heap.  Maintains a forest of (partial)
 * binomial trees, resulting from rank-wise paired merging.  Uses an array to
 * index trees by rank.  Imposes standard heap variant, as well as requiring
 * that there remains at most one tree of any given rank after any deletion
 * (managed by merging).  Furthermore, a series of ("cascading") cuts is
 * performed after a key decrease if the parent loses it's second child.
 */
typedef struct fibonacci_heap_t {
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    fibonacci_node *minimum;
    //! An array of roots of the heap, indexed by rank
    fibonacci_node *roots[MAXRANK];
    //! A collection of operation counters
    heap_stats *stats;
} fibonacci_heap;

/**
 * Creates a new, empty heap.
 *
 * @return  Pointer to the new heap
 */
fibonacci_heap* create_heap();

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void destroy_heap( fibonacci_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void clear_heap( fibonacci_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's key
 */
uint32_t get_key( fibonacci_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's item
 */
void* get_item( fibonacci_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t get_size( fibonacci_heap *heap );

/**
 * Takes an item-key pair to insert it into the heap and creates a new
 * corresponding node.  Inserts the node as a new root.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
fibonacci_node* insert( fibonacci_heap *heap, void* item, uint32_t key );

/**
 * Returns the minimum item from the heap without modifying the heap.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* find_min( fibonacci_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <delete> to remove the node.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* delete_min( fibonacci_heap *heap );

/**
 * Removes an arbitrary item from the heap.  Requires that the location
 * of the item's corresponding node is known.  After removing the node,
 * makes its children new roots in the heap.  Iteratively merges trees
 * of the same rank such that no two of the same rank remain afterward.
 * May initiate sequence of cascading cuts from node's parent.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Item removed
 */
void* delete( fibonacci_heap *heap, fibonacci_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will restructure the heap to repair any
 * potential structural violations.  Cuts the node from its parent and
 * makes it a new root, and potentially performs a series of cascading
 * cuts.
 *
 * @param heap  Heap in which the node resides
 * @param node  Node to change
 * @param delta Amount by which to decrease the requested key
 */
void decrease_key( fibonacci_heap *heap, fibonacci_node *node, uint32_t delta );

/**
 * Moves all elements from the secondary heap into this one.  Simply
 * appends root lists for very fast transfer.  Leaves secondary heap
 * empty.
 *
 * @param heap          Primary heap to be melded - target
 * @param other_heap    Secondary heap to be melded - source
 */
void meld( fibonacci_heap *heap, fibonacci_heap *other_heap );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool empty( fibonacci_heap *heap );

/**
 * Merges two node lists into one to update the root system of the heap.
 * Iteratively links the roots such that no two roots of the same rank
 * remain.
 *
 * @param heap  Heap to which the two lists belong
 * @param a     First node list
 * @param b     Second node list
 */
void merge_roots( fibonacci_heap *heap, fibonacci_node *a, fibonacci_node *b );

/**
 * Links two trees, making the item with lesser key the parent, breaking
 * ties arbitrarily.
 *
 * @param a First root
 * @param b Second root
 * @return  The resulting merged tree
 */
fibonacci_node* link( fibonacci_node *a, fibonacci_node *b );

/**
 * Recurses up the tree to make a series of cascading cuts.  Cuts each
 * node that has lost two children from its parent.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to cut
 */
void cut_from_parent( fibonacci_heap *heap, fibonacci_node *node );

/**
 * Appends two linked lists such that the head of the second comes
 * directly after the head from the first.
 *
 * @param a First head
 * @param b Second head
 * @return  Final, merged list
 */
fibonacci_node* append_lists( fibonacci_node *a, fibonacci_node *b );

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param heap  Heap to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
bool attempt_insert( fibonacci_heap *heap, fibonacci_node *node );

/**
 * Scans through the roots array to find the tree with the minimum-value root.
 *
 * @param heap  Heap to fix
 */
void set_min( fibonacci_heap *heap );

#endif
