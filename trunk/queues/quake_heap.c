#include "quake_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

quake_heap* create_heap( uint32_t capacity )
{
    map = create_mem_map( 2 * capacity );
    quake_heap *heap = (quake_heap*) calloc( 1, sizeof( quake_heap ) );
    return heap;
}

void destroy_heap( quake_heap *heap )
{
    clear_heap( heap );
    free( heap );
    destroy_mem_map( map );
}

void clear_heap( quake_heap *heap )
{
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( quake_heap *heap, quake_node *node )
{
    return node->key;
}

item_type* get_item( quake_heap *heap, quake_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t get_size( quake_heap *heap )
{
    return heap->size;
}

quake_node* insert( quake_heap *heap, item_type item, key_type key )
{
    quake_node *wrapper = heap_node_alloc( map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->parent = wrapper;
    
    make_root( heap, wrapper );
    heap->size++;
    (heap->nodes[0])++;

    return wrapper;
}

quake_node* find_min( quake_heap *heap )
{
    if ( empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type delete_min( quake_heap *heap )
{
    return delete( heap, heap->minimum );
}

key_type delete( quake_heap *heap, quake_node *node )
{
    key_type key = node->key;
    cut( heap, node );

    fix_roots( heap );
    fix_decay( heap );

    heap->size--;

    return key;
}

void decrease_key( quake_heap *heap, quake_node *node, key_type new_key )
{
    node->key = new_key;
    if ( is_root( heap, node ) )
    {
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
    }
    else
    {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else
            node->parent->right = NULL;

        make_root( heap, node );
    }
}

bool empty( quake_heap *heap )
{
    return ( heap->size == 0 );
}

void make_root( quake_heap *heap, quake_node* node )
{
    if ( node == NULL )
        return;

    if ( heap->minimum == NULL )
    {
         heap->minimum = node;
         node->parent = node;
    }
    else
    {
        node->parent = heap->minimum->parent;
        heap->minimum->parent = node;
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
    }
}

void remove_from_roots( quake_heap *heap, quake_node *node )
{
    quake_node *current = node->parent;
    while ( current->parent != node )
        current = current->parent;
    if ( current == node )
        heap->minimum = NULL;
    else
    {
        current->parent = node->parent;
        if ( heap->minimum == node )
            heap->minimum = current;
    }
}

void cut( quake_heap *heap, quake_node *node )
{
    if ( node == NULL )
        return;

    if ( is_root( heap, node ) )
        remove_from_roots( heap, node );
    else
    {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else if ( node->parent->right == node )
            node->parent->right = NULL;
    }
        
    cut( heap, node->left );
    make_root( heap, node->right );

    (heap->nodes[node->height])--;
    heap_node_free( map, node );
}

quake_node* join( quake_heap *heap, quake_node *a, quake_node *b )
{
    quake_node *parent, *child, *duplicate;

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

    duplicate = clone_node( heap, parent );
    if ( duplicate->left != NULL )
        duplicate->left->parent = duplicate;
    if ( duplicate->right != NULL )
        duplicate->right->parent = duplicate;

    duplicate->parent = parent;
    child->parent = parent;

    parent->parent = NULL;
    parent->left = duplicate;
    parent->right = child;

    parent->height++;
    (heap->nodes[parent->height])++;

    return parent;
}

void fix_roots( quake_heap *heap )
{
    quake_node *current, *next, *tail, *head, *joined;
    uint32_t i, height;

    if ( heap->minimum == NULL )
        return;

    for ( i = 0; i <= heap->highest_node; i++ )
        heap->roots[i] = NULL;
    heap->highest_node = 0;

    current = heap->minimum->parent;
    tail = heap->minimum;
    heap->minimum->parent = NULL;

    while ( current != NULL )
    {
        next = current->parent;
        current->parent = NULL;
        if ( ! attempt_insert( heap, current ) )
        {
            height = current->height;
            joined = join( heap, current, heap->roots[height] );
            if ( current == tail )
            {
                tail = joined; 
                next = tail;
            }
            else
            {
                tail->parent = joined;
                tail = tail->parent;
            }
            heap->roots[height] = NULL;
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i <= heap->highest_node; i++ )
    {
        if ( heap->roots[i] != NULL )
        {
            if ( head == NULL )
            {
                head = heap->roots[i];
                tail = heap->roots[i];
            }
            else
            {
                tail->parent = heap->roots[i];
                tail = tail->parent;
            }
        }
    }
    tail->parent = head;

    heap->minimum = head;
    fix_min( heap );
}

bool attempt_insert( quake_heap *heap, quake_node *node )
{
    uint32_t height = node->height;
    if ( ( heap->roots[height] != NULL ) && ( heap->roots[height] != node ) )
        return FALSE;

    if ( height > heap->highest_node )
        heap->highest_node = height;
    heap->roots[height] = node;

    return TRUE;
}

void fix_min( quake_heap *heap )
{
    quake_node *start = heap->minimum;
    quake_node *current = heap->minimum->parent;
    while ( current != start )
    {
        if ( current->key < heap->minimum->key )
            heap->minimum = current;
        current = current->parent;
    }
}

void fix_decay( quake_heap *heap )
{
    uint32_t i;
    check_decay( heap );
    if ( violation_exists( heap ) )
    {
        for ( i = heap->violation; i < MAXRANK; i++ )
        {
            if ( heap->roots[i] != NULL )
                prune( heap, heap->roots[i] );
        }
    }
}

void check_decay( quake_heap *heap )
{
    uint32_t i;
    for ( i = 1; i <= heap->highest_node; i++ )
    {
        if ( ( (float) heap->nodes[i] ) > ( (float) ( ALPHA * (float) heap->nodes[i-1] ) ) )
            break;
    }
    heap->violation = i;
}

bool violation_exists( quake_heap *heap )
{
    return ( heap->violation < MAXRANK );
}

void prune( quake_heap *heap, quake_node *node )
{
    quake_node *duplicate, *child;

    if ( node == NULL )
        return;

    if ( node->height < heap->violation )
    {
        if ( ! is_root( heap, node ) )
            make_root( heap, node );
            
        return;
    }

    duplicate = node->left;
    child = node->right;

    prune( heap, child );        

    node->left = duplicate->left;
    if ( node->left != NULL )
        node->left->parent = node;
    node->right = duplicate->right;
    if ( node->right != NULL )
        node->right->parent = node;
    (heap->nodes[node->height])--;
    node->height--;
    heap_node_free( map, duplicate );

    prune( heap, node );
}

quake_node* clone_node( quake_heap *heap, quake_node *original )
{
    quake_node *clone = heap_node_alloc( map );
        
    ITEM_ASSIGN( clone->item, original->item );
    clone->key = original->key;
    clone->height = original->height;
    clone->left = original->left;
    clone->right = original->right;

    return clone;
}

bool is_root( quake_heap *heap, quake_node *node )
{
    return ( ( node->parent->left != node ) && ( node->parent->right != node ) );
}
