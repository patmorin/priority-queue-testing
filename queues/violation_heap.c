#include "violation_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

violation_heap* create_heap( uint32_t capacity ) {
    map = create_mem_map( capacity );
    violation_heap *heap = (violation_heap*) calloc( 1, sizeof( violation_heap ) );
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( violation_heap ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
    return heap;
}

void destroy_heap( violation_heap *heap ) {
    clear_heap( heap );
    FREE_STATS
    free( heap );
    destroy_mem_map( map );
}

void clear_heap( violation_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( violation_heap *heap, violation_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

item_type* get_item( violation_heap *heap, violation_node *node ) {
        ADD_TRAVERSALS(1) // node
    return (item_type*) &(node->item);
}

uint32_t get_size( violation_heap *heap ) {
    return heap->size;
}

violation_node* insert( violation_heap *heap, item_type item, key_type key ) {
    INCR_INSERT
    
    violation_node* wrapper = heap_node_alloc( map );
        INCR_ALLOCS
        ADD_SIZE( sizeof( violation_node ) )
        ADD_TRAVERSALS(1) // wrapper
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->next = wrapper;
    heap->size++;
        ADD_UPDATES(4) // wrapper, heap
        FIX_MAX_NODES

    merge_into_roots( heap, wrapper );

        ADD_TRAVERSALS(1) // heap->minimum
    if ( ( heap->minimum == NULL ) || ( key < heap->minimum->key ) )
        heap->minimum = wrapper;
    
    return wrapper;
}

violation_node* find_min( violation_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type delete_min( violation_heap *heap ) {
    INCR_DELETE_MIN
    
    return delete( heap, heap->minimum );
}

key_type delete( violation_heap *heap, violation_node *node ) {
    INCR_DELETE
    
    key_type key = node->key;
        ADD_TRAVERSALS(1) // node
    violation_node *prev;

    if ( get_parent( heap, node ) == NULL ) {
        prev = find_prev_root( heap, node );
        prev->next = node->next;
            ADD_TRAVERSALS(1) // prev
            ADD_UPDATES(1) // prev
    }
    else {
        if ( node->next != get_parent( heap, node ) )
            node->next->prev = node->prev;
        else
            node->next->child = node->prev;
            ADD_TRAVERSALS(1) // node->next
            ADD_UPDATES(1) // node->next
        if ( node->prev != NULL ) {
            node->prev->next = node->next;
                ADD_TRAVERSALS(1) // node->prev
                ADD_UPDATES(1) // node->prev
        }
    }

    if ( heap->minimum == node ) {
        if ( node->next != node )
            heap->minimum = node->next;
        else
            heap->minimum = node->child;
            ADD_UPDATES(1) // heap
    }

    if ( node->child != NULL ) {
        strip_list( heap, node->child );
        merge_into_roots( heap, node->child );
    }
    fix_roots( heap );

    heap_node_free( map, node );
        SUB_SIZE( sizeof( violation_node ) )
    heap->size--;
        ADD_UPDATES(1) // heap

    return key;
}

void decrease_key( violation_heap *heap, violation_node *node, key_type new_key ) {
    INCR_DECREASE_KEY
    
    node->key = new_key;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(1) // node
    violation_node *parent, *first_child, *second_child, *replacement;
        
    if ( get_parent( heap, node ) == NULL ) {
            ADD_TRAVERSALS(1) // heap->minimum
        if ( node->key < heap->minimum->key ) {
            heap->minimum = node;
                ADD_UPDATES(1) // heap
        }
        return;
    }
    else {
        parent = get_parent( heap, node );
            ADD_TRAVERSALS(1) // parent
        if ( ( is_active( heap, node ) ) &&
                ! ( node->key < parent->key ) )
            return;
        first_child = node->child;
        if ( first_child != NULL ) {
            // determine active child of greater rank
            second_child = first_child->prev;
                ADD_TRAVERSALS(1) // first_child
            if ( second_child == NULL ) {
                node->child = NULL;
                    ADD_UPDATES(1) // noce
                replacement = first_child;
            }
            else {
                    ADD_TRAVERSALS(1) // second_child
                if ( second_child->rank > first_child->rank ) {
                    if ( second_child->prev != NULL ) {
                        second_child->prev->next = first_child;
                            ADD_TRAVERSALS(1) // second_child->prev
                            ADD_UPDATES(1) // second_child->prev
                    }
                    first_child->prev = second_child->prev;
                        ADD_UPDATES(1) // first_child
                    replacement = second_child;
                }
                else {
                    node->child = second_child;
                    second_child->next = node;
                        ADD_UPDATES(2) // node, second_child
                    replacement = first_child;
                }
            }

            // swap child into place of this node
            replacement->next = node->next;
            replacement->prev = node->prev;
                ADD_UPDATES(2) // replacement
            if ( replacement->next != NULL ) {
                    ADD_TRAVERSALS(1) // replacement->next
                if ( replacement->next->child == node )
                    replacement->next->child = replacement;
                else
                    replacement->next->prev = replacement;
                    ADD_UPDATES(1) // replacement->next
            }
            if ( replacement->prev != NULL ) {
                replacement->prev->next = replacement;
                    ADD_TRAVERSALS(1) // replacement->prev
                    ADD_UPDATES(1) // replacement->prev
            }

            propagate_ranks( heap, replacement );
        }
        else {
                ADD_TRAVERSALS(1) // node->next
            if ( node->next->child == node )
                node->next->child = node->prev;
            else
                node->next->prev = node->prev;
                ADD_UPDATES(1) // node->next
            if ( node->prev != NULL ) {
                node->prev->next = node->next;
                    ADD_TRAVERSALS(1) //node->prev
                    ADD_UPDATES(1) //node->prev
            }
            propagate_ranks( heap, node->next );
        }

        // make node a root
        node->next = node;
        node->prev = NULL;
            ADD_UPDATES(1)
        merge_into_roots( heap, node );
    }
}

bool empty( violation_heap *heap ) {
    return ( heap->size == 0 );
}

void merge_into_roots( violation_heap *heap, violation_node *list ) {
    violation_node *temp;
    if ( heap->minimum == NULL ) {
        heap->minimum = list;
            ADD_UPDATES(1) // heap
    }
    else if ( ( list != NULL ) && ( heap->minimum != list ) ) {
        temp = heap->minimum->next;
        heap->minimum->next = list->next;
        list->next = temp;
            ADD_TRAVERSALS(2) // heap->minimum, list
            ADD_UPDATES(2) // heap->minimum, list

        if ( list->key < heap->minimum->key ) {
            heap->minimum = list;
                ADD_UPDATES(1) // heap
        }
    }
}

violation_node* triple_join( violation_heap *heap, violation_node *a,
        violation_node *b, violation_node *c ) {
            
    violation_node *parent, *child1, *child2;
    
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
        ADD_TRAVERSALS(3) // a, b, c

    return join( heap, parent, child1, child2 );
}

violation_node* join( violation_heap *heap, violation_node *parent,
        violation_node *child1, violation_node *child2 ) {

    violation_node *active1, *active2;
    uint32_t rank1, rank2;

        ADD_TRAVERSALS(3) // parent, child1, child2
    if ( parent->child != NULL ) {
        active1 = parent->child;
        active2 = parent->child->prev;
            ADD_TRAVERSALS(1) // parent->child
        if ( active2 != NULL ) {
            rank1 = active1->rank;
            rank2 = active2->rank;
            if ( rank1 < rank2 ) {
                active1->prev = active2->prev;
                if ( active1->prev != NULL ) {
                    active1->prev->next = active1;
                        ADD_TRAVERSALS(1) // active1->prev
                        ADD_UPDATES(1) // active1->prev
                }
                active2->next = parent;
                active1->next = active2;
                active2->prev = active1;
                parent->child = active2;
                    ADD_UPDATES(5) // active1, active2, parent
            }
        }
    }

    child1->next = parent;
    child1->prev = child2;
    child2->next = child1;
    child2->prev = parent->child;
        ADD_UPDATES(4) // child1, child2
    
    if ( parent->child != NULL ) {
        parent->child->next = child2;
            ADD_UPDATES(1) // parent->child
    }
    parent->child = child1;

    parent->rank++;
        ADD_UPDATES(1) // parent

    return parent;
}

void fix_roots( violation_heap *heap ) {
    violation_node *current, *next, *head, *tail;
    int i;
    int32_t rank;

    for ( i = 0; i <= heap->largest_rank; i++ ) {
        heap->roots[i][0] = NULL;
        heap->roots[i][1] = NULL;
            ADD_UPDATES(2) // heap
    }
    
    if ( heap->minimum == NULL )
        return;

    head = heap->minimum->next;
    heap->minimum->next = NULL;
        ADD_TRAVERSALS(1) // heap->minimum
        ADD_UPDATES(1) // heap->minimum
    tail = heap->minimum;
    current = head;
    while ( current != NULL ) {
        next = current->next;
        current->next = NULL;
            ADD_TRAVERSALS(1) // current
            ADD_UPDATES(1) // current
        if ( ! attempt_insert( heap, current ) ) {
            rank = current->rank;
            tail->next = triple_join( heap, current, heap->roots[rank][0],
                heap->roots[rank][1] );
                ADD_UPDATES(1) // tail
            if ( tail == current )
                next = tail->next;
            tail = tail->next;
            heap->roots[rank][0] = NULL;
            heap->roots[rank][1] = NULL;
                ADD_UPDATES(2) // heap
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i <= heap->largest_rank; i++ ) {
        if ( heap->roots[i][0] != NULL ) {
            if ( head == NULL )
                head = heap->roots[i][0];
            else {
                tail->next = heap->roots[i][0];
                    ADD_UPDATES(1) // tail->next
            }
            tail = heap->roots[i][0];
        }
        if ( heap->roots[i][1] != NULL ) {
            if ( head == NULL )
                head = heap->roots[i][1];
            else {
                tail->next = heap->roots[i][1];
                    ADD_UPDATES(1) // tail->next
            }
            tail = heap->roots[i][1];
        }
    }

    tail->next = head;
        ADD_UPDATES(1)

    set_min( heap );
}

bool attempt_insert( violation_heap *heap, violation_node *node ) {
    int32_t rank = node->rank;
    if ( ( heap->roots[rank][0] != NULL ) && ( heap->roots[rank][0] != node ) ) {
        if ( ( heap->roots[rank][1] != NULL ) && ( heap->roots[rank][1] != node ) )
            return FALSE;
        else {
            heap->roots[rank][1] = node;
                ADD_UPDATES(1) // heap
        }
    }
    else {
        heap->roots[rank][0] = node;
            ADD_UPDATES(1) // heap
    }

    if ( rank > heap->largest_rank ) {
        heap->largest_rank = rank;
            ADD_UPDATES(1) // heap
    }

    return TRUE;    
}

void set_min( violation_heap *heap ) {
    int i;
    heap->minimum = NULL;
    for ( i = 0; i <= heap->largest_rank; i++ ) {
        if ( heap->roots[i][0] != NULL ) {
            if ( heap->minimum == NULL )
                heap->minimum = heap->roots[i][0];
            else if ( heap->roots[i][0]->key < heap->minimum->key )
                heap->minimum = heap->roots[i][0];
                ADD_UPDATES(1) // heap
        }
        if ( heap->roots[i][1] != NULL ) {
            if ( heap->minimum == NULL )
                heap->minimum = heap->roots[i][1];
            else if ( heap->roots[i][1]->key < heap->minimum->key )
                heap->minimum = heap->roots[i][1];
                ADD_UPDATES(1) // heap
        }                    
    }
}

violation_node* find_prev_root( violation_heap *heap, violation_node *node ) {
    violation_node *prev = node->next;
        ADD_TRAVERSALS(1) // node
    while ( prev->next != node ) {
        prev = prev->next;
            ADD_TRAVERSALS(1) // prev
    }
    
    return prev;
}

void propagate_ranks( violation_heap *heap, violation_node *node ) {
    int32_t rank1 = -1;
    int32_t rank2 = -1;
    int32_t new_rank, total;
    bool updated;
    violation_node *parent;

        ADD_TRAVERSALS(1) // node
    if ( node->child != NULL ) {
            ADD_TRAVERSALS(1) // node->child
        rank1 = node->child->rank;
        if ( node->child->prev != NULL ) {
            rank2 = node->child->prev->rank;
                ADD_TRAVERSALS(1) // node->child->prev
                ADD_UPDATES(1) // node->child->prev
        }
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
        ADD_UPDATES(1) // node
    
    parent = get_parent( heap, node );
    if ( updated && ( parent != NULL ) && ( is_active( heap, parent ) ) )
        propagate_ranks( heap, get_parent( heap, node ) );
}

void strip_list( violation_heap *heap, violation_node *node ) {
    violation_node *current = node;
    violation_node *prev;
        ADD_TRAVERSALS(1) // current
    while ( current->prev != NULL ) {
        prev = current->prev;
        current->prev = NULL;
        current = prev;
            ADD_TRAVERSALS(1) // current
            ADD_UPDATES(1) // current
    }
    node->next = current;
}

bool is_active( violation_heap *heap, violation_node *node ) {
    if ( get_parent( heap, node ) == NULL )
        return TRUE;
    else {
            ADD_TRAVERSALS(2) // node, node->next
        if ( node->next->child == node )
            return TRUE;
        else {
                ADD_TRAVERSALS(1) // node->next->next
            if ( node->next->next->child == node->next )
                return TRUE ;
            else
                return FALSE;
        }
    }
}

violation_node* get_parent( violation_heap *heap, violation_node *node ) {
        ADD_TRAVERSALS(2) // node, node->next
    if ( node->next->child == node )
        return node->next;
    else if ( ( node->prev == NULL ) && ( node->next->prev == NULL ) )
        return NULL;
    else
        return ( get_parent( heap, node->next ) );
}
