#include "implicit_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
static mem_map *map;

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void push( implicit_heap *heap, uint32_t src, uint32_t dst );
static void dump( implicit_heap *heap, implicit_node *node, uint32_t dst );
static uint32_t heapify_down( implicit_heap *heap, implicit_node *node );
static uint32_t heapify_up( implicit_heap *heap, implicit_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

implicit_heap* pq_create( uint32_t capacity )
{
    map = mm_create( capacity );
    implicit_heap *heap = (implicit_heap*) calloc( 1, sizeof( implicit_heap ) );
    heap->nodes = (implicit_node**) calloc( capacity,
        sizeof( implicit_node* ) );
    heap->capacity = capacity;
        
    return heap;
}

void pq_destroy( implicit_heap *heap )
{
    pq_clear( heap );
    free( heap->nodes );
    free( heap );
    mm_destroy( map );
}

void pq_clear( implicit_heap *heap )
{
    mm_clear( map );
    heap->size = 0;
}

key_type pq_get_key( implicit_heap *heap, implicit_node *node )
{
    return node->key;
}

item_type* pq_get_item( implicit_heap *heap, implicit_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( implicit_heap *heap )
{
    return heap->size;
}

implicit_node* pq_insert( implicit_heap *heap, item_type item, key_type key )
{
    implicit_node *node = pq_alloc_node( map );
    ITEM_ASSIGN( node->item, item );
    node->key = key;
    node->index = heap->size++;

    heap->nodes[node->index] = node;
    heapify_up( heap, node );

    return node;
}

implicit_node* pq_find_min( implicit_heap *heap )
{
    if ( pq_empty( heap ) )
        return NULL;
    return heap->nodes[0];
}

key_type pq_delete_min( implicit_heap *heap )
{
    return pq_delete( heap, heap->nodes[0] );
}

key_type pq_delete( implicit_heap *heap, implicit_node* node )
{
    key_type key = node->key;
    node->key = MAX_KEY;
    implicit_node *last_node = heap->nodes[heap->size - 1];
    uint32_t bubble = heapify_down( heap, node );
    push( heap, last_node->index, bubble );

    pq_free_node( map, node );
    heap->size--;

    if ( node != last_node )
        heapify_up( heap, last_node );

    return key;
}

void pq_decrease_key( implicit_heap *heap, implicit_node *node,
    key_type new_key )
{
    node->key = new_key;
    heapify_up( heap, node );
}

bool pq_empty( implicit_heap *heap )
{
    return ( heap->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Takes two node positions and pushes the src pointer into the second.
 * Essentially this is a single-sided swap, and produces a duplicate
 * record which is meant to be overwritten later.  A chain of these
 * operations will make up a heapify operation, and will be followed by
 * a @ref <dump> operation to finish the simulated "swapping" effect.
 *
 * @param heap  Heap to which both nodes belong
 * @param src   Index of data to be duplicated
 * @param dst   Index of data to overwrite
 */
static void push( implicit_heap *heap, uint32_t src, uint32_t dst )
{
    if ( ( src >= heap->size ) || ( dst >= heap->size ) || ( src == dst ) )
        return;
    
    heap->nodes[dst] = heap->nodes[src];
    heap->nodes[dst]->index = src;
}

/**
 * Places a node in a certain location in the tree, updating both the
 * heap structure and the node record.
 * 
 * @param heap  Heap to which the node belongs
 * @param node  Pointer to node to be dumped
 * @param dst   Index of location to dump node
 */
static void dump( implicit_heap *heap, implicit_node *node, uint32_t dst )
{
    heap->nodes[dst] = node;
    node->index = dst;
}

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param heap  Heap to which node belongs
 * @param node  Potentially violating node
 */
static uint32_t heapify_up( implicit_heap *heap, implicit_node *node )
{
    if ( node == NULL )
        return -1;

    uint32_t i;
    for ( i = node->index; i > 0; i = (i-1)/BRANCHING_FACTOR )
    {
        if ( node->key < heap->nodes[(i-1)/BRANCHING_FACTOR]->key )
            push( heap, (i-1)/BRANCHING_FACTOR, i );
        else
        {
            dump( heap, node, i );
            break;
        }
    }
    
    return node->index;
}

/**
 * Takes a node that is potentially at a higher position in the tree
 * than it should be, and pushes it down to the correct location.
 *
 * @param heap  Heap to which node belongs
 * @param node  Potentially violating node
 */
static uint32_t heapify_down( implicit_heap *heap, implicit_node *node )
{
    if ( node == NULL )
        return -1;

    // repeatedly swap with smallest child if node violates heap order
    uint32_t i, k, min_k, smallest_child;
    for ( i = node->index; ( BRANCHING_FACTOR*i + 2 ) <= heap->size;
        i = smallest_child )
    {
        min_k = 0;
        for( k = 0; k < BRANCHING_FACTOR; k++ )
        {
            if( BRANCHING_FACTOR * i + k > heap->size )
                break;
            if( heap->nodes[BRANCHING_FACTOR*i + k + 1]-> key <
                    heap->nodes[BRANCHING_FACTOR*i + min_k + 1]->key )
                min_k = k;
        }
        smallest_child = BRANCHING_FACTOR * i + min_k + 1;

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
