#include "rank_pairing_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
static mem_map *map;

rank_pairing_heap* pq_create( uint32_t capacity )
{
    map = mm_create( capacity );
    rank_pairing_heap *heap = (rank_pairing_heap*) calloc( 1, sizeof( rank_pairing_heap ) );
    return heap;
}

void pq_destroy( rank_pairing_heap *heap )
{
    pq_clear( heap );
    free( heap );
    mm_destroy( map );
}

void pq_clear( rank_pairing_heap *heap )
{
    mm_clear( map );
    heap->minimum = NULL;
    memset( heap->roots, 0, MAXRANK * sizeof( rank_pairing_node* ) );
    heap->largest_rank = 0;
    heap->size = 0;
}

key_type pq_get_key( rank_pairing_heap *heap, rank_pairing_node *node )
{
    return node->key;
}

item_type* pq_get_item( rank_pairing_heap *heap, rank_pairing_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( rank_pairing_heap *heap )
{
    return heap->size;
}

rank_pairing_node* pq_insert( rank_pairing_heap *heap, item_type item, uint32_t key )
{
    rank_pairing_node *wrapper = pq_alloc_node( map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->right = wrapper;
    heap->size++;
    merge_roots( heap, heap->minimum, wrapper );

    if ( ( heap->minimum == NULL ) || ( key < heap->minimum->key ) )
        heap->minimum = wrapper;
    
    return wrapper;
}

rank_pairing_node* pq_find_min( rank_pairing_heap *heap )
{
    if ( pq_empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type pq_delete_min( rank_pairing_heap *heap )
{
    return pq_delete( heap, heap->minimum );
}

key_type pq_delete( rank_pairing_heap *heap, rank_pairing_node *node )
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

    left_list = ( node->left != NULL ) ? sever_spine( heap, node->left ) : NULL;
    right_list = ( ( node->parent != NULL ) && ( node->right != NULL ) ) ?
        sever_spine( heap, node->right ) : NULL;
    merge_lists( heap, left_list, right_list );
    full_list = pick_min( heap, left_list, right_list );

    if ( heap->minimum == node )
        heap->minimum = ( node->right == node ) ? full_list : node->right;

    // in order to guarantee linking complies with analysis we save the
    // original minimum so that we perform a one-pass link on the new
    // trees before we do general multi-pass linking
    old_min = heap->minimum;
    merge_roots( heap, heap->minimum, full_list );
    heap->minimum = old_min;
    fix_roots( heap );                

    pq_free_node( map, node );
    heap->size--;

    return key;
}

void pq_decrease_key( rank_pairing_heap *heap, rank_pairing_node *node, key_type new_key )
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

        propagate_ranks( heap, node );
        node->parent = NULL;
        node->right = node;
        merge_roots( heap, heap->minimum, node );
    }
    else
    {
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
    }
}

bool pq_empty( rank_pairing_heap *heap )
{
    return ( heap->size == 0 );
}

void merge_roots( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b )
{
    merge_lists( heap, a, b );
    heap->minimum = pick_min( heap, a, b );
}

rank_pairing_node* merge_lists( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b )
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

rank_pairing_node* pick_min( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b )
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

rank_pairing_node* join( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b )
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

void fix_roots( rank_pairing_heap *heap )
{
    rank_pairing_node *output_head = NULL;
    rank_pairing_node *output_tail = NULL;
    rank_pairing_node *current, *next, *joined;
    uint32_t i, rank;

    if ( heap->minimum == NULL )
        return;

    heap->largest_rank = 0;

    current = heap->minimum->right;
    heap->minimum->right = NULL;
    while ( current != NULL ) {
        next = current->right;
        if ( !attempt_insert( heap, current ) )
        {
            rank = current->rank;
            // keep a running list of joined trees
            joined = join( heap, current, heap->roots[rank] );
            if ( output_head == NULL )
                output_head = joined;
            else
                output_tail->right = joined;
            output_tail = joined;
            heap->roots[rank] = NULL;
        }
        current = next;
    }

    // move the untouched trees to the list and repair pointers
    for ( i = 0; i <= heap->largest_rank; i++ )
    {
        if ( heap->roots[i] != NULL )
        {
            if ( output_head == NULL )
                output_head = heap->roots[i];
            else
                output_tail->right = heap->roots[i];
            output_tail = heap->roots[i];
            heap->roots[i] = NULL;
        }
    }

    output_tail->right = output_head;

    heap->minimum = output_head;
    fix_min( heap );
}

bool attempt_insert( rank_pairing_heap *heap, rank_pairing_node *node )
{
    uint32_t rank = node->rank;
    if ( ( heap->roots[rank] != NULL ) && ( heap->roots[rank] != node ) )
        return FALSE;
    heap->roots[rank] = node;

    if ( rank > heap->largest_rank )
        heap->largest_rank = rank;

    return TRUE;
}

void fix_min( rank_pairing_heap *heap )
{
    if ( heap->minimum == NULL )
        return;
    rank_pairing_node *start = heap->minimum;
    rank_pairing_node *current = heap->minimum->right;
    while ( current != start )
    {
        if ( current->key < heap->minimum->key )
            heap->minimum = current;
        current = current->right;
    }
}

void propagate_ranks( rank_pairing_heap *heap, rank_pairing_node *node )
{
    uint32_t k = 0;
    
    if ( node == NULL )
        return;

    if ( ( node->parent != NULL ) && ( node->left != NULL ) )
        k = node->left->rank + 1;
    else if ( node->left != NULL )
    {
        if ( node->right != NULL )
        {
            if ( node->left->rank == node->right->rank )
                k = node->left->rank + 1;
            else
                k = ( node->left->rank > node->right->rank ) ? node->left->rank : node->right->rank;
        }
        else
            k = node->left->rank;
    }
    else if ( node->right != NULL )
        k = node->right->rank + 1;

    if ( node->rank >= k )
        node->rank = k;

    propagate_ranks( heap, node->parent );
}

rank_pairing_node* sever_spine( rank_pairing_heap *heap, rank_pairing_node *node )
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
