#include "pairing_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
static mem_map *map;

pairing_heap* pq_create( uint32_t capacity )
{
    map = mm_create( capacity );
    pairing_heap *heap = (pairing_heap*) calloc( 1, sizeof( pairing_heap ) );
    return heap;
}

void pq_destroy( pairing_heap *heap )
{
    pq_clear( heap );
    free( heap );
    mm_destroy( map );
}

void pq_clear( pairing_heap *heap )
{
    mm_clear( map );
    heap->root = NULL;
    heap->size = 0;
}

key_type pq_get_key( pairing_heap *heap, pairing_node *node )
{
    return node->key;
}

item_type* pq_get_item( pairing_heap *heap, pairing_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( pairing_heap *heap )
{
    return heap->size;
}

pairing_node* pq_insert( pairing_heap *heap, item_type item, key_type key )
{
    pairing_node *wrapper = pq_alloc_node( map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    heap->size++;

    heap->root = merge( heap, heap->root, wrapper );

    return wrapper;
}

pairing_node* pq_find_min( pairing_heap *heap )
{
    if ( pq_empty( heap ) )
        return NULL;
    return heap->root;
}

key_type pq_delete_min( pairing_heap *heap )
{
    return pq_delete( heap, heap->root );
}

key_type pq_delete( pairing_heap *heap, pairing_node *node )
{
    key_type key = node->key;

    if ( node == heap->root )
        heap->root = collapse( heap, node->child );
    else
    {
        if ( node->prev->child == node )
            node->prev->child = node->next;
        else
            node->prev->next = node->next;

        if ( node->next != NULL )
            node->next->prev = node->prev;

        heap->root = merge( heap, heap->root, collapse( heap, node->child ) );
    }

    pq_free_node( map, node );
    heap->size--;

    return key;
}

void pq_decrease_key( pairing_heap *heap, pairing_node *node, key_type new_key )
{
    node->key = new_key;
    if ( node == heap->root )
        return;

    if ( node->prev->child == node )
        node->prev->child = node->next;
    else
        node->prev->next = node->next;

    if ( node->next != NULL )
        node->next->prev = node->prev;

    heap->root = merge( heap, heap->root, node );
}

bool pq_empty( pairing_heap *heap )
{
    return ( heap->size == 0 );
}

pairing_node* merge( pairing_heap *heap, pairing_node *a, pairing_node *b )
{
    pairing_node *parent, *child;

    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;

    if ( b->key < a->key )
    {
        parent = b;
        child = a;
    }
    else
    {
        parent = a;
        child = b;
    }

    child->next = parent->child;
    if ( parent->child != NULL )
        parent->child->prev = child;
    child->prev = parent;
    parent->child = child;

    parent->next = NULL;
    parent->prev = NULL;

    return parent;
}

pairing_node* collapse( pairing_heap *heap, pairing_node *node )
{
    pairing_node *tail, *a, *b, *next, *result;

    if ( node == NULL )
        return NULL;

    next = node;
    tail = NULL;
    while ( next != NULL )
    {
        a = next;
        b = a->next;
        if ( b != NULL )
        {
            next = b->next;
            result = merge( heap, a, b );
            // tack the result onto the end of the temporary list
            result->prev = tail;
            tail = result;                    
        }
        else
        {
            a->prev = tail;
            tail = a;
            break;
        }
    }
    
    result = NULL;
    while ( tail != NULL )
    {
        // trace back through to merge the list
        next = tail->prev;
        result = merge( heap, result, tail );
        tail = next;
    }

    return result;
}
