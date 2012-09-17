#include "fibonacci_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void merge_and_fix_roots( fibonacci_heap *queue, fibonacci_node *a,
    fibonacci_node *b );
static fibonacci_node* link( fibonacci_heap *queue, fibonacci_node *a,
    fibonacci_node *b );
static void cut_from_parent( fibonacci_heap *queue, fibonacci_node *node );
static fibonacci_node* append_lists( fibonacci_heap *queue, fibonacci_node *a,
    fibonacci_node *b );
static bool attempt_insert( fibonacci_heap *queue, fibonacci_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

fibonacci_heap* pq_create( mem_map *map )
{
    fibonacci_heap *queue = (fibonacci_heap*) calloc( 1,
        sizeof( fibonacci_heap ) );
    queue->map = map;

    return queue;
}

void pq_destroy( fibonacci_heap *queue ){
    pq_clear( queue );
    free( queue );
}

void pq_clear( fibonacci_heap *queue )
{
    mm_clear( queue->map );
    queue->minimum = NULL;
    memset( queue->roots, 0, MAXRANK * sizeof( fibonacci_node* ) );
    queue->largest_rank = 0;
    queue->size = 0;
}

key_type pq_get_key( fibonacci_heap *queue, fibonacci_node *node )
{
    return node->key;
}

item_type* pq_get_item( fibonacci_heap *queue, fibonacci_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( fibonacci_heap *queue )
{
    return queue->size;
}

fibonacci_node* pq_insert( fibonacci_heap *queue, item_type item, key_type key )
{
    fibonacci_node* wrapper = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->next_sibling = wrapper;
    wrapper->prev_sibling = wrapper;
    queue->size++;

    queue->minimum = append_lists( queue, queue->minimum, wrapper );

    return wrapper;
}

fibonacci_node* pq_find_min( fibonacci_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->minimum;
}

key_type pq_delete_min( fibonacci_heap *queue )
{
    fibonacci_node *node = queue->minimum;
    key_type key = node->key;
    fibonacci_node *child = node->first_child;

    // remove from sibling list
    node->next_sibling->prev_sibling = node->prev_sibling;
    node->prev_sibling->next_sibling = node->next_sibling;

    // find new temporary minimum
    if ( node->next_sibling != node )
        queue->minimum = node->next_sibling;
    else
        queue->minimum = child;

    pq_free_node( queue->map, 0, node );
    queue->size--;

    merge_and_fix_roots( queue, queue->minimum, child );

    return key;
}

key_type pq_delete( fibonacci_heap *queue, fibonacci_node *node )
{
    if( node == queue->minimum )
        return pq_delete_min( queue );

    key_type key = node->key;
    fibonacci_node *child = node->first_child;

    // remove from sibling list
    node->next_sibling->prev_sibling = node->prev_sibling;
    node->prev_sibling->next_sibling = node->next_sibling;

    if ( node->parent != NULL )
    {
        node->parent->rank--;
        // if not a root, see if we need to update parent's first child
        if ( node->parent->first_child == node )
        {
            if ( node->parent->rank == 0 )
                node->parent->first_child = NULL;
            else
                node->parent->first_child = node->next_sibling;
        }
        if ( node->parent->marked == FALSE )
            node->parent->marked = TRUE;
        else
            cut_from_parent( queue, node->parent );
    }

    pq_free_node( queue->map, 0, node );
    queue->size--;

    append_lists( queue, queue->minimum, child );

    return key;
}

void pq_decrease_key( fibonacci_heap *queue, fibonacci_node *node,
    key_type new_key )
{
    node->key = new_key;
    cut_from_parent( queue, node );
}

bool pq_empty( fibonacci_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Merges two node lists into one to update the root system of the queue.
 * Iteratively links the roots such that no two roots of the same rank
 * remain.  Assumes that the roots array is empty to begin with, and clears out
 * this array before completing execution.  Breaks list of roots to simplify
 * insertion and linking, and rebuilds the circular list at the end.
 *
 * @param queue Queue to which the two lists belong
 * @param a     First node list
 * @param b     Second node list
 */
static void merge_and_fix_roots( fibonacci_heap *queue, fibonacci_node *a,
    fibonacci_node *b )
{
    fibonacci_node *start = append_lists( queue, a, b );
    fibonacci_node *current, *next;
    int32_t i, rank;

    if ( start == NULL )
        return;

    // break the circular list
    start->prev_sibling->next_sibling = NULL;
    start->prev_sibling = NULL;
    // insert an initial node
    queue->roots[start->rank] = start;
    queue->largest_rank = start->rank;
    start->parent = NULL;
    current = start->next_sibling;

    // insert the rest of the nodes
    while( current != NULL )
    {
        // extract from the list
        next = current->next_sibling;
        if( next != NULL )
            next->prev_sibling = NULL;
        current->next_sibling = NULL;
        current->parent = NULL;

        // insert into the registry
        while ( !attempt_insert( queue, current ) )
        {
            rank = current->rank;
            current = link( queue, current, queue->roots[rank] );
            queue->roots[rank] = NULL;
        }
        current = next;
    }

    // pick the largest tree out of the registry to start reforming the list
    start = queue->roots[queue->largest_rank];
    queue->roots[queue->largest_rank] = NULL;
    queue->minimum = start;

    current = start;
    // pull the rest out and clear the registry for later use
    for ( i = queue->largest_rank - 1; i >= 0; i-- )
    {
        if( queue->roots[i] != NULL )
        {
            if( queue->roots[i]->key < queue->minimum->key )
                queue->minimum = queue->roots[i];
            current->prev_sibling = queue->roots[i];
            queue->roots[i]->next_sibling = current;
            current = queue->roots[i];
            queue->roots[i] = NULL;
        }
    }
    current->prev_sibling = start;
    start->next_sibling = current;
    queue->largest_rank = 0;
}

/**
 * Links two trees, making the item with lesser key the parent, breaking
 * ties arbitrarily.
 *
 * @param queue Queue to which roots belong
 * @param a     First root
 * @param b     Second root
 * @return      The resulting merged tree
 */
static fibonacci_node* link( fibonacci_heap *queue, fibonacci_node *a,
    fibonacci_node *b )
{
    fibonacci_node *parent, *child;
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    // roots are automatically unmarked
    child->marked = FALSE;
    child->parent = parent;
    child->next_sibling = child;
    child->prev_sibling = child;
    parent->first_child = append_lists( queue, parent->first_child, child );
    parent->rank++;

    return parent;
}

/**
 * Recurses up the tree to make a series of cascading cuts.  Cuts each
 * node that has lost two children from its parent.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to cut
 */
static void cut_from_parent( fibonacci_heap *queue, fibonacci_node *node )
{
    fibonacci_node *next, *prev;
    if ( node->parent != NULL ) {
        next = node->next_sibling;
        prev = node->prev_sibling;

        next->prev_sibling = node->prev_sibling;
        prev->next_sibling = node->next_sibling;

        node->next_sibling = node;
        node->prev_sibling = node;

        node->parent->rank--;
        if ( node->parent->first_child == node )
        {
            if ( node->parent->rank == 0 )
                node->parent->first_child = NULL;
            else
                node->parent->first_child = next;
        }
        if ( node->parent->marked == FALSE )
            node->parent->marked = TRUE;
        else
            cut_from_parent( queue, node->parent );

        queue->minimum = append_lists( queue, node, queue->minimum );
        node->parent = NULL;
    }
}

/**
 * Appends two linked lists such that the head of the second comes directly
 * after the head from the first.  Returns a pointer to the list head of lesser
 * key.  This provides the convenience of automatically selecting the minimum of
 * two heaps which are being melded, provided that their respetive minimums are
 * passed in as the heads.
 *
 * @param queue Queue to which lists belong
 * @param a     First head
 * @param b     Second head
 * @return      Pointer to merged list, starting with head of lesser key
 */
static fibonacci_node* append_lists( fibonacci_heap *queue, fibonacci_node *a,
    fibonacci_node *b )
{
    fibonacci_node *list, *a_prev, *b_prev;

    if ( a == NULL )
        list = b;
    else if ( ( b == NULL ) || ( a == b ) )
        list = a;
    else
    {
        a_prev = a->prev_sibling;
        b_prev = b->prev_sibling;

        a_prev->next_sibling = b;
        b_prev->next_sibling = a;

        a->prev_sibling = b_prev;
        b->prev_sibling = a_prev;

        list = ( a->key <= b->key ) ? a : b;
    }

    return list;
}

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param queue Queue to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
static bool attempt_insert( fibonacci_heap *queue, fibonacci_node *node )
{
    uint32_t rank = node->rank;
    fibonacci_node *occupant = queue->roots[rank];
    if ( ( occupant != NULL ) && ( occupant != node ) )
        return FALSE;

    queue->roots[rank] = node;
    if ( rank > queue->largest_rank )
        queue->largest_rank = rank;

    return TRUE;
}
