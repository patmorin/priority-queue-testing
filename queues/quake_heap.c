#include "quake_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

static void make_root( quake_heap *queue, quake_node *node );
static void remove_from_roots( quake_heap *queue, quake_node *node );
static void cut( quake_heap *queue, quake_node *node );
static quake_node* join( quake_heap *queue, quake_node *a, quake_node *b );
static void fix_roots( quake_heap *queue );
static bool attempt_insert( quake_heap *queue, quake_node *node );
static void fix_min( quake_heap *queue );
static void fix_decay( quake_heap *queue );
static void check_decay( quake_heap *queue );
static bool violation_exists( quake_heap *queue );
static void prune( quake_heap *queue, quake_node *node );
static quake_node* clone_node( quake_heap *queue, quake_node *original );
static bool is_root( quake_heap *queue, quake_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

quake_heap* pq_create( mem_map *map )
{
    quake_heap *queue = calloc( 1, sizeof( quake_heap ) );
    queue->map = map;
    
    return queue;
}

void pq_destroy( quake_heap *queue )
{
    pq_clear( queue );
    free( queue );
    mm_destroy( queue->map );
}

void pq_clear( quake_heap *queue )
{
    mm_clear( queue->map );
    queue->minimum = NULL;
    memset( queue->roots, 0, MAXRANK * sizeof( quake_node* ) );
    memset( queue->nodes, 0, MAXRANK * sizeof( uint32_t ) );
    queue->highest_node = 0;
    queue->violation = 0;
    queue->size = 0;
}

key_type pq_get_key( quake_heap *queue, quake_node *node )
{
    return node->key;
}

item_type* pq_get_item( quake_heap *queue, quake_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( quake_heap *queue )
{
    return queue->size;
}

quake_node* pq_insert( quake_heap *queue, item_type item, key_type key )
{
    quake_node *wrapper = pq_alloc_node( queue->map );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->parent = wrapper;
    
    make_root( queue, wrapper );
    queue->size++;
    (queue->nodes[0])++;

    return wrapper;
}

quake_node* pq_find_min( quake_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->minimum;
}

key_type pq_delete_min( quake_heap *queue )
{
    return pq_delete( queue, queue->minimum );
}

key_type pq_delete( quake_heap *queue, quake_node *node )
{
    key_type key = node->key;
    cut( queue, node );

    fix_roots( queue );
    fix_decay( queue );

    queue->size--;

    return key;
}

void pq_decrease_key( quake_heap *queue, quake_node *node, key_type new_key )
{
    node->key = new_key;
    if ( is_root( queue, node ) )
    {
        if ( node->key < queue->minimum->key )
            queue->minimum = node;
    }
    else
    {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else
            node->parent->right = NULL;

        make_root( queue, node );
    }
}

quake_heap* pq_meld( quake_heap *a, quake_heap *b )
{
    quake_heap *result, *trash;
    quake_node *temp;
    
    if( a->size >= b->size )
    {
        result = a;
        trash = b;
    }
    else
    {
        result = b;
        trash = a;
    }
        
    if( trash->minimum == NULL )
        return result;
    temp = result->minimum->parent;
    result->minimum->parent = trash->minimum->parent;
    trash->minimum->parent = temp;
    
    int k;
    for( k = 0; k < result->highest_node; k++ )
        result->nodes[k] += trash->nodes[k];

    return result;
}

bool pq_empty( quake_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

/**
 * Joins a node with the list of roots.
 *
 * @param queue Queue in which to operate
 * @param node  Node to make a new root
 */
static void make_root( quake_heap *queue, quake_node* node )
{
    if ( node == NULL )
        return;

    if ( queue->minimum == NULL )
    {
         queue->minimum = node;
         node->parent = node;
    }
    else
    {
        node->parent = queue->minimum->parent;
        queue->minimum->parent = node;
        if ( node->key < queue->minimum->key )
            queue->minimum = node;
    }
}

/**
 * Removes a node from the list of roots.
 *
 * @param queue Queue the node belongs to
 * @param node  Node to remove
 */
static void remove_from_roots( quake_heap *queue, quake_node *node )
{
    quake_node *current = node->parent;
    while ( current->parent != node )
        current = current->parent;
    if ( current == node )
        queue->minimum = NULL;
    else
    {
        current->parent = node->parent;
        if ( queue->minimum == node )
            queue->minimum = current;
    }
}

/**
 * Removes the node from the structure.  Recurses down through the left
 * child, which contains the same item, separating the right subtrees, making
 * the right child a new root.
 *
 * @param queue Queue the node belongs to
 * @param node  Node to remove
 */
static void cut( quake_heap *queue, quake_node *node )
{
    if ( node == NULL )
        return;

    if ( is_root( queue, node ) )
        remove_from_roots( queue, node );
    else
    {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else if ( node->parent->right == node )
            node->parent->right = NULL;
    }
        
    cut( queue, node->left );
    make_root( queue, node->right );

    (queue->nodes[node->height])--;
    pq_free_node( queue->map, node );
}

/**
 * Links two trees, making the larger-key tree the child of the lesser.
 * Creates a duplicate node to take the larger-key root's place.
 * Promotes the larger-key root as the new root of the joined tree.
 *
 * @param queue Queue in which to operate
 * @param a     First node
 * @param b     Second node
 * @return      Returns the resulting tree
 */
static quake_node* join( quake_heap *queue, quake_node *a, quake_node *b )
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

    duplicate = clone_node( queue, parent );
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
    (queue->nodes[parent->height])++;

    return parent;
}

/**
 * Performs an iterative linking on the list of roots until no two trees
 * of the same height remain.
 *
 * @param queue Queue whose roots to fix
 */
static void fix_roots( quake_heap *queue )
{
    quake_node *current, *next, *tail, *head, *joined;
    uint32_t i, height;

    if ( queue->minimum == NULL )
        return;

    for ( i = 0; i <= queue->highest_node; i++ )
        queue->roots[i] = NULL;
    queue->highest_node = 0;

    current = queue->minimum->parent;
    tail = queue->minimum;
    queue->minimum->parent = NULL;

    while ( current != NULL )
    {
        next = current->parent;
        current->parent = NULL;
        if ( !attempt_insert( queue, current ) )
        {
            height = current->height;
            joined = join( queue, current, queue->roots[height] );
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
            queue->roots[height] = NULL;
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i <= queue->highest_node; i++ )
    {
        if ( queue->roots[i] != NULL )
        {
            if ( head == NULL )
            {
                head = queue->roots[i];
                tail = queue->roots[i];
            }
            else
            {
                tail->parent = queue->roots[i];
                tail = tail->parent;
            }
        }
    }
    tail->parent = head;

    queue->minimum = head;
    fix_min( queue );
}

/**
 * Attempt to insert a tree in the height-indexed array.  Inserts if the
 * correct spot is empty or already contains the current node, reports
 * failure if it is occupied.
 *
 * @param queue Queue to insert into
 * @param node  Node to insert
 * @return      True if inserted, false if not
 */
static bool attempt_insert( quake_heap *queue, quake_node *node )
{
    uint32_t height = node->height;
    if ( ( queue->roots[height] != NULL ) && ( queue->roots[height] != node ) )
        return FALSE;

    if ( height > queue->highest_node )
        queue->highest_node = height;
    queue->roots[height] = node;

    return TRUE;
}

/**
 * Scans through the roots list starting from the current, potentially
 * inaccurate, minimum to find the tree with the minimum-value
 * root.
 * 
 * @param queue Queue to fix
 */
static void fix_min( quake_heap *queue )
{
    quake_node *start = queue->minimum;
    quake_node *current = queue->minimum->parent;
    while ( current != start )
    {
        if ( current->key < queue->minimum->key )
            queue->minimum = current;
        current = current->parent;
    }
}

/**
 * If a decay violation exists, this will remove all nodes of height
 * greater than or equal to the first violation.
 * 
 * @param queue Queue to fix
 */
static void fix_decay( quake_heap *queue )
{
    uint32_t i;
    check_decay( queue );
    if ( violation_exists( queue ) )
    {
        for ( i = queue->violation; i < MAXRANK; i++ )
        {
            if ( queue->roots[i] != NULL )
                prune( queue, queue->roots[i] );
        }
    }
}

/**
 * Searches for a decay violation and saves its location if it exists.
 * 
 * @param queue Queue to check
 */
static void check_decay( quake_heap *queue )
{
    uint32_t i;
    for ( i = 1; i <= queue->highest_node; i++ )
    {
        if ( ( (float) queue->nodes[i] ) > ( (float) ( ALPHA *
                (float) queue->nodes[i-1] ) ) )
            break;
    }
    queue->violation = i;
}

/**
 * Checks if a decay violation was found.
 *
 * @param queue Queue to check
 * @return      True if exists, false otherwise
 */
static bool violation_exists( quake_heap *queue )
{
    return ( queue->violation < MAXRANK );
}

/**
 * If the current node is higher than the violation, this function
 * rotates the current node down into the place of it's duplicate, and
 * deletes the duplicate.  Then it recurses on itself and its old
 * non-duplicate child.
 *
 * @param queue Queue to fix
 * @param node  Node to check and prune
 */
static void prune( quake_heap *queue, quake_node *node )
{
    quake_node *duplicate, *child;

    if ( node == NULL )
        return;

    if ( node->height < queue->violation )
    {
        if ( !is_root( queue, node ) )
            make_root( queue, node );
            
        return;
    }

    duplicate = node->left;
    child = node->right;

    prune( queue, child );        

    node->left = duplicate->left;
    if ( node->left != NULL )
        node->left->parent = node;
    node->right = duplicate->right;
    if ( node->right != NULL )
        node->right->parent = node;
    (queue->nodes[node->height])--;
    node->height--;
    pq_free_node( queue->map, duplicate );

    prune( queue, node );
}

/**
 * Copies internal data of another node for purposes of tournament resolution.
 *
 * @param queue     Queue to which node belongs
 * @param original  Node to copy data from
 * @return          Copy of the new node
 */
static quake_node* clone_node( quake_heap *queue, quake_node *original )
{
    quake_node *clone = pq_alloc_node( queue->map );
        
    ITEM_ASSIGN( clone->item, original->item );
    clone->key = original->key;
    clone->height = original->height;
    clone->left = original->left;
    clone->right = original->right;

    return clone;
}

/**
 * Determines whether this node is a root
 *
 * @param queue Queue in which node resides
 * @param node  Node to query
 * @return      True if root, false otherwise
 */
static bool is_root( quake_heap *queue, quake_node *node )
{
    return ( ( node->parent->left != node ) &&
        ( node->parent->right != node ) );
}
