#include "rank_relaxed_weak_queue.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================

#define OCCUPIED(a,b)       ( a & ( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_SET(a,b)   ( a |= ( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_UNSET(a,b) ( a &= ~( ( (uint64_t) 1 ) << b ) )
#define REGISTRY_LEADER(a)  ( (uint32_t) ( 63 - __builtin_clzll( a ) ) )

static void register_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node );
static void unregister_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node );

static void insert_root( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *new_root );
static void restore_invariants( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );
static rank_relaxed_weak_node* join( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b );

static inline rank_relaxed_weak_node* get_distinguished_parent(
    rank_relaxed_weak_node *node );
static void swap_with_distinquished_parent( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node );

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

static inline void print_relations( rank_relaxed_weak_node *node );

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
    //printf("\tDeleting node %d\n",old_min->item);

    unregister_node( queue, MARKS, old_min );

    //printf("\t\tSwapping to root\n");
    while( old_min->parent != NULL )
        swap_with_distinquished_parent( queue, old_min );

    unregister_node( queue, ROOTS, old_min );
    unregister_node( queue, MARKS, old_min );

    //printf("\t\tSevering spine\n");
    sever_spine( queue, old_min->right );
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

static void register_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node )
{
    //printf("\t\t\t\t\t\tRegistering node %d (%d) as %d\n",node->item,node->rank,type);
    print_relations( node );
    if( !OCCUPIED( queue->registry[type], node->rank ) )
    {
        REGISTRY_SET( queue->registry[type], node->rank );
        queue->nodes[type][node->rank] = node;
    }
}

static void unregister_node( rank_relaxed_weak_queue *queue, int type,
    rank_relaxed_weak_node *node )
{
    //printf("\t\t\t\t\t\tUnregistering node %d (%d) as %d\n",node->item,node->rank,type);
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
    //printf("\tInserting node %d with rank %d\n",new_root->item,rank);
    while( OCCUPIED( queue->registry[ROOTS], rank ) )
    {
        //printf("\t\tAlready node %d at rank %d\n",queue->nodes[ROOTS][rank]->item,rank);
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

    //printf("\tCleaning marks starting from %d\n",new_mark->item);

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

        if( new_mark->parent->left == new_mark )
        {
            // check if node's sibling is marked
            if( new_mark->parent->right != NULL &&
                new_mark->parent->right->marked )
            {
                new_mark = transformation_sibling( queue, new_mark );

                continue;
            }

            // node is a left child and all neighbors unmarked
            if( !new_mark->parent->marked &&
                    ( new_mark->left == NULL || !new_mark->left->marked ) &&
                    ( new_mark->right == NULL || !new_mark->right->marked  ) )
                new_mark = transformation_cleaning( queue, new_mark );
        }

        // node is a right child and parent unmarked
        if( new_mark->parent->right == new_mark && !new_mark->parent->marked )
        {
            if( OCCUPIED( queue->registry[MARKS], new_mark->rank ) )
            {
                new_mark = transformation_pair( queue, new_mark );

                continue;
            }

            break;
        }

        // node is a left child and parent marked
        if( new_mark->parent->left == new_mark && new_mark->parent->marked )
        {
            new_mark = transformation_zigzag( queue, new_mark );
            if( new_mark != NULL )
                continue;

            break;
        }

        // node is a right child and parent marked
        if( new_mark->parent->right == new_mark && new_mark->parent->marked )
        {
            new_mark = transformation_parent( queue, new_mark );

            continue;
        }

        break;
    }

    if( new_mark != NULL && new_mark->marked )
        register_node( queue, MARKS, new_mark );

    //printf("\tDone cleaning marks\n");
}

