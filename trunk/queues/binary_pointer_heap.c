#include "binary_pointer_heap.h"

binary_pointer_heap* create_heap() {
    binary_pointer_heap *heap = (binary_pointer_heap*) calloc( 1, sizeof( binary_pointer_heap ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_pointer_heap ) )
        ALLOC_STATS
        INCR_ALLOCS
        ADD_SIZE( sizeof( heap_stats ) )
        ADD_UPDATES(1)
    return heap;
}

void destroy_heap( binary_pointer_heap *heap ) {
    clear_heap( heap );
    FREE_STATS
    free( heap );
}

void clear_heap( binary_pointer_heap *heap ) {
    while( ! empty( heap ) )
        delete_min( heap );
}

KEY_T get_key( binary_pointer_heap *heap, binary_pointer_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->key;
}

void* get_item( binary_pointer_heap *heap, binary_pointer_node *node ) {
        ADD_TRAVERSALS(1) // node
    return node->item;
}

uint32_t get_size( binary_pointer_heap *heap ) {
    return heap->size;
}

binary_pointer_node* insert( binary_pointer_heap *heap, void *item, uint32_t key ) {
    INCR_INSERT
    
    binary_pointer_node* parent;
    binary_pointer_node* node = (binary_pointer_node*) calloc( 1, sizeof( binary_pointer_node ) );
        INCR_ALLOCS
        ADD_SIZE( sizeof( binary_pointer_node ) )
    node->item = item;
    node->key = key;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(2) // node

    if ( heap->root == NULL ) {
        heap->root = node;
            ADD_UPDATES(1)
    }
    else {
        parent = find_insertion_point( heap );
        
        if ( parent->left == NULL )
            parent->left = node;
        else
            parent->right = node;

        node->parent = parent;
            ADD_TRAVERSALS(1) // parent
            ADD_UPDATES(2) // parent, node
    }

    heap->size++;
        FIX_MAX_NODES
    heapify_up( heap, node );
    
    return node;
}

binary_pointer_node* find_min( binary_pointer_heap *heap ) {
    INCR_FIND_MIN
    
    if ( empty( heap ) )
        return NULL;
    return heap->root;
}

KEY_T delete_min( binary_pointer_heap *heap ) {
    INCR_DELETE_MIN
    
    return delete( heap, heap->root );
}

KEY_T delete( binary_pointer_heap *heap, binary_pointer_node* node ) {
    INCR_DELETE
    
    KEY_T key = node->key;
        ADD_TRAVERSALS(1) // node
    binary_pointer_node *last_node = find_last_node( heap );
    swap( heap, node, last_node);

    // figure out if this node is a left or right child and clear
    // reference from parent
    if ( node->parent != NULL ) {
        if ( node->parent->left == node )
            node->parent->left = NULL;
        else
            node->parent->right = NULL;

            ADD_TRAVERSALS(1) // node->parent
            ADD_UPDATES(1) // node->parent->...
    }

    free( node );
        SUB_SIZE( sizeof( binary_pointer_node ) )
    heap->size--;
        ADD_UPDATES(1)
    

    if ( empty( heap ) ) {
        heap->root = NULL;
            ADD_UPDATES(1)
    }
    else if ( node != last_node)
        heapify_down( heap, last_node );

    return key;
}

void decrease_key( binary_pointer_heap *heap, binary_pointer_node *node, KEY_T new_key ) {
    INCR_DECREASE_KEY

    node->key = new_key;
        ADD_TRAVERSALS(1) // node
        ADD_UPDATES(1) // node->key
    heapify_up( heap, node );
}

bool empty( binary_pointer_heap *heap ) {
    return ( heap->size == 0 );
}

void swap( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b ) {
    if ( ( a == NULL ) || ( b == NULL ) || ( a == b ) )
        return;
    
    if ( a->parent == b ) {
            ADD_TRAVERSALS(1)
        swap_connected( heap, b, a );
    }
    else if ( b->parent == a ) {
            ADD_TRAVERSALS(2)
        swap_connected( heap, a, b );
    }
    else
        swap_disconnected( heap, a, b );

    if ( heap->root == a )
        heap->root = b;
    else if ( heap->root == b )
        heap->root = a;

        ADD_UPDATES(1)
}

void swap_connected( binary_pointer_heap *heap, binary_pointer_node *parent, binary_pointer_node *child ) {
    binary_pointer_node *temp;

    child->parent = parent->parent;
    parent->parent = child;
        ADD_TRAVERSALS(2) // child, parent
        ADD_UPDATES(2) // child,parent->parent

    if ( child == parent->left ) {
        parent->left = child->left;
        child->left = parent;

        temp = child->right;
        child->right = parent->right;
        parent->right = temp;
    }
    else {
        temp = child->left;
        child->left = parent->left;
        parent->left = temp;

        parent->right = child->right;
        child->right = parent;
    }
        ADD_UPDATES(4) // parent,child->left,right
    fill_back_pointers( heap, parent, child );
}

