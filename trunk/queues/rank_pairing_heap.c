#include "rank_pairing_heap.h"

rank_pairing_heap* create_heap() {
    rank_pairing_heap *heap = (rank_pairing_heap*) calloc( 1, sizeof( rank_pairing_heap ) );
    heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );
    return heap;
}

void destroy_heap( rank_pairing_heap *heap ) {
    clear_heap( heap );
    free( heap->stats );
    free( heap );
}

void clear_heap( rank_pairing_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

uint32_t get_key( rank_pairing_node *node ) {
    return node->key;
}

void* get_item( rank_pairing_node *node ) {
    return node->item;
}

uint32_t get_size( rank_pairing_heap *heap ) {
    return heap->size;
}

rank_pairing_node* insert( rank_pairing_heap *heap, void* item, uint32_t key ) {
    INCR_INSERT
    
    rank_pairing_node *wrapper = (rank_pairing_node*) calloc( 1, sizeof( rank_pairing_node ) );
    wrapper->item = item;
    wrapper->key = key;
    wrapper->right = wrapper;
    heap->size++;
    if ( heap->size > heap->stats->max_size )
        heap->stats->max_size = heap->size;

    merge_roots( heap, heap->minimum, wrapper );

    if ( ( heap->minimum == NULL ) || ( key < heap->minimum->key ) )
        heap->minimum = wrapper;
    
    return wrapper;
}

void* find_min( rank_pairing_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum->item;
}

void* delete_min( rank_pairing_heap *heap ) {
    INCR_DELETE_MIN
    
    if ( empty( heap ) )
        return NULL;
    return delete( heap, heap->minimum );
}

void* delete( rank_pairing_heap *heap, rank_pairing_node *node ) {
    INCR_DELETE

    rank_pairing_node *old_min;
    rank_pairing_node *left_list;
    rank_pairing_node *right_list;
    rank_pairing_node *full_list;
    rank_pairing_node *current;
    if ( node == NULL )
        return NULL;
    void* item = node->item;

    if ( node->parent != NULL ) {
        if ( node->parent->right == node )
            node->parent->right = NULL;
        else
            node->parent->left = NULL;
    }
    else {
        current = node;
        while ( current->right != node )
            current = current->right;
        current->right = node->right;
    }

    left_list = ( node->left != NULL ) ? sever_spine( node->left ) : NULL;
    right_list = ( ( node->parent != NULL ) && ( node->right != NULL ) ) ?
        sever_spine( node->right ) : NULL;
    merge_lists( left_list, right_list );
    full_list = pick_min( left_list, right_list );

    if ( heap->minimum == node )
        heap->minimum = ( node->right == node ) ? full_list : node->right;

    // in order to guarantee linking complies with analysis we save the
    // original minimum so that we perform a one-pass link on the new
    // trees before we do general multi-pass linking
    old_min = heap->minimum;
    merge_roots( heap, heap->minimum, full_list );
    heap->minimum = old_min;
    fix_roots( heap );                

    free( node );
    heap->size--;

    return item;
}

void decrease_key( rank_pairing_heap *heap, rank_pairing_node *node, uint32_t delta ) {
    INCR_DECREASE_KEY

    node->key -= delta;
    if ( node->parent != NULL ) {
        if ( node->parent->right == node )
            node->parent->right = node->right;
        else
            node->parent->left = node->right;
        if ( node->right != NULL ) {
            node->right->parent = node->parent;
            node->right = NULL;
        }

        propagate_ranks( node );
        node->parent = NULL;
        node->right = node;
        merge_roots( heap, heap->minimum, node );
    }
    else if ( node->key < heap->minimum->key )
        heap->minimum = node;
}

void meld( rank_pairing_heap *heap, rank_pairing_heap *other_heap ) {
    INCR_MELD
    
    int i;
    
    merge_roots( heap, heap->minimum, other_heap->minimum );

    heap->size += other_heap->size;
    other_heap->size = 0;
    other_heap->minimum = NULL;
    for ( i = 0; i < MAXRANK; i++ )
        other_heap->roots[i] = NULL;
}

bool empty( rank_pairing_heap *heap ) {
    return ( heap->size == 0 );
}

void merge_roots( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b ) {
    merge_lists( a, b );
    heap->minimum = pick_min( a, b );
}

rank_pairing_node* merge_lists( rank_pairing_node *a, rank_pairing_node *b ) {
    rank_pairing_node *temp;
    rank_pairing_node *list;
    if ( a == NULL )
        list = b;
    else if ( b == NULL )
        list = a;
    else if ( a == b )
        list = a;
    else {
        temp = a->right;
        a->right = b->right;
        b->right = temp;
        list = a;
    }

    return list;
}

rank_pairing_node* pick_min( rank_pairing_node *a, rank_pairing_node *b ) {
    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;
    else if ( b->key < a->key )
        return b;
    else
        return a;
}

rank_pairing_node* join( rank_pairing_node *a, rank_pairing_node *b ) {
    rank_pairing_node *parent;
    rank_pairing_node *child;
    
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    child->right = parent->left;
    if ( child->right != NULL )
        child->right->parent = child;
    parent->left = child;
    child->parent = parent;
    parent->rank++;

    return parent;
}

void fix_roots( rank_pairing_heap *heap ) {
    rank_pairing_node *output_head = NULL;
    rank_pairing_node *output_tail = NULL;
    rank_pairing_node *current;
    rank_pairing_node *next;
    rank_pairing_node *joined;
    uint32_t i, rank;

    if ( heap->minimum == NULL )
        return;

    current = heap->minimum->right;
    heap->minimum->right = NULL;
    while ( current != NULL ) {
        next = current->right;
        if ( ! attempt_insert( heap, current ) ) {
            rank = current->rank;
            // keep a running list of joined trees
            joined = join( current, heap->roots[rank] );
            if ( output_head != NULL )
                output_tail->right = joined;
            else
                output_head = joined;
            output_tail = joined;
            heap->roots[rank] = NULL;
        }
        current = next;
    }

    // move the untouched trees to the list and repair pointers
    for ( i = 0; i < MAXRANK; i++ ) {
        if ( heap->roots[i] != NULL ) {
            if ( output_head != NULL )
                output_tail->right = heap->roots[i];
            else 
                output_head = heap->roots[i];
            output_tail = heap->roots[i];
            heap->roots[i] = NULL;
        }
    }

    output_tail->right = output_head;

    heap->minimum = output_head;
    fix_min( heap );
}

bool attempt_insert( rank_pairing_heap *heap, rank_pairing_node *node ) {
    uint32_t rank = node->rank;
    if ( ( heap->roots[rank] != NULL ) && ( heap->roots[rank] != node ) )
        return FALSE;
    heap->roots[rank] = node;

    return TRUE;
}

void fix_min( rank_pairing_heap *heap ) {
    if ( heap->minimum == NULL )
        return;
    rank_pairing_node *start = heap->minimum;
    rank_pairing_node *current = heap->minimum->right;
    while ( current != start ) {
        if ( current->key < heap->minimum->key )
            heap->minimum = current;
        current = current->right;
    }
}

void propagate_ranks( rank_pairing_node *node ) {
    uint32_t k = 0;
    
    if ( node == NULL )
        return;

    if ( ( node->parent != NULL ) && ( node->left != NULL ) )
            k = node->left->rank + 1;
    else if ( node->left != NULL ) {
        if ( node->right != NULL ) {
            if ( node->left->rank == node->right->rank )
                k = node->left->rank + 1;
            else
                k = ( node->left->rank > node->right->rank ) ?
                        node->left->rank : node->right->rank;
        }
        else {
            k = node->left->rank;
        }
    }
    else if ( node->right != NULL )
        k = node->right->rank + 1;

    if ( node->rank >= k )
        node->rank = k;

    propagate_ranks( node->parent );
}

rank_pairing_node* sever_spine( rank_pairing_node *node ) {
    rank_pairing_node *current = node;
    while ( current->right != NULL ) {
        current->parent = NULL;
        current = current->right;
    }
    current->parent = NULL;
    current->right = node;

    return node;
}
