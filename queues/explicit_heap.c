#include "explicit_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void swap( explicit_heap *queue, explicit_node *a, explicit_node *b );
static void swap_connected( explicit_heap *queue, explicit_node *parent,
    explicit_node *child );
static void swap_disconnected( explicit_heap *queue, explicit_node *a,
    explicit_node *b );
static void fill_back_pointers( explicit_heap *queue, explicit_node *a,
    explicit_node *b );
static void heapify_down( explicit_heap *queue, explicit_node *node );
static void heapify_up( explicit_heap *queue, explicit_node *node );
static explicit_node* find_last_node( explicit_heap *queue );
static explicit_node* find_insertion_point( explicit_heap *queue );
static explicit_node* find_node( explicit_heap *queue, uint32_t n );
static uint32_t int_log2( uint32_t n );
static bool is_leaf( explicit_heap *queue, explicit_node* node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

explicit_heap* pq_create( mem_map *map )
{
    explicit_heap *queue = (explicit_heap*) calloc( 1, sizeof( explicit_heap ) );
    queue->map = map;
    
    return queue;
}

void pq_destroy( explicit_heap *queue )
{
    pq_clear( queue );
    free( queue );
    mm_destroy( queue->map );
}

void pq_clear( explicit_heap *queue )
{
    mm_clear( queue->map );
    queue->root = NULL;
    queue->size = 0;
}

key_type pq_get_key( explicit_heap *queue, explicit_node *node )
{
    return node->key;
}

item_type* pq_get_item( explicit_heap *queue, explicit_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( explicit_heap *queue )
{
    return queue->size;
}

explicit_node* pq_insert( explicit_heap *queue, item_type item, key_type key )
{
    int i;
    explicit_node* parent;
    explicit_node* node = pq_alloc_node( queue->map );
    ITEM_ASSIGN( node->item, item );
    node->key = key;

    if ( queue->root == NULL )
        queue->root = node;
    else
    {
        parent = find_insertion_point( queue );
        
        for( i = 0; i < BRANCHING_FACTOR; i++ )
        {
            if ( parent->children[i] == NULL )
                parent->children[i] = node;
        }

        node->parent = parent;
    }

    queue->size++;
    heapify_up( queue, node );
    
    return node;
}

explicit_node* pq_find_min( explicit_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->root;
}

key_type pq_delete_min( explicit_heap *queue )
{
    return pq_delete( queue, queue->root );
}

key_type pq_delete( explicit_heap *queue, explicit_node* node )
{
    int i;
    key_type key = node->key;
    explicit_node *last_node = find_last_node( queue );
    swap( queue, node, last_node);

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

    pq_free_node( queue->map, node );
    queue->size--;
    
    if ( pq_empty( queue ) )
        queue->root = NULL;
    else if ( node != last_node)
        heapify_down( queue, last_node );

    return key;
}

void pq_decrease_key( explicit_heap *queue, explicit_node *node,
    key_type new_key )
{
    node->key = new_key;
    heapify_up( queue, node );
}

bool pq_empty( explicit_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Takes two nodes and switches their positions in the tree.  Does not
 * make any assumptions about null pointers or relative locations in
 * tree, and thus checks all edge cases to be safe.
 *
 * @param queue Queue to which both nodes belong
 * @param a     First node to switch
 * @param b     Second node to switch
 */
static void swap( explicit_heap *queue, explicit_node *a, explicit_node *b )
{
    if ( ( a == NULL ) || ( b == NULL ) || ( a == b ) )
        return;
    
    if ( a->parent == b )
        swap_connected( queue, b, a );
    else if ( b->parent == a )
        swap_connected( queue, a, b );
    else
        swap_disconnected( queue, a, b );

    if ( queue->root == a )
        queue->root = b;
    else if ( queue->root == b )
        queue->root = a;
}

/**
 * Takes two nodes known to be in a parent-child relationship and swaps
 * their positions in the tree.
 *
 * @param queue     Queue to which both nodes belong
 * @param parent    Parent node
 * @param child     Child node
 */
static void swap_connected( explicit_heap *queue, explicit_node *parent,
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
    
    fill_back_pointers( queue, parent, child );
}

/**
 * Takes two nodes known not to be in a parent-child relationship and
 * swaps their positions in the tree.
 *
 * @param queue Queue to which both nodes belong
 * @param a     First node
 * @param b     Second node
 */
static void swap_disconnected( explicit_heap *queue, explicit_node *a,
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

    fill_back_pointers( queue, a, b );
}

/**
 * Takes two nodes which have recently had their internal pointers
 * swapped, and updates surrounding nodes to point to the correct nodes.
 *
 * @param queue Queue to which both nodes belong
 * @param a First node
 * @param b Second node
 */
static void fill_back_pointers( explicit_heap *queue, explicit_node *a,
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
 * @param queue Queue to which the node belongs
 * @param node  Potentially violating node
 */
static void heapify_down( explicit_heap *queue, explicit_node *node )
{
    if ( node == NULL )
        return;

    // repeatedly swap with smallest child if node violates queue order
    explicit_node* smallest_child;
    int k, min_k;
    while ( !is_leaf( queue, node ) )
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
            swap( queue, smallest_child, node );
        else
            break;
    }
}

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param queue Queue to which node belongs
 * @param node  Potentially violating node
 */
static void heapify_up( explicit_heap *queue, explicit_node *node )
{
    if ( node == NULL )
        return;

    while ( node->parent != NULL )
    {
        if ( node->key < node->parent->key )
            swap( queue, node, node->parent );
        else
            break;
    }
}

/**
 * Finds the last node in the tree and returns a pointer to its
 * location.
 *
 * @param queue Queue to query
 * @return      Pointer to the last node in the tree
 */
static explicit_node* find_last_node( explicit_heap *queue )
{
    return find_node( queue, queue->size - 1 );
}

/**
 * Retrieves the proper parent for a newly inserted node.  Exploits
 * properties of complete binary trees and current node count.
 *
 * @param queue Queue to query
 * @return      Node which will be the parent of a new insertion
 */
static explicit_node* find_insertion_point( explicit_heap *queue )
{
    return find_node( queue, ( queue->size ) / BRANCHING_FACTOR );
}

/**
 * Finds an arbitrary node based on an integer index corresponding to
 * a level-order traversal of the tree.  The root corresponds to 0, its
 * first child 1, second child 2, and so on.
 *
 * @param queue Queue to query
 * @param n     Index of node to find
 * @return      Located node
 */
static explicit_node* find_node( explicit_heap *queue, uint32_t n )
{
    uint32_t log, path, i;
    uint32_t mask = BRANCHING_FACTOR - 1;
    explicit_node *current, *next;
    uint32_t location = n-1;

    if( n == 0 )
        return queue->root;
        
    log = int_log2(n-1) / BRANCHING_POWER;
    current = queue->root;
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
 * Finds the floor of the base-2 logarithm of a uint32_t integer using GCC's
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
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      True if leaf, false otherwise
 */
static bool is_leaf( explicit_heap *queue, explicit_node* node )
{
    return ( node->children[0] == NULL );
}
