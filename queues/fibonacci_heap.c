#include "fibonacci_heap.h"

fibonacci_heap* create_heap() {
    fibonacci_heap *heap = (fibonacci_heap*) calloc( 1, sizeof( fibonacci_heap ) );
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( fibonacci_heap ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
    return heap;
}

void destroy_heap( fibonacci_heap *heap ) {
    clear_heap( heap );
    FREE_STATS
    free( heap );
}

void clear_heap( fibonacci_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( fibonacci_heap *heap, fibonacci_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

item_type* get_item( fibonacci_heap *heap, fibonacci_node *node ) {
        ADD_TRAVERSALS(1) // node
    return (item_type*) &(node->item);
}

uint32_t get_size( fibonacci_heap *heap ) {
    return heap->size;
}

fibonacci_node* insert( fibonacci_heap *heap, item_type item, key_type key ) {
    INCR_INSERT
    
    fibonacci_node* wrapper = (fibonacci_node*) calloc( 1, sizeof( fibonacci_node ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( fibonacci_node ) )
        ADD_TRAVERSALS(1) // wrapper
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->next_sibling = wrapper;
    wrapper->prev_sibling = wrapper;
    heap->size++;
        ADD_UPDATES(5) // wrapper, heap
        FIX_MAX_NODES

    merge_roots( heap, heap->minimum, wrapper );

    return wrapper;
}

fibonacci_node* find_min( fibonacci_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type delete_min( fibonacci_heap *heap ) {
    INCR_DELETE_MIN
    
    return delete( heap, heap->minimum );
}

key_type delete( fibonacci_heap *heap, fibonacci_node *node ) {
    INCR_DELETE
    
    key_type key = node->key;
    fibonacci_node *child = node->first_child;
        ADD_TRAVERSALS(1) // node

    // remove from sibling list
    node->next_sibling->prev_sibling = node->prev_sibling;
    node->prev_sibling->next_sibling = node->next_sibling;
        ADD_TRAVERSALS(2) // node->next,prev_sibling
        ADD_UPDATES(2) // node->next,prev_sibling

    if ( node->parent != NULL ) {
        node->parent->rank--;
            ADD_TRAVERSALS(1) // node->parent
            ADD_UPDATES(1) // node->parent
        // if not a root, see if we need to update parent's first child
        if ( node->parent->first_child == node ) {
            if ( node->parent->rank == 0 )
                node->parent->first_child = NULL;
            else
                node->parent->first_child = node->next_sibling;

                ADD_UPDATES(1) // node->parent->first_child
        }                
        if ( node->parent->marked == FALSE ) {
            node->parent->marked = TRUE;
                ADD_UPDATES(1) // node->parent->marked
        }
        else
            cut_from_parent( heap, node->parent );
    }
    else if ( node == heap-> minimum ) {
        // if node was minimum, find new temporary minimum
        if ( node->next_sibling != node )
            heap->minimum = node->next_sibling;
        else
            heap->minimum = child;

            ADD_UPDATES(1) // heap->minimum
    }

    free( node );
        SUB_SIZE( sizeof( fibonacci_node ) )
    heap->size--;

    merge_roots( heap, heap->minimum, child );
    
    return key;
}

void decrease_key( fibonacci_heap *heap, fibonacci_node *node, key_type new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
        ADD_UPDATES(1) // node
    cut_from_parent( heap, node );
}

bool empty( fibonacci_heap *heap ) {
    return ( heap->size == 0 );
}

void merge_roots( fibonacci_heap *heap, fibonacci_node *a, fibonacci_node *b ) {
    fibonacci_node *start = append_lists( heap, a, b );
    fibonacci_node *current, *linked;
    uint32_t i, rank;

    // clear array to insert into for rank comparisons
    for ( i = 0; i <= heap->largest_rank; i++ )
        heap->roots[i] = NULL;
        ADD_UPDATES(heap->largest_rank + 2)
    heap->largest_rank = 0;

    if ( start == NULL )
        return;

    // insert an initial node
    heap->roots[start->rank] = start;
    if ( start->rank > heap->largest_rank )
        heap->largest_rank = start->rank;
    start->parent = NULL;
    current = start->next_sibling;
        ADD_TRAVERSALS(2) // heap->roots, start
        ADD_UPDATES(2) // heap->roots, start
    // insert the rest of the nodes
    while( current != start ) {
        current->parent = NULL;
            ADD_TRAVERSALS(1) // current
        while ( ! attempt_insert( heap, current ) ) {
            rank = current->rank;
            linked = link( heap, current, heap->roots[rank] );
            // when two trees get linked, we're not sure which one is
            // the new root, so we have to check everything again
            start = linked;
            current = linked;
            heap->roots[rank] = NULL;
                ADD_UPDATES(1) // heap->roots
        }
        current = current->next_sibling;
    }

    set_min( heap );
}

fibonacci_node* link( fibonacci_heap *heap, fibonacci_node *a, fibonacci_node *b ) {
    fibonacci_node *parent, *child;
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }
        ADD_TRAVERSALS(2) // a,b

    child->prev_sibling->next_sibling = child->next_sibling;
    child->next_sibling->prev_sibling = child->prev_sibling;
        ADD_TRAVERSALS(2) // child->next,prev_sibling
    child->prev_sibling = child;
    child->next_sibling = child;
        ADD_UPDATES(4) // child->next,prev_sibling, child

    // roots are automatically unmarked
    child->marked = FALSE;
    child->parent = parent;
    parent->first_child = append_lists( heap, parent->first_child, child );
    parent->rank++;
        ADD_UPDATES(4) // child, parent

    return parent;
}

void cut_from_parent( fibonacci_heap *heap, fibonacci_node *node ) {
    fibonacci_node *next, *prev;
        ADD_TRAVERSALS(1) // node
    if ( node->parent != NULL ) {
        next = node->next_sibling;
        prev = node->prev_sibling;
        
        next->prev_sibling = node->prev_sibling;
        prev->next_sibling = node->next_sibling;
            ADD_TRAVERSALS(2) // next, prev
            ADD_UPDATES(2) // next, prev
            
        node->next_sibling = node;
        node->prev_sibling = node;
            ADD_UPDATES(2) // node

        node->parent->rank--;
            ADD_TRAVERSALS(1) //node->parent
            ADD_UPDATES(1) // node->parent
        if ( node->parent->first_child == node ) {
            if ( node->parent->rank == 0 )
                node->parent->first_child = NULL;
            else
                node->parent->first_child = next;
                ADD_UPDATES(1) // node->parent
        }                
        if ( node->parent->marked == FALSE ) {
            node->parent->marked = TRUE;
                ADD_UPDATES(1) //node->parent
        }
        else
            cut_from_parent( heap, node->parent );
            
        merge_roots( heap, node, heap->minimum );
    }
}

fibonacci_node* append_lists( fibonacci_heap *heap, fibonacci_node *a, fibonacci_node *b ) {
    fibonacci_node *list, *a_prev, *b_prev;
    
    if ( a == NULL )
        list = b;
    else if ( ( b == NULL ) || ( a == b ) )
        list = a;
    else {
        a_prev = a->prev_sibling;
        b_prev = b->prev_sibling;
        
        a_prev->next_sibling = b;
        b_prev->next_sibling = a;
        
        a->prev_sibling = b_prev;
        b->prev_sibling = a_prev;

            ADD_TRAVERSALS(4) // a, b, a_prev, b_prev
            ADD_UPDATES(4) // a, b, a_prev, b_prev

        list = a;
    }

    return list;
}

bool attempt_insert( fibonacci_heap *heap, fibonacci_node *node ) {
    uint32_t rank = node->rank;
    fibonacci_node *occupant = heap->roots[rank];
        ADD_TRAVERSALS(1) // node
    if ( ( occupant != NULL ) && ( occupant != node ) )
        return FALSE;
        
    heap->roots[rank] = node;
        ADD_UPDATES(1) // heap->roots
    if ( rank > heap->largest_rank )
        heap->largest_rank = rank;

    return TRUE;
}

void set_min( fibonacci_heap *heap ) {
    uint32_t i;
    heap->minimum = NULL;
        ADD_UPDATES(1) // heap
    for ( i = 0; i <= heap->largest_rank; i++ ) {
        if ( heap->roots[i] == NULL )
            continue;
            
            ADD_TRAVERSALS(2) // heap->roots[i], heap->minimum
        if ( ( heap->minimum == NULL ) ||
                ( heap->roots[i]->key < heap->minimum->key ) ) {
            heap->minimum = heap->roots[i];
                ADD_UPDATES(1) // heap->minimum
        }
    }
}
