#include "quake_heap.h"

quake_heap* create_heap() {
    quake_heap *heap = (quake_heap*) calloc( 1, sizeof( quake_heap ) );
    heap->stats = (heap_stats*) calloc( 1, sizeof( heap_stats ) );
    return heap;
}

void destroy_heap( quake_heap *heap ) {
    clear_heap( heap );
    free( heap->stats );
    free( heap );
}

void clear_heap( quake_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

uint32_t get_key( quake_node *node ) {
    return node->key;
}

void* get_item( quake_node *node ) {
    return node->item;
}

uint32_t get_size( quake_heap *heap ) {
    return heap->size;
}

quake_node* insert( quake_heap *heap, void *item, uint32_t key ) {
    INCR_INSERT
    
    quake_node *wrapper = (quake_node*) calloc( 1, sizeof( quake_node ) );
    wrapper->item = item;
    wrapper->key = key;
    wrapper->parent = wrapper;
    
    make_root( heap, wrapper );
    heap->size++;
    if ( heap->size > heap->stats->max_size )
        heap->stats->max_size = heap->size;
    (heap->nodes[0])++;

    return wrapper;
}

quake_node* find_min( quake_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->minimum;
}

KEY_T delete_min( quake_heap *heap ) {
    INCR_DELETE_MIN
    
    if ( empty( heap ) )
        return NULL;
    return delete( heap, heap->minimum );
}

KEY_T delete( quake_heap *heap, quake_node *node ) {
    INCR_DELETE
    
    KEY_T key = node->key;
    cut( heap, node );

    fix_roots( heap );
    fix_decay( heap );

    heap->size--;

    return key;
}

void decrease_key( quake_heap *heap, quake_node *node, KEY_T new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
    if ( is_root( node ) ) {
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
    }
    else {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else
            node->parent->right = NULL;

        make_root( heap, node );
    }
}

void meld( quake_heap *heap, quake_heap *other_heap ) {
    INCR_MELD
    
    quake_node *temp;
    uint32_t i;
    if ( empty( other_heap ) )
        return;

    if ( empty( heap ) ) {
        heap->minimum = other_heap->minimum;
        for ( i = 0; i < MAXRANK; i++ )
            heap->nodes[i] = other_heap->nodes[i];
        heap->size = other_heap->size;
    }
    else {
        temp = heap->minimum->parent;
        heap->minimum->parent = other_heap->minimum->parent;
        other_heap->minimum->parent = temp;
        if ( other_heap->minimum->key < heap->minimum->key )
            heap->minimum = other_heap->minimum;
        
        for ( i = 0; i < MAXRANK; i++ )
            heap->nodes[i] += other_heap->nodes[i];
        heap->size += other_heap->size;
    }

    other_heap->minimum = NULL;
    other_heap->size = 0;
    for ( i = 0; i < MAXRANK; i++ )
        other_heap->nodes[i] = 0;
}

bool empty( quake_heap *heap ) {
    return ( heap->size == 0 );
}

void make_root( quake_heap *heap, quake_node* node ) {
    if ( node == NULL )
        return;

    if ( heap->minimum == NULL ) {
         heap->minimum = node;
         node->parent = node;
    }
    else {
        node->parent = heap->minimum->parent;
        heap->minimum->parent = node;
        if ( node->key < heap->minimum->key )
            heap->minimum = node;
    }
}

void remove_from_roots( quake_heap *heap, quake_node *node ) {
    quake_node *current = node->parent;
    while ( current->parent != node )
        current = current->parent;
    if ( current == node )
        heap->minimum = NULL;
    else {
        current->parent = node->parent;
        if ( heap->minimum == node )
            heap->minimum = current;
    }
}

void cut( quake_heap *heap, quake_node *node ) {
    if ( node == NULL )
        return;

    if ( is_root( node ) )
        remove_from_roots( heap, node );
    else {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else if ( node->parent->right == node )
            node->parent->right = NULL;
    }
        
    cut( heap, node->left );
    make_root( heap, node->right );

    (heap->nodes[node->height])--;
    free( node );
}

quake_node* join( quake_heap *heap, quake_node *a, quake_node *b ) {
    quake_node *parent, *child, *duplicate;

    if ( b->key < a->key ) {
        parent = b;
        child = a;
    }
    else {
        parent = a;
        child = b;
    }

    duplicate = clone_node( parent );
    if ( duplicate->left != NULL )
        duplicate->left->parent = duplicate;
    if ( duplicate->right != NULL )
        duplicate->right->parent = duplicate;

    duplicate->parent = parent;
    child->parent = parent;

    parent->parent = NULL;
    parent->left = duplicate;
    parent->right = child;

    parent->height++;
    (heap->nodes[parent->height])++;
    
    return parent;
}

void fix_roots( quake_heap *heap ) {
    quake_node *current, *next, *tail, *head, *joined;
    uint32_t i, height;

    if ( heap->minimum == NULL )
        return;

    for ( i = 0; i < MAXRANK; i++ )
        heap->roots[i] = NULL;

    current = heap->minimum->parent;
    tail = heap->minimum;
    heap->minimum->parent = NULL;

    while ( current != NULL ) {
        next = current->parent;
        current->parent = NULL;
        if ( ! attempt_insert( heap, current ) ) {
            height = current->height;
            joined = join( heap, current, heap->roots[height] );
            if ( current == tail ) {
                tail = joined; 
                next = tail;
            }
            else {
                tail->parent = joined;
                tail = tail->parent;
            }
            heap->roots[height] = NULL;
        }
        current = next;
    }

    head = NULL;
    tail = NULL;
    for ( i = 0; i < MAXRANK; i++ ) {
        if ( heap->roots[i] != NULL ) {
            if ( head != NULL ) {
                tail->parent = heap->roots[i];
                tail = tail->parent;
            }
            else {
                head = heap->roots[i];
                tail = heap->roots[i];
            }
        }
    }
    tail->parent = head;

    heap->minimum = head;
    fix_min( heap );
}

bool attempt_insert( quake_heap *heap, quake_node *node ) {
    uint32_t height = node->height;
    if ( ( heap->roots[height] != NULL ) && ( heap->roots[height] != node ) )
        return FALSE;

    heap->roots[height] = node;

    return TRUE;
}

void fix_min( quake_heap *heap ) {
    quake_node *start = heap->minimum;
    quake_node *current = heap->minimum->parent;
    while ( current != start ) {
        if ( current->key < heap->minimum->key )
            heap->minimum = current;
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
    for ( i = 1; i < MAXRANK; i++ ) {
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

    if ( node->height < heap->violation ) {                
        if ( ! is_root( node ) )
            make_root( heap, node );
            
        return;
    }

    duplicate = node->left;
    child = node->right;

    prune( heap, child );        

    node->left = duplicate->left;
    if ( node->left != NULL )
        node->left->parent = node;
    node->right = duplicate->right;
    if ( node->right != NULL )
        node->right->parent = node;
    (heap->nodes[node->height])--;
    node->height--;
    free( duplicate );

    prune( heap, node );
}

quake_node* clone_node( quake_node *original ) {
    quake_node *clone = (quake_node*) calloc( 1, sizeof( quake_node ) );
    
    clone->item = original->item;
    clone->key = original->key;
    clone->height = original->height;
    clone->left = original->left;
    clone->right = original->right;

    return clone;
}

bool is_root( quake_node *node ) {
    return ( ( node->parent->left != node ) && ( node->parent->right != node ) );
}
