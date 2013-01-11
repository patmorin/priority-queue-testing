#include "implicit_simple_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void push( implicit_simple_heap *queue, uint32_t src, uint32_t dst );
static void dump( implicit_simple_heap *queue, implicit_simple_node *node, uint32_t dst );
static uint32_t heapify_down( implicit_simple_heap *queue, implicit_simple_node *node );
static uint32_t heapify_up( implicit_simple_heap *queue, implicit_simple_node *node );
static void grow_heap( implicit_simple_heap *queue );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

implicit_simple_heap* pq_create( mem_map *map )
{
    implicit_simple_heap *queue = calloc( 1, sizeof( implicit_simple_heap ) );
#ifndef USE_EAGER
    queue->capacity = 1;
    queue->nodes = calloc( 1, sizeof( implicit_simple_node ) );
#else
    queue->capacity = map->capacities[0];
    queue->nodes = calloc( queue->capacity, sizeof( implicit_simple_node ) );
#endif
    queue->map = map;

    return queue;
}

void pq_destroy( implicit_simple_heap *queue )
{
    pq_clear( queue );
    free( queue->nodes );
    free( queue );
}

void pq_clear( implicit_simple_heap *queue )
{
    mm_clear( queue->map );
    queue->size = 0;
}

key_type pq_get_key( implicit_simple_heap *queue, implicit_simple_node *node )
{
    return node->key;
}

item_type* pq_get_item( implicit_simple_heap *queue, implicit_simple_node *node )
{
    return node->item;
}

uint32_t pq_get_size( implicit_simple_heap *queue )
{
    return queue->size;
}

implicit_simple_node* pq_insert( implicit_simple_heap *queue, item_type item, key_type key )
{
#ifndef USE_EAGER
    if( queue->size == queue->capacity )
        grow_heap( queue );
#endif
    implicit_simple_node *node = &(queue->nodes[queue->size++]);
    node->key = key;
    ITEM_ASSIGN( node->item, item );

    heapify_up( queue, node );

    return 0;
}

implicit_simple_node* pq_find_min( implicit_simple_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return &(queue->nodes[0]);
}

key_type pq_delete_min( implicit_simple_heap *queue )
{
    implicit_simple_node node = queue->nodes[0];
    key_type key = node.key;

    queue->size--;
    push( queue, queue->size, 0 );
    if ( queue->size > 1 )
        heapify_down( queue, &(queue->nodes[0]) );

    return key;
}

key_type pq_delete( implicit_simple_heap *queue, implicit_simple_node* node )
{
    return 0;
}

void pq_decrease_key( implicit_simple_heap *queue, implicit_simple_node *node,
    key_type new_key )
{
    node->key = new_key;
    heapify_up( queue, node );
}

bool pq_empty( implicit_simple_heap *queue )
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
static void push( implicit_simple_heap *queue, uint32_t src, uint32_t dst )
{
    queue->nodes[dst] = queue->nodes[src];
}

/**
 * Places a node in a certain location in the tree, updating both the
 * queue structure and the node record.
 *
 * @param queue Queue to which the node belongs
 * @param node  Pointer to node to be dumped
 * @param dst   Index of location to dump node
 */
static void dump( implicit_simple_heap *queue, implicit_simple_node *node, uint32_t dst )
{
    queue->nodes[dst] = *node;
}

/**
 * Takes a node that is potentially at a higher position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param queue Queue to which node belongs
 * @param node  Potentially violating node
 */
static uint32_t heapify_down( implicit_simple_heap *queue, implicit_simple_node *node )
{
    implicit_simple_node saved = *node;
    uint32_t sentinel, i, min;
    uint32_t base = node - queue->nodes;
    while( base * BRANCHING_FACTOR + 1 < queue->size )
    {
        i = base * BRANCHING_FACTOR + 1;
        sentinel = i + BRANCHING_FACTOR;
        if( sentinel > queue->size )
            sentinel = queue->size;

        min = i++;
        for( i = i; i < sentinel; i++ )
        {
            if( queue->nodes[i].key < queue->nodes[min].key )
                min = i;
        }

        if ( queue->nodes[min].key < saved.key )
            push( queue, min, base );
        else
            break;

        base = min;
    }

    dump( queue, &saved, base );

    return 0;
}

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param queue Queue to which node belongs
 * @param node  Potentially violating node
 */
static uint32_t heapify_up( implicit_simple_heap *queue, implicit_simple_node *node )
{
    implicit_simple_node saved = *node;
    uint32_t i;
    for( i = node - queue->nodes; i > 0; i = (i-1)/BRANCHING_FACTOR )
    {
        if ( saved.key < queue->nodes[(i-1)/BRANCHING_FACTOR].key )
            push( queue, (i-1)/BRANCHING_FACTOR, i );
        else
            break;
    }
    dump( queue, &saved, i );

    return 0;
}

static void grow_heap( implicit_simple_heap *queue )
{
    uint32_t new_capacity = queue->capacity * 2;
    implicit_simple_node **new_array = realloc( queue->nodes, new_capacity *
        sizeof( implicit_simple_node* ) );

    if( new_array == NULL )
        exit( -1 );

    queue->capacity = new_capacity;
    queue->nodes = new_array;
}
