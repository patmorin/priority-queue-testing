#include "pairing_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static pairing_node* merge( pairing_heap *queue, pairing_node *a,
    pairing_node *b );
static pairing_node* collapse( pairing_heap *queue, pairing_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

pairing_heap* pq_create( mem_map *map )
{
    pairing_heap *queue = calloc( 1, sizeof( pairing_heap ) );
    queue->map = map;

    return queue;
}

void pq_destroy( pairing_heap *queue )
{
    pq_clear( queue );
    free( queue );
}

void pq_clear( pairing_heap *queue )
{
    mm_clear( queue->map );
    queue->root = NULL;
    queue->size = 0;
}

key_type pq_get_key( pairing_heap *queue, pairing_node *node )
{
    return node->key;
}

item_type* pq_get_item( pairing_heap *queue, pairing_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( pairing_heap *queue )
{
    return queue->size;
}

pairing_node* pq_insert( pairing_heap *queue, item_type item, key_type key )
{
    pairing_node *wrapper = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    queue->size++;

    queue->root = merge( queue, queue->root, wrapper );

    return wrapper;
}

pairing_node* pq_find_min( pairing_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->root;
}

key_type pq_delete_min( pairing_heap *queue )
{
    return pq_delete( queue, queue->root );
}

key_type pq_delete( pairing_heap *queue, pairing_node *node )
{
    key_type key = node->key;

    if ( node == queue->root )
        queue->root = collapse( queue, node->child );
    else
    {
        if ( node->prev->child == node )
            node->prev->child = node->next;
        else
            node->prev->next = node->next;

        if ( node->next != NULL )
            node->next->prev = node->prev;

        queue->root = merge( queue, queue->root, collapse( queue, node->child ) );
    }

    pq_free_node( queue->map, 0, node );
    queue->size--;

    return key;
}

void pq_decrease_key( pairing_heap *queue, pairing_node *node, key_type new_key )
{
    node->key = new_key;
    if ( node == queue->root )
        return;

    if ( node->prev->child == node )
        node->prev->child = node->next;
    else
        node->prev->next = node->next;

    if ( node->next != NULL )
        node->next->prev = node->prev;

    queue->root = merge( queue, queue->root, node );
}

bool pq_empty( pairing_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Merges two nodes together, making the item of greater key the child
 * of the other.
 *
 * @param queue Queue in which to operate
 * @param a     First node
 * @param b     Second node
 * @return      Resulting tree root
 */
static pairing_node* merge( pairing_heap *queue, pairing_node *a,
    pairing_node *b )
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

/**
 * Performs an iterative pairwise merging of a list of nodes until a
 * single tree remains.  Implements the two-pass method without using
 * explicit recursion (to prevent stack overflow with large lists).
 * Performs the first pass in place while maintaining only the minimal list
 * structure needed to iterate back through during the second pass.
 *
 * @param queue Queue in which to operate
 * @param node  Head of the list to collapse
 * @return      Root of the collapsed tree
 */
static pairing_node* collapse( pairing_heap *queue, pairing_node *node )
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
            result = merge( queue, a, b );
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
        result = merge( queue, result, tail );
        tail = next;
    }

    return result;
}
