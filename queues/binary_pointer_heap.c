#include "binary_pointer_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

binary_pointer_heap* create_heap( uint32_t capacity )
{
    map = create_mem_map( capacity );
    binary_pointer_heap *heap = (binary_pointer_heap*) calloc( 1, sizeof( binary_pointer_heap ) );
    return heap;
}

void destroy_heap( binary_pointer_heap *heap )
{
    clear_heap( heap );
    free( heap );
    destroy_mem_map( map );
}

void clear_heap( binary_pointer_heap *heap )
{
    while( !empty( heap ) )
        delete_min( heap );
}

key_type get_key( binary_pointer_heap *heap, binary_pointer_node *node )
{
    return node->key;
}

item_type* get_item( binary_pointer_heap *heap, binary_pointer_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t get_size( binary_pointer_heap *heap )
{
    return heap->size;
}

binary_pointer_node* insert( binary_pointer_heap *heap, item_type item, key_type key )
{
    binary_pointer_node* parent;
    binary_pointer_node* node = heap_node_alloc( map );
    ITEM_ASSIGN( node->item, item );
    node->key = key;

    if ( heap->root == NULL )
        heap->root = node;
    else
    {
        parent = find_insertion_point( heap );
        
        if ( parent->left == NULL )
            parent->left = node;
        else
            parent->right = node;

        node->parent = parent;
    }

    heap->size++;
    heapify_up( heap, node );
    
    return node;
}

binary_pointer_node* find_min( binary_pointer_heap *heap )
{
    if ( empty( heap ) )
        return NULL;
    return heap->root;
}

key_type delete_min( binary_pointer_heap *heap )
{
    return delete( heap, heap->root );
}

key_type delete( binary_pointer_heap *heap, binary_pointer_node* node )
{
    key_type key = node->key;
    binary_pointer_node *last_node = find_last_node( heap );
    swap( heap, node, last_node);

    // figure out if this node is a left or right child and clear
    // reference from parent
    if ( node->parent != NULL )
    {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else
            node->parent->right = NULL;
    }

    heap_node_free( map, node );
    heap->size--;
    
    if ( empty( heap ) )
        heap->root = NULL;
    else if ( node != last_node)
        heapify_down( heap, last_node );

    return key;
}

void decrease_key( binary_pointer_heap *heap, binary_pointer_node *node, key_type new_key )
{
    node->key = new_key;
    heapify_up( heap, node );
}

bool empty( binary_pointer_heap *heap )
{
    return ( heap->size == 0 );
}

void swap( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b )
{
    if ( ( a == NULL ) || ( b == NULL ) || ( a == b ) )
        return;
    
    if ( a->parent == b )
        swap_connected( heap, b, a );
    else if ( b->parent == a )
        swap_connected( heap, a, b );
    else
        swap_disconnected( heap, a, b );

    if ( heap->root == a )
        heap->root = b;
    else if ( heap->root == b )
        heap->root = a;
}

void swap_connected( binary_pointer_heap *heap, binary_pointer_node *parent, binary_pointer_node *child )
{
    binary_pointer_node *temp;

    child->parent = parent->parent;
    parent->parent = child;

    if ( child == parent->left )
    {
        parent->left = child->left;
        child->left = parent;

        temp = child->right;
        child->right = parent->right;
        parent->right = temp;
    }
    else
    {
        temp = child->left;
        child->left = parent->left;
        parent->left = temp;

        parent->right = child->right;
        child->right = parent;
    }
    fill_back_pointers( heap, parent, child );
}

void swap_disconnected( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b )
{
    binary_pointer_node *temp;

    temp = a->parent;
    a->parent = b->parent;
    b->parent = temp;

    temp = a->left;
    a->left = b->left;
    b->left = temp;

    temp = a->right;
    a->right = b->right;
    b->right = temp;

    fill_back_pointers( heap, a, b );
}

void fill_back_pointers( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b )
{
    if ( a->parent != NULL )
    {
        if ( a->parent->left == b )
            a->parent->left = a;
        else if ( a->parent->left != a )
            a->parent->right = a;
    }

    if ( b->parent != NULL )
    {
        if ( b->parent->left == a )
            b->parent->left = b;
        else if ( b->parent->left != b )
            b->parent->right = b;
    }

    if ( a->left != NULL )
        a->left->parent = a;
    if ( a->right != NULL )
        a->right->parent = a;

    if ( b->left != NULL )
        b->left->parent = b;
    if ( b->right != NULL )
        b->right->parent = b;
}

void heapify_down( binary_pointer_heap *heap, binary_pointer_node *node )
{
    if ( node == NULL )
        return;

    // repeatedly swap with smallest child if node violates heap order
    binary_pointer_node* smallest_child;
    while ( !is_leaf( heap, node ) )
    {
        if ( node->right == NULL )
            smallest_child = node->left;
        else if ( node->left->key < node->right->key )
            smallest_child = node->left;
        else
            smallest_child = node->right;

        if ( smallest_child->key < node->key )
            swap( heap, smallest_child, node );
        else
            break;
    }
}

void heapify_up( binary_pointer_heap *heap, binary_pointer_node *node )
{
    if ( node == NULL )
        return;

    while ( node->parent != NULL )
    {
        if ( node->key < node->parent->key )
            swap( heap, node, node->parent );
        else
            break;
    }
}

binary_pointer_node* find_last_node( binary_pointer_heap *heap )
{
    return find_node( heap, heap->size );
}

binary_pointer_node* find_insertion_point( binary_pointer_heap *heap )
{
    return find_node( heap, ( heap->size + 1 ) / 2 );
}

binary_pointer_node* find_node( binary_pointer_heap *heap, uint32_t n )
{
    uint32_t log, path, i;
    binary_pointer_node *current, *next;

    log = int_log2(n);
    current = heap->root;
    // i < log is used instead of i >= 0 because i is uint32_t
    // it will loop around to MAX_INT after it passes 0
    for ( i = ( log - 1 ); i < log; i-- )
    {
        path = ( n & ( 1 << i ) );
        
        if ( path == LEFT )
            next = current->left;
        else
            next = current->right;
            
        if ( next == NULL )
            break;
        else
            current = next;
    }

    return current;     
}

uint32_t int_log2( uint32_t n )
{
    if ( n == 0 )
        return 0;
    return ( 31 - __builtin_clz( n ) );
}

bool is_leaf( binary_pointer_heap *heap, binary_pointer_node* node )
{
    return ( node->left == NULL );
}
