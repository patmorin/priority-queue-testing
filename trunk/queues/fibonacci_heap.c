#include "fibonacci_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
static mem_map *map;

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void merge_roots( fibonacci_heap *heap, fibonacci_node *a,
    fibonacci_node *b );
static fibonacci_node* link( fibonacci_heap *heap, fibonacci_node *a,
    fibonacci_node *b );
static void cut_from_parent( fibonacci_heap *heap, fibonacci_node *node );
static fibonacci_node* append_lists( fibonacci_heap *heap, fibonacci_node *a,
    fibonacci_node *b );
static bool attempt_insert( fibonacci_heap *heap, fibonacci_node *node );
static void set_min( fibonacci_heap *heap );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

fibonacci_heap* pq_create( uint32_t capacity )
{
    map = mm_create( capacity );
    fibonacci_heap *heap = (fibonacci_heap*) calloc( 1,
        sizeof( fibonacci_heap ) );
    return heap;
}

void pq_destroy( fibonacci_heap *heap ){
    pq_clear( heap );
    free( heap );
    mm_destroy( map );
}

void pq_clear( fibonacci_heap *heap )
{
    mm_clear( map );
    heap->minimum = NULL;
    memset( heap->roots, 0, MAXRANK * sizeof( fibonacci_node* ) );
    heap->largest_rank = 0;
    heap->size = 0;
}

key_type pq_get_key( fibonacci_heap *heap, fibonacci_node *node )
{
    return node->key;
}

item_type* pq_get_item( fibonacci_heap *heap, fibonacci_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( fibonacci_heap *heap )
{
    return heap->size;
}

fibonacci_node* pq_insert( fibonacci_heap *heap, item_type item, key_type key )
{
    fibonacci_node* wrapper = pq_alloc_node( map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->next_sibling = wrapper;
    wrapper->prev_sibling = wrapper;
    heap->size++;

    merge_roots( heap, heap->minimum, wrapper );

    return wrapper;
}

fibonacci_node* pq_find_min( fibonacci_heap *heap )
{
    if ( pq_empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type pq_delete_min( fibonacci_heap *heap )
{
    return pq_delete( heap, heap->minimum );
}

key_type pq_delete( fibonacci_heap *heap, fibonacci_node *node )
{
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
            cut_from_parent( heap, node->parent );
    }
    else if ( node == heap-> minimum )
    {
        // if node was minimum, find new temporary minimum
        if ( node->next_sibling != node )
            heap->minimum = node->next_sibling;
        else
            heap->minimum = child;
    }

    pq_free_node( map, node );
    heap->size--;

    merge_roots( heap, heap->minimum, child );
    
    return key;
}

void pq_decrease_key( fibonacci_heap *heap, fibonacci_node *node,
    key_type new_key )
{
    node->key = new_key;
    cut_from_parent( heap, node );
}

bool pq_empty( fibonacci_heap *heap )
{
    return ( heap->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Merges two node lists into one to update the root system of the heap.
 * Iteratively links the roots such that no two roots of the same rank
 * remain.
 *
 * @param heap  Heap to which the two lists belong
 * @param a     First node list
 * @param b     Second node list
 */
static void merge_roots( fibonacci_heap *heap, fibonacci_node *a,
    fibonacci_node *b )
{
    fibonacci_node *start = append_lists( heap, a, b );
    fibonacci_node *current, *linked;
    uint32_t i, rank;

    // clear array to insert into for rank comparisons
    for ( i = 0; i <= heap->largest_rank; i++ )
        heap->roots[i] = NULL;
    heap->largest_rank = 0;

    if ( start == NULL )
        return;

    // insert an initial node
    heap->roots[start->rank] = start;
    if ( start->rank > heap->largest_rank )
        heap->largest_rank = start->rank;
    start->parent = NULL;
    current = start->next_sibling;
    // insert the rest of the nodes
    while( current != start )
    {
        current->parent = NULL;
        while ( !attempt_insert( heap, current ) )
        {
            rank = current->rank;
            linked = link( heap, current, heap->roots[rank] );
            // when two trees get linked, we're not sure which one is
            // the new root, so we have to check everything again
            start = linked;
            current = linked;
            heap->roots[rank] = NULL;
        }
        current = current->next_sibling;
    }

    set_min( heap );
}

/**
 * Links two trees, making the item with lesser key the parent, breaking
 * ties arbitrarily.
 *
 * @param heap  Heap to which roots belong
 * @param a     First root
 * @param b     Second root
 * @return      The resulting merged tree
 */
static fibonacci_node* link( fibonacci_heap *heap, fibonacci_node *a,
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

    child->prev_sibling->next_sibling = child->next_sibling;
    child->next_sibling->prev_sibling = child->prev_sibling;
    child->prev_sibling = child;
    child->next_sibling = child;

    // roots are automatically unmarked
    child->marked = FALSE;
    child->parent = parent;
    parent->first_child = append_lists( heap, parent->first_child, child );
    parent->rank++;

    return parent;
}

/**
 * Recurses up the tree to make a series of cascading cuts.  Cuts each
 * node that has lost two children from its parent.
 *
 * @param heap  Heap to which node belongs
 * @param node  Node to cut
 */
static void cut_from_parent( fibonacci_heap *heap, fibonacci_node *node )
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
            cut_from_parent( heap, node->parent );
            
        merge_roots( heap, node, heap->minimum );
    }
}

/**
 * Appends two linked lists such that the head of the second comes
 * directly after the head from the first.
 *
 * @param heap  Heap to which lists belong
 * @param a     First head
 * @param b     Second head
 * @return      Final, merged list
 */
static fibonacci_node* append_lists( fibonacci_heap *heap, fibonacci_node *a,
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

        list = a;
    }

    return list;
}

/**
 * Attempt to insert a tree in the rank-indexed array.  Inserts if the
 * correct spot is empty, reports failure if it is occupied.
 *
 * @param heap  Heap to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
static bool attempt_insert( fibonacci_heap *heap, fibonacci_node *node )
{
    uint32_t rank = node->rank;
    fibonacci_node *occupant = heap->roots[rank];
    if ( ( occupant != NULL ) && ( occupant != node ) )
        return FALSE;
        
    heap->roots[rank] = node;
    if ( rank > heap->largest_rank )
        heap->largest_rank = rank;

    return TRUE;
}

/**
 * Scans through the roots array to find the tree with the minimum-value root.
 *
 * @param heap  Heap to fix
 */
static void set_min( fibonacci_heap *heap )
{
    uint32_t i;
    heap->minimum = NULL;
    for ( i = 0; i <= heap->largest_rank; i++ )
    {
        if ( heap->roots[i] == NULL )
            continue;
            
        if ( ( heap->minimum == NULL ) || ( heap->roots[i]->key <
                heap->minimum->key ) )
            heap->minimum = heap->roots[i];
    }
}