void swap_disconnected( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b ) {
    binary_pointer_node *temp;

        ADD_TRAVERSALS(2) // a,b
    temp = a->parent;
    a->parent = b->parent;
    b->parent = temp;

    temp = a->left;
    a->left = b->left;
    b->left = temp;

    temp = a->right;
    a->right = b->right;
    b->right = temp;

    ADD_UPDATES(6) // a,b->left,right,parent

    fill_back_pointers( heap, a, b );
}

void fill_back_pointers( binary_pointer_heap *heap, binary_pointer_node *a, binary_pointer_node *b ) {
        ADD_TRAVERSALS(2) // a,b
    if ( a->parent != NULL ) {
            ADD_TRAVERSALS(1) // a->parent
        if ( a->parent->left == b )
            a->parent->left = a;
        else if ( a->parent->left != a )
            a->parent->right = a;

            ADD_UPDATES(1) // a->parent->left,right
    }

    if ( b->parent != NULL ) {
            ADD_TRAVERSALS(1) // b->parent
        if ( b->parent->left == a )
            b->parent->left = b;
        else if ( b->parent->left != b )
            b->parent->right = b;

            ADD_UPDATES(1) // b->parent->left,right
    }

    if ( a->left != NULL ) {
        a->left->parent = a;
            ADD_TRAVERSALS(1) // a->left
            ADD_UPDATES(1) // a->left->parent
    }
    if ( a->right != NULL ) {
        a->right->parent = a;
            ADD_TRAVERSALS(1) // a->right
            ADD_UPDATES(1) // a->right->parent
    }

    if ( b->left != NULL ) {
        b->left->parent = b;
            ADD_TRAVERSALS(1) // b->left
            ADD_UPDATES(1) // b->left->parent
    }
    if ( b->right != NULL ) {
        b->right->parent = b;
            ADD_TRAVERSALS(1) // b->right
            ADD_UPDATES(1) // b->right->parent
    }
}

void heapify_down( binary_pointer_heap *heap, binary_pointer_node *node ) {
    if ( node == NULL )
        return;

        ADD_TRAVERSALS(1) // node
    // repeatedly swap with smallest child if node violates heap order
    binary_pointer_node* smallest_child;
    while ( ! is_leaf( heap, node ) ) {
        if ( node->right == NULL )
            smallest_child = node->left;
        else if ( node->left->key < node->right->key ) {
                ADD_TRAVERSALS(2) // node->left,right
            smallest_child = node->left;
        }
        else {
                ADD_TRAVERSALS(2) // node->left,right from previous branch
            smallest_child = node->right;
        }

        if ( smallest_child->key < node->key )
            swap( heap, smallest_child, node );
        else
            break;
    }
}

void heapify_up( binary_pointer_heap *heap, binary_pointer_node *node ) {
    if ( node == NULL )
        return;

        ADD_TRAVERSALS(1) // node
    while ( node->parent != NULL ) {
            ADD_TRAVERSALS(1) // node->parent
        if ( node->key < node->parent->key )
            swap( heap, node, node->parent );
        else
            break;
    }
}

binary_pointer_node* find_last_node( binary_pointer_heap *heap ) {
    return find_node( heap, heap->size );
}

binary_pointer_node* find_insertion_point( binary_pointer_heap *heap ) {
    return find_node( heap, ( heap->size + 1 ) / 2 );
}

binary_pointer_node* find_node( binary_pointer_heap *heap, uint32_t n ) {
    uint32_t log, path, i;
    binary_pointer_node *current, *next;

    log = int_log2(n);
    current = heap->root;
    // i < log is used instead of i >= 0 because i is uint32_t
    // it will loop around to MAX_INT after it passes 0
    for ( i = ( log - 1 ); i < log; i-- ) {
        path = ( n & ( 1 << i ) );
        
        if ( path == LEFT )
            next = current->left;
        else
            next = current->right;
            ADD_TRAVERSALS(1) // current->left,right
            
        if ( next == NULL )
            break;
        else
            current = next;
    }

    return current;     
}

uint32_t int_log2( uint32_t n ) {
    if ( n == 0 )
        return 0;
    return ( 31 - __builtin_clz( n ) );
}

bool is_leaf( binary_pointer_heap *heap, binary_pointer_node* node ) {
        ADD_TRAVERSALS(1) // node
    return ( ( node->left == NULL ) && ( node->right == NULL ) );
}

