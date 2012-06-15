#include "binomial_queue.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void make_root( binomial_queue *queue, binomial_node *node );
static void fix_roots( binomial_queue *queue );
static void remove_from_queue( binomial_queue *queue, binomial_node *node );
static void cherry_pick_min( binomial_queue *queue );
static binomial_node* join( binomial_node *a, binomial_node *b );
static binomial_node* attempt_insert( binomial_queue *queue,
    binomial_node *node );
static void break_tree( binomial_queue *queue, binomial_node *node );
static void print_tree( binomial_node *node );

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
    binomial_node *wrapper = pq_alloc_node( queue->map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    queue->size++;

    make_root( queue, wrapper );
    fix_roots( queue );

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
    return pq_delete( queue, queue->minimum );
}

key_type pq_delete( binomial_queue *queue, binomial_node *node )
{
    key_type key = node->key;

    remove_from_queue( queue, node );
    fix_roots( queue );

    pq_free_node( queue->map, node );
    queue->size--;

    return key;
}

void pq_decrease_key( binomial_queue *queue, binomial_node *node,
    key_type new_key )
{
    node->key = new_key;
    if ( node->parent == NULL || node->key < node->parent->key )
    {
        remove_from_queue( queue, node );
        make_root( queue, node );
        fix_roots( queue );
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
    node->next_sibling = queue->minimum;
    queue->minimum = node;
}

/**
 * Links roots until there is no more than one of each rank.  Picks the minimum
 * root to put at the head of the list, then links the rest together.
 *
 * @param queue Queue to fix
 */
static void fix_roots( binomial_queue *queue )
{
    uint32_t i;
    binomial_node *current = queue->minimum;
    binomial_node *next;

    while( current != NULL )
    {
        next = current->next_sibling;
        current->next_sibling = NULL;
        while( current )
            current = attempt_insert( queue, current );
        current = next;
    }

    cherry_pick_min( queue );
    current = queue->minimum;
    for( i = 0; i < MAXRANK; i++ )
    {
        if( queue->roots[i] != NULL )
        {
            current->next_sibling = queue->roots[i];
            queue->roots[i] = NULL;
            current = current->next_sibling;
        }
    }
}

/**
 * Removes a node from the queue.  If the node isn't a root, all the trees in
 * which the node are contained need to be broken so as to maintain binomial
 * form.  Furthermore, the node's subtree needs to be broken apart.  All these
 * new trees formed are mades roots, but the root list is left a mess.  Cleanup
 * should be done afterward using @ref <fix_roots>.
 *
 * @param queue Queue to which the node currently belongs
 * @param node  Node to remove
 */
static void remove_from_queue( binomial_queue *queue, binomial_node *node )
{
    binomial_node *current, *next, *head;
    head = node;

    // node isn't a root, break apart trees as necessary
    while( head->parent != NULL )
    {
        next = head->parent;
        current = head->parent->first_child;
        while( 1 )
        {
            next->first_child = current->next_sibling;
            next->rank--;
            if( current == node )
                break_tree( queue, current );
            else
                make_root( queue, current );

            if( current == head )
                break;

            current = next->first_child;
        }
        head = next;
    }

    // node was already a root
    if( head == node )
    {
        if( node == queue->minimum )
            queue->minimum = node->next_sibling;
        else
        {
            current = queue->minimum;
            while( current->next_sibling != node )
                current = current->next_sibling;
            current->next_sibling = node->next_sibling;
        }
        break_tree( queue, node );
    }
}

/**
 * Picks and sets the minimum root.  Assumes the roots array has been filled
 * with all relevant roots.  After selecting the minimum it removes it from the
 * array.
 *
 * @param queue Queue from which to select the minimum
 */
static void cherry_pick_min( binomial_queue *queue )
{
    uint32_t i;
    uint32_t min = 0;

    for( i = 1; i < MAXRANK; i++ )
    {
        if( queue->roots[i] == NULL )
            continue;

        if( queue->roots[min] == NULL ||
                queue->roots[i]->key < queue->roots[min]->key )
            min = i;
    }

    queue->minimum = queue->roots[min];
    queue->roots[min] = NULL;
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
    if( a->key <= b->key )
    {
        parent = a;
        child = b;
    }
    else
    {
        parent = b;
        child = a;
    }

    child->next_sibling = parent->first_child;
    child->parent = parent;
    parent->first_child = child;

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
    binomial_node *result = NULL;
    uint32_t rank = node->rank;

    if( queue->roots[rank] != NULL )
    {
        result = join( node, queue->roots[rank] );
        queue->roots[rank] = NULL;
    }
    else
        queue->roots[rank] = node;

    return result;
}

/**
 * Breaks apart a tree given the root.  Makes all children new roots, and leaves
 * the specified node as a rank-0 tree.
 *
 * @param queue Queue in which to operate
 * @param node  Node whose subtree to break
 */
static void break_tree( binomial_queue *queue, binomial_node *node )
{
    binomial_node *current, *next;

    current = node->first_child;
    while( current != NULL )
    {
        next = current->next_sibling;
        make_root( queue, current );
        current = next;
    }

    node->rank = 0;
    node->first_child = NULL;
    node->next_sibling = NULL;
}
