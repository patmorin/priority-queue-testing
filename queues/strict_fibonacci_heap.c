#include "strict_fibonacci_heap.h"

//==============================================================================
// STATIC DECLARATIONS
//==============================================================================



//==============================================================================
// PUBLIC METHODS
//==============================================================================

strict_fibonacci_heap* pq_create( mem_map *map )
{
    strict_fibonacci_heap *queue = (strict_fibonacci_heap*) calloc( 1,
        sizeof( strict_fibonacci_heap ) );
    queue->map = map;

    queue->active = pq_alloc_node( map, STRICT_NODE_ACTIVE );
    queue->active->flag = 1;
    
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
    queue->non_linkable_child = NULL;
    queue->q_head = NULL;
    
    queue->active = NULL;
    queue->rank_list = NULL;
    queue->fix_list = NULL;
    queue->singles = NULL;
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
/*    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->right = wrapper;
    wrapper->left = wrapper;

    strict_fibonacci_node *parent, *child;
    if( queue->root == NULL )
        queue->root = wrapper;
    else
    {
        if( wrapper->key < queue->root->key )
        {
            parent = wrapper;
            child = queue->root;
        }
        else
        {
            parent = queue->root;
            child = wrapper;
        }

        link( queue, parent, child );
        queue->root = parent;

        child->q_next = child;
        child->q_prev = child;
        enqueue_node( queue, child );

        post_meld_reduction( queue );
    }
    
    queue->size++;
    garbage_collection( queue );
*/
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
/*    strict_fibonacci_node *current, *new_root, *old_root, *next, *prev, *end;
    int i, j;

    old_root = queue->root;
    
    if( old_root->left_child == NULL )
    {
        //printf("No children.\n");
        old_root = queue->root;
        make_passive( queue, old_root );
        queue->root = NULL;
        queue->non_linkable_child = NULL;
    }
    else
    {
        //printf("Finding minimum child.\n");
        new_root = old_root->left_child;
        current = new_root->right;
        while( current != old_root->left_child )
        {
            if( current->item->key < new_root->item->key )
                new_root = current;
            current = current->right;
        }

        //printf("Making new root passive.\n");
        if( new_root->active != NULL && new_root->active->flag )
            make_passive( queue, new_root );

        //printf("Pulling new root out of list.\n");
        next = new_root->right;
        prev = new_root->left;
        if( old_root->left_child == new_root )
        {
            if( next == new_root )
                old_root->left_child = NULL;
            else
                old_root->left_child = next;
        }
        prev->right = next;
        next->left = prev;
        new_root->right = new_root;
        new_root->left = new_root;
        new_root->parent = NULL;
        
        //printf("Moving linkable passive children of the new root to the right.\n");
        if( new_root->left_child != NULL )
        {
            end = new_root->left_child->left;
            current = end->right;
            while( current != end )
            {
                next = current->right;
                if( is_linkable( current ) )
                    move_to_right( current );
                current = next;
            }

            //printf("Setting non-linkable child.\n");
            current = new_root->left_child->left;
            while( is_linkable( current ) && current != new_root->left_child )
                current = current->left;
            queue->non_linkable_child = current;
        }
        else
            queue->non_linkable_child = NULL;

        //printf("Dequeueing new root and setting root.\n");
        dequeue_node( queue, new_root );
        queue->root = new_root;
        
        //printf("Linking old root's children.\n");
        while( old_root->left_child != NULL )
            link( queue, new_root, old_root->left_child );

        //printf("Releasing old root.\n");
        make_passive( queue, old_root );

        //printf("Linking queued nodes.\n");
        for( i = 0; i < 2; i++ )
        {
            current = queue->q_head;
            queue->q_head = current->q_next;
            for( j = 0; j < 2; j++ )
            {
                if( current->left_child != NULL &&
                        !is_active( current->left_child->left ) )
                    link( queue, new_root, current->left_child->left );
            }
        } 
    }

    pq_free_node( queue->map, STRICT_TYPE_ITEM, old_root->item );
    pq_free_node( queue->map, STRICT_TYPE_FIB_NODE, old_root );
    
    //printf("Reduction time.\n");
    post_delete_min_reduction( queue );
    //printf("Garbage collection.\n");
    garbage_collection( queue );
*/    
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
    
/*    node->key = new_key;

    if( old_parent == NULL )
        return;

    if( node->key <= queue->root->key )
    {
        tmp_item = node->item;
        node->item = queue->root->item;
        queue->root->item = tmp_item;
        node->item->node = node;
        queue->root->item->node = queue->root;
    }
     //TODO fix root swapping?

    link( queue, queue->root, node );
    if( is_active( node ) )
    {
        if( is_active( old_parent ) )
        {
            decrease_rank( queue, old_parent );
            if( !is_active_root( old_parent ) )
                increase_loss( queue, old_parent );
            make_active_root( queue, node );
        }
    }
        
    post_decrease_key_reduction( queue );
    garbage_collection( queue );
*/
}

strict_fibonacci_heap* pq_meld( strict_fibonacci_heap *a,
    strict_fibonacci_heap *b )
{
    strict_fibonacci_heap *new_heap = pq_create( a->map );
/*    strict_fibonacci_heap *big, *small;

    strict_fibonacci_node *big_head, *big_tail, *small_head, *small_tail;
    strict_fibonacci_node *parent, *child;

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

    new_heap->size = big->size + small->size;
    new_heap->q_head = big->q_head;
    new_heap->active = big->active;
    new_heap->rank_list = big->rank_list;
    new_heap->fix_list = big->fix_list;
    new_heap->singles = big->singles;

    if( small->active != NULL )
        small->active->flag = 0;

    if( big->root->item->key < small->root->item->key )
    {
        parent = big->root;
        child = small->root;
        new_heap->root = big->root;
        new_heap->non_linkable_child = big->non_linkable_child;
    }
    else
    {
        parent = small->root;
        child = big->root;
        new_heap->root = small->root;
        new_heap->non_linkable_child = small->non_linkable_child;
    }

    link( new_heap, parent, child );
    enqueue_node( new_heap, child );

    big_head = big->q_head;
    big_tail = big_head->q_prev;
    small_head = small->q_head;
    small_tail = small_head->q_prev;

    big_head->q_prev = small_tail;
    small_tail->q_next = big_head;
    small_head->q_prev = big_tail;
    big_tail->q_next = small_head;

    release_to_garbage_collector( new_heap, small );
    free( small );
    free( big );

    garbage_collection( new_heap );
*/
    return new_heap;
}

bool pq_empty( strict_fibonacci_heap *queue )
{
    return ( queue->size == 0 );
}

//==============================================================================
// STATIC METHODS
//==============================================================================

