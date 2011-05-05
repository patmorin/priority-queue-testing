#ifndef PAIRING_HEAP
#define PAIRING_HEAP

#include "heap_common.h"
/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node is contained in a doubly linked list of
 * siblings and has a pointer to it's first child.  If a node is the
 * first of its siblings, then its next pointer points to their
 * collective parent.  The last child is marked by a null prev pointer.
 */
typedef struct pairing_node_t {
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
} pairing_node;
    
/**
 * A mutable, meldable, two-pass Pairing heap.  Maintains a single multiary tree
 * with no structural constraints other than the standard heap invariant.
 * Handles most operations through cutting and pairwise merging.  Primarily uses
 * iteration for merging rather than the standard recursion methods (due to
 * concerns for stackframe overhead).
 */
typedef struct pairing_heap_t {
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    pairing_node *root;
    //! A collection of operation counters
    STAT_STRUCTURE
} pairing_heap;

typedef pairing_heap* pq_ptr;
typedef pairing_node* it_type;

/**
 * Creates a new, empty heap.
 *
 * @return  Pointer to the new heap
 */
pairing_heap* create_heap();

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void destroy_heap( pairing_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void clear_heap( pairing_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type get_key( pairing_heap *heap, pairing_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* get_item( pairing_heap *heap, pairing_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t get_size( pairing_heap *heap );

/**
 * Takes an item-key pair to insert it into the heap and creates a new
 * corresponding node.  Merges the new node with the root of the heap.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
pairing_node* insert( pairing_heap *heap, item_type item, key_type key );

/**
 * Returns the minimum item from the heap without modifying any data.
 *
 * @param heap  Heap to query
 * @return      Node with minimum key
 */
pairing_node* find_min( pairing_heap *heap );

/**
 * deletes the minimum item from the heap and returns it, restructuring
 * the heap along the way to maintain the heap property.  Relies on the
 * @ref <delete> method to delete the root of the tree.
 *
 * @param heap  Heap to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type delete_min( pairing_heap *heap );

/**
 * deletes an arbitrary item from the heap and modifies heap structure
 * to preserve heap properties.  Requires that the location of the
 * item's corresponding node is known.  deletes the node from its list
 * of siblings, then merges all its children into a new tree and
 * subsequently merges that tree with the root.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the item to delete
 * @return      Key of item deleted
 */
key_type delete( pairing_heap *heap, pairing_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.  Cuts the node
 * from its list of siblings and merges it with the root.
 *
 * @param heap      Heap in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void decrease_key( pairing_heap *heap, pairing_node *node, key_type new_key );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool empty( pairing_heap *heap );

/**
 * Merges two nodes together, making the item of greater key the child
 * of the other.
 *
 * @param heap  Heap in which to operate
 * @param a     First node
 * @param b     Second node
 * @return      Resulting tree root
 */
pairing_node* merge( pairing_heap *heap, pairing_node *a, pairing_node *b );

/**
 * Performs an iterative pairwise merging of a list of nodes until a
 * single tree remains.  Implements the two-pass method without using
 * explicit recursion (to prevent stack overflow with large lists).
 * Performs the first pass in place while maintaining a minimum of list
 * structure to iterate back through during the second pass.
 *
 * @param heap  Heap in which to operate
 * @param node  Head of the list to collapse
 * @return      Root of the collapsed tree
 */
pairing_node* collapse( pairing_heap *heap, pairing_node *node );

#endif
