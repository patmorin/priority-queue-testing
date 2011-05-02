#include "pairing_heap.h"

pairing_heap* create_heap() {
    pairing_heap *heap = (pairing_heap*) calloc( 1, sizeof( pairing_heap ) );
    heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );
    return heap;
}

void destroy_heap( pairing_heap *heap ) {
    clear_heap( heap );
    free( heap->stats );
    free( heap );
}

void clear_heap( pairing_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

uint32_t get_key( pairing_node *node ) {
    return node->key;
}

void* get_item( pairing_node *node ) {
    return node->item;
}

uint32_t get_size( pairing_heap *heap ) {
    return heap->size;
}

pairing_node* insert( pairing_heap *heap, void *item, uint32_t key ) {
    INCR_INSERT
    
    pairing_node *wrapper = (pairing_node*) calloc( 1, sizeof( pairing_node ) );
    wrapper->item = item;
    wrapper->key = key;
    heap->size++;
    if ( heap->size > heap->stats->max_size )
        heap->stats->max_size = heap->size;

    heap->root = merge( heap->root, wrapper );

    return wrapper;
}

pairing_node* find_min( pairing_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->root;
}

KEY_T delete_min( pairing_heap *heap ) {
    INCR_DELETE_MIN
    
    if ( empty( heap ) )
        return NULL;
    return delete( heap, heap->root );
}

KEY_T delete( pairing_heap *heap, pairing_node *node ) {
    INCR_DELETE
    
    KEY_T key = node->key;
    
    if ( node == heap->root )
        heap->root = collapse( node->child );
    else {
        if ( node->prev->child == node )
            node->prev->child = node->next;
        else
            node->prev->next = node->next;

        if ( node->next != NULL )
            node->next->prev = node->prev;

        heap->root = merge( heap->root, collapse( node->child ) );
    }

    free( node );
    heap->size--;

    return key;
}

void decrease_key( pairing_heap *heap, pairing_node *node, KEY_T new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
    if ( node == heap->root )
        return;

    if ( node->prev->child == node )
        node->prev->child = node->next;
    else
        node->prev->next = node->next;

    if ( node->next != NULL )
        node->next->prev = node->prev;

    heap->root = merge( heap->root, node );
}

void meld( pairing_heap *heap, pairing_heap *other_heap ) {
    INCR_MELD
    
    heap->root = merge( heap->root, other_heap->root );
    heap->size += other_heap->size;
    
    other_heap->size = 0;
    other_heap->root = NULL;
}

bool empty( pairing_heap *heap ) {
    return ( heap->size == 0 );
}

pairing_node* merge( pairing_node *a, pairing_node *b ) {
    pairing_node *parent, *child;

    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;

    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
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

pairing_node* collapse( pairing_node *node ) {
    pairing_node *tail, *a, *b, *next, *result;

    if ( node == NULL )
        return NULL;

    next = node;
    tail = NULL;
    while ( next != NULL ) {
        a = next;
        b = a->next;
        if ( b != NULL ) {
            next = b->next;
            result = merge( a, b );
            // tack the result onto the end of the temporary list
            result->prev = tail;
            tail = result;                    
        }
        else {
            a->prev = tail;
            tail = a;
            break;
        }
    }
    
    result = NULL;
    while ( tail != NULL ) {
        // trace back through to merge the list
        next = tail->prev;
        result = merge( result, tail );
        tail = next;
    }

    return result;
}
