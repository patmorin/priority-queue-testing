#include "implicit_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void push( implicit_heap *queue, uint32_t src, uint32_t dst );
static void dump( implicit_heap *queue, implicit_node *node, uint32_t dst );
static uint32_t heapify_down( implicit_heap *queue, implicit_node *node );
static uint32_t heapify_up( implicit_heap *queue, implicit_node *node );
//static void grow_heap( implicit_heap *queue );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

implicit_heap* pq_create( mem_map *map )
{
    implicit_heap *queue = calloc( 1, sizeof( implicit_heap ) );
    queue->capacity = map->capacities[0];
    queue->nodes = (implicit_node**) calloc( queue->capacity,
        sizeof( implicit_node* ) );
    queue->map = map;

    return queue;
}

void pq_destroy( implicit_heap *queue )
{
    pq_clear( queue );
    free( queue->nodes );
    free( queue );
}

void pq_clear( implicit_heap *queue )
{
    mm_clear( queue->map );
    queue->size = 0;
}

key_type pq_get_key( implicit_heap *queue, implicit_node *node )
{
    return node->key;
}

item_type* pq_get_item( implicit_heap *queue, implicit_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( implicit_heap *queue )
{
    return queue->size;
}

implicit_node* pq_insert( implicit_heap *queue, item_type item, key_type key )
{
    implicit_node *node = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( node->item, item );
    node->key = key;
    node->index = queue->size++;

    //if( queue->size == queue->capacity )
    //    grow_heap( queue );
    queue->nodes[node->index] = node;
    heapify_up( queue, node );

    return node;
}

implicit_node* pq_find_min( implicit_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->nodes[0];
}

key_type pq_delete_min( implicit_heap *queue )
{
    return pq_delete( queue, queue->nodes[0] );
}

key_type pq_delete( implicit_heap *queue, implicit_node* node )
{
    key_type key = node->key;
    implicit_node *last_node = queue->nodes[queue->size - 1];
    push( queue, last_node->index, node->index );

    pq_free_node( queue->map, 0, node );
    queue->size--;

    if ( node != last_node )
        heapify_down( queue, last_node );

    return key;
}

void pq_decrease_key( implicit_heap *queue, implicit_node *node,
    key_type new_key )
{
    node->key = new_key;
    heapify_up( queue, node );
}

bool pq_empty( implicit_heap *queue )
{
    return ( queue->size == 0 );
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
 * @param queue Queue to which both nodes belong
 * @param src   Index of data to be duplicated
 * @param dst   Index of data to overwrite
 */
static void push( implicit_heap *queue, uint32_t src, uint32_t dst )
{
    if ( ( src >= queue->size ) || ( dst >= queue->size ) || ( src == dst ) )
        return;

    queue->nodes[dst] = queue->nodes[src];
    queue->nodes[dst]->index = dst;
}

/**
 * Places a node in a certain location in the tree, updating both the
 * queue structure and the node record.
 *
 * @param queue Queue to which the node belongs
 * @param node  Pointer to node to be dumped
 * @param dst   Index of location to dump node
 */
static void dump( implicit_heap *queue, implicit_node *node, uint32_t dst )
{
    queue->nodes[dst] = node;
    node->index = dst;
}

/**
 * Takes a node that is potentially at a higher position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param queue Queue to which node belongs
 * @param node  Potentially violating node
 */
static uint32_t heapify_down( implicit_heap *queue, implicit_node *node )
{
    if ( node == NULL )
        return -1;

    uint32_t sentinel, i, min;
    uint32_t base = node->index;
    while( base * BRANCHING_FACTOR + 1 < queue->size )
    {
        i = base * BRANCHING_FACTOR + 1;
        sentinel = i + BRANCHING_FACTOR;
        if( sentinel > queue->size )
            sentinel = queue->size;

        min = i++;
        for( i = i; i < sentinel; i++ )
        {
            if( queue->nodes[i]->key < queue->nodes[min]->key )
                min = i;
        }

        if ( queue->nodes[min]->key < node->key )
            push( queue, min, base );
        else
            break;

        base = min;
    }

    dump( queue, node, base );

    return node->index;
}

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param queue Queue to which node belongs
 * @param node  Potentially violating node
 */
static uint32_t heapify_up( implicit_heap *queue, implicit_node *node )
{
    if ( node == NULL )
        return -1;

    uint32_t i;
    for( i = node->index; i > 0; i = (i-1)/BRANCHING_FACTOR )
    {
        if ( node->key < queue->nodes[(i-1)/BRANCHING_FACTOR]->key )
            push( queue, (i-1)/BRANCHING_FACTOR, i );
        else
            break;
    }
    dump( queue, node, i );

    return node->index;
}

/*static void grow_heap( implicit_heap *queue )
{
    uint32_t new_capacity = queue->capacity * 2;
    implicit_node **new_array = realloc( queue->nodes, new_capacity *
        sizeof( implicit_node* ) );

    if( new_array == NULL )
        exit( -1 );

    queue->capacity = new_capacity;
    queue->nodes = new_array;
}*/
