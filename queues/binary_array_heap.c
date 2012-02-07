#include "binary_array_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

binary_array_heap* create_heap( uint32_t capacity )
{
    map = create_mem_map( capacity );
    binary_array_heap *heap = (binary_array_heap*) calloc( 1, sizeof( binary_array_heap ) );
    heap->nodes = (binary_array_node**) calloc( capacity, sizeof( binary_array_node* ) );
    heap->capacity = capacity;
        
    return heap;
}

void destroy_heap( binary_array_heap *heap )
{
    clear_heap( heap );
    free( heap->nodes );
    free( heap );
    destroy_mem_map( map );
}

void clear_heap( binary_array_heap *heap )
{
    uint32_t i;
    for ( i = 0; i < heap->size; i++ )
        heap_node_free( map, heap->nodes[i] );
    heap->size = 0;
}

key_type get_key( binary_array_heap *heap, binary_array_node *node )
{
    return node->key;
}

item_type* get_item( binary_array_heap *heap, binary_array_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t get_size( binary_array_heap *heap )
{
    return heap->size;
}

binary_array_node* insert( binary_array_heap *heap, item_type item, key_type key )
{
    binary_array_node *node = heap_node_alloc( map );
    ITEM_ASSIGN( node->item, item );
    node->key = key;
    node->index = heap->size++;

    heap->nodes[node->index] = node;
    heapify_up( heap, node );

    return node;
}

binary_array_node* find_min( binary_array_heap *heap )
{
    if ( empty( heap ) )
        return NULL;
    return heap->nodes[0];
}

key_type delete_min( binary_array_heap *heap )
{
    return delete( heap, heap->nodes[0] );
}

key_type delete( binary_array_heap *heap, binary_array_node* node )
{
    key_type key = node->key;
    node->key = MAX_KEY;
    binary_array_node *last_node = heap->nodes[heap->size - 1];
    uint32_t bubble = heapify_down( heap, node );
    push( heap, last_node->index, bubble );

    heap_node_free( map, node );
    heap->size--;

    if ( node != last_node )
        heapify_up( heap, last_node );

    return key;
}

void decrease_key( binary_array_heap *heap, binary_array_node *node, key_type new_key )
{
    node->key = new_key;
    heapify_up( heap, node );
}

bool empty( binary_array_heap *heap )
{
    return ( heap->size == 0 );
}

void push( binary_array_heap *heap, uint32_t src, uint32_t dst )
{
    if ( ( src >= heap->size ) || ( dst >= heap->size ) || ( src == dst ) )
        return;
    
    heap->nodes[dst] = heap->nodes[src];
    heap->nodes[dst]->index = src;
}

void dump( binary_array_heap *heap, binary_array_node *node, uint32_t dst )
{
    heap->nodes[dst] = node;
    node->index = dst;
}

uint32_t heapify_down( binary_array_heap *heap, binary_array_node *node )
{
    if ( node == NULL )
        return -1;

    // repeatedly swap with smallest child if node violates heap order
    uint32_t i, smallest_child;
    for ( i = node->index; ( 2*i + 2 ) <= heap->size; i = smallest_child )
    {
        if ( ( 2*i + 2 == heap->size ) || ( heap->nodes[2*i + 1]->key <= heap->nodes[2*i + 2]->key ) )
            smallest_child = 2*i + 1;
        else
            smallest_child = 2*i + 2;

        if ( heap->nodes[smallest_child]->key < node->key )
            push( heap, smallest_child, i );
        else
        {
            dump( heap, node, i );
            break;
        }
    }
    
    return node->index;
}

uint32_t heapify_up( binary_array_heap *heap, binary_array_node *node )
{
    if ( node == NULL )
        return -1;

    uint32_t i;
    for ( i = node->index; i > 0; i = (i-1)/2 )
    {
        if ( node->key < heap->nodes[(i-1)/2]->key )
            push( heap, (i-1)/2, i );
        else
        {
            dump( heap, node, i );
            break;
        }
    }
    
    return node->index;
}
