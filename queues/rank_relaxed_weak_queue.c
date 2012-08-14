#include "rank_relaxed_weak_queue.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

#define OCCUPIED(a,b)       ( a & ( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_SET(a,b)   ( a |= ( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_UNSET(a,b) ( a &= ~( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_LEADER(a)  ( (uint32_t) ( 63 - __builtin_clzll( a ) ) )

static inline void register_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node );
static inline void unregister_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node );

static void insert_root( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *new_root );
static void restore_invariants( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );
static rank_relaxed_weak_node* join( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b );

static void swap_parent_with_right_child( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *parent, rank_relaxed_weak_node *child );
static void swap_parent_with_left_child( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *parent, rank_relaxed_weak_node *child );
static void swap_disconnected( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b );

static void switch_node_ranks( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b );
static inline void flip_subtree( rank_relaxed_weak_node *node );
static inline void swap_subtrees( rank_relaxed_weak_node *a,
    rank_relaxed_weak_node **sub_a, rank_relaxed_weak_node *b,
    rank_relaxed_weak_node **sub_b );

static void sever_spine( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );
static void replace_node( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node, rank_relaxed_weak_node *replacement );
static void fix_min( rank_relaxed_weak_queue *queue );

static rank_relaxed_weak_node* transformation_cleaning(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node );
static rank_relaxed_weak_node* transformation_pair(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node );
static rank_relaxed_weak_node* transformation_parent(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node );
static rank_relaxed_weak_node* transformation_sibling(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node );
static rank_relaxed_weak_node* transformation_zigzag(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

rank_relaxed_weak_queue* pq_create( mem_map *map )
{
    rank_relaxed_weak_queue *queue = calloc( 1,
        sizeof( rank_relaxed_weak_queue ) );
    queue->map = map;

    return queue;
}

void pq_destroy( rank_relaxed_weak_queue *queue )
{
    pq_clear( queue );
    free( queue );
}

void pq_clear( rank_relaxed_weak_queue *queue )
{
    mm_clear( queue->map );
    queue->size = 0;
    queue->minimum = NULL;
    memset( queue->nodes[ROOTS], 0, MAXRANK * sizeof( rank_relaxed_weak_node* ) );
    memset( queue->nodes[MARKS], 0, MAXRANK * sizeof( rank_relaxed_weak_node* ) );
    queue->registry[ROOTS] = 0;
    queue->registry[MARKS] = 0;
}

key_type pq_get_key( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node )
{
    return node->key;
}

item_type* pq_get_item( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( rank_relaxed_weak_queue *queue )
{
    return queue->size;
}

rank_relaxed_weak_node* pq_insert( rank_relaxed_weak_queue *queue,
    item_type item, key_type key )
{
    rank_relaxed_weak_node *wrapper = pq_alloc_node( queue->map, 0 );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    queue->size++;
    insert_root( queue, wrapper );

    if ( ( queue->minimum == NULL ) || ( key < queue->minimum->key ) )
        queue->minimum = wrapper;

    return wrapper;
}

rank_relaxed_weak_node* pq_find_min( rank_relaxed_weak_queue *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->minimum;
}

key_type pq_delete_min( rank_relaxed_weak_queue *queue )
{
    rank_relaxed_weak_node *old_min = queue->minimum;
    key_type min_key = old_min->key;

    uint32_t replacement_rank;
    rank_relaxed_weak_node *replacement;
    if( old_min->parent == NULL )
        replacement = old_min;
    else
    {
        replacement_rank = REGISTRY_LEADER( queue->registry[ROOTS] );
        replacement = queue->nodes[ROOTS][replacement_rank];
    }

    unregister_node( queue, ROOTS, replacement );
    unregister_node( queue, MARKS, replacement );
    replacement->marked = 0;

    sever_spine( queue, replacement->right );
    replacement->parent = NULL;
    replacement->left = NULL;
    replacement->right = NULL;
    replacement->rank = 0;

    unregister_node( queue, ROOTS, old_min );
    unregister_node( queue, MARKS, old_min );

    if( replacement != old_min )
        replace_node( queue, old_min, replacement );

    fix_min( queue );

    pq_free_node( queue->map, 0, old_min );
    queue->size--;

    return min_key;
}

key_type pq_delete( rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    pq_decrease_key( queue, node, 0 );
    key_type min_key = pq_delete_min( queue );

    return min_key;
}

void pq_decrease_key( rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node,
    key_type new_key )
{
    node->key = new_key;
    if( node->parent != NULL )
    {
        if( !node->marked )
            node->marked = 1;
        restore_invariants( queue, node );
    }

    if( node->key <= queue->minimum->key )
        queue->minimum = node;
}

bool pq_empty( rank_relaxed_weak_queue *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

static inline void register_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node )
{
    if( !OCCUPIED( queue->registry[type], node->rank ) )
    {
        REGISTRY_SET( queue->registry[type], node->rank );
        queue->nodes[type][node->rank] = node;
    }
}

static inline void unregister_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node )
{
    if( queue->nodes[type][node->rank] == node )
    {
        REGISTRY_UNSET( queue->registry[type], node->rank );
        queue->nodes[type][node->rank] = NULL;
    }
}

static void insert_root( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *new_root )
{
    rank_relaxed_weak_node *tree = new_root;
    uint32_t rank = tree->rank;
    while( OCCUPIED( queue->registry[ROOTS], rank ) )
    {
        tree = join( queue, tree, queue->nodes[ROOTS][rank] );
        rank++;
    }

    register_node( queue, ROOTS, tree );
}

static void restore_invariants( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *new_mark = node;

    unregister_node( queue, MARKS, node );
    new_mark->marked = 1;

    // make sure children aren't marked
    if( new_mark->right != NULL && new_mark->right->marked )
        new_mark = transformation_parent( queue, new_mark->right );

    while( 1 )
    {
        if( new_mark == NULL )
            break;

        // check if node is a root
        if( new_mark->parent == NULL )
        {
            unregister_node( queue, MARKS, new_mark );
            new_mark->marked = 0;
            new_mark = NULL;

            break;
        }
        else if( new_mark->parent->parent == NULL )
        {
            new_mark = transformation_parent( queue, new_mark );
            if( new_mark != NULL )
            {
                unregister_node( queue, MARKS, new_mark );
                new_mark->marked = 0;
                new_mark = NULL;
            }

            break;
        }

        if( new_mark->parent->left == new_mark )
        {
            // check if node's sibling is marked
            if( new_mark->parent->right != NULL &&
                new_mark->parent->right->marked )
            {
                new_mark = transformation_sibling( queue, new_mark );

                continue;
            }

            // node is a left child and parent marked
            else if( new_mark->parent->left == new_mark && new_mark->parent->marked )
            {
                new_mark = transformation_zigzag( queue, new_mark );
                if( new_mark != NULL )
                    continue;

                break;
            }

            // node is a left child and all neighbors unmarked
            else
                new_mark = transformation_cleaning( queue, new_mark );
        }

        // node is a right child and parent marked
        if( new_mark->parent->right == new_mark && new_mark->parent->marked )
        {
            new_mark = transformation_parent( queue, new_mark );

            continue;
        }

        // node is a right child and parent unmarked
        else if( new_mark->parent->right == new_mark && !new_mark->parent->marked )
        {
            if( OCCUPIED( queue->registry[MARKS], new_mark->rank ) )
            {
                new_mark = transformation_pair( queue, new_mark );

                continue;
            }

            break;
        }


        break;
    }

    if( new_mark != NULL && new_mark->marked )
        register_node( queue, MARKS, new_mark );
}

static rank_relaxed_weak_node* join( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b )
{
    rank_relaxed_weak_node *parent, *child;
    if( a->key <= b->key )
    {
        parent = a;
        child = b;
    }
    else
    {
        parent = b;
        child = a;
    }

    unregister_node( queue, ROOTS, parent );
    unregister_node( queue, ROOTS, child );
    unregister_node( queue, MARKS, parent );
    unregister_node( queue, MARKS, child );
    if( child->marked )
        child->marked = 0;

    if( child->left != NULL )
    {
        parent->left = child->left;
        parent->left->parent = parent;
    }
    child->left = parent->right;
    parent->right = child;
    child->parent = parent;

    if( child->left != NULL )
    {
        child->left->parent = child;
        if( child->left->marked )
        {
            if( child->right->marked )
                transformation_sibling( queue, child->left );
            else
                transformation_cleaning( queue, child->left );
        }
    }

    parent->rank++;

    return parent;
}

static void swap_parent_with_right_child( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *parent, rank_relaxed_weak_node *child )
{
    rank_relaxed_weak_node *parent_parent = parent->parent;
    rank_relaxed_weak_node *parent_left = parent->left;
    rank_relaxed_weak_node *child_left = child->left;
    rank_relaxed_weak_node *child_right = child->right;

    parent->parent = child;
    child->right = parent;

    child->parent = parent_parent;
    if( parent_parent != NULL )
    {
        if( parent_parent->left == parent )
            parent_parent->left = child;
        else
            parent_parent->right = child;
    }

    child->left = parent_left;
    if( parent_left != NULL )
        parent_left->parent = child;

    parent->left = child_left;
    if( child_left != NULL )
        child_left->parent = parent;

    parent->right = child_right;
    if( child_right != NULL )
        child_right->parent = parent;

    switch_node_ranks( queue, parent, child );
}

static void swap_parent_with_left_child( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *parent, rank_relaxed_weak_node *child )
{
    rank_relaxed_weak_node *parent_parent = parent->parent;
    rank_relaxed_weak_node *parent_right = parent->right;
    rank_relaxed_weak_node *child_left = child->left;
    rank_relaxed_weak_node *child_right = child->right;

    parent->parent = child;
    child->left = parent;

    child->parent = parent_parent;
    if( parent_parent != NULL )
    {
        if( parent_parent->left == parent )
            parent_parent->left = child;
        else
            parent_parent->right = child;
    }

    child->right = parent_right;
    if( parent_right != NULL )
        parent_right->parent = child;

    parent->left = child_left;
    if( child_left != NULL )
        child_left->parent = parent;

    parent->right = child_right;
    if( child_right != NULL )
        child_right->parent = parent;

    switch_node_ranks( queue, parent, child );
}

static void swap_disconnected( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b )
{
    rank_relaxed_weak_node *a_parent = a->parent;
    rank_relaxed_weak_node *a_left = a->left;
    rank_relaxed_weak_node *a_right = a->right;
    rank_relaxed_weak_node *b_parent = b->parent;
    rank_relaxed_weak_node *b_left = b->left;
    rank_relaxed_weak_node *b_right = b->right;

    a->parent = b_parent;
    if( b_parent != NULL )
    {
        if( b_parent->left == b )
            b_parent->left = a;
        else
            b_parent->right = a;
    }

    b->parent = a_parent;
    if( a_parent != NULL )
    {
        if( a_parent->left == a )
            a_parent->left = b;
        else
            a_parent->right = b;
    }

    a->left = b_left;
    if( b_left != NULL )
        b_left->parent = a;

    b->left = a_left;
    if( a_left != NULL )
        a_left->parent = b;

    a->right = b_right;
    if( b_right != NULL )
        b_right->parent = a;

    b->right = a_right;
    if( a_right != NULL )
        a_right->parent = b;

    switch_node_ranks( queue, a, b );
}

static void switch_node_ranks( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b )
{
    int a_unrooted = ( queue->nodes[ROOTS][a->rank] == a );
    int b_unrooted = ( queue->nodes[ROOTS][b->rank] == b );

    if( a_unrooted )
        unregister_node( queue, ROOTS, a );
    if( b_unrooted )
        unregister_node( queue, ROOTS, b );
    if( a->marked )
        unregister_node( queue, MARKS, a );
    if( b->marked )
        unregister_node( queue, MARKS, b );

    uint32_t temp_rank = a->rank;
    a->rank = b->rank;
    b->rank = temp_rank;

    if( a_unrooted )
        register_node( queue, ROOTS, b );
    if( b_unrooted )
        register_node( queue, ROOTS, a );
}

static inline void flip_subtree( rank_relaxed_weak_node *node )
{
    if( node->parent == NULL )
        return;
    rank_relaxed_weak_node *temp = node->left;
    node->left = node->right;
    node->right = temp;
}

static inline void swap_subtrees( rank_relaxed_weak_node *a,
    rank_relaxed_weak_node **sub_a, rank_relaxed_weak_node *b,
    rank_relaxed_weak_node **sub_b )
{
    rank_relaxed_weak_node *temp = *sub_a;
    *sub_a = *sub_b;
    *sub_b = temp;

    if( *sub_a != NULL )
        (*sub_a)->parent = a;
    if( *sub_b != NULL )
        (*sub_b)->parent = b;
}

static void sever_spine( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *next;
    rank_relaxed_weak_node *current = node;
    while( current != NULL )
    {
        next = current->left;

        if( current->marked )
        {
            unregister_node( queue, MARKS, current );
            current->marked = 0;
        }
        current->left = NULL;
        current->parent = NULL;
        insert_root( queue, current );

        current = next;
    }
}

static void replace_node( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node, rank_relaxed_weak_node *replacement )
{
    rank_relaxed_weak_node *current, *next;
    rank_relaxed_weak_node *result = replacement;

    if( node->right != NULL )
    {
        current = node->right;
        while( current->left != NULL )
            current = current->left;

        while( current != node )
        {
            next = current->parent;
            if( current->marked )
            {
                unregister_node( queue, MARKS, current );
                current->marked = 0;
            }
            current->parent = NULL;
            current->left = NULL;
            result = join( queue, result, current );

            current = next;
        }
    }

    result->parent = node->parent;
    if( result->parent != NULL )
        result->parent->right = result;
    result->left = node->left;
    if( result->left != NULL )
        result->left->parent = result;

    result->marked = 1;
    register_node( queue, MARKS, result );
    restore_invariants( queue, result );

    if( result->parent == NULL )
        register_node( queue, ROOTS, result );
}

static void fix_min( rank_relaxed_weak_queue *queue )
{
    int i;
    uint32_t rank;
    rank_relaxed_weak_node *current;
    rank_relaxed_weak_node *min = NULL;

    uint64_t search_registry;
    rank_relaxed_weak_node **search_list;
    for( i = 0; i < 2; i++ )
    {
        search_registry = queue->registry[i];
        search_list = queue->nodes[i];

        while( search_registry )
        {
            rank = REGISTRY_LEADER( search_registry );
            current = search_list[rank];
            if( min == NULL || current->key <= min->key )
                min = current;
            REGISTRY_UNSET( search_registry, rank );
        }
    }

    queue->minimum = min;
}

static rank_relaxed_weak_node* transformation_cleaning(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *sibling = node->parent->right;
    flip_subtree( node->parent );
    swap_subtrees( node, &(node->left), sibling, &(sibling->left) );

    return node;
}

static rank_relaxed_weak_node* transformation_pair(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *primary = node;
    rank_relaxed_weak_node *extra = queue->nodes[MARKS][primary->rank];

    rank_relaxed_weak_node *tmp = primary;
    if( extra->parent->key < primary->parent->key )
    {
        primary = extra;
        extra = tmp;
    }

    rank_relaxed_weak_node *extra_parent = extra->parent;
    rank_relaxed_weak_node *primary_parent = primary->parent;

    unregister_node( queue, MARKS, extra );

    swap_disconnected( queue, primary, extra_parent );
    swap_subtrees( extra_parent, &(extra_parent->right), extra, &(extra->left) );

    if( extra_parent->key < primary_parent->key )
    {
        swap_parent_with_right_child( queue, primary_parent, extra_parent );
        flip_subtree( primary_parent );
    }

    rank_relaxed_weak_node *result = primary;
    rank_relaxed_weak_node *unmarked = extra;
    if( extra->key < primary->key )
    {
        swap_parent_with_right_child( queue, primary, extra );
        flip_subtree( primary );
        result = extra;
        unmarked = primary;
    }
    unregister_node( queue, MARKS, unmarked );
    unmarked->marked = 0;

    return result;
}

static rank_relaxed_weak_node* transformation_parent(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *parent = node->parent;
    rank_relaxed_weak_node *result = parent;
    rank_relaxed_weak_node *unmarked = node;

    if( parent->marked )
        unregister_node( queue, MARKS, parent );
    unregister_node( queue, MARKS, node );

    if( node->key < parent->key )
    {
        swap_parent_with_right_child( queue, parent, node );
        flip_subtree( parent );
        result = node;
        unmarked = parent;
    }
    unregister_node( queue, MARKS, unmarked );
    unmarked->marked = 0;

    if( !result->marked )
        result = NULL;

    return result;
}

static rank_relaxed_weak_node* transformation_sibling(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *parent = node->parent;
    rank_relaxed_weak_node *sibling = parent->right;
    rank_relaxed_weak_node *result = node;
    rank_relaxed_weak_node *unmarked = sibling;

    unregister_node( queue, MARKS, sibling );

    swap_parent_with_left_child( queue, parent, node );
    swap_subtrees( parent, &(parent->right), sibling, &(sibling->left) );

    if( sibling->key < node->key )
    {
        swap_parent_with_right_child( queue, node, sibling );
        flip_subtree( node );
        result = sibling;
        unmarked = node;
    }
    unregister_node( queue, MARKS, unmarked );
    unmarked->marked = 0;

    return result;
}

static rank_relaxed_weak_node* transformation_zigzag(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *parent = node->parent;
    rank_relaxed_weak_node *grand = parent->parent;

    if( grand->key < node->key )
        node->marked = 0;
    else
    {
        swap_disconnected( queue, grand, node );
        flip_subtree( grand );
    }

    unregister_node( queue, MARKS, parent );

    return parent;
}
