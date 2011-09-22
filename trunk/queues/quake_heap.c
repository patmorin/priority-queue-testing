#include "quake_heap.h"
#include "memory_management.h"

//! memory map to use for allocation
mem_map *map;

quake_heap* create_heap( uint32_t capacity ) {
    map = create_mem_map( 2 * capacity );
    quake_heap *heap = (quake_heap*) calloc( 1, sizeof( quake_heap ) );
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( quake_heap ) )
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
    return heap;
}

void destroy_heap( quake_heap *heap ) {
    clear_heap( heap );
    FREE_STATS
    free( heap );
    destroy_mem_map( map );
}

void clear_heap( quake_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

key_type get_key( quake_heap *heap, quake_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

item_type* get_item( quake_heap *heap, quake_node *node ) {
        ADD_TRAVERSALS(1) // node
    return (item_type*) &(node->item);
}

uint32_t get_size( quake_heap *heap ) {
    return heap->size;
}

quake_node* insert( quake_heap *heap, item_type item, key_type key ) {
    INCR_INSERT
    
    quake_node *wrapper = heap_node_alloc( map );
        INCR_ALLOCS
        ADD_SIZE( sizeof( quake_node ) )
        ADD_TRAVERSALS(1) // wrapper
    ITEM_ASSIGN( wrapper->item, item );
    wrapper->key = key;
    wrapper->parent = wrapper;
        ADD_UPDATES(3) // wrapper
    
    make_root( heap, wrapper );
    heap->size++;
    (heap->nodes[0])++;
        FIX_MAX_NODES
        ADD_UPDATES(2) // heap

    return wrapper;
}

quake_node* find_min( quake_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum;
}

key_type delete_min( quake_heap *heap ) {
    INCR_DELETE_MIN
    
    return delete( heap, heap->minimum );
}

key_type delete( quake_heap *heap, quake_node *node ) {
    INCR_DELETE
    
    key_type key = node->key;
    cut( heap, node );

    fix_roots( heap );
    fix_decay( heap );

    heap->size--;
        ADD_UPDATES(1) // heap

    return key;
}

void decrease_key( quake_heap *heap, quake_node *node, key_type new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(1) // node
    if ( is_root( heap, node ) ) {
            ADD_TRAVERSALS(1) // heap->minimum
        if ( node->key < heap->minimum->key ) {
            heap->minimum = node;
                ADD_UPDATES(1) // heap
        }
    }
    else {
            ADD_TRAVERSALS(1) // node->parent
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else
            node->parent->right = NULL;
            ADD_UPDATES(1) // node->parent

        make_root( heap, node );
    }
}

bool empty( quake_heap *heap ) {
    return ( heap->size == 0 );
}

void make_root( quake_heap *heap, quake_node* node ) {
    if ( node == NULL )
        return;

        ADD_TRAVERSALS(1) // ndoe
    if ( heap->minimum == NULL ) {
         heap->minimum = node;
         node->parent = node;
            ADD_UPDATES(2) // heap, node
    }
    else {
            ADD_TRAVERSALS(1) // heap->minimum
        node->parent = heap->minimum->parent;
        heap->minimum->parent = node;
            ADD_UPDATES(2) // heap->minimum, node
        if ( node->key < heap->minimum->key ) {
            heap->minimum = node;
                ADD_UPDATES(1) // heap
        }
    }
}

void remove_from_roots( quake_heap *heap, quake_node *node ) {
    quake_node *current = node->parent;
        ADD_TRAVERSALS(1) // node
    while ( current->parent != node ) {
        current = current->parent;
            ADD_TRAVERSALS(1) // current
    }
    if ( current == node ) {
        heap->minimum = NULL;
            ADD_UPDATES(1) // heap
    }
    else {
        current->parent = node->parent;
            ADD_UPDATES(1) // current
        if ( heap->minimum == node ) {
            heap->minimum = current;
            ADD_UPDATES(1) // heap
        }
    }
}

void cut( quake_heap *heap, quake_node *node ) {
    if ( node == NULL )
        return;

        ADD_TRAVERSALS(1) // node, preemptively
    if ( is_root( heap, node ) )
        remove_from_roots( heap, node );
    else {
            ADD_TRAVERSALS(1) // node->parent
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else if ( node->parent->right == node )
            node->parent->right = NULL;
            ADD_UPDATES(1) // node->parent
    }
        
    cut( heap, node->left );
    make_root( heap, node->right );

    (heap->nodes[node->height])--;
        ADD_UPDATES(1) // heap->nodes
    heap_node_free( map, node );
}

quake_node* join( quake_heap *heap, quake_node *a, quake_node *b ) {
    quake_node *parent, *child, *duplicate;

        ADD_TRAVERSALS(2) // a, b
    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    duplicate = clone_node( heap, parent );
        ADD_TRAVERSALS(1) // duplicate
    if ( duplicate->left != NULL ) {
        duplicate->left->parent = duplicate;
            ADD_TRAVERSALS(1) // duplicate->left
            ADD_UPDATES(1) // duplicate->left
    }
    if ( duplicate->right != NULL ) {
        duplicate->right->parent = duplicate;
            ADD_TRAVERSALS(1) // duplicate->right
            ADD_UPDATES(1) // duplicate->right
    }

    duplicate->parent = parent;
    child->parent = parent;

    parent->parent = NULL;
    parent->left = duplicate;
    parent->right = child;

    parent->height++;
    (heap->nodes[parent->height])++;

        ADD_UPDATES(7) // duplicate, child, parent, heap->nodes
    
    return parent;
}

void fix_roots( quake_heap *heap ) {
    quake_node *current, *next, *tail, *head, *joined;
    uint32_t i, height;

    if ( heap->minimum == NULL )
        return;

    for ( i = 0; i <= heap->highest_node; i++ )
        heap->roots[i] = NULL;
        ADD_UPDATES( heap->highest_node + 1 )
    heap->highest_node = 0;

    current = heap->minimum->parent;
    tail = heap->minimum;
    heap->minimum->parent = NULL;
        ADD_TRAVERSALS(1) // heap->minimum
        ADD_UPDATES(1) // heap->minimum

    while ( current != NULL ) {
        next = current->parent;
        current->parent = NULL;
            ADD_TRAVERSALS(1) // current
            ADD_UPDATES(1) // current
        if ( ! attempt_insert( heap, current ) ) {
            height = current->height;
            joined = join( heap, current, heap->roots[height] );
            if ( current == tail ) {
                tail = joined; 
                next = tail;
            }
            else {
                tail->parent = joined;
                    ADD_TRAVERSALS(1) // tail
                    ADD_UPDATES(1) // tail
                tail = tail->parent;
            }
            heap->roots[height] = NULL;
                ADD_UPDATES(1) // heap
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i <= heap->highest_node; i++ ) {
        if ( heap->roots[i] != NULL ) {
            if ( head == NULL ) {
                head = heap->roots[i];
                tail = heap->roots[i];
            }
            else {
                tail->parent = heap->roots[i];
                tail = tail->parent;
                    ADD_TRAVERSALS(1) // tail
                    ADD_UPDATES(1) // tail
            }
        }
    }
    tail->parent = head;
        ADD_UPDATES(1) // tail

    heap->minimum = head;
        ADD_UPDATES(1) // tail
    fix_min( heap );
}

bool attempt_insert( quake_heap *heap, quake_node *node ) {
    uint32_t height = node->height;
        ADD_TRAVERSALS(1) // node
    if ( ( heap->roots[height] != NULL ) && ( heap->roots[height] != node ) )
        return FALSE;

    if ( height > heap->highest_node ) {
        heap->highest_node = height;
            ADD_UPDATES(1) // heap
    }
    heap->roots[height] = node;
        ADD_UPDATES(1) // heap

    return TRUE;
}

void fix_min( quake_heap *heap ) {
    quake_node *start = heap->minimum;
    quake_node *current = heap->minimum->parent;
        ADD_TRAVERSALS(1) // heap->minimum
    while ( current != start ) {
            ADD_TRAVERSALS(1) // current
        if ( current->key < heap->minimum->key ) {
            heap->minimum = current;
                ADD_UPDATES(1) // heap
        }
        current = current->parent;
    }
}

void fix_decay( quake_heap *heap ) {
    uint32_t i;
    check_decay( heap );
    if ( violation_exists( heap ) ) {
        for ( i = heap->violation; i < MAXRANK; i++ ) {
            if ( heap->roots[i] != NULL )
                prune( heap, heap->roots[i] );
        }
    }
}

void check_decay( quake_heap *heap ) {
    uint32_t i;
    for ( i = 1; i <= heap->highest_node; i++ ) {
        if ( ( (float) heap->nodes[i] ) > ( (float) ( ALPHA * (float) heap->nodes[i-1] ) ) )
            break;
    }
    heap->violation = i;
}

bool violation_exists( quake_heap *heap ) {
    return ( heap->violation < MAXRANK );
}

void prune( quake_heap *heap, quake_node *node ) {
    quake_node *duplicate, *child;

    if ( node == NULL )
        return;

        ADD_TRAVERSALS(1) // node
    if ( node->height < heap->violation ) {                
        if ( ! is_root( heap, node ) )
            make_root( heap, node );
            
        return;
    }

    duplicate = node->left;
    child = node->right;

    prune( heap, child );        

    node->left = duplicate->left;
        ADD_UPDATES(1) // node
    if ( node->left != NULL ) {
        node->left->parent = node;
            ADD_TRAVERSALS(1) // node->left
            ADD_UPDATES(1) // node->left
    }
    node->right = duplicate->right;
        ADD_UPDATES(1) // node
    if ( node->right != NULL ) {
        node->right->parent = node;
            ADD_TRAVERSALS(1) // node->right
            ADD_UPDATES(1) // node->right
    }
    (heap->nodes[node->height])--;
    node->height--;
        ADD_UPDATES(1) // heap, node
    heap_node_free( map, duplicate );
        SUB_SIZE( sizeof( quake_node ) )

    prune( heap, node );
}

quake_node* clone_node( quake_heap *heap, quake_node *original ) {
    quake_node *clone = heap_node_alloc( map );
        INCR_ALLOCS
        ADD_SIZE( sizeof( quake_node ) )
        
    ITEM_ASSIGN( clone->item, original->item );
    clone->key = original->key;
    clone->height = original->height;
    clone->left = original->left;
    clone->right = original->right;
        ADD_TRAVERSALS(1) // clone
        ADD_UPDATES(5) // clone

    return clone;
}

bool is_root( quake_heap *heap, quake_node *node ) {
        ADD_TRAVERSALS(2) // node, parent
    return ( ( node->parent->left != node ) && ( node->parent->right != node ) );
}
