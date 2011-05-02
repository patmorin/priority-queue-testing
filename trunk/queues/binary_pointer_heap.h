#ifndef BINARY_POINTER_HEAP
#define BINARY_POINTER_HEAP

#include "heap_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node has a pointer to its parent and its left and
 * right siblings.
 */
typedef struct binary_pointer_node_t {
    //! Pointer to parent node
    struct binary_pointer_node_t *parent;
    //! Pointer to left child node
    struct binary_pointer_node_t *left;
    //! Pointer to right child node
    struct binary_pointer_node_t *right;

    //! Pointer to a piece of client data
    void* item;
    //! Key for the item
    KEY_T key;
} binary_pointer_node;

/**
 * A mutable, meldable, node-based binary heap.  Maintains a single, complete
 * binary tree.  Imposes the standard heap invariant.
 */
typedef struct binary_pointer_heap_t {
    //! The root of the binary tree representing the heap
    binary_pointer_node *root;
    //! The number of items held in the heap
    uint32_t size;
    //! A collection of operation counters
    STAT_STRUCTURE
} binary_pointer_heap;

/**
 * Creates a new, empty heap.
 *
 * @return  Pointer to the new heap
 */
binary_pointer_heap* create_heap();

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void destroy_heap( binary_pointer_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void clear_heap( binary_pointer_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's key
 */
uint32_t get_key( binary_pointer_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's item
 */
void* get_item( binary_pointer_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t get_size( binary_pointer_heap *heap );

/**
 * Takes a item-key pair to insert into the heap and creates a new
 * corresponding node.  Inserts the node at the base of the tree in the
 * next open spot and reorders to preserve the heap property.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
binary_pointer_node* insert( binary_pointer_heap *heap, void *item, uint32_t key );

/**
 * Returns the minimum item from the heap without modifying the heap.
 *
 * @param heap  Heap to query
 * @return      Node with minimum key
 */
binary_pointer_node* find_min( binary_pointer_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <delete> to remove the root node of the tree, containing the
 * minimum element.
 *
 * @param heap  Heap to query
 * @return      Minimum key, corresponding to item deleted
 */
KEY_T delete_min( binary_pointer_heap *heap ) ;

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
KEY_T delete( binary_pointer_heap *heap, binary_pointer_node* node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.
 *
 * @param heap      Heap in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void decrease_key( binary_pointer_heap *heap, binary_pointer_node *node, KEY_T new_key );

/**
 * Moves all elements from the secondary heap into this one.  Leaves the
 * secondary heap empty.
 *
 * @param heap          Primary heap to be melded - target
 * @param other_heap    Secondary heap to be melded - source
 */
void meld( binary_pointer_heap *heap, binary_pointer_heap *other_heap );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool empty( binary_pointer_heap *heap );

/**
 * Takes two nodes and switches their positions in the tree.  Does not
 * make any assumptions about null pointers or relative locations in
 * tree, and thus checks all edge cases to be safe.
 *
 * @param heap  Heap to which both nodes belong
 * @param a     First node to switch
 * @param b     Second node to switch
 */
void swap( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b );

/**
 * Takes two nodes known to be in a parent-child relationship and swaps
 * their positions in the tree.
 *
 * @param parent    Parent node
 * @param child     Child node
 */
void swap_connected( binary_pointer_node *parent, binary_pointer_node *child );

/**
 * Takes two nodes known not to be in a parent-child relationship and
 * swaps their positions in the tree.
 *
 * @param a First node
 * @param b Second node
 */
void swap_disconnected( binary_pointer_node *a, binary_pointer_node *b );

/**
 * Takes two nodes which have recently had their internal pointers
 * swapped, and updates surrounding nodes to point to the correct nodes.
 *
 * @param a First node
 * @param b Second node
 */
void fill_back_pointers( binary_pointer_node *a, binary_pointer_node *b );

/**
 * Takes a node that is potentially at a higher position in the tree
 * than it should be, and pushes it down to the correct location.
 *
 * @param heap  Heap to which the node belongs
 * @param node  Potentially violating node
 */
void heapify_down( binary_pointer_heap *heap, binary_pointer_node *node );

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param heap  Heap to which node belongs
 * @param node  Potentially violating node
 */
void heapify_up( binary_pointer_heap *heap, binary_pointer_node *node );

/**
 * Finds the last node in the tree and returns a pointer to its
 * location.
 *
 * @param heap  Heap to query
 * @return      Pointer to the last node in the tree
 */
binary_pointer_node* find_last_node( binary_pointer_heap *heap );

/**
 * Retrieves the proper parent for a newly inserted node.  Exploits
 * properties of complete binary trees and current node count.
 *
 * @param heap  Heap to query
 * @return      Node which will be the parent of a new insertion
 */
binary_pointer_node* find_insertion_point( binary_pointer_heap *heap );

/**
 * Finds an arbitrary node based in an integer index corresponding to
 * an inorder traversal of the tree.  The root corresponds to 1, its
 * left child 2, right child 3, and so on.
 *
 * @param heap  Heap to query
 * @param n     Index of node to find
 * @return      Located node
 */
binary_pointer_node* find_node( binary_pointer_heap *heap, uint32_t n );

/**
 * Finds the floor of the base-2 logarithm of an uint32_t integer using GCC's
 * built-in method for counting leading zeros.  Should be supported quickly by
 * most x86* machines.
 *
 * @param n Integer to find log of
 * @return  Log of n
 */
uint32_t int_log2( uint32_t n );

/**
 * Determines whether this node is a leaf based on child pointers.
 *
 * @param node  Node to query
 * @return      True if leaf, false otherwise
 */
bool is_leaf( binary_pointer_node* node );

#endif