static rank_relaxed_weak_node* join( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b )
{
    //if( a->rank != b->rank )
    //    printf("JOINING NODES OF DIFFERING RANK\n");
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
    print_relations( parent );
    print_relations( child );

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

static inline rank_relaxed_weak_node* get_distinguished_parent(
    rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *current = node;
    print_relations( node );
    while( current->parent != NULL && current->parent->right != current )
    {
        current = current->parent;
        print_relations( current );
    }

    return current->parent;
}

static void swap_with_distinquished_parent( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *node )
{
    rank_relaxed_weak_node *dist = get_distinguished_parent( node );
    //printf("\tSwapping %d with d.p. %d\n",node->item,dist->item);
    if( dist == node->parent )
        swap_parent_with_right_child( queue, dist, node );
    else
        swap_disconnected( queue, dist, node );

    if( dist->marked )
        restore_invariants( queue, dist );
}

static void swap_parent_with_right_child( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *parent, rank_relaxed_weak_node *child )
{
    rank_relaxed_weak_node *parent_parent = parent->parent;
    rank_relaxed_weak_node *parent_left = parent->left;
    rank_relaxed_weak_node *child_left = child->left;
    rank_relaxed_weak_node *child_right = child->right;

    //printf("\t\tSwapping p. %d with r. %d\n",parent->item,child->item);
    print_relations( parent );
    print_relations( child );

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
    print_relations( parent );
    print_relations( child );
}

static void swap_parent_with_left_child( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *parent, rank_relaxed_weak_node *child )
{
    rank_relaxed_weak_node *parent_parent = parent->parent;
    rank_relaxed_weak_node *parent_right = parent->right;
    rank_relaxed_weak_node *child_left = child->left;
    rank_relaxed_weak_node *child_right = child->right;

    //printf("\t\tSwapping p. %d with l. %d\n",parent->item,child->item);
    print_relations( parent );
    print_relations( child );

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
    print_relations( parent );
    print_relations( child );
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

    //printf("\t\tSwapping a. %d with b. %d\n",a->item,b->item);
    print_relations( a );
    print_relations( b );

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
    print_relations( a );
    print_relations( b );
}

static void switch_node_ranks( rank_relaxed_weak_queue *queue,
    rank_relaxed_weak_node *a, rank_relaxed_weak_node *b )
{
    int a_unrooted = ( queue->nodes[ROOTS][a->rank] == a );
    int b_unrooted = ( queue->nodes[ROOTS][b->rank] == b );

    unregister_node( queue, ROOTS, a );
    unregister_node( queue, ROOTS, b );
    unregister_node( queue, MARKS, a );
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
    //printf("\t\tCleaning transformation\n");
    rank_relaxed_weak_node *sibling = node->parent->right;
    flip_subtree( node->parent );

    if( sibling != NULL )
        swap_subtrees( node, &(node->left), sibling, &(sibling->left) );
    else
    {
        if( node->left != NULL )
            node->left->parent = node->parent;
        node->parent->left = node->left;
        node->left = NULL;
    }
    //printf("\t\tDone with cleaning transformation\n");

    return node;
}

static rank_relaxed_weak_node* transformation_pair(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    //printf("\t\tPair transformation\n");
    rank_relaxed_weak_node *match = queue->nodes[MARKS][node->rank];
    rank_relaxed_weak_node *match_parent = match->parent;
    rank_relaxed_weak_node *node_parent = node->parent;
    //if( node->rank != match->rank )
    //    printf("TRANSFORMING NODES OF DIFFERING RANK\n");

    print_relations( node );
    print_relations( match );
    print_relations( node_parent );
    print_relations( match_parent );

    unregister_node( queue, MARKS, match );

    swap_disconnected( queue, node, match_parent );
    swap_subtrees( match_parent, &(match_parent->right), match, &(match->left) );

    if( match_parent->key < node_parent->key )
    {
        swap_parent_with_right_child( queue, node_parent, match_parent );
        flip_subtree( node_parent );
    }

    rank_relaxed_weak_node *result = node;
    rank_relaxed_weak_node *unmarked = match;
    if( match->key < node->key )
    {
        swap_parent_with_right_child( queue, node, match );
        flip_subtree( node );
        result = match;
        unmarked = node;
    }
    unregister_node( queue, MARKS, unmarked );
    unmarked->marked = 0;

    //printf("\t\tDone with pair transformation\n");
    return result;
}

static rank_relaxed_weak_node* transformation_parent(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    //printf("\t\tParent transformation\n");
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

    //printf("\t\tDone with parent transformation\n");
    return result;
}

static rank_relaxed_weak_node* transformation_sibling(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    //printf("\t\tSibling transformation\n");
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

    //printf("\t\tDone with sibling transformation\n");
    return result;
}

static rank_relaxed_weak_node* transformation_zigzag(
    rank_relaxed_weak_queue *queue, rank_relaxed_weak_node *node )
{
    //printf("\t\tZigzag transformation\n");
    rank_relaxed_weak_node *new_mark, *result;
    rank_relaxed_weak_node *right_tree = node->parent;
    rank_relaxed_weak_node *left_tree = right_tree->parent;
    rank_relaxed_weak_node *insertion_parent = left_tree->parent;
    int insert_left = ( insertion_parent != NULL &&
        insertion_parent->left == left_tree );

    left_tree->right = node;
    node->parent = left_tree;
    right_tree->left = NULL;
    right_tree->parent = NULL;
    unregister_node( queue, ROOTS, left_tree );
    unregister_node( queue, MARKS, left_tree );
    left_tree->rank--;

    new_mark = transformation_parent( queue, node );

    if( new_mark != NULL )
        left_tree = new_mark;
    result = join( queue, left_tree, right_tree );

    if( insertion_parent == NULL )
        insert_root( queue, result );
    else
    {
        result->parent = insertion_parent;
        if( insert_left )
            insertion_parent->left = result;
        else
            insertion_parent->right = result;
    }

    if( insert_left && result->marked  )
    {
        new_mark = result;
        if( new_mark->parent->right && new_mark->parent->right->marked )
            new_mark = transformation_sibling( queue, new_mark );
        else
            new_mark = transformation_cleaning( queue, new_mark );
    }

    //printf("\t\tDone with zigzag transformation\n");
    return new_mark;
}

static inline void print_relations( rank_relaxed_weak_node *node )
{
    if( node == NULL )
        return;
    //printf("\t\t\tRelation of %d (%d): %d (%d), %d (%d), %d (%d)\n",node->item,node->rank,!node->parent?0:node->parent->item,!node->parent?0:node->parent->rank,!node->left?0:node->left->item,!node->left?0:node->left->rank,!node->right?0:node->right->item,!node->right?0:node->right->rank);
}
