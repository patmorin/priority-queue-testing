#include "rank_pairing_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void merge_roots( rank_pairing_heap *queue, rank_pairing_node *a,
    rank_pairing_node *b );
static rank_pairing_node* merge_lists( rank_pairing_heap *queue,
    rank_pairing_node *a, rank_pairing_node *b );
static rank_pairing_node* pick_min( rank_pairing_heap *queue,
    rank_pairing_node *a, rank_pairing_node *b );
static rank_pairing_node* join( rank_pairing_heap *queue, rank_pairing_node *a,
    rank_pairing_node *b );
static void fix_roots( rank_pairing_heap *queue );
static bool attempt_insert( rank_pairing_heap *queue, rank_pairing_node *node );
static void fix_min( rank_pairing_heap *queue );
static void propagate_ranks_t1( rank_pairing_heap *queue,
    rank_pairing_node *node );
static void propagate_ranks_t2( rank_pairing_heap *queue,
    rank_pairing_node *node );
static rank_pairing_node* sever_spine( rank_pairing_heap *queue,
    rank_pairing_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

rank_pairing_heap* pq_create( mem_map *map )
{
    rank_pairing_heap *queue = calloc( 1, sizeof( rank_pairing_heap ) );
    queue->map = map;

    return queue;
}

void pq_destroy( rank_pairing_heap *queue )
{
    pq_clear( queue );
    free( queue );
}

void pq_clear( rank_pairing_heap *queue )
{
    mm_clear( queue->map );
    queue->minimum = NULL;
    memset( queue->roots, 0, MAXRANK * sizeof( rank_pairing_node* ) );
    queue->largest_rank = 0;
    queue->size = 0;
}

key_type pq_get_key( rank_pairing_heap *queue, rank_pairing_node *node )
{
    return node->key;
}

item_type* pq_get_item( rank_pairing_heap *queue, rank_pairing_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( rank_pairing_heap *queue )
{
    return queue->size;
}

rank_pairing_node* pq_insert( rank_pairing_heap *queue, item_type item,
    key_type key )
{
    rank_pairing_node *wrapper = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->right = wrapper;
    queue->size++;
    merge_roots( queue, queue->minimum, wrapper );

    if ( ( queue->minimum == NULL ) || ( key < queue->minimum->key ) )
        queue->minimum = wrapper;

    return wrapper;
}

rank_pairing_node* pq_find_min( rank_pairing_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->minimum;
}

key_type pq_delete_min( rank_pairing_heap *queue )
{
    return pq_delete( queue, queue->minimum );
}

key_type pq_delete( rank_pairing_heap *queue, rank_pairing_node *node )
{
    rank_pairing_node *old_min, *left_list, *right_list, *full_list, *current;
    key_type key = node->key;

    if ( node->parent != NULL )
    {
        if ( node->parent->right == node )
            node->parent->right = NULL;
        else
            node->parent->left = NULL;
    }
    else
    {
        current = node;
        while ( current->right != node )
            current = current->right;
        current->right = node->right;
    }

    left_list = ( node->left != NULL ) ? sever_spine( queue, node->left ) : NULL;
    right_list = ( ( node->parent != NULL ) && ( node->right != NULL ) ) ?
        sever_spine( queue, node->right ) : NULL;
    merge_lists( queue, left_list, right_list );
    full_list = pick_min( queue, left_list, right_list );

    if ( queue->minimum == node )
        queue->minimum = ( node->right == node ) ? full_list : node->right;

    // in order to guarantee linking complies with analysis we save the
    // original minimum so that we perform a one-pass link on the new
    // trees before we do general multi-pass linking
    old_min = queue->minimum;
    merge_roots( queue, queue->minimum, full_list );
    queue->minimum = old_min;
    fix_roots( queue );

    pq_free_node( queue->map, 0, node );
    queue->size--;

    return key;
}

void pq_decrease_key( rank_pairing_heap *queue, rank_pairing_node *node,
    key_type new_key )
{
    node->key = new_key;
    if ( node->parent != NULL )
    {
        if ( node->parent->right == node )
            node->parent->right = node->right;
        else
            node->parent->left = node->right;
        if ( node->right != NULL )
        {
            node->right->parent = node->parent;
            node->right = NULL;
        }

#ifdef USE_TYPE_1
        propagate_ranks_t1( queue, node );
#else
        propagate_ranks_t2( queue, node );
#endif
        node->parent = NULL;
        node->right = node;
        merge_roots( queue, queue->minimum, node );
    }
    else
    {
        if ( node->key < queue->minimum->key )
            queue->minimum = node;
    }
}

bool pq_empty( rank_pairing_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Merges two node lists into one and finds the minimum.  Expects node lists to
 * be passed by a pointer to the minimum in each list.
 *
 * @param queue Queue the two lists belong to
 * @param a     First node list
 * @param b     Second node list
 */
static void merge_roots( rank_pairing_heap *queue, rank_pairing_node *a,
    rank_pairing_node *b )
{
    merge_lists( queue, a, b );
    queue->minimum = pick_min( queue, a, b );
}

/**
 * Merges two node lists into one and returns a pointer into the new list.
 *
 * @param queue Queue to which both nodes belong
 * @param a     First node list
 * @param b     Second node list
 * @return      An arbitrary node in the list
 */
static rank_pairing_node* merge_lists( rank_pairing_heap *queue,
    rank_pairing_node *a, rank_pairing_node *b )
{
    rank_pairing_node *temp, *list;
    if ( a == NULL )
        list = b;
    else if ( b == NULL )
        list = a;
    else if ( a == b )
        list = a;
    else
    {
        temp = a->right;
        a->right = b->right;
        b->right = temp;
        list = a;
    }

    return list;
}

/**
 * Picks and returns the minimum between two nodes.
 *
 * @param queue Queue to which both nodes belong
 * @param a     First node
 * @param b     Second node
 * @return      Minimum of the two nodes
 */
static rank_pairing_node* pick_min( rank_pairing_heap *queue,
    rank_pairing_node *a, rank_pairing_node *b )
{
    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;
    else
    {
        if ( b->key < a->key )
            return b;
        else
            return a;
    }
}

/**
 * Links two trees, making the larger-key tree the child of the lesser.
 *
 * @param queue Queue to which both nodes belong
 * @param a     First node
 * @param b     Second node
 * @return      Returns the resulting tree
 */
static rank_pairing_node* join( rank_pairing_heap *queue, rank_pairing_node *a,
    rank_pairing_node *b )
{
    rank_pairing_node *parent, *child;

    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    child->right = parent->left;
    if ( child->right != NULL )
        child->right->parent = child;
    parent->left = child;
    child->parent = parent;
    parent->rank++;

    return parent;
}

/**
 * Performs a one-pass linking run through the list of roots.  Links
 * trees of equal ranks.
 *
 * @param queue Queue whose roots to fix
 */
static void fix_roots( rank_pairing_heap *queue )
{
    rank_pairing_node *output_head = NULL;
    rank_pairing_node *output_tail = NULL;
    rank_pairing_node *current, *next, *joined;
    uint32_t i, rank;

    if ( queue->minimum == NULL )
        return;

    queue->largest_rank = 0;

    current = queue->minimum->right;
    queue->minimum->right = NULL;
    while ( current != NULL ) {
        next = current->right;
        if ( !attempt_insert( queue, current ) )
        {
            rank = current->rank;
            // keep a running list of joined trees
            joined = join( queue, current, queue->roots[rank] );
            if ( output_head == NULL )
                output_head = joined;
            else
                output_tail->right = joined;
            output_tail = joined;
            queue->roots[rank] = NULL;
        }
        current = next;
    }

    // move the untouched trees to the list and repair pointers
    for ( i = 0; i <= queue->largest_rank; i++ )
    {
        if ( queue->roots[i] != NULL )
        {
            if ( output_head == NULL )
                output_head = queue->roots[i];
            else
                output_tail->right = queue->roots[i];
            output_tail = queue->roots[i];
            queue->roots[i] = NULL;
        }
    }

    output_tail->right = output_head;

    queue->minimum = output_head;
    fix_min( queue );
}

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param queue Queue to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
static bool attempt_insert( rank_pairing_heap *queue, rank_pairing_node *node )
{
    uint32_t rank = node->rank;
    if ( ( queue->roots[rank] != NULL ) && ( queue->roots[rank] != node ) )
        return FALSE;
    queue->roots[rank] = node;

    if ( rank > queue->largest_rank )
        queue->largest_rank = rank;

    return TRUE;
}

/**
 * Scans through the roots list starting from the current, potentially
 * inaccurate, minimum to find the tree with the minimum-value
 * root.
 *
 * @param queue Queue to fix
 */
static void fix_min( rank_pairing_heap *queue )
{
    if ( queue->minimum == NULL )
        return;
    rank_pairing_node *start = queue->minimum;
    rank_pairing_node *current = queue->minimum->right;
    while ( current != start )
    {
        if ( current->key < queue->minimum->key )
            queue->minimum = current;
        current = current->right;
    }
}

/**
 * Propagates rank corrections upward from the initial node using the type-1
 * rank rule.
 *
 * @param queue Queue to update
 * @param node  Initial node to begin updating from.
 */
static void propagate_ranks_t1( rank_pairing_heap *queue,
    rank_pairing_node *node )
{
    int32_t k = 0;
    int32_t u = -1;
    int32_t v = -1;

    if ( node == NULL )
        return;

    if ( ( node->parent != NULL ) && ( node->left != NULL ) )
        k = node->left->rank + 1;
    else
    {
        if ( node->left != NULL )
            u = node->left->rank;
        if ( node->right != NULL )
            v = node->right->rank;

        if( u > v )
            k = u;
        else if( v > u )
            k = v;
        else
            k = u + 1;
    }

    if ( node->rank > k )
    {
        node->rank = k;
        propagate_ranks_t1( queue, node->parent );
    }
}

/**
 * Propagates rank corrections upward from the initial node using the type-2
 * rank rule.
 *
 * @param queue Queue to update
 * @param node  Initial node to begin updating from.
 */
static void propagate_ranks_t2( rank_pairing_heap *queue,
    rank_pairing_node *node )
{
    int32_t k = 0;
    int32_t u = -1;
    int32_t v = -1;

    if ( node == NULL )
        return;

    if ( ( node->parent != NULL ) && ( node->left != NULL ) )
        k = node->left->rank + 1;
    else
    {
        if ( node->left != NULL )
            u = node->left->rank;
        if ( node->right != NULL )
            v = node->right->rank;

        if( u > v + 1 )
            k = u;
        else if( v > u + 1 )
            k = v;
        else
            k = ( u >= v ) ? u + 1 : v + 1;
    }

    if ( node->rank > k )
    {
        node->rank = k;
        propagate_ranks_t2( queue, node->parent );
    }
}

/**
 * Converts the given node and its right spine to a singly-linked circular list
 * of roots.
 *
 * @param queue Queue in which the node resides
 * @param node  Root of the spine
 */
static rank_pairing_node* sever_spine( rank_pairing_heap *queue,
    rank_pairing_node *node )
{
    rank_pairing_node *current = node;
    while ( current->right != NULL )
    {
        current->parent = NULL;
        current = current->right;
    }
    current->parent = NULL;
    current->right = node;

    return node;
}
