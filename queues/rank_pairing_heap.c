#include "rank_pairing_heap.h"

rank_pairing_heap* create_heap() {
    rank_pairing_heap *heap = (rank_pairing_heap*) calloc( 1, sizeof( rank_pairing_heap ) );
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( rank_pairing_heap ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
    return heap;
}

void destroy_heap( rank_pairing_heap *heap ) {
    clear_heap( heap );
    FREE_STATS
    free( heap );
}

void clear_heap( rank_pairing_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( rank_pairing_heap *heap, rank_pairing_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

item_type* get_item( rank_pairing_heap *heap, rank_pairing_node *node ) {
        ADD_TRAVERSALS(1) // node
    return (item_type*) &(node->item);
}

uint32_t get_size( rank_pairing_heap *heap ) {
    return heap->size;
}

rank_pairing_node* insert( rank_pairing_heap *heap, item_type item, uint32_t key ) {
    INCR_INSERT
    
    rank_pairing_node *wrapper = (rank_pairing_node*) calloc( 1, sizeof( rank_pairing_node ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( rank_pairing_node ) )
        ADD_TRAVERSALS(1) // wrapper
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->right = wrapper;
    heap->size++;
        ADD_UPDATES(4) // wrapper, heap
        FIX_MAX_NODES
    merge_roots( heap, heap->minimum, wrapper );

        ADD_TRAVERSALS(1) // heap->minimum
    if ( ( heap->minimum == NULL ) || ( key < heap->minimum->key ) )
        heap->minimum = wrapper;
    
    return wrapper;
}

rank_pairing_node* find_min( rank_pairing_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type delete_min( rank_pairing_heap *heap ) {
    INCR_DELETE_MIN
    
    return delete( heap, heap->minimum );
}

key_type delete( rank_pairing_heap *heap, rank_pairing_node *node ) {
    INCR_DELETE

    rank_pairing_node *old_min, *left_list, *right_list, *full_list, *current;
    key_type key = node->key;
        ADD_TRAVERSALS(1) // node

    if ( node->parent != NULL ) {
        if ( node->parent->right == node )
            node->parent->right = NULL;
        else
            node->parent->left = NULL;

            ADD_TRAVERSALS(1) // node->parent
            ADD_UPDATES(1) // node->parent
    }
    else {
        current = node;
            ADD_TRAVERSALS(1) // current
        while ( current->right != node ) {
            current = current->right;
                ADD_TRAVERSALS(1) // current
        }
        current->right = node->right;
            ADD_UPDATES(1) // current
    }

    left_list = ( node->left != NULL ) ? sever_spine( heap, node->left ) : NULL;
    right_list = ( ( node->parent != NULL ) && ( node->right != NULL ) ) ?
        sever_spine( heap, node->right ) : NULL;
    merge_lists( heap, left_list, right_list );
    full_list = pick_min( heap, left_list, right_list );

        ADD_TRAVERSALS(1) // heap->minimum
    if ( heap->minimum == node ) {
        heap->minimum = ( node->right == node ) ? full_list : node->right;
            ADD_UPDATES(1) // heap->minimum
    }

    // in order to guarantee linking complies with analysis we save the
    // original minimum so that we perform a one-pass link on the new
    // trees before we do general multi-pass linking
    old_min = heap->minimum;
    merge_roots( heap, heap->minimum, full_list );
    heap->minimum = old_min;
        ADD_UPDATES(1) // heap->minimum
    fix_roots( heap );                

    free( node );
        SUB_SIZE( sizeof( rank_pairing_node ) )
    heap->size--;
        ADD_UPDATES(1) // heap

    return key;
}

void decrease_key( rank_pairing_heap *heap, rank_pairing_node *node, key_type new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(1) // node
    if ( node->parent != NULL ) {
            ADD_TRAVERSALS(1) // node->parent
        if ( node->parent->right == node )
            node->parent->right = node->right;
        else
            node->parent->left = node->right;
            ADD_UPDATES(1) // node->parent
        if ( node->right != NULL ) {
            node->right->parent = node->parent;
            node->right = NULL;
                ADD_TRAVERSALS(1) // node->right
                ADD_UPDATES(1) // node->right, node
        }

        propagate_ranks( heap, node );
        node->parent = NULL;
        node->right = node;
            ADD_UPDATES(2) // node
        merge_roots( heap, heap->minimum, node );
    }
    else {
            ADD_TRAVERSALS(1) // heap->minimum
        if ( node->key < heap->minimum->key ) {
            heap->minimum = node;
                ADD_UPDATES(1) // heap->minimum
        }
    }
}

bool empty( rank_pairing_heap *heap ) {
    return ( heap->size == 0 );
}

void merge_roots( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b ) {
    merge_lists( heap, a, b );
    heap->minimum = pick_min( heap, a, b );
        ADD_TRAVERSALS(1) // heap->minimum
        ADD_UPDATES(1) // heap->minimum
}

rank_pairing_node* merge_lists( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b ) {
    rank_pairing_node *temp, *list;
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
            ADD_TRAVERSALS(2) // a, b
            ADD_UPDATES(2) // a, b
    }

    return list;
}

rank_pairing_node* pick_min( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b ) {
    if ( a == NULL )
        return b;
    else if ( b == NULL )
        return a;
    else if ( a == b )
        return a;
    else {
            ADD_TRAVERSALS(2) // a, b
        if ( b->key < a->key )
            return b;
        else
            return a;
    }
}

rank_pairing_node* join( rank_pairing_heap *heap, rank_pairing_node *a, rank_pairing_node *b ) {
    rank_pairing_node *parent, *child;

        ADD_TRAVERSALS(2) // a, b
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    child->right = parent->left;
        ADD_UPDATES(1) // child
    if ( child->right != NULL ) {
        child->right->parent = child;
            ADD_TRAVERSALS(1) // child->right
            ADD_UPDATES(1) // child->right
    }
    parent->left = child;
    child->parent = parent;
    parent->rank++;
        ADD_UPDATES(3) // parent, child

    return parent;
}

void fix_roots( rank_pairing_heap *heap ) {
    rank_pairing_node *output_head = NULL;
    rank_pairing_node *output_tail = NULL;
    rank_pairing_node *current, *next, *joined;
    uint32_t i, rank;

    if ( heap->minimum == NULL )
        return;

    heap->largest_rank = 0;
        ADD_UPDATES(1) // heap

    current = heap->minimum->right;
    heap->minimum->right = NULL;
        ADD_TRAVERSALS(1) // heap->minimum
        ADD_UPDATES(1) // heap->minimum
    while ( current != NULL ) {
        next = current->right;
            ADD_TRAVERSALS(1) // current
        if ( ! attempt_insert( heap, current ) ) {
            rank = current->rank;
            // keep a running list of joined trees
            joined = join( heap, current, heap->roots[rank] );
            if ( output_head == NULL )
                output_head = joined;
            else {
                output_tail->right = joined;
                    ADD_UPDATES(1) // output_tail
            }
            output_tail = joined;
            heap->roots[rank] = NULL;
                ADD_UPDATES(1) // output_tail
        }
        current = next;
    }

    // move the untouched trees to the list and repair pointers
    for ( i = 0; i <= heap->largest_rank; i++ ) {
        if ( heap->roots[i] != NULL ) {
            if ( output_head == NULL )
                output_head = heap->roots[i];
            else  {
                output_tail->right = heap->roots[i];
                    ADD_UPDATES(1) // output_tail
            }
            output_tail = heap->roots[i];
            heap->roots[i] = NULL;
                ADD_UPDATES(1) // heap
        }
    }

    output_tail->right = output_head;

    heap->minimum = output_head;
        ADD_UPDATES(1) // output_tail, heap
    fix_min( heap );
}

bool attempt_insert( rank_pairing_heap *heap, rank_pairing_node *node ) {
    uint32_t rank = node->rank;
    if ( ( heap->roots[rank] != NULL ) && ( heap->roots[rank] != node ) )
        return FALSE;
    heap->roots[rank] = node;
        ADD_UPDATES(1) // heap

    if ( rank > heap->largest_rank ) {
        heap->largest_rank = rank;
            ADD_UPDATES(1) // heap
    }

    return TRUE;
}

void fix_min( rank_pairing_heap *heap ) {
    if ( heap->minimum == NULL )
        return;
    rank_pairing_node *start = heap->minimum;
    rank_pairing_node *current = heap->minimum->right;
        ADD_TRAVERSALS(1) // heap->minimum
    while ( current != start ) {
            ADD_TRAVERSALS(1) // current
        if ( current->key < heap->minimum->key ) {
            heap->minimum = current;
                ADD_UPDATES(1) // heap
        }
        current = current->right;
            ADD_UPDATES(1) // current
    }
}

void propagate_ranks( rank_pairing_heap *heap, rank_pairing_node *node ) {
    uint32_t k = 0;
    
    if ( node == NULL )
        return;

        ADD_TRAVERSALS(2) // node, node->left,right
    if ( ( node->parent != NULL ) && ( node->left != NULL ) )
        k = node->left->rank + 1;
    else if ( node->left != NULL ) {
        if ( node->right != NULL ) {
            if ( node->left->rank == node->right->rank )
                k = node->left->rank + 1;
            else {
                    ADD_TRAVERSALS(1) // node->right
                k = ( node->left->rank > node->right->rank ) ?
                        node->left->rank : node->right->rank;
            }
        }
        else
            k = node->left->rank;
    }
    else if ( node->right != NULL ) {
        k = node->right->rank + 1;
    }

    if ( node->rank >= k ) {
        node->rank = k;
            ADD_UPDATES(1) // node
    }

    propagate_ranks( heap, node->parent );
}

rank_pairing_node* sever_spine( rank_pairing_heap *heap, rank_pairing_node *node ) {
    rank_pairing_node *current = node;
        ADD_TRAVERSALS(1) // current
    while ( current->right != NULL ) {
        current->parent = NULL;
        current = current->right;
            ADD_TRAVERSALS(1) // current
            ADD_UPDATES(1) // current
    }
    current->parent = NULL;
    current->right = node;
        ADD_UPDATES(2) // current

    return node;
}
