#include "violation_heap.h"

violation_heap* create_heap() {
    violation_heap *heap = (violation_heap*) calloc( 1, sizeof( violation_heap ) );
    heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );
    return heap;
}

void destroy_heap( violation_heap *heap ) {
    clear_heap( heap );
    free( heap->stats );
    free( heap );
}

void clear_heap( violation_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

uint32_t get_key( violation_node *node ) {
    return node->key;
}

void* get_item( violation_node *node ) {
    return node->item;
}

uint32_t get_size( violation_heap *heap ) {
    return heap->size;
}

violation_node* insert( violation_heap *heap, void *item, uint32_t key ) {
    INCR_INSERT
    
    violation_node* wrapper = (violation_node*) calloc( 1, sizeof( violation_node ) );
    wrapper->item = item;
    wrapper->key = key;
    wrapper->next = wrapper;
    heap->size++;
    if ( heap->size > heap->stats->max_size )
        heap->stats->max_size = heap->size;

    merge_into_roots( heap, wrapper );

    if ( ( heap->minimum == NULL ) || ( key < heap->minimum->key ) )
        heap->minimum = wrapper;
    
    return wrapper;
}

void* find_min( violation_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum->item;
}

void* delete_min( violation_heap *heap ) {
    INCR_DELETE_MIN
    
    if ( empty( heap ) )
        return NULL;
    return delete( heap, heap->minimum );
}

void* delete( violation_heap *heap, violation_node *node ) {
    INCR_DELETE
    
    if ( node == NULL )
        return NULL;
    void *item = node->item;
    violation_node *prev;

    if ( get_parent( node ) == NULL ) {
        prev = find_prev_root( node );
        prev->next = node->next;
    }
    else {
        if ( node->next != get_parent( node ) )
            node->next->prev = node->prev;
        else
            node->next->child = node->prev;
        if ( node->prev != NULL )
            node->prev->next = node->next;
    }

    if ( heap->minimum == node ) {
        if ( node->next != node )
            heap->minimum = node->next;
        else
            heap->minimum = node->child;
    }

    if ( node->child != NULL ) {
        strip_list( node->child );
        merge_into_roots( heap, node->child );
    }
    fix_roots( heap );

    free( node );
    heap->size--;

    return item;
}

void decrease_key( violation_heap *heap, violation_node *node, uint32_t delta ) {
    INCR_DECREASE_KEY
    
    node->key -= delta;
    violation_node *parent;
    violation_node *first_child;
    violation_node *second_child;
    violation_node *replacement;
        
    if ( get_parent( node ) == NULL ) {
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
        return;
    }
    else {
        parent = get_parent( node );
        if ( ( is_active( node ) ) &&
                ! ( node->key < parent->key ) )
            return;
        first_child = node->child;
        if ( first_child != NULL ) {
            // determine active child of greater rank
            second_child = first_child->prev;
            if ( second_child == NULL ) {
                node->child = NULL;
                replacement = first_child;
            }
            else {
                if ( second_child->rank > first_child->rank ) {
                    if ( second_child->prev != NULL )
                        second_child->prev->next = first_child;
                    first_child->prev = second_child->prev;
                    replacement = second_child;
                }
                else {
                    node->child = second_child;
                    second_child->next = node;
                    replacement = first_child;
                }
            }

            // swap child into place of this node
            replacement->next = node->next;
            replacement->prev = node->prev;
            if ( replacement->next != NULL ) {
                if ( replacement->next->child == node )
                    replacement->next->child = replacement;
                else
                    replacement->next->prev = replacement;
            }
            if ( replacement->prev != NULL )
                replacement->prev->next = replacement;

            propagate_ranks( replacement );
        }
        else {
            if ( node->next->child == node )
                node->next->child = node->prev;
            else
                node->next->prev = node->prev;
            if ( node->prev != NULL )
                node->prev->next = node->next;
            propagate_ranks( node->next );
        }

        // make node a root
        node->next = node;
        node->prev = NULL;
        merge_into_roots( heap, node );
    }
}

void meld( violation_heap *heap, violation_heap *other_heap ) {
    INCR_MELD
    
    int i;
    
    merge_into_roots( heap, other_heap->minimum );

    heap->size += other_heap->size;
    other_heap->size = 0;
    other_heap->minimum = NULL;
    for ( i = 0; i < MAXRANK; i++ ) {
        other_heap->roots[i][0] = NULL;
        other_heap->roots[i][1] = NULL;
    }
}

bool empty( violation_heap *heap ) {
    return ( heap->size == 0 );
}

void merge_into_roots( violation_heap *heap, violation_node *list ) {
    violation_node *temp;
    if ( heap->minimum == NULL )
        heap->minimum = list;
    else if ( ( list != NULL ) && ( heap->minimum != list ) ) {
        temp = heap->minimum->next;
        heap->minimum->next = list->next;
        list->next = temp;

        if ( list->key < heap->minimum->key )
            heap->minimum = list;
    }
}

violation_node* triple_join( violation_node *a, violation_node *b,
        violation_node *c ) {
            
    violation_node *parent;
    violation_node *child1;
    violation_node *child2;
    
    if ( a->key < b->key ) {
        if ( a->key < c->key ) {
            parent = a;
            child1 = ( b->rank >= c->rank ) ? b : c;
            child2 = ( b->rank >= c->rank ) ? c : b;
        }
        else {
            parent = c;
            child1 = ( a->rank >= b->rank ) ? a : b;
            child2 = ( a->rank >= b->rank ) ? b : a;
        }
    }
    else {
        if ( b->key < c->key ) {
            parent = b;
            child1 = ( a->rank >= c->rank ) ? a : c;
            child2 = ( a->rank >= c->rank ) ? c : a;
        }
        else {
            parent = c;
            child1 = ( a->rank >= b->rank ) ? a : b;
            child2 = ( a->rank >= b->rank ) ? b : a;
        }
    }

    return join( parent, child1, child2 );
}

violation_node* join( violation_node *parent, violation_node *child1,
        violation_node *child2 ) {

    violation_node *active1;
    violation_node *active2;
    uint32_t rank1, rank2;

    if ( parent->child != NULL ) {
        active1 = parent->child;
        active2 = parent->child->prev;
        if ( active2 != NULL ) {
            rank1 = active1->rank;
            rank2 = active2->rank;
            if ( rank1 < rank2 ) {
                active1->prev = active2->prev;
                if ( active1->prev != NULL )
                    active1->prev->next = active1;
                active2->next = parent;
                active1->next = active2;
                active2->prev = active1;
                parent->child = active2;
            }
        }
    }

    child1->next = parent;
    child1->prev = child2;
    child2->next = child1;
    child2->prev = parent->child;
    if ( parent->child != NULL )
        parent->child->next = child2;
    parent->child = child1;

    parent->rank++;

    return parent;
}

void fix_roots( violation_heap *heap ) {
    violation_node *current;
    violation_node *next;
    violation_node *head;
    violation_node *tail;
    int i;
    int32_t rank;

    for ( i = 0; i < MAXRANK; i++ ) {
        heap->roots[i][0] = NULL;
        heap->roots[i][1] = NULL;
    }
    
    if ( heap->minimum == NULL )
        return;

    head = heap->minimum->next;
    heap->minimum->next = NULL;
    tail = heap->minimum;
    current = head;
    while ( current != NULL ) {
        next = current->next;
        current->next = NULL;
        if ( ! attempt_insert( heap, current ) ) {
            rank = current->rank;
            tail->next = triple_join( current, heap->roots[rank][0],
                heap->roots[rank][1] );
            if ( tail == current )
                next = tail->next;
            tail = tail->next;
            heap->roots[rank][0] = NULL;
            heap->roots[rank][1] = NULL;
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i < MAXRANK; i++ ) {
        if ( heap->roots[i][0] != NULL ) {
            if ( head == NULL )
                head = heap->roots[i][0];
            else
                tail->next = heap->roots[i][0];
            tail = heap->roots[i][0];
        }
        if ( heap->roots[i][1] != NULL ) {
            if ( head == NULL )
                head = heap->roots[i][1];
            else
                tail->next = heap->roots[i][1];
            tail = heap->roots[i][1];
        }
    }

    tail->next = head;

    set_min( heap );
}

bool attempt_insert( violation_heap *heap, violation_node *node ) {
    int32_t rank = node->rank;
    if ( ( heap->roots[rank][0] != NULL ) && ( heap->roots[rank][0] != node ) ) {
        if ( ( heap->roots[rank][1] != NULL ) && ( heap->roots[rank][1] != node ) )
            return FALSE;
        else
            heap->roots[rank][1] = node;
    }
    else
        heap->roots[rank][0] = node;

    return TRUE;    
}

void set_min( violation_heap *heap ) {
    int i;
    heap->minimum = NULL;
    for ( i = 0; i < MAXRANK; i++ ) {
        if ( heap->roots[i][0] != NULL ) {
            if ( heap->minimum == NULL )
                heap->minimum = heap->roots[i][0];
            else if ( heap->roots[i][0]->key < heap->minimum->key )
                heap->minimum = heap->roots[i][0];
        }
        if ( heap->roots[i][1] != NULL ) {
            if ( heap->minimum == NULL )
                heap->minimum = heap->roots[i][1];
            else if ( heap->roots[i][1]->key < heap->minimum->key )
                heap->minimum = heap->roots[i][1];
        }                    
    }
}

violation_node* find_prev_root( violation_node *node ) {
    violation_node *prev = node->next;
    while ( prev->next != node )
        prev = prev->next;
    
    return prev;
}

void propagate_ranks( violation_node *node ) {
    int32_t rank1 = -1;
    int32_t rank2 = -1;
    int32_t new_rank;
    int32_t total;
    bool updated;
    violation_node *parent;

    if ( node->child != NULL ) {
        rank1 = node->child->rank;
        if ( node->child->prev != NULL )
            rank2 = node->child->prev->rank;
    }

    total = rank1 + rank2;
    if ( total == -2 )
        new_rank = 0;
    else if ( total == -1 )
        new_rank = 1;
    else 
        new_rank = ( ( total / 2 ) + ( total % 2 ) + 1 );
    updated = new_rank < node->rank;
    node->rank = new_rank;
    
    parent = get_parent( node );
    if ( updated && ( parent != NULL ) && ( is_active( parent ) ) )
        propagate_ranks( get_parent( node ) );
}

void strip_list( violation_node *node ) {
    violation_node *current = node;
    violation_node *prev;
    while ( current->prev != NULL ) {
        prev = current->prev;
        current->prev = NULL;
        current = prev;
    }
    node->next = current;
}

bool is_active( violation_node *node ) {
    if ( get_parent( node ) == NULL )
        return TRUE;
    else if ( node->next->child == node )
        return TRUE;
    else if ( node->next->next->child == node->next )
        return TRUE ;
    else
        return FALSE;
}

violation_node* get_parent( violation_node *node ) {
    if ( node->next->child == node )
        return node->next;
    else if ( ( node->prev == NULL ) && ( node->next->prev == NULL ) )
        return NULL;
    else
        return ( get_parent( node->next ) );
}
