#include "binary_array_heap.h"

binary_array_heap* create_heap() {
    binary_array_heap *heap = (binary_array_heap*) calloc( 1, sizeof( binary_array_heap ) );
    heap->nodes = (binary_array_node**) calloc( 1, sizeof( binary_array_node* ) );
    heap->capacity = 1;
    heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );

    return heap;
}

void destroy_heap( binary_array_heap *heap ) {
    clear_heap( heap );
    free( heap->nodes );
    free( heap->stats );
    free( heap );
}

void clear_heap( binary_array_heap *heap ) {
    uint32_t i;
    for ( i = 0; i < heap->size; i++ )
        free( heap->nodes[i] );
    resize_heap( heap, 1 );
}

uint32_t get_key( binary_array_node *node ) {
    return node->key;
}

void* get_item( binary_array_node *node ) {
    return node->item;
}

uint32_t get_size( binary_array_heap *heap ) {
    return heap->size;
}

binary_array_node* insert( binary_array_heap *heap, void *item, uint32_t key ) {
    heap->stats->count_insert++;
    
    binary_array_node *node = (binary_array_node*) calloc( 1, sizeof( binary_array_node ) );
    node->item = item;
    node->key = key;
    node->index = heap->size++;
    if ( heap->size > heap->stats->max_size )
        heap->stats->max_size = heap->size;

    heap->nodes[node->index] = node;
    heapify_up( heap, node );
    if ( heap->size == heap->capacity )
        resize_heap( heap, heap->capacity * 2 );

    return node;
}

void* find_min( binary_array_heap *heap ) {
    heap->stats->count_find_min++;
    
    if ( empty( heap ) )
        return NULL;
    return heap->nodes[0]->item;
}

void* delete_min( binary_array_heap *heap ) {
    heap->stats->count_delete_min++;
    
    if ( empty( heap ) )
        return NULL;
    return delete( heap, heap->nodes[0] );
}

void* delete( binary_array_heap *heap, binary_array_node* node ) {
    heap->stats->count_delete++;
    
    if ( node == NULL )
        return NULL;

    void* item = node->item;
    binary_array_node *last_node = heap->nodes[heap->size - 1];
    swap( heap, node->index, last_node->index );

    free( node );
    heap->size--;
    if ( heap->size < heap->capacity / 4 )
            resize_heap( heap, heap->capacity / 4 );

    if ( node != last_node )
        heapify_down( heap, last_node );

    return item;
}

void decrease_key( binary_array_heap *heap, binary_array_node *node, uint32_t delta ) {
    heap->stats->count_decrease_key++;

    node->key -= delta;
    heapify_up( heap, node );
}

void meld( binary_array_heap *heap, binary_array_heap *other_heap ) {
    heap->stats->count_meld++;
    
    binary_array_node *current_node;
    while ( other_heap->size > 0 ) {

        current_node = other_heap->nodes[--(other_heap->size)];
        heap->nodes[heap->size++] = current_node;
        heapify_up( heap, current_node );
        if ( heap->size == heap->capacity )
            resize_heap( heap, heap->capacity * 2 );
    }

    resize_heap( other_heap, 1 );
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
}

void heapify_down( binary_array_heap *heap, binary_array_node *node ) {
    if ( node == NULL )
        return;

    // repeatedly swap with smallest child if node violates heap order
    uint32_t i, smallest_child;
    for ( i = node->index; ( 2*i + 2 ) <= heap->size; i = node->index ) {
        if ( ( 2*i + 2 == heap->size ) || ( heap->nodes[2*i + 1]->key <= heap->nodes[2*i + 2]->key ) )
            smallest_child = 2*i + 1;
        else
            smallest_child = 2*i + 2;
            
        if ( heap->nodes[smallest_child]->key < heap->nodes[i]->key )
            swap( heap, smallest_child, i );
        else
            break;
    }
}

void heapify_up( binary_array_heap *heap, binary_array_node *node ) {
    if ( node == NULL )
        return;

    uint32_t i;
    for ( i = node->index; i > 0; i = (i-1)/2 ) {
        if ( heap->nodes[i]->key < heap->nodes[(i-1)/2]->key )
            swap( heap, i, (i-1)/2 );
        else
            break;
    }
}

void resize_heap( binary_array_heap *heap, uint32_t new_capacity ) {
    binary_array_node **new_array = (binary_array_node**) realloc( heap->nodes, ( new_capacity * sizeof( binary_array_node* ) ) );
    if ( new_array == NULL ) {
        printf( "Realloc fail..." );
        exit( 1 );
    }
    heap->nodes = new_array;
    heap->capacity = new_capacity;
}
