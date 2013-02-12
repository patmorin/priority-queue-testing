#include "binomial_queue.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void make_root( binomial_queue *queue, binomial_node *node );
static void cherry_pick_min( binomial_queue *queue );
static binomial_node* join( binomial_node *a, binomial_node *b );
static binomial_node* attempt_insert( binomial_queue *queue,
    binomial_node *node );
static void break_tree( binomial_queue *queue, binomial_node *node );
static void swap_with_parent( binomial_queue *queue, binomial_node *node,
    binomial_node *parent );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

binomial_queue* pq_create( mem_map *map )
{
    binomial_queue *queue = calloc( 1, sizeof( binomial_queue ) );
    queue->map = map;

    return queue;
}

void pq_destroy( binomial_queue *queue )
{
    pq_clear( queue );
    free( queue );
}

void pq_clear( binomial_queue *queue )
{
    mm_clear( queue->map );
    queue->minimum = NULL;
    queue->registry = 0;
    memset( queue->roots, 0, MAXRANK * sizeof( binomial_node* ) );
    queue->size = 0;
}

key_type pq_get_key( binomial_queue *queue, binomial_node *node )
{
    return node->key;
}

item_type* pq_get_item( binomial_queue *queue, binomial_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( binomial_queue *queue )
{
    return queue->size;
}

binomial_node* pq_insert( binomial_queue *queue, item_type item, key_type key )
{
    binomial_node *wrapper = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    queue->size++;
    make_root( queue, wrapper );

    return wrapper;
}

binomial_node* pq_find_min( binomial_queue *queue )
{
    if ( pq_empty( queue ) )
        return NULL;

    return queue->minimum;
}

key_type pq_delete_min( binomial_queue *queue )
{
    key_type key = queue->minimum->key;
    binomial_node *old_min = queue->minimum;

    REGISTRY_UNSET( queue->registry, old_min->rank );
    queue->roots[old_min->rank] = NULL;

    break_tree( queue, old_min );
    cherry_pick_min( queue );

    pq_free_node( queue->map, 0, old_min );
    queue->size--;

    return key;
}

key_type pq_delete( binomial_queue *queue, binomial_node *node )
{
    key_type key = node->key;

    pq_decrease_key( queue, node, 0 );
    pq_delete_min( queue );

    return key;
}

void pq_decrease_key( binomial_queue *queue, binomial_node *node,
    key_type new_key )
{
    node->key = new_key;
    binomial_node *current, *parent;
    while( node->parent != NULL )
    {
        parent = node->parent;
        current = node;
        while( parent->right == current )
        {
            current = current->parent;
            parent = current->parent;
        }

        if( node->key < current->key )
            swap_with_parent( queue, node, parent );
        else
            break;
    }
}

bool pq_empty( binomial_queue *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Makes a given node a root.
 *
 * @param queue Queue in which the node will be a root
 * @param node  Node to make a root
 */
static void make_root( binomial_queue *queue, binomial_node *node )
{
    node->parent = NULL;
    node->right = NULL;

    if( queue->minimum == NULL || node->key < queue->minimum->key )
        queue->minimum = node;

    binomial_node *result = node;
    while( result != NULL )
        result = attempt_insert( queue, result );
}

/**
 * Picks and sets the minimum root.  Assumes the roots array has been filled
 * with all relevant roots.
 *
 * @param queue Queue from which to select the minimum
 */
static void cherry_pick_min( binomial_queue *queue )
{
    uint32_t rank;
    uint64_t registry = queue->registry;
    uint32_t min = REGISTRY_LEADER( registry );
    if( min >= MAXRANK )
        return;

    REGISTRY_UNSET( registry, min );
    while( registry )
    {
        rank = REGISTRY_LEADER( registry );
        REGISTRY_UNSET( registry, rank );
        if( queue->roots[rank]->key < queue->roots[min]->key )
            min = rank;
    }

    queue->minimum = queue->roots[min];
}

/**
 * Joins two binomial trees of equal rank, making the lesser-keyed root the
 * parent.
 *
 * @param a Root of first tree
 * @param b Root of second tree
 * @return  The resulting tree
 */
static binomial_node* join( binomial_node *a, binomial_node *b )
{
    binomial_node *parent, *child;
    if( b->key < a->key)
    {
        parent = b;
        child = a;
    }
    else
    {
        parent = a;
        child = b;
    }

    child->right = parent->left;
    if( parent->left != NULL )
        parent->left->parent = child;
    child->parent = parent;
    parent->left = child;

    parent->rank++;

    return parent;
}

/**
 * Attempts to insert a root into the roots array.  If the correct slot is
 * empty it inserts the root and returns.  If there is already a root with the
 * same rank, it joins the two and returns a pointer to the resulting tree.
 *
 * @param queue Queue in which the tree resides
 * @param node  Root of the tree to insert
 * @return      NULL if successful, merged tree if not
 */
static binomial_node* attempt_insert( binomial_queue *queue,
    binomial_node *node )
{
    uint32_t rank = node->rank;
    binomial_node *result = NULL;

    if( OCCUPIED( queue->registry, rank ) )
    {
        result = join( node, queue->roots[rank] );
        queue->roots[rank] = NULL;
        REGISTRY_UNSET( queue->registry, rank );
    }
    else
    {
        queue->roots[rank] = node;
        REGISTRY_SET( queue->registry, rank );
    }

    return result;
}

/**
 * Breaks apart a tree given the root.  Makes all children new roots, and leaves
 * the node ready for deletion.
 *
 * @param queue Queue in which to operate
 * @param node  Node whose subtree to break
 */
static void break_tree( binomial_queue *queue, binomial_node *node )
{
    binomial_node *current, *next;

    current = node->left;
    while( current != NULL )
    {
        next = current->right;
        make_root( queue, current );
        current = next;
    }
}

/**
 * Swaps a node with its heap order parent.
 *
 * @param queue     Queue in which to operate
 * @param node      Node to swap
 * @param parent    Heap order parent of the node
 */
static void swap_with_parent( binomial_queue *queue, binomial_node *node,
    binomial_node *parent )
{
    binomial_node *s = node->parent;
    binomial_node *a = node->left;
    binomial_node *b = node->right;
    binomial_node *g = parent->parent;
    binomial_node *c = parent->left;
    binomial_node *d = parent->right;

    // fix ranks
    uint32_t temp = node->rank;
    node->rank = parent->rank;
    parent->rank = temp;

    // easy subtree steps
    parent->left = a;
    if( a != NULL )
        a->parent = parent;
    parent->right = b;
    if( b != NULL )
        b->parent = parent;
    node->right = d;
    if( d != NULL )
        d->parent = node;

    if( s == parent )
    {
        // we are in the first-child case (node == c)
        node->left = parent;
        parent->parent = node;
    }
    else
    {
        // we aren't the first child (node != c)
        node->left = c;
        if( c != NULL )
            c->parent = node;
        parent->parent = s;
        s->right = parent;
    }

    node->parent = g;
    if( g == NULL )
    {
        // parent was a root
        queue->roots[node->rank] = node;
        if( queue->minimum == parent )
            queue->minimum = node;
    }
    else
    {
        // not dealing with a root
        if( g->left == parent )
            g->left = node;
        else
            g->right = node;
    }
}
