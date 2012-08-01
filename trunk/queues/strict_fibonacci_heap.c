#include "strict_fibonacci_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

//--------------------------------------
// BASIC NODE FUNCTIONS
//--------------------------------------

static inline int is_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static inline void choose_order_pair( strict_fibonacci_node *a,
    strict_fibonacci_node *b, strict_fibonacci_node **parent,
    strict_fibonacci_node **child );
static inline void choose_order_triple( strict_fibonacci_node *a,
    strict_fibonacci_node *b, strict_fibonacci_node *c,
    strict_fibonacci_node **grand, strict_fibonacci_node **parent,
    strict_fibonacci_node **child );
static inline void remove_from_siblings( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void link( strict_fibonacci_heap *queue, strict_fibonacci_node *parent,
    strict_fibonacci_node *child );
static strict_fibonacci_node* select_new_root( strict_fibonacci_heap *queue );

//--------------------------------------
// QUEUE MANAGEMENT
//--------------------------------------

static void enqueue_node( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void dequeue_node( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static strict_fibonacci_node* consume_node( strict_fibonacci_heap *queue );

//--------------------------------------
// ACTIVE NODE MANAGEMENT
//--------------------------------------

static void increase_rank( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void decrease_rank( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void increase_loss( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void decrease_loss( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void switch_node_rank( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node, rank_record *new_rank );
static void insert_fix_node( strict_fibonacci_heap *queue, fix_node *fix,
    int type );
static void remove_fix_node( strict_fibonacci_heap *queue, fix_node *fix,
    int type );
static void check_rank( strict_fibonacci_heap *queue, rank_record *rank,
    int type );
static void move_rank( strict_fibonacci_heap *queue, rank_record *rank,
    int type, int direction );

//--------------------------------------
// NODE TYPE CONVERSIONS
//--------------------------------------

static void convert_active_to_root( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void convert_active_to_loss( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void convert_root_to_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void convert_loss_to_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void convert_passive_to_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void convert_to_passive( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );

//--------------------------------------
// REDUCTIONS
//--------------------------------------

static int reduce_active_roots( strict_fibonacci_heap *queue );
static int reduce_root_degree( strict_fibonacci_heap *queue );
static int reduce_loss( strict_fibonacci_heap *queue );
static void post_meld_reduction( strict_fibonacci_heap *queue );
static void post_delete_min_reduction( strict_fibonacci_heap *queue );
static void post_decrease_key_reduction( strict_fibonacci_heap *queue );

//--------------------------------------
// GARBAGE COLLECTION & ALLOCATION
//--------------------------------------

static rank_record* create_rank_record( strict_fibonacci_heap *queue,
    uint32_t rank, rank_record *pred );
static void release_active_record( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void release_rank_record( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node );
static void release_to_garbage_collector( strict_fibonacci_heap *queue,
    strict_fibonacci_heap *garbage_queue );
static void garbage_collection( strict_fibonacci_heap *queue );

//==============================================================================
// PUBLIC METHODS
//==============================================================================

strict_fibonacci_heap* pq_create( mem_map *map )
{
    strict_fibonacci_heap *queue = (strict_fibonacci_heap*) calloc( 1,
        sizeof( strict_fibonacci_heap ) );
    queue->map = map;

    return queue;
}

void pq_destroy( strict_fibonacci_heap *queue ){
    pq_clear( queue );
    free( queue );
}

void pq_clear( strict_fibonacci_heap *queue )
{
    mm_clear( queue->map );
    queue->size = 0;

    queue->root = NULL;
    queue->q_head = NULL;

    queue->active = NULL;
    queue->rank_list = NULL;
    queue->fix_list[0] = NULL;
    queue->fix_list[1] = NULL;
}

key_type pq_get_key( strict_fibonacci_heap *queue, strict_fibonacci_node *node )
{
    return node->key;
}

item_type* pq_get_item( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    return (item_type*) &(node->item);
}

uint32_t pq_get_size( strict_fibonacci_heap *queue )
{
    return queue->size;
}

strict_fibonacci_node* pq_insert( strict_fibonacci_heap *queue, item_type item,
    key_type key )
{
    strict_fibonacci_node* wrapper = pq_alloc_node( queue->map,
        STRICT_NODE_FIB );
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->right = wrapper;
    wrapper->left = wrapper;
    wrapper->q_next = wrapper;
    wrapper->q_prev = wrapper;

    strict_fibonacci_node *parent, *child;
    if( queue->root == NULL )
        queue->root = wrapper;
    else
    {
        choose_order_pair( wrapper, queue->root, &parent, &child );
        link( queue, parent, child );
        queue->root = parent;
        enqueue_node( queue, child );

        post_meld_reduction( queue );
    }

    queue->size++;
    garbage_collection( queue );

    return wrapper;
}

strict_fibonacci_node* pq_find_min( strict_fibonacci_heap *queue )
{
    if ( pq_empty( queue ) )
        return NULL;
    return queue->root;
}

key_type pq_delete_min( strict_fibonacci_heap *queue )
{
    if( pq_empty( queue ) )
        return 0;

    key_type key = queue->root->key;
    strict_fibonacci_node *current, *new_root, *old_root;
    int i, j;

    old_root = queue->root;

    if( old_root->left_child == NULL )
    {
        old_root = queue->root;
        if( is_active( queue, old_root ) )
            convert_to_passive( queue, old_root );
        queue->root = NULL;
    }
    else
    {
        new_root = select_new_root( queue );
        remove_from_siblings( queue, new_root );
        dequeue_node( queue, new_root );
        queue->root = new_root;

        if( is_active( queue, new_root ) )
            convert_to_passive( queue, new_root );
        if( is_active( queue, old_root ) )
            convert_to_passive( queue, old_root );

        while( old_root->left_child != NULL )
            link( queue, new_root, old_root->left_child );

        for( i = 0; i < 2; i++ )
        {
            current = consume_node( queue );
            if( current != NULL )
            {
                for( j = 0; j < 2; j++ )
                {
                    if( current->left_child != NULL &&
                            !is_active( queue, current->left_child->left ) )
                        link( queue, new_root, current->left_child->left );
                    else
                        break;
                }
            }
        }
    }

    pq_free_node( queue->map, STRICT_NODE_FIB, old_root );

    post_delete_min_reduction( queue );
    garbage_collection( queue );

    queue->size--;

    return key;
}

key_type pq_delete( strict_fibonacci_heap *queue, strict_fibonacci_node *node )
{
    key_type key = node->key;

    pq_decrease_key( queue, node, 0 );
    pq_delete_min( queue );

    return key;
}

void pq_decrease_key( strict_fibonacci_heap *queue, strict_fibonacci_node *node,
    key_type new_key )
{
    strict_fibonacci_node *old_parent = node->parent;

    node->key = new_key;

    if( old_parent == NULL || node->key > old_parent->key)
        return;

    strict_fibonacci_node *parent, *child;
    choose_order_pair( node, queue->root, &parent, &child );

    link( queue, parent, child );
    queue->root = parent;
    queue->root->parent = NULL;

    if( parent == node )
    {
        dequeue_node( queue, parent );
        enqueue_node( queue, child );
    }

    if( is_active( queue, node ) )
    {
        if( is_active( queue, old_parent ) )
            decrease_rank( queue, old_parent );
        if( node->type != STRICT_TYPE_ROOT )
            convert_active_to_root( queue, node );
    }
    if( is_active( queue, old_parent ) && old_parent->type != STRICT_TYPE_ROOT )
        increase_loss( queue, old_parent );

    post_decrease_key_reduction( queue );
    garbage_collection( queue );
}

strict_fibonacci_heap* pq_meld( strict_fibonacci_heap *a,
    strict_fibonacci_heap *b )
{
    strict_fibonacci_heap *new_heap = pq_create( a->map );
    strict_fibonacci_heap *big, *small;

    strict_fibonacci_node *big_head, *big_tail, *small_head, *small_tail;
    strict_fibonacci_node *parent, *child;

    // pick which heap to preserve
    if( a->size < b->size )
    {
        big = b;
        small = a;
    }
    else
    {
        big = a;
        small = b;
    }

    // set heap fields
    new_heap->size = big->size + small->size;
    new_heap->q_head = big->q_head;
    new_heap->active = big->active;
    new_heap->rank_list = big->rank_list;
    new_heap->fix_list[0] = big->fix_list[0];
    new_heap->fix_list[1] = big->fix_list[1];

    if( small->active != NULL )
        small->active->flag = 0;

    // merge the queues
    big_head = big->q_head;
    big_tail = big_head->q_prev;
    small_head = small->q_head;
    small_tail = small_head->q_prev;

    big_head->q_prev = small_tail;
    small_tail->q_next = big_head;
    small_head->q_prev = big_tail;
    big_tail->q_next = small_head;

    // actually link the two trees
    choose_order_pair( big->root, small->root, &parent, &child );
    link( new_heap, parent, child );
    new_heap->root = parent;
    enqueue_node( new_heap, child );

    // take care of some garbage collection
    release_to_garbage_collector( new_heap, small );
    free( small );
    free( big );
    garbage_collection( new_heap );

    return new_heap;
}

bool pq_empty( strict_fibonacci_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

//--------------------------------------
// BASIC NODE FUNCTIONS
//--------------------------------------

static inline int is_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    if( node->active == NULL )
        return 0;

    if( !node->active->flag )
    {
        release_active_record( queue, node );
        release_rank_record( queue, node );
        if( node->fix != NULL )
            node->fix = NULL;
        node->type = STRICT_TYPE_PASSIVE;

        return 0;
    }

    return 1;
}

static inline void choose_order_pair( strict_fibonacci_node *a,
    strict_fibonacci_node *b, strict_fibonacci_node **parent,
    strict_fibonacci_node **child )
{
    if( a->key <= b->key )
    {
        *parent = a;
        *child = b;
    }
    else
    {
        *parent = b;
        *child = a;
    }
}

static inline void choose_order_triple( strict_fibonacci_node *a,
    strict_fibonacci_node *b, strict_fibonacci_node *c,
    strict_fibonacci_node **grand, strict_fibonacci_node **parent,
    strict_fibonacci_node **child )
{
    if( a->key < b->key )
    {
        if( b->key < c->key )
        {
            *grand = a;
            *parent = b;
            *child = c;
        }
        else if( a->key < c->key )
        {
            *grand = a;
            *parent = c;
            *child = b;
        }
        else
        {
            *grand = c;
            *parent = a;
            *child = b;
        }
    }
    else
    {
        if( a->key < c->key )
        {
            *grand = b;
            *parent = a;
            *child = c;
        }
        else if( b->key < c->key )
        {
            *grand = b;
            *parent = c;
            *child = a;
        }
        else
        {
            *grand = c;
            *parent = b;
            *child = a;
        }
    }
}

static inline void remove_from_siblings( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    if( node->parent == NULL )
        return;

    strict_fibonacci_node *next = node->right;
    strict_fibonacci_node *prev;

    if( next == node )
    {
        node->parent->left_child = NULL;
    }
    else
    {
        prev = node->left;
        next->left = prev;
        prev->right = next;
        if( node->parent->left_child == node )
            node->parent->left_child = next;
    }

    node->right = node;
    node->left = node;
    node->parent = NULL;
}

static void link( strict_fibonacci_heap *queue, strict_fibonacci_node *parent,
    strict_fibonacci_node *child )
{
    if( parent == child->parent )
        return;

    if( child == queue->root )
        remove_from_siblings( queue, parent );
    else
        remove_from_siblings( queue, child );

    strict_fibonacci_node *next = parent->left_child;
    strict_fibonacci_node *prev;

    if( parent->left_child == NULL )
        parent->left_child = child;
    else
    {
        prev = next->left;
        child->right = next;
        child->left = prev;
        prev->right = child;
        next->left = child;

        if( is_active( queue, child ) )
            parent->left_child = child;
    }

    child->parent = parent;
}

static strict_fibonacci_node* select_new_root( strict_fibonacci_heap *queue )
{
    strict_fibonacci_node *old_root = queue->root;
    strict_fibonacci_node *new_root = old_root->left_child;

    strict_fibonacci_node *current = new_root->right;
    while( current != old_root->left_child )
    {
        if( current->key < new_root->key )
            new_root = current;
        current = current->right;
    }

    return new_root;
}

//--------------------------------------
// QUEUE MANAGEMENT
//--------------------------------------

static void enqueue_node( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    strict_fibonacci_node *next, *prev;

    if( queue->q_head != NULL )
    {
        next = queue->q_head;
        prev = next->q_prev;

        node->q_next = next;
        node->q_prev = prev;
        next->q_prev = node;
        prev->q_next = node;
    }

    queue->q_head = node->q_next;
}

static void dequeue_node( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    strict_fibonacci_node *prev;
    strict_fibonacci_node *next = node->q_next;
    if( next == node )
        queue->q_head = NULL;
    else
    {
        prev = node->q_prev;

        next->q_prev = prev;
        prev->q_next = next;

        node->q_next = node;
        node->q_prev = node;

        queue->q_head = next;
    }
}

static strict_fibonacci_node* consume_node( strict_fibonacci_heap *queue )
{
    if( queue->q_head == NULL )
        return NULL;

    strict_fibonacci_node *target = queue->q_head;
    queue->q_head = target->q_next;

    return target;
}

//--------------------------------------
// ACTIVE NODE MANAGEMENT
//--------------------------------------

static void increase_rank( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    rank_record *new_rank = node->rank->inc;
    uint32_t target_rank = node->rank->rank + 1;
    if( new_rank->rank != target_rank )
        new_rank = create_rank_record( queue, target_rank, new_rank );

    switch_node_rank( queue, node, new_rank );
}

static void decrease_rank( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    rank_record *new_rank = node->rank->dec;
    uint32_t target_rank = node->rank->rank - 1;
    if( new_rank->rank != target_rank )
        new_rank = create_rank_record( queue, target_rank, node->rank );

    switch_node_rank( queue, node, new_rank );
}

static void increase_loss( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    node->loss++;
    if( node->loss == 1 )
        convert_active_to_loss( queue, node );
}

static void decrease_loss( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    node->loss = 0;
    convert_loss_to_active( queue, node );
}

static void switch_node_rank( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node, rank_record *new_rank )
{
    int type = ( node->type == STRICT_TYPE_ROOT ) ? STRICT_FIX_ROOT :
        STRICT_FIX_LOSS;

    fix_node *fix = node->fix;
    if( fix != NULL )
        remove_fix_node( queue, fix, type );

    release_rank_record( queue, node );
    node->rank = new_rank;
    new_rank->ref_count++;

    if( fix != NULL )
    {
        fix->rank = new_rank;
        insert_fix_node( queue, fix, type );
    }
}

static void insert_fix_node( strict_fibonacci_heap *queue, fix_node *fix,
    int type )
{
    rank_record *rank = fix->rank;

    if( rank->head[type] == NULL )
    {
        rank->head[type] = fix;
        rank->tail[type] = fix;

        if( queue->fix_list[type] == NULL )
        {
            fix->right = fix;
            fix->left = fix;
            queue->fix_list[type] = fix;
            return;
        }
        else
        {
            fix->right = queue->fix_list[type];
            fix->left = fix->right->left;
            fix->right->left = fix;
            fix->left->right = fix;
        }
    }
    else
    {
        fix->right = rank->head[type];
        fix->left = fix->right->left;
        fix->right->left = fix;
        fix->left->right = fix;

        if( queue->fix_list[type] == rank->head[type] )
            queue->fix_list[type] = fix;
        rank->head[type] = fix;
    }

    check_rank( queue, rank, type );
}

static void remove_fix_node( strict_fibonacci_heap *queue, fix_node *fix,
    int type )
{
    rank_record *rank = fix->rank;

    if( queue->fix_list[type] == fix )
    {
        if( fix->right == fix )
            queue->fix_list[type] = NULL;
        else
            queue->fix_list[type] = fix->right;
    }

    if( rank->head[type] == fix )
    {
        if( rank->tail[type] == fix )
        {
            rank->head[type] = NULL;
            rank->tail[type] = NULL;
        }
        else
            rank->head[type] = fix->right;
    }
    else if( rank->tail[type] == fix )
        rank->tail[type] = fix->left;

    fix_node *next = fix->right;
    fix_node *prev = fix->left;

    next->left = prev;
    prev->right = next;

    check_rank( queue, rank, type );
}

static void check_rank( strict_fibonacci_heap *queue, rank_record *rank,
    int type )
{
    if( rank->head[type] == NULL )
        return;

    int status = ( rank->head[type] != rank->tail[type] ||
        rank->head[type]->node->loss > 1 );

    if( rank->transformable[type] && !status )
        move_rank( queue, rank, type, STRICT_DIR_DEMOTE );
    else if( !rank->transformable[type] && status )
        move_rank( queue, rank, type, STRICT_DIR_PROMOTE );
}

static void move_rank( strict_fibonacci_heap *queue, rank_record *rank,
    int type, int direction )
{
    fix_node *head = rank->head[type];
    fix_node *tail = rank->tail[type];
    fix_node *pred = head->left;
    fix_node *succ = tail->right;

    rank->transformable[type] = direction;

    if( pred == tail )
        return;

    if( queue->fix_list[type] == head && direction == STRICT_DIR_PROMOTE )
        return;

    if( queue->fix_list[type] == succ && direction == STRICT_DIR_DEMOTE )
        return;

    if( queue->fix_list[type] == succ && direction == STRICT_DIR_PROMOTE )
    {
        queue->fix_list[type] = head;
        return;
    }

    if( queue->fix_list[type] == head )
        queue->fix_list[type] = succ;

    pred->right = succ;
    succ->left = pred;

    succ = queue->fix_list[type];
    pred = succ->left;

    succ->left = tail;
    pred->right = head;
    head->left = pred;
    tail->right = succ;

    if( direction == STRICT_DIR_PROMOTE )
        queue->fix_list[type] = rank->head[type];
}

//--------------------------------------
// NODE TYPE CONVERSIONS
//--------------------------------------

static void convert_active_to_root( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    if( is_active( queue, node ) && node->type == STRICT_TYPE_LOSS )
        convert_loss_to_active( queue, node );

    fix_node *fix = pq_alloc_node( queue->map, STRICT_NODE_FIX );
    fix->node = node;
    fix->rank = node->rank;
    node->fix = fix;
    node->type = STRICT_TYPE_ROOT;

    insert_fix_node( queue, fix, STRICT_FIX_ROOT );
}

static void convert_active_to_loss( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    fix_node *fix = pq_alloc_node( queue->map, STRICT_NODE_FIX );
    fix->node = node;
    fix->rank = node->rank;
    node->fix = fix;
    node->type = STRICT_TYPE_LOSS;

    insert_fix_node( queue, fix, STRICT_FIX_LOSS );
}

static void convert_root_to_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    remove_fix_node( queue, node->fix, STRICT_FIX_ROOT );
    pq_free_node( queue->map, STRICT_NODE_FIX, node->fix );
    node->fix = NULL;
    node->type = STRICT_TYPE_ACTIVE;
}

static void convert_loss_to_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    remove_fix_node( queue, node->fix, STRICT_FIX_LOSS );
    pq_free_node( queue->map, STRICT_NODE_FIX, node->fix );
    node->fix = NULL;
    node->type = STRICT_TYPE_ACTIVE;
}

static void convert_passive_to_active( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    if( queue->active == NULL )
    {
        queue->active = pq_alloc_node( queue->map, STRICT_NODE_ACTIVE );
        queue->active->flag = 1;
    }
    node->active = queue->active;
    queue->active->ref_count++;

    rank_record *rank = queue->rank_list;
    if( rank == NULL || rank->rank != 0 )
        rank = create_rank_record( queue, 0, queue->rank_list );
    node->rank = rank;
    rank->ref_count++;
    node->type = STRICT_TYPE_ACTIVE;

    node->parent->left_child = node;
}

static void convert_to_passive( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    if( node->fix != NULL )
    {
        remove_fix_node( queue, node->fix, ( node->type == STRICT_TYPE_ROOT ) ?
            STRICT_FIX_ROOT : STRICT_FIX_LOSS );
        pq_free_node( queue->map, STRICT_NODE_FIX, node->fix );
        node->fix = NULL;
    }

    release_rank_record( queue, node );
    release_active_record( queue, node );
    node->type = STRICT_TYPE_PASSIVE;

    if( node->parent != NULL )
        link( queue, node->parent, node );

    if( node->left_child == NULL )
        return;

    strict_fibonacci_node *current = node->left_child;
    if( is_active( queue, current ) && current->type == STRICT_TYPE_ACTIVE )
        convert_active_to_root( queue, current );
    current = current->right;
    while( current != node->left_child )
    {
        if( is_active( queue, current ) && current->type == STRICT_TYPE_ACTIVE )
            convert_active_to_root( queue, current );
        current = current->right;
    }
}

//--------------------------------------
// REDUCTIONS
//--------------------------------------

static int reduce_active_roots( strict_fibonacci_heap *queue )
{
    fix_node *head = queue->fix_list[STRICT_FIX_ROOT];
    if( head == NULL )
        return 0;

    rank_record *rank = head->rank;
    if( head == rank->tail[STRICT_FIX_ROOT] )
        return 0;

    fix_node *next = head->right;
    strict_fibonacci_node *parent, *child;
    choose_order_pair( head->node, next->node, &parent, &child );

    link( queue, parent, child );
    convert_root_to_active( queue, child );
    increase_rank( queue, parent );

    strict_fibonacci_node *extra = parent->left_child->left;
    if( !is_active( queue, extra ) )
        link( queue, queue->root, extra );

    return 1;
}

static int reduce_root_degree( strict_fibonacci_heap *queue )
{
    if( queue->root == NULL || queue->root->left_child == NULL )
        return 0;

    strict_fibonacci_node *x = queue->root->left_child->left;
    if( x == queue->root->left_child || is_active( queue, x ) )
        return 0;

    strict_fibonacci_node *y = x->left;
    if( y == queue->root->left_child || is_active( queue, y ) )
        return 0;

    strict_fibonacci_node *z = y->left;
    if( z == queue->root->left_child || is_active( queue, z ) )
        return 0;

    strict_fibonacci_node *grand, *parent, *child;
    choose_order_triple( x, y, z, &grand, &parent, &child );

    convert_passive_to_active( queue, parent );
    convert_passive_to_active( queue, grand );
    increase_rank( queue, grand );
    if( !is_active( queue, queue->root ) )
        convert_active_to_root( queue, grand );

    link( queue, parent, child );
    link( queue, grand, parent );

    return 1;
}

static int reduce_loss( strict_fibonacci_heap *queue )
{
    int reduction = 2;
    fix_node *head = queue->fix_list[STRICT_FIX_LOSS];
    if( head == NULL )
        return 0;

    rank_record *rank = head->rank;
    if( head == rank->tail[STRICT_FIX_LOSS] && head->node->loss < 2 )
        return 0;

    fix_node *single;
    fix_node *next = head->right;
    strict_fibonacci_node *child, *parent, *old_parent;
    if( head->node->loss > 1 )
    {
        reduction = 1;
        single = head;
    }
    else if( next->node->loss > 1 )
    {
        reduction = 1;
        single = next;
    }

    if( reduction == 1 )
    {
        child = single->node;
        parent = queue->root;
        old_parent = child->parent;

        if( child != parent )
            link( queue, parent, child );

        decrease_loss( queue, child );

        if( !is_active( queue, parent ) )
            convert_active_to_root( queue, child );
    }
    else
    {
        choose_order_pair( head->node, next->node, &parent, &child );
        old_parent = child->parent;

        link( queue, parent, child );

        decrease_loss( queue, child );
        decrease_loss( queue, parent );
        increase_rank( queue, parent );
    }

    if( is_active( queue, old_parent ) )
    {
        decrease_rank( queue, old_parent );
        if( is_active( queue, old_parent ) &&
                old_parent->type != STRICT_TYPE_ROOT )
            increase_loss( queue, old_parent );
    }

    return 1;
}

static void post_meld_reduction( strict_fibonacci_heap *queue )
{
    int count_root = 0;
    int count_degree = 0;

    reduce_loss( queue );

    while( count_root < 1 && count_degree < 1 )
    {
        if( count_root < 1 && reduce_active_roots( queue ) )
            count_root++;
        else if( count_degree < 1 && reduce_root_degree( queue ) )
            count_degree++;
        else
            break;
    }
}

static void post_delete_min_reduction( strict_fibonacci_heap *queue )
{
    while( 1 )
    {
        if( reduce_active_roots( queue ) )
            continue;
        else if( reduce_root_degree( queue ) )
            continue;
        else
            break;
    }
}

static void post_decrease_key_reduction( strict_fibonacci_heap *queue )
{
    int count_root = 0;
    int count_degree = 0;

    reduce_loss( queue );

    while( count_root < 6 && count_degree < 4 )
    {
        if( count_root < 6 && reduce_active_roots( queue ) )
            count_root++;
        else if( count_degree < 4 && reduce_root_degree( queue ) )
            count_degree++;
        else
            break;
    }
}

//--------------------------------------
// GARBAGE COLLECTION & ALLOCATION
//--------------------------------------

static rank_record* create_rank_record( strict_fibonacci_heap *queue,
    uint32_t rank, rank_record *succ )
{
    rank_record *pred;
    rank_record *new_rank = pq_alloc_node( queue->map, STRICT_NODE_RANK );
    new_rank->rank = rank;
    new_rank->inc = new_rank;
    new_rank->dec = new_rank;

    if( succ == NULL )
        queue->rank_list = new_rank;
    else
    {
        pred = succ->dec;

        new_rank->inc = succ;
        new_rank->dec = pred;

        succ->dec = new_rank;
        pred->inc = new_rank;
    }

    return new_rank;
}

static void release_active_record( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    node->active->ref_count--;
    if( node->active->ref_count == 0 )
    {
        if( node->active == queue->active )
            queue->active = NULL;
        pq_free_node( queue->map, STRICT_NODE_ACTIVE, node->active );
    }
    node->active = NULL;
}

static void release_rank_record( strict_fibonacci_heap *queue,
    strict_fibonacci_node *node )
{
    rank_record *rank = node->rank;
    rank->ref_count--;
    if( rank->ref_count == 0 )
    {
        if( rank->inc == rank )
        {
            if( queue->rank_list == rank )
                queue->rank_list = NULL;
        }
        else
        {
            if( queue->rank_list == rank )
                queue->rank_list = rank->inc;

            rank->inc->dec = rank->dec;
            rank->dec->inc = rank->inc;
        }

        pq_free_node( queue->map, STRICT_NODE_RANK, rank );
    }
    node->rank = NULL;
}

static void release_to_garbage_collector( strict_fibonacci_heap *queue,
    strict_fibonacci_heap *garbage_queue )
{
    int i;
    fix_node *tail, *head, *g_tail, *g_head;

    for( i = 0; i < 2; i++ )
    {
        if( garbage_queue->fix_list[i] != NULL )
        {
            if( queue->garbage_fix == NULL )
                queue->garbage_fix = garbage_queue->fix_list[i];
            else
            {
                head = queue->garbage_fix;
                tail = head->left;
                g_head = garbage_queue->fix_list[i];
                g_tail = g_head->left;

                head->left = g_tail;
                tail->right = g_head;
                g_tail->right = head;
                g_head->left = tail;
            }
        }
    }
}

static void garbage_collection( strict_fibonacci_heap *queue )
{
    fix_node *fix;

    if( queue->garbage_fix != NULL )
    {
        fix = queue->garbage_fix;
        if( fix->right == fix )
            queue->garbage_fix = NULL;
        else
        {
            queue->garbage_fix = fix->right;
            fix->right->left = fix->left;
            fix->left->right = fix->right;
        }
    }
}
