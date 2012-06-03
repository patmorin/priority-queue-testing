#include "violation_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
static mem_map *map;

violation_heap* pq_create( uint32_t capacity )
{
    map = mm_create( capacity );
    violation_heap *heap = (violation_heap*) calloc( 1, sizeof( violation_heap ) );
    return heap;
}

void pq_destroy( violation_heap *heap )
{
    pq_clear( heap );
    free( heap );
    mm_destroy( map );
}

void pq_clear( violation_heap *heap )
{
    mm_clear( map );
    heap->minimum = NULL;
    memset( heap->roots, 0, 2 * MAXRANK * sizeof( violation_node* ) );
    heap->largest_rank = 0;
    heap->size = 0;
}

key_type pq_get_key( violation_heap *heap, violation_node *node )
{
    return node->key;
}

item_type* pq_get_item( violation_heap *heap, violation_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( violation_heap *heap )
{
    return heap->size;
}

violation_node* pq_insert( violation_heap *heap, item_type item, key_type key )
{
    violation_node* wrapper = pq_alloc_node( map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->next = wrapper;
    heap->size++;

    merge_into_roots( heap, wrapper );

    if ( ( heap->minimum == NULL ) || ( key < heap->minimum->key ) )
        heap->minimum = wrapper;
    
    return wrapper;
}

violation_node* pq_find_min( violation_heap *heap )
{
    if ( pq_empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type pq_delete_min( violation_heap *heap )
{
    return pq_delete( heap, heap->minimum );
}

key_type pq_delete( violation_heap *heap, violation_node *node )
{
    key_type key = node->key;
    violation_node *prev;

    if ( get_parent( heap, node ) == NULL )
    {
        prev = find_prev_root( heap, node );
        prev->next = node->next;
    }
    else
    {
        if ( node->next != get_parent( heap, node ) )
            node->next->prev = node->prev;
        else
            node->next->child = node->prev;
            
        if ( node->prev != NULL )
            node->prev->next = node->next;
    }

    if ( heap->minimum == node )
    {
        if ( node->next != node )
            heap->minimum = node->next;
        else
            heap->minimum = node->child;
    }

    if ( node->child != NULL )
    {
        strip_list( heap, node->child );
        merge_into_roots( heap, node->child );
    }
    fix_roots( heap );

    pq_free_node( map, node );
    heap->size--;

    return key;
}

void pq_decrease_key( violation_heap *heap, violation_node *node, key_type new_key )
{
    node->key = new_key;
    violation_node *parent, *first_child, *second_child, *replacement;
        
    if ( get_parent( heap, node ) == NULL )
    {
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
        return;
    }
    else
    {
        parent = get_parent( heap, node );
        if ( ( is_active( heap, node ) ) && !( node->key < parent->key ) )
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

            propagate_ranks( heap, replacement );
        }
        else
        {
            if ( node->next->child == node )
                node->next->child = node->prev;
            else
                node->next->prev = node->prev;

            if ( node->prev != NULL )
                node->prev->next = node->next;

            propagate_ranks( heap, node->next );
        }

        // make node a root
        node->next = node;
        node->prev = NULL;
        merge_into_roots( heap, node );
    }
}

bool pq_empty( violation_heap *heap )
{
    return ( heap->size == 0 );
}

void merge_into_roots( violation_heap *heap, violation_node *list )
{
    violation_node *temp;
    if ( heap->minimum == NULL )
        heap->minimum = list;
    else if ( ( list != NULL ) && ( heap->minimum != list ) )
    {
        temp = heap->minimum->next;
        heap->minimum->next = list->next;
        list->next = temp;

        if ( list->key < heap->minimum->key )
            heap->minimum = list;
    }
}

violation_node* triple_join( violation_heap *heap, violation_node *a, violation_node *b, violation_node *c )
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

    return join( heap, parent, child1, child2 );
}

violation_node* join( violation_heap *heap, violation_node *parent, violation_node *child1, violation_node *child2 )
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

void fix_roots( violation_heap *heap )
{
    violation_node *current, *next, *head, *tail;
    int i;
    int32_t rank;

    for ( i = 0; i <= heap->largest_rank; i++ )
    {
        heap->roots[i][0] = NULL;
        heap->roots[i][1] = NULL;
    }
    
    if ( heap->minimum == NULL )
        return;

    head = heap->minimum->next;
    heap->minimum->next = NULL;
    tail = heap->minimum;
    current = head;
    while ( current != NULL )
    {
        next = current->next;
        current->next = NULL;
        if ( !attempt_insert( heap, current ) )
        {
            rank = current->rank;
            tail->next = triple_join( heap, current, heap->roots[rank][0], heap->roots[rank][1] );
            if ( tail == current )
                next = tail->next;
            tail = tail->next;
            heap->roots[rank][0] = NULL;
            heap->roots[rank][1] = NULL;
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i <= heap->largest_rank; i++ )
    {
        if ( heap->roots[i][0] != NULL )
        {
            if ( head == NULL )
                head = heap->roots[i][0];
            else
                tail->next = heap->roots[i][0];
            tail = heap->roots[i][0];
        }
        if ( heap->roots[i][1] != NULL )
        {
            if ( head == NULL )
                head = heap->roots[i][1];
            else
                tail->next = heap->roots[i][1];
            tail = heap->roots[i][1];
        }
    }

    tail->next = head;

    set_min( heap );
}

bool attempt_insert( violation_heap *heap, violation_node *node )
{
    int32_t rank = node->rank;
    if ( ( heap->roots[rank][0] != NULL ) && ( heap->roots[rank][0] != node ) )
    {
        if ( ( heap->roots[rank][1] != NULL ) && ( heap->roots[rank][1] != node ) )
            return FALSE;
        else
            heap->roots[rank][1] = node;
    }
    else
        heap->roots[rank][0] = node;

    if ( rank > heap->largest_rank )
        heap->largest_rank = rank;

    return TRUE;    
}

void set_min( violation_heap *heap )
{
    int i;
    heap->minimum = NULL;
    for ( i = 0; i <= heap->largest_rank; i++ )
    {
        if ( heap->roots[i][0] != NULL )
        {
            if ( heap->minimum == NULL )
                heap->minimum = heap->roots[i][0];
            else if ( heap->roots[i][0]->key < heap->minimum->key )
                heap->minimum = heap->roots[i][0];
        }
        if ( heap->roots[i][1] != NULL )
        {
            if ( heap->minimum == NULL )
                heap->minimum = heap->roots[i][1];
            else if ( heap->roots[i][1]->key < heap->minimum->key )
                heap->minimum = heap->roots[i][1];
        }                    
    }
}

violation_node* find_prev_root( violation_heap *heap, violation_node *node )
{
    violation_node *prev = node->next;
    while ( prev->next != node )
        prev = prev->next;
    
    return prev;
}

void propagate_ranks( violation_heap *heap, violation_node *node )
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
    
    parent = get_parent( heap, node );
    if ( updated && ( parent != NULL ) && ( is_active( heap, parent ) ) )
        propagate_ranks( heap, get_parent( heap, node ) );
}

void strip_list( violation_heap *heap, violation_node *node )
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

bool is_active( violation_heap *heap, violation_node *node )
{
    if ( get_parent( heap, node ) == NULL )
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

violation_node* get_parent( violation_heap *heap, violation_node *node )
{
    if ( node->next->child == node )
        return node->next;
    else if ( ( node->prev == NULL ) && ( node->next->prev == NULL ) )
        return NULL;
    else
        return ( get_parent( heap, node->next ) );
}
