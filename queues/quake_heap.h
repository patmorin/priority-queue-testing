#ifndef QUAKE_HEAP
#define QUAKE_HEAP

//==============================================================================
// DEFINES AND INCLUDES
//==============================================================================

#include "heap_common.h"

//==============================================================================
// STRUCTS
//==============================================================================

/**
 * Holds an inserted element, as well as pointers to maintain tree
// STRUcture.  Acts as a handle to clients for the purpose of
 * mutability.  Keeps track of the height of the node as well as pointer
 * to the node's parent, left child (duplicate), and right child.
 */
typedef struct quake_node_t
{
    //! Parent node
    struct quake_node_t *parent;
    //! Left child
    struct quake_node_t *left;
    //! Right child, or next root if this node is a root
    struct quake_node_t *right;

    //! The height of this node
    uint8_t height;

    //! Pointer to a piece of client data
    item_type item;
    //! Key for the item
    key_type key;
} quake_node;

/**
 * A mutable, meldable, Quake heap.  Maintains a forest of (binary) tournament
 * trees of unique height.  Maintains standard heap invariant and guarantees
 * exponential decay in node height.
 */
typedef struct quake_heap_t
{
    //! The number of items held in the heap
    uint32_t size;
    //! Pointer to the minimum node in the heap
    quake_node *minimum;
    //! An array of roots of the heap, indexed by height
    quake_node *roots[MAXRANK];
    //! An array of counters corresponding to the number of nodes at height
    //! equal to the index
    uint32_t nodes[MAXRANK];
    //! Current height of highest node in heap
    uint32_t highest_node;
    //! Index at which first decay violation occurs, MAXRANK if none
    uint32_t violation;
} quake_heap;

typedef quake_heap* pq_ptr;
typedef quake_node it_type;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty heap.
 *
 * @param capacity  Maximum number of nodes the heap is expected to hold
 * @return          Pointer to the new heap
 */
quake_heap* pq_create( uint32_t capacity );

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void pq_destroy( quake_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void pq_clear( quake_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
key_type pq_get_key( quake_heap *heap, quake_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
item_type* pq_get_item( quake_heap *heap, quake_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t pq_get_size( quake_heap *heap );

/**
 * Takes an item-key pair to insert it into the heap and creates a new
 * corresponding node.  inserts the node as a new root in the heap.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
quake_node* pq_insert( quake_heap *heap, item_type item, key_type key );

/**
 * Returns the minimum item from the heap.
 *
 * @param heap  Heap to query
 * @return      Node with minimum key
 */
quake_node* pq_find_min( quake_heap *heap );

/**
 * Removes the minimum item from the heap and returns it, restructuring
 * the heap along the way to maintain the heap property.  Relies on
 * @ref <pq_delete> to extract the minimum.
 *
 * @param heap  Heap to query
 * @return      Minimum key, corresponding to item deleted
 */
key_type pq_delete_min( quake_heap *heap );

/**
 * Removes an arbitrary item from the heap and modifies heap structure
 * to preserve heap properties.  Requires that the location of the
 * item's corresponding node is known.  Merges roots so that no two
 * roots of the same height remain.  Checks to make sure that the
 * exponential decay invariant is maintained, and corrects if not.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the item to remove
 * @return      Key of item removed
 */
key_type pq_delete( quake_heap *heap, quake_node *node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.  Removes the
 * subtree rooted at the given node and makes it a new tree in the heap.
 *
 * @param heap      Heap in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
void pq_decrease_key( quake_heap *heap, quake_node *node, key_type new_key );

/**
 * Combines two different item-disjoint heaps which share a memory map.
 * Merges node lists and adds the rank lists.  Returns a pointer to the
 * resulting heap.
 * 
 * @param a First heap
 * @param b Second heap
 * @return  Resulting merged heap
 */
quake_heap* meld( quake_heap *a, quake_heap *b );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool pq_empty( quake_heap *heap );

/**
 * Joins a node with the list of roots.
 *
 * @param heap  Heap in which to operate
 * @param node  Node to make a new root
 */
void make_root( quake_heap *heap, quake_node *node );

/**
 * Removes a node from the list of roots.
 *
 * @param heap  Heap the node belongs to
 * @param node  Node to remove
 */
void remove_from_roots( quake_heap *heap, quake_node *node );

/**
 * Removes the node from the structure.  Recurses down through the left
 * child, which contains the same item, making the other child a new
 * root.
 *
 * @param heap  Heap the node belongs to
 * @param node  Node to remove
 */
void cut( quake_heap *heap, quake_node *node );

/**
 * Links two trees, making the larger-key tree the child of the lesser.
 * Creates a duplicate node to take the larger-key root's place.
 * Promotes the larger-key root as the new root of the joined tree.
 *
 * @param heap  Heap in which to operate
 * @param a     First node
 * @param b     Second node
 * @return      Returns the resulting tree
 */
quake_node* join( quake_heap *heap, quake_node *a, quake_node *b );

/**
 * Performs an iterative linking on the list of roots until no two trees
 * of the same height remain.
 *
 * @param heap  Heap whose roots to fix
 */
void fix_roots( quake_heap *heap );

/**
 * Attempt to insert a tree in the height-indexed array.  inserts if the
 * correct spot is empty or already contains the current node, reports
 * failure if it is occupied.
 *
 * @param heap  Heap to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
bool attempt_insert( quake_heap *heap, quake_node *node );

/**
 * Scans through the roots list starting from the current, potentially
 * inaccurate, minimum to find the tree with the minimum-value
 * root.
 * 
 * @param heap  Heap to fix
 */
void fix_min( quake_heap *heap );

/**
 * If a decay violation exists, this will remove all nodes of height
 * greater than or equal to the first violation.
 * 
 * @param heap  Heap to fix
 */
void fix_decay( quake_heap *heap );

/**
 * Searches for a decay violation and saves its location if it exists.
 * 
 * @param heap  Heap to check
 */
void check_decay( quake_heap *heap );

/**
 * Checks if a decay violation was found.
 *
 * @param heap  Heap to check
 * @return      True if exists, false otherwise
 */
bool violation_exists( quake_heap *heap );

/**
 * If the current node is higher than the violation, this function
 * rotates the current node down into the place of it's duplicate, and
 * deletes the duplicate.  Then it recurses on itself and its
 * non-duplicate child.
 *
 * @param heap  Heap to fix
 * @param node  Node to check and prune
 */
void prune( quake_heap *heap, quake_node *node );

/**
 * Copies internal data of another node for purposes of tournament resolution.
 *
 * @param heap      Heap to which node belongs
 * @param original  Node to copy data from
 * @return          Copy of the new node
 */
quake_node* clone_node( quake_heap *heap, quake_node *original );

/**
 * Determines whether this node is a root
 *
 * @param heap  Heap in which node resides
 * @param node  Node to query
 * @return      True if root, false otherwise
 */
bool is_root( quake_heap *heap, quake_node *node );

#endif

