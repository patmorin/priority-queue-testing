#include "pairing_heap.h"

pairing_heap* create_heap() {
    pairing_heap *heap = (pairing_heap*) calloc( 1, sizeof( pairing_heap ) );
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( pairing_heap ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
    return heap;
}

void destroy_heap( pairing_heap *heap ) {
    clear_heap( heap );
    FREE_STATS
    free( heap );
}

void clear_heap( pairing_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( pairing_heap *heap, pairing_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

item_type* get_item( pairing_heap *heap, pairing_node *node ) {
        ADD_TRAVERSALS(1) // node
    return (item_type*) &(node->item);
}

uint32_t get_size( pairing_heap *heap ) {
    return heap->size;
}

pairing_node* insert( pairing_heap *heap, item_type item, key_type key ) {
    INCR_INSERT
    
    pairing_node *wrapper = (pairing_node*) calloc( 1, sizeof( pairing_node ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( pairing_node ) )
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    heap->size++;
        ADD_TRAVERSALS(1) // wrapper
        ADD_UPDATES(3) // wrapper, heap
        FIX_MAX_NODES

    heap->root = merge( heap, heap->root, wrapper );
        ADD_UPDATES(1) // heap

    return wrapper;
}

pairing_node* find_min( pairing_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->root;
}

key_type delete_min( pairing_heap *heap ) {
    INCR_DELETE_MIN
    
    return delete( heap, heap->root );
}

key_type delete( pairing_heap *heap, pairing_node *node ) {
    INCR_DELETE
    
    key_type key = node->key;
        ADD_TRAVERSALS(1) // node

    if ( node == heap->root ) {
        heap->root = collapse( heap, node->child );
            ADD_UPDATES(1) // heap
    }
    else {
            ADD_TRAVERSALS(1) // node->prev
        if ( node->prev->child == node )
            node->prev->child = node->next;
        else
            node->prev->next = node->next;
            ADD_UPDATES(1) // node->prev

        if ( node->next != NULL ) {
            node->next->prev = node->prev;
                ADD_UPDATES(1) // node->next
        }

        heap->root = merge( heap, heap->root, collapse( heap, node->child ) );
            ADD_UPDATES(1) // heap
    }

    free( node );
        SUB_SIZE( sizeof( pairing_node ) )
    heap->size--;

    return key;
}

void decrease_key( pairing_heap *heap, pairing_node *node, key_type new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(1) // node
    if ( node == heap->root )
        return;

        ADD_TRAVERSALS(1) // node->prev
    if ( node->prev->child == node )
        node->prev->child = node->next;
    else
        node->prev->next = node->next;
        ADD_UPDATES(1) // node->prev

    if ( node->next != NULL ) {
        node->next->prev = node->prev;
            ADD_UPDATES(1) // node->next
    }

    heap->root = merge( heap, heap->root, node );
        ADD_UPDATES(1) // heap
}

bool empty( pairing_heap *heap ) {
    return ( heap->size == 0 );
}

pairing_node* merge( pairing_heap *heap, pairing_node *a, pairing_node *b ) {
    pairing_node *parent, *child;

    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;

        ADD_TRAVERSALS(2) // a, b
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    child->next = parent->child;
    if ( parent->child != NULL ) {
        parent->child->prev = child;
            ADD_TRAVERSALS(1) // parent->child
            ADD_UPDATES(1) // parent->child
    }
    child->prev = parent;
    parent->child = child;
        ADD_UPDATES(3) // child, parent

    parent->next = NULL;
    parent->prev = NULL;
        ADD_UPDATES(2) // parent

    return parent;
}

pairing_node* collapse( pairing_heap *heap, pairing_node *node ) {
    pairing_node *tail, *a, *b, *next, *result;

    if ( node == NULL )
        return NULL;

    next = node;
    tail = NULL;
    while ( next != NULL ) {
        a = next;
        b = a->next;
            ADD_TRAVERSALS(1) // a
        if ( b != NULL ) {
            next = b->next;
                ADD_TRAVERSALS(1) // b
            result = merge( heap, a, b );
            // tack the result onto the end of the temporary list
            result->prev = tail;
                ADD_UPDATES(1) // result
            tail = result;                    
        }
        else {
            a->prev = tail;
                ADD_UPDATES(1) // a
            tail = a;
            break;
        }
    }
    
    result = NULL;
    while ( tail != NULL ) {
        // trace back through to merge the list
        next = tail->prev;
            ADD_TRAVERSALS(1) // tail
        result = merge( heap, result, tail );
        tail = next;
    }

    return result;
}
