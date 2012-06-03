#include "explicit_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
static mem_map *map;

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void swap( explicit_heap *heap, explicit_node *a, explicit_node *b );
static void swap_connected( explicit_heap *heap, explicit_node *parent,
    explicit_node *child );
static void swap_disconnected( explicit_heap *heap, explicit_node *a,
    explicit_node *b );
static void fill_back_pointers( explicit_heap *heap, explicit_node *a,
    explicit_node *b );
static void heapify_down( explicit_heap *heap, explicit_node *node );
static void heapify_up( explicit_heap *heap, explicit_node *node );
static explicit_node* find_last_node( explicit_heap *heap );
static explicit_node* find_insertion_point( explicit_heap *heap );
static explicit_node* find_node( explicit_heap *heap, uint32_t n );
static uint32_t int_log2( uint32_t n );
static bool is_leaf( explicit_heap *heap, explicit_node* node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

explicit_heap* pq_create( uint32_t capacity )
{
    map = mm_create( capacity );
    explicit_heap *heap = (explicit_heap*) calloc( 1, sizeof( explicit_heap ) );
    return heap;
}

void pq_destroy( explicit_heap *heap )
{
    pq_clear( heap );
    free( heap );
    mm_destroy( map );
}

void pq_clear( explicit_heap *heap )
{
    mm_clear( map );
    heap->root = NULL;
    heap->size = 0;
}

key_type pq_get_key( explicit_heap *heap, explicit_node *node )
{
    return node->key;
}

item_type* pq_get_item( explicit_heap *heap, explicit_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( explicit_heap *heap )
{
    return heap->size;
}

explicit_node* pq_insert( explicit_heap *heap, item_type item, key_type key )
{
    int i;
    explicit_node* parent;
    explicit_node* node = pq_alloc_node( map );
    ITEM_ASSIGN( node->item, item );
    node->key = key;

    if ( heap->root == NULL )
        heap->root = node;
    else
    {
        parent = find_insertion_point( heap );
        
        for( i = 0; i < BRANCHING_FACTOR; i++ )
        {
            if ( parent->children[i] == NULL )
                parent->children[i] = node;
        }

        node->parent = parent;
    }

    heap->size++;
    heapify_up( heap, node );
    
    return node;
}

explicit_node* pq_find_min( explicit_heap *heap )
{
    if ( pq_empty( heap ) )
        return NULL;
    return heap->root;
}

key_type pq_delete_min( explicit_heap *heap )
{
    return pq_delete( heap, heap->root );
}

key_type pq_delete( explicit_heap *heap, explicit_node* node )
{
    int i;
    key_type key = node->key;
    explicit_node *last_node = find_last_node( heap );
    swap( heap, node, last_node);

    // figure out if this node is a left or right child and clear
    // reference from parent
    if ( node->parent != NULL )
    {
        for( i = 0; i < BRANCHING_FACTOR; i++ )
        {
            if ( node->parent->children[i] == node )
                node->parent->children[i] = NULL;
        }
    }

    pq_free_node( map, node );
    heap->size--;
    
    if ( pq_empty( heap ) )
        heap->root = NULL;
    else if ( node != last_node)
        heapify_down( heap, last_node );

    return key;
}

void pq_decrease_key( explicit_heap *heap, explicit_node *node,
    key_type new_key )
{
    node->key = new_key;
    heapify_up( heap, node );
}

bool pq_empty( explicit_heap *heap )
{
    return ( heap->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Takes two nodes and switches their positions in the tree.  Does not
 * make any assumptions about null pointers or relative locations in
 * tree, and thus checks all edge cases to be safe.
 *
 * @param heap  Heap to which both nodes belong
 * @param a     First node to switch
 * @param b     Second node to switch
 */
static void swap( explicit_heap *heap, explicit_node *a, explicit_node *b )
{
    if ( ( a == NULL ) || ( b == NULL ) || ( a == b ) )
        return;
    
    if ( a->parent == b )
        swap_connected( heap, b, a );
    else if ( b->parent == a )
        swap_connected( heap, a, b );
    else
        swap_disconnected( heap, a, b );

    if ( heap->root == a )
        heap->root = b;
    else if ( heap->root == b )
        heap->root = a;
}

/**
 * Takes two nodes known to be in a parent-child relationship and swaps
 * their positions in the tree.
 *
 * @param heap  Heap to which both nodes belong
 * @param parent    Parent node
 * @param child     Child node
 */
static void swap_connected( explicit_heap *heap, explicit_node *parent,
    explicit_node *child )
{
    explicit_node *temp;

    child->parent = parent->parent;
    parent->parent = child;

    int i;
    for( i = 0; i < BRANCHING_FACTOR; i++ )
    {
        if( parent->children[i] == child )
        {
            parent->children[i] = child->children[i];
            child->children[i] = parent;
        }
        else
        {
            temp = parent->children[i];
            parent->children[i] = child->children[i];
            child->children[i] = temp;
        }
    }
    
    fill_back_pointers( heap, parent, child );
}

/**
 * Takes two nodes known not to be in a parent-child relationship and
 * swaps their positions in the tree.
 *
 * @param heap  Heap to which both nodes belong
 * @param a First node
 * @param b Second node
 */
static void swap_disconnected( explicit_heap *heap, explicit_node *a,
    explicit_node *b )
{
    explicit_node *temp[BRANCHING_FACTOR];
    
    temp[0] = a->parent;
    a->parent = b->parent;
    b->parent = temp[0];

    memcpy( temp, a->children, BRANCHING_FACTOR * sizeof( explicit_node* ) );
    memcpy( a->children, b->children, BRANCHING_FACTOR *
        sizeof( explicit_node* ) );
    memcpy( b->children, temp, BRANCHING_FACTOR * sizeof( explicit_node* ) );

    fill_back_pointers( heap, a, b );
}

/**
 * Takes two nodes which have recently had their internal pointers
 * swapped, and updates surrounding nodes to point to the correct nodes.
 *
 * @param heap  Heap to which both nodes belong
 * @param a First node
 * @param b Second node
 */
static void fill_back_pointers( explicit_heap *heap, explicit_node *a,
    explicit_node *b )
{
    int i;
    
    if ( a->parent != NULL )
    {
        for( i = 0; i < BRANCHING_FACTOR; i++ )
        {
            if( a->parent->children[i] == a || a->parent->children[i] == b )
            {
                a->parent->children[i] = a;
                break;
            }
        }
    }

    if ( b->parent != NULL )
    {
        for( i = 0; i < BRANCHING_FACTOR; i++ )
        {
            if( b->parent->children[i] == a || b->parent->children[i] == b )
            {
                b->parent->children[i] = b;
                break;
            }
        }
    }

    for( i = 0; i < BRANCHING_FACTOR; i++ )
    {
        if( a->children[i] != NULL )
            a->children[i]->parent = a;
        if( b->children[i] != NULL )
            b->children[i]->parent = b;
    }
}

/**
 * Takes a node that is potentially at a higher position in the tree
 * than it should be, and pushes it down to the correct location.
 *
 * @param heap  Heap to which the node belongs
 * @param node  Potentially violating node
 */
static void heapify_down( explicit_heap *heap, explicit_node *node )
{
    if ( node == NULL )
        return;

    // repeatedly swap with smallest child if node violates heap order
    explicit_node* smallest_child;
    int k, min_k;
    while ( !is_leaf( heap, node ) )
    {
        min_k = 0;
        for( k = 1; k < BRANCHING_FACTOR; k++ )
        {
            if( node->children[k] == NULL )
                break;
            if( node->children[k]->key < node->children[min_k]->key )
                min_k = k;
        }
        smallest_child = node->children[min_k];

        if ( smallest_child->key < node->key )
            swap( heap, smallest_child, node );
        else
            break;
    }
}

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param heap  Heap to which node belongs
 * @param node  Potentially violating node
 */
static void heapify_up( explicit_heap *heap, explicit_node *node )
{
    if ( node == NULL )
        return;

    while ( node->parent != NULL )
    {
        if ( node->key < node->parent->key )
            swap( heap, node, node->parent );
        else
            break;
    }
}

/**
 * Finds the last node in the tree and returns a pointer to its
 * location.
 *
 * @param heap  Heap to query
 * @return      Pointer to the last node in the tree
 */
static explicit_node* find_last_node( explicit_heap *heap )
{
    return find_node( heap, heap->size - 1 );
}

/**
 * Retrieves the proper parent for a newly inserted node.  Exploits
 * properties of complete binary trees and current node count.
 *
 * @param heap  Heap to query
 * @return      Node which will be the parent of a new insertion
 */
static explicit_node* find_insertion_point( explicit_heap *heap )
{
    return find_node( heap, ( heap->size ) / BRANCHING_FACTOR );
}

/**
 * Finds an arbitrary node based in an integer index corresponding to
 * an level-order traversal of the tree.  The root corresponds to 0, its
 * first child 1, second child 2, and so on.
 *
 * @param heap  Heap to query
 * @param n     Index of node to find
 * @return      Located node
 */
static explicit_node* find_node( explicit_heap *heap, uint32_t n )
{
    uint32_t log, path, i;
    uint32_t mask = BRANCHING_FACTOR - 1;
    explicit_node *current, *next;
    uint32_t location = n-1;

    if( n == 0 )
        return heap->root;
        
    log = int_log2(n-1) / BRANCHING_POWER;
    current = heap->root;
    // i < log is used instead of i >= 0 because i is uint32_t
    // it will loop around to MAX_INT after it passes 0
    for ( i = log; i < log; i-- )
    {
        path = ( ( location & ( mask << ( i * BRANCHING_POWER ) ) ) >>
            (i * BRANCHING_FACTOR ) ) - 1;
        next = current->children[path];
            
        if ( next == NULL )
            break;
        else
            current = next;
    }

    return current;     
}

/**
 * Finds the floor of the base-2 logarithm of an uint32_t integer using GCC's
 * built-in method for counting leading zeros.  Should be supported quickly by
 * most x86* machines.
 *
 * @param n Integer to find log of
 * @return  Log of n
 */
static uint32_t int_log2( uint32_t n )
{
    if ( n == 0 )
        return 0;
    return ( 31 - __builtin_clz( n ) );
}

/**
 * Determines whether this node is a leaf based on child pointers.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to query
 * @return      True if leaf, false otherwise
 */
static bool is_leaf( explicit_heap *heap, explicit_node* node )
{
    return ( node->children[0] == NULL );
}
