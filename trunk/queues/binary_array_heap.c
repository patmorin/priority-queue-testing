#include "binary_array_heap.h"

binary_array_heap* create_heap() {
    binary_array_heap *heap = (binary_array_heap*) calloc( 1, sizeof( binary_array_heap ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_array_heap ) )
    heap->nodes = (binary_array_node**) calloc( 1, sizeof( binary_array_node* ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_array_node* ) )
    heap->capacity = 1;
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
        ADD_UPDATES(3) // heap
        ADD_TRAVERSALS(1) // heap
    
    return heap;
}

void destroy_heap( binary_array_heap *heap ) {
    clear_heap( heap );
    free( heap->nodes );
        FREE_STATS
    free( heap );
        ADD_TRAVERSALS(1) // heap
}

void clear_heap( binary_array_heap *heap ) {
    uint32_t i;
        ADD_TRAVERSALS(1) // heap
    for ( i = 0; i < heap->size; i++ )
        free( heap->nodes[i] );
        SUB_SIZE( heap->size * sizeof( binary_array_node ) )
    heap->size = 0;
        ADD_UPDATES(1)
    resize_heap( heap, 1 );
}

KEY_T get_key( binary_array_heap *heap, binary_array_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

void* get_item( binary_array_heap *heap, binary_array_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->item;
}

uint32_t get_size( binary_array_heap *heap ) {
    return heap->size;
}

binary_array_node* insert( binary_array_heap *heap, void *item, uint32_t key ) {
        INCR_INSERT
    
    binary_array_node *node = (binary_array_node*) calloc( 1, sizeof( binary_array_node ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_array_node ) )
    node->item = item;
    node->key = key;
    node->index = heap->size++;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(3) // node
        FIX_MAX_NODES
        ADD_TRAVERSALS(1) // heap->stats
        ADD_UPDATES(1) // max_size

    heap->nodes[node->index] = node;
        ADD_TRAVERSALS(1) // heap->nodes
    heapify_up( heap, node );
        FIX_MAX_NODES

    return node;
}

binary_array_node* find_min( binary_array_heap *heap ) {
        INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
        ADD_TRAVERSALS(1) // heap->nodes
    return heap->nodes[0];
}

KEY_T delete_min( binary_array_heap *heap ) {
        INCR_DELETE_MIN
    
        ADD_TRAVERSALS(1) // heap->nodes
    return delete( heap, heap->nodes[0] );
}

KEY_T delete( binary_array_heap *heap, binary_array_node* node ) {
        INCR_DELETE
    
    KEY_T key = node->key;
    binary_array_node *last_node = heap->nodes[heap->size - 1];
    swap( heap, node->index, last_node->index );
        ADD_TRAVERSALS(2) // heap->nodes and node->index

    free( node );
        SUB_SIZE( sizeof( binary_array_node ) )
    heap->size--;
        ADD_UPDATES(1)
    if ( heap->size < heap->capacity / 4 )
            resize_heap( heap, heap->capacity / 4 );

    if ( node != last_node )
        heapify_down( heap, last_node );

    return key;
}

void decrease_key( binary_array_heap *heap, binary_array_node *node, KEY_T new_key ) {
        INCR_DECREASE_KEY

    node->key = new_key;
        ADD_TRAVERSALS(1)
        ADD_UPDATES(1)
    heapify_up( heap, node );
}

bool empty( binary_array_heap *heap ) {
    return ( heap->size == 0 );
}

void swap( binary_array_heap *heap, uint32_t a, uint32_t b ) {
    if ( ( a >= heap->size ) || ( b >= heap->size ) || ( a == b ) )
        return;
    
    binary_array_node *temp = heap->nodes[a];
    heap->nodes[a] = heap->nodes[b];
    heap->nodes[b] = temp;

    heap->nodes[a]->index = a;
    heap->nodes[b]->index = b;
        ADD_TRAVERSALS(4); // heap->nodes[a,b], heap->nodes[a,b]->index
        ADD_UPDATES(4); // heap->nodes[a,b], heap->nodes[a,b]->index
}

void heapify_down( binary_array_heap *heap, binary_array_node *node ) {
    if ( node == NULL )
        return;

    // repeatedly swap with smallest child if node violates heap order
    uint32_t i, smallest_child;
        ADD_TRAVERSALS(1) // initial node access
    for ( i = node->index; ( 2*i + 2 ) <= heap->size; i = node->index ) {
            ADD_TRAVERSALS(4) // heap->nodes twice, heap->nodes[...]->index
        if ( ( 2*i + 2 == heap->size ) || ( heap->nodes[2*i + 1]->key <= heap->nodes[2*i + 2]->key ) )
            smallest_child = 2*i + 1;
        else
            smallest_child = 2*i + 2;

        if ( heap->nodes[smallest_child]->key < node->key )
            swap( heap, smallest_child, i );
        else
            break;
    }
}

void heapify_up( binary_array_heap *heap, binary_array_node *node ) {
    if ( node == NULL )
        return;

    uint32_t i;
    ADD_TRAVERSALS(1) // initial node access
    for ( i = node->index; i > 0; i = (i-1)/2 ) {
        ADD_TRAVERSALS(2) // heap->nodes, heap->nodes[...]->key
        if ( node->key < heap->nodes[(i-1)/2]->key )
            swap( heap, i, (i-1)/2 );
        else
            break;
    }
}

void resize_heap( binary_array_heap *heap, uint32_t new_capacity ) {
        SUB_SIZE( heap->capacity * sizeof( binary_array_node* ) )
    binary_array_node **new_array = (binary_array_node**) realloc( heap->nodes, ( new_capacity * sizeof( binary_array_node* ) ) );
        INCR_ALLOCS
        ADD_SIZE( new_capacity * sizeof( binary_array_node* ) )
    if ( new_array == NULL ) {
        printf( "Realloc fail..." );
        exit( 1 );
    }
    heap->nodes = new_array;
    heap->capacity = new_capacity;
        ADD_UPDATES(2);
}
