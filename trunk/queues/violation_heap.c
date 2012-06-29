#include "violation_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void merge_into_roots( violation_heap *queue, violation_node *list );
static violation_node* triple_join( violation_heap *queue, violation_node *a,
    violation_node *b, violation_node *c );
static violation_node* join( violation_heap *queue, violation_node *parent,
    violation_node *child1, violation_node *child2 );
static void fix_roots( violation_heap *queue );
static bool attempt_insert( violation_heap *queue, violation_node *node );
static void set_min( violation_heap *queue );
static violation_node* find_prev_root( violation_heap *queue, violation_node *node );
static void propagate_ranks( violation_heap *queue, violation_node *node );
static void strip_list( violation_heap *queue, violation_node *node );
static bool is_active( violation_heap *queue, violation_node *node );
static violation_node* get_parent( violation_heap *queue, violation_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

violation_heap* pq_create( mem_map *map )
{
    violation_heap *queue = calloc( 1, sizeof( violation_heap ) );
    queue->map = map;
    
    return queue;
}

void pq_destroy( violation_heap *queue )
{
    pq_clear( queue );
    free( queue );
}

void pq_clear( violation_heap *queue )
{
    mm_clear( queue->map );
    queue->minimum = NULL;
    memset( queue->roots, 0, 2 * MAXRANK * sizeof( violation_node* ) );
    queue->largest_rank = 0;
    queue->size = 0;
}

key_type pq_get_key( violation_heap *queue, violation_node *node )
{
    return node->key;
}

item_type* pq_get_item( violation_heap *queue, violation_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( violation_heap *queue )
{
    return queue->size;
}

violation_node* pq_insert( violation_heap *queue, item_type item, key_type key )
{
    violation_node* wrapper = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->next = wrapper;
    queue->size++;

    merge_into_roots( queue, wrapper );

    if ( ( queue->minimum == NULL ) || ( key < queue->minimum->key ) )
        queue->minimum = wrapper;
    
    return wrapper;
}

violation_node* pq_find_min( violation_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->minimum;
}

key_type pq_delete_min( violation_heap *queue )
{
    return pq_delete( queue, queue->minimum );
}

key_type pq_delete( violation_heap *queue, violation_node *node )
{
    key_type key = node->key;
    violation_node *prev;

    if ( get_parent( queue, node ) == NULL )
    {
        prev = find_prev_root( queue, node );
        prev->next = node->next;
    }
    else
    {
        if ( node->next != get_parent( queue, node ) )
            node->next->prev = node->prev;
        else
            node->next->child = node->prev;
            
        if ( node->prev != NULL )
            node->prev->next = node->next;
    }

    if ( queue->minimum == node )
    {
        if ( node->next != node )
            queue->minimum = node->next;
        else
            queue->minimum = node->child;
    }

    if ( node->child != NULL )
    {
        strip_list( queue, node->child );
        merge_into_roots( queue, node->child );
    }
    fix_roots( queue );

    pq_free_node( queue->map, 0, node );
    queue->size--;

    return key;
}

void pq_decrease_key( violation_heap *queue, violation_node *node,
    key_type new_key )
{
    node->key = new_key;
    violation_node *parent, *first_child, *second_child, *replacement;
        
    if ( get_parent( queue, node ) == NULL )
    {
        if ( node->key < queue->minimum->key )
            queue->minimum = node;
        return;
    }
    else
    {
        parent = get_parent( queue, node );
        if ( ( is_active( queue, node ) ) && !( node->key < parent->key ) )
            return;
        first_child = node->child;
        if ( first_child != NULL )
        {
            // determine active child of greater rank
            second_child = first_child->prev;
            if ( second_child == NULL )
            {
                node->child = NULL;
                replacement = first_child;
            }
            else
            {
                if ( second_child->rank > first_child->rank )
                {
                    if ( second_child->prev != NULL )
                        second_child->prev->next = first_child;
                    first_child->prev = second_child->prev;
                    replacement = second_child;
                }
                else
                {
                    node->child = second_child;
                    second_child->next = node;
                    replacement = first_child;
                }
            }

            // swap child into place of this node
            replacement->next = node->next;
            replacement->prev = node->prev;
            if ( replacement->next != NULL )
            {
                if ( replacement->next->child == node )
                    replacement->next->child = replacement;
                else
                    replacement->next->prev = replacement;
            }
            if ( replacement->prev != NULL )
                replacement->prev->next = replacement;

            propagate_ranks( queue, replacement );
        }
        else
        {
            if ( node->next->child == node )
                node->next->child = node->prev;
            else
                node->next->prev = node->prev;

            if ( node->prev != NULL )
                node->prev->next = node->next;

            propagate_ranks( queue, node->next );
        }

        // make node a root
        node->next = node;
        node->prev = NULL;
        merge_into_roots( queue, node );
    }
}

bool pq_empty( violation_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Merges a new node list into the root list.
 *
 * @param queue Queue to merge list into
 * @param list  List to merge
 */
void merge_into_roots( violation_heap *queue, violation_node *list )
{
    violation_node *temp;
    if ( queue->minimum == NULL )
        queue->minimum = list;
    else if ( ( list != NULL ) && ( queue->minimum != list ) )
    {
        temp = queue->minimum->next;
        queue->minimum->next = list->next;
        list->next = temp;

        if ( list->key < queue->minimum->key )
            queue->minimum = list;
    }
}

/**
 * Links three trees, making the smallest-keyed item the parent.
 *
 * @param queue Queue to which nodes belong
 * @param a     First node
 * @param b     Second node
 * @param c     Third node
 * @return      Returns the resulting tree
 */
static violation_node* triple_join( violation_heap *queue, violation_node *a,
    violation_node *b, violation_node *c )
{
    violation_node *parent, *child1, *child2;
    
    if ( a->key < b->key )
    {
        if ( a->key < c->key )
        {
            parent = a;
            child1 = ( b->rank >= c->rank ) ? b : c;
            child2 = ( b->rank >= c->rank ) ? c : b;
        }
        else
        {
            parent = c;
            child1 = ( a->rank >= b->rank ) ? a : b;
            child2 = ( a->rank >= b->rank ) ? b : a;
        }
    }
    else
    {
        if ( b->key < c->key )
        {
            parent = b;
            child1 = ( a->rank >= c->rank ) ? a : c;
            child2 = ( a->rank >= c->rank ) ? c : a;
        }
        else
        {
            parent = c;
            child1 = ( a->rank >= b->rank ) ? a : b;
            child2 = ( a->rank >= b->rank ) ? b : a;
        }
    }

    return join( queue, parent, child1, child2 );
}

/**
 * Makes two nodes the last two children of a third parent node.
 *
 * @param queue     Queue to which nodes belong
 * @param parent    Parent node
 * @param child1    Child of greater rank
 * @param child2    Child of lesser rank
 * @return          Root of new tree
 */
static violation_node* join( violation_heap *queue, violation_node *parent,
    violation_node *child1, violation_node *child2 )
{
    violation_node *active1, *active2;
    uint32_t rank1, rank2;

    if ( parent->child != NULL )
    {
        active1 = parent->child;
        active2 = parent->child->prev;
        if ( active2 != NULL )
        {
            rank1 = active1->rank;
            rank2 = active2->rank;
            if ( rank1 < rank2 )
            {
                active1->prev = active2->prev;
                if ( active1->prev != NULL )
                    active1->prev->next = active1;
                active2->next = parent;
                active1->next = active2;
                active2->prev = active1;
                parent->child = active2;
            }
        }
    }

    child1->next = parent;
    child1->prev = child2;
    child2->next = child1;
    child2->prev = parent->child;
    
    if ( parent->child != NULL )
        parent->child->next = child2;
    parent->child = child1;

    parent->rank++;

    return parent;
}

/**
 * Iterates through roots and three-way joins trees of the same rank
 * until no three trees remain with the same rank.
 *
 * @param queue Queue whose roots to fix
 */
static void fix_roots( violation_heap *queue )
{
    violation_node *current, *next, *head, *tail;
    int i;
    int32_t rank;

    for ( i = 0; i <= queue->largest_rank; i++ )
    {
        queue->roots[i][0] = NULL;
        queue->roots[i][1] = NULL;
    }
    
    if ( queue->minimum == NULL )
        return;

    head = queue->minimum->next;
    queue->minimum->next = NULL;
    tail = queue->minimum;
    current = head;
    while ( current != NULL )
    {
        next = current->next;
        current->next = NULL;
        if ( !attempt_insert( queue, current ) )
        {
            rank = current->rank;
            tail->next = triple_join( queue, current, queue->roots[rank][0],
                queue->roots[rank][1] );
            if ( tail == current )
                next = tail->next;
            tail = tail->next;
            queue->roots[rank][0] = NULL;
            queue->roots[rank][1] = NULL;
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i <= queue->largest_rank; i++ )
    {
        if ( queue->roots[i][0] != NULL )
        {
            if ( head == NULL )
                head = queue->roots[i][0];
            else
                tail->next = queue->roots[i][0];
            tail = queue->roots[i][0];
        }
        if ( queue->roots[i][1] != NULL )
        {
            if ( head == NULL )
                head = queue->roots[i][1];
            else
                tail->next = queue->roots[i][1];
            tail = queue->roots[i][1];
        }
    }

    tail->next = head;

    set_min( queue );
}

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param queue Queue to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
static bool attempt_insert( violation_heap *queue, violation_node *node )
{
    int32_t rank = node->rank;
    if ( ( queue->roots[rank][0] != NULL ) && ( queue->roots[rank][0] != node ) )
    {
        if ( ( queue->roots[rank][1] != NULL ) && ( queue->roots[rank][1] !=
                node ) )
            return FALSE;
        else
            queue->roots[rank][1] = node;
    }
    else
        queue->roots[rank][0] = node;

    if ( rank > queue->largest_rank )
        queue->largest_rank = rank;

    return TRUE;    
}

/**
 * Scans through the roots array to find the tree with the minimum-value
 * root.
 *
 * @param queue Queue to fix
 */
static void set_min( violation_heap *queue )
{
    int i;
    queue->minimum = NULL;
    for ( i = 0; i <= queue->largest_rank; i++ )
    {
        if ( queue->roots[i][0] != NULL )
        {
            if ( queue->minimum == NULL )
                queue->minimum = queue->roots[i][0];
            else if ( queue->roots[i][0]->key < queue->minimum->key )
                queue->minimum = queue->roots[i][0];
        }
        if ( queue->roots[i][1] != NULL )
        {
            if ( queue->minimum == NULL )
                queue->minimum = queue->roots[i][1];
            else if ( queue->roots[i][1]->key < queue->minimum->key )
                queue->minimum = queue->roots[i][1];
        }                    
    }
}

/**
 * Loops around a singly-linked list of roots to find the root prior to
 * the specified node.
 *
 * @param queue The queue in which the node resides
 * @param node  The specified node to start from
 * @return      The node prior to the start
 */
static violation_node* find_prev_root( violation_heap *queue,
    violation_node *node )
{
    violation_node *prev = node->next;
    while ( prev->next != node )
        prev = prev->next;
    
    return prev;
}

/**
 * Propagates rank changes upward from the initial node.
 *
 * @param queue Queue in which node resides
 * @param node  Initial node to begin updating from.
 */
static void propagate_ranks( violation_heap *queue, violation_node *node )
{
    int32_t rank1 = -1;
    int32_t rank2 = -1;
    int32_t new_rank, total;
    bool updated;
    violation_node *parent;

    if ( node->child != NULL )
    {
        rank1 = node->child->rank;
        if ( node->child->prev != NULL )
            rank2 = node->child->prev->rank;
    }

    total = rank1 + rank2;
    if ( total == -2 )
        new_rank = 0;
    else if ( total == -1 )
        new_rank = 1;
    else 
        new_rank = ( ( total / 2 ) + ( total % 2 ) + 1 );
    updated = new_rank < node->rank;
    node->rank = new_rank;
    
    parent = get_parent( queue, node );
    if ( updated && ( parent != NULL ) && ( is_active( queue, parent ) ) )
        propagate_ranks( queue, get_parent( queue, node ) );
}

/**
 * Converts a doubly-linked list into a circular singly-linked list.
 *
 * @param queue Queue in which node resides
 * @param node  Last node in the list
 */
static void strip_list( violation_heap *queue, violation_node *node )
{
    violation_node *current = node;
    violation_node *prev;
    while ( current->prev != NULL )
    {
        prev = current->prev;
        current->prev = NULL;
        current = prev;
    }
    node->next = current;
}

/**
 * Determines whether this node is active, meaning it is one of
 * the last two children of its parent.
 *
 * @param queue Queue in which node resides
 * @param node  Node to query
 * @return      True if active, false if not
 */
static bool is_active( violation_heap *queue, violation_node *node )
{
    if ( get_parent( queue, node ) == NULL )
        return TRUE;
    else
    {
        if ( node->next->child == node )
            return TRUE;
        else
        {
            if ( node->next->next->child == node->next )
                return TRUE ;
            else
                return FALSE;
        }
    }
}

/**
 * Returns the parent of the current node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Parent of the queried node, NULL if root
 */
static violation_node* get_parent( violation_heap *queue, violation_node *node )
{
    if ( node->next->child == node )
        return node->next;
    else if ( ( node->prev == NULL ) && ( node->next->prev == NULL ) )
        return NULL;
    else
        return ( get_parent( queue, node->next ) );
}
