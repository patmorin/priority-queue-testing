#include "fibonacci_heap.h"

fibonacci_heap* create_heap() {
    fibonacci_heap *heap = (fibonacci_heap*) calloc( 1, sizeof( fibonacci_heap ) );
    heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );
    return heap;
}

void destroy_heap( fibonacci_heap *heap ) {
    clear_heap( heap );
    free( heap->stats );
    free( heap );
}

void clear_heap( fibonacci_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

uint32_t get_key( fibonacci_node *node ) {
    return node->key;
}

void* get_item( fibonacci_node *node ) {
    return node->item;
}

uint32_t get_size( fibonacci_heap *heap ) {
    return heap->size;
}

fibonacci_node* insert( fibonacci_heap *heap, void* item, uint32_t key ) {
    heap->stats->count_insert++;
    
    fibonacci_node* wrapper = (fibonacci_node*) calloc( 1, sizeof( fibonacci_node ) );
    wrapper->item = item;
    wrapper->key = key;
    wrapper->next_sibling = wrapper;
    wrapper->prev_sibling = wrapper;
    heap->size++;
    if ( heap->size > heap->stats->max_size )
        heap->stats->max_size = heap->size;

    merge_roots( heap, heap->minimum, wrapper );

    return wrapper;
}

void* find_min( fibonacci_heap *heap ) {
    heap->stats->count_find_min++;
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum->item;
}

void* delete_min( fibonacci_heap *heap ) {
    heap->stats->count_delete_min++;
    
    if ( empty( heap ) )
        return NULL;
    return delete( heap, heap->minimum );
}

void* delete( fibonacci_heap *heap, fibonacci_node *node ) {
    heap->stats->count_delete++;
    
    void *item = node->item;
    fibonacci_node *child = node->first_child;

    // remove from sibling list
    node->next_sibling->prev_sibling = node->prev_sibling;
    node->prev_sibling->next_sibling = node->next_sibling;

    if ( node->parent != NULL ) {
        node->parent->rank--;
        // if not a root, see if we need to update parent's first child
        if ( node->parent->first_child == node ) {
            if ( node->parent->rank == 0 )
                node->parent->first_child = NULL;
            else
                node->parent->first_child = node->next_sibling;
        }                
        if ( node->parent->marked == FALSE )
            node->parent->marked = TRUE;
        else
            cut_from_parent( heap, node->parent );
    }
    else if ( node == heap-> minimum ) {
        // if node was minimum, find new temporary minimum
        if ( node->next_sibling != node )
            heap->minimum = node->next_sibling;
        else
            heap->minimum = node->first_child;
    }

    node->parent = NULL;
    free( node );
    heap->size--;

    merge_roots( heap, heap->minimum, child );
    
    return item;
}

void decrease_key( fibonacci_heap *heap, fibonacci_node *node, uint32_t delta ) {
    heap->stats->count_decrease_key++;

    node->key -= delta;
    cut_from_parent( heap, node );
}

void meld( fibonacci_heap *heap, fibonacci_heap *other_heap ) {
    heap->stats->count_meld++;
    
    int i;
    
    merge_roots( heap, heap->minimum, other_heap->minimum );

    heap->size += other_heap->size;
    other_heap->size = 0;
    other_heap->minimum = NULL;
    for ( i = 0; i < MAXRANK; i++ )
        other_heap->roots[i] = NULL;
}

bool empty( fibonacci_heap *heap ) {
    return ( heap->size == 0 );
}

void merge_roots( fibonacci_heap *heap, fibonacci_node *a, fibonacci_node *b ) {
    // start of list
    fibonacci_node *start = append_lists( a, b );
    // current tree to try and place
    fibonacci_node *current;
    // holder for newly linked tree
    fibonacci_node *linked;
    uint32_t i, rank;

    // clear array to insert into for rank comparisons
    for ( i = 0; i < MAXRANK; i++ )
        heap->roots[i] = NULL;

    if ( start == NULL )
        return;

    // insert an initial node
    heap->roots[start->rank] = start;
    start->parent = NULL;
    current = start->next_sibling;
    // insert the rest of the nodes
    while( current != start ) {
        current->parent = NULL;
        while ( ! attempt_insert( heap, current ) ) {
            rank = current->rank;
            linked = link( current, heap->roots[rank] );
            // when two trees get linked, we're not sure which one is
            // the new root, so we have to check everything again
            start = linked;
            current = linked;
            heap->roots[rank] = NULL;
        }
        current = current->next_sibling;
    }

    set_min( heap );
}

fibonacci_node* link( fibonacci_node *a, fibonacci_node *b ) {
    fibonacci_node *parent;
    fibonacci_node *child;
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    child->prev_sibling->next_sibling = child->next_sibling;
    child->next_sibling->prev_sibling = child->prev_sibling;
    child->prev_sibling = child;
    child->next_sibling = child;

    // roots are automatically unmarked
    child->marked = FALSE;
    child->parent = parent;
    parent->first_child = append_lists( parent->first_child, child );
    parent->rank++;

    return parent;
}

void cut_from_parent( fibonacci_heap *heap, fibonacci_node *node ) {
    fibonacci_node* next;
    fibonacci_node* prev;
    if ( node->parent != NULL ) {
        next = node->next_sibling;
        prev = node->prev_sibling;
        
        next->prev_sibling = node->prev_sibling;
        prev->next_sibling = node->next_sibling;
            
        node->next_sibling = node;
        node->prev_sibling = node;

        node->parent->rank--;
        if ( node->parent->first_child == node ) {
            if ( node->parent->rank == 0 )
                node->parent->first_child = NULL;
            else
                node->parent->first_child = next;
        }                
        if ( node->parent->marked == FALSE )
            node->parent->marked = TRUE;
        else
            cut_from_parent( heap, node->parent );
            
        merge_roots( heap, node, heap->minimum );
    }
}

fibonacci_node* append_lists( fibonacci_node *a, fibonacci_node *b ) {
    fibonacci_node *list;
    fibonacci_node *a_prev;
    fibonacci_node *b_prev;
    
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

        list = a;
    }

    return list;
}

bool attempt_insert( fibonacci_heap *heap, fibonacci_node *node ) {
    fibonacci_node *occupant = heap->roots[node->rank];
    if ( ( occupant != NULL ) && ( occupant != node ) )
        return FALSE;
        
    heap->roots[node->rank] = node;

    return TRUE;
}

void set_min( fibonacci_heap *heap ) {
    uint32_t i;
    heap->minimum = NULL;
    for ( i = 0; i < MAXRANK; i++ ) {
        if ( heap->roots[i] == NULL )
            continue;
        if ( ( heap->minimum == NULL ) ||
                ( heap->roots[i]->key < heap->minimum->key ) )
            heap->minimum = heap->roots[i];
    }
}
