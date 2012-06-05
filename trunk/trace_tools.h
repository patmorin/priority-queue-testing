#ifndef PQ_TRACE_TOOLS
#define PQ_TRACE_TOOLS

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#define PQ_OP_CREATE        0
#define PQ_OP_DESTROY       1
#define PQ_OP_CLEAR         2
#define PQ_OP_GET_KEY       3
#define PQ_OP_GET_ITEM      4
#define PQ_OP_GET_SIZE      5
#define PQ_OP_INSERT        6
#define PQ_OP_FIND_MIN      7
#define PQ_OP_DELETE        8
#define PQ_OP_DELETE_MIN    9
#define PQ_OP_DECREASE_KEY  10
#define PQ_OP_MELD          11
#define PQ_OP_EMPTY         12

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "typedefs.h"

struct pq_trace_header
{
    uint64_t op_count;
    uint32_t pq_ids;
    uint32_t node_ids;
    uint32_t max_live_nodes;
} __attribute__ ((packed, aligned(4)));

struct pq_op_create
{
    uint32_t code;
    //! specified destination for created pointer
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_destroy
{
    uint32_t code;
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_clear
{
    uint32_t code;
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_get_key
{
    uint32_t code;
    uint32_t pq_id;
    uint32_t node_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_get_item
{
    uint32_t code;
    uint32_t pq_id;
    uint32_t node_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_get_size
{
    uint32_t code;
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_insert
{
    uint32_t code;
    uint32_t pq_id;
    uint32_t node_id;
    key_type key;
    item_type item;
} __attribute__ ((packed, aligned(4)));

struct pq_op_find_min
{
    uint32_t code;
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_delete
{
    uint32_t code;
    uint32_t pq_id;
    uint32_t node_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_delete_min
{
    uint32_t code;
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_decrease_key
{
    uint32_t code;
    uint32_t pq_id;
    uint32_t node_id;
    key_type key;
} __attribute__ ((packed, aligned(4)));

struct pq_op_meld
{
    uint32_t code;
    uint32_t pq_src1_id;
    uint32_t pq_src2_id;
    //! id to use for the newly generated heap, i.e. where to store the pointer
    uint32_t pq_dst_id;
} __attribute__ ((packed, aligned(4)));

struct pq_op_empty
{
    uint32_t code;
    uint32_t pq_id;
} __attribute__ ((packed, aligned(4)));

typedef struct pq_trace_header pq_trace_header;
typedef struct pq_op_create pq_op_create;
typedef struct pq_op_destroy pq_op_destroy;
typedef struct pq_op_clear pq_op_clear;
typedef struct pq_op_get_key pq_op_get_key;
typedef struct pq_op_get_item pq_op_get_item;
typedef struct pq_op_get_size pq_op_get_size;
typedef struct pq_op_insert pq_op_insert;
typedef struct pq_op_find_min pq_op_find_min;
typedef struct pq_op_delete pq_op_delete;
typedef struct pq_op_delete_min pq_op_delete_min;
typedef struct pq_op_decrease_key pq_op_decrease_key;
typedef struct pq_op_meld pq_op_meld;
typedef struct pq_op_empty pq_op_empty;

/**
 * Dummy struct.  Primarily for use as a placeholder for allocation and to
 * determine op type via the code field.  Should be large enough to encompass
 * any other op struct.
 */
typedef struct pq_op_insert pq_op_blank;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Writes a proper trace header with the information specified in the input.
 * Rewinds the file to the beginning to allow for the header to be written at
 * the end.
 *
 * @param file      File to write header to
 * @param header    Header to write
 * @return          0 on success, -1 on error
 */
int pq_trace_write_header( int file, pq_trace_header header );

/**
 * Reads header from the specified file and writes to passed struct.  Assumes
 * file is currently at beginning.
 *
 * @param file      File to read from.
 * @param header    Address of struct to write header info to
 * @return          0 on success, -1 on error
 */
int pq_trace_read_header( int file, pq_trace_header *header );

/**
 * Takes any priority queue operation struct and writes to the current position
 * in the input file.  Detects operation type based on code field.
 *
 * @param file  File to write to
 * @param op    Operation to write out
 * @return      0 on success, -1 on error
 */
int pq_trace_write_op( int file, void *op );

/**
 * Reads an operation from the input file.  Writes to address specified by op.
 * For memory safety, it must be at least as long as the longest operation
 * struct.  For any practical key and item types, this will be pq_op_insert.
 *
 * @param file  File to read from
 * @param op    Operation to write out
 * @return      0 on success, -1 on error
 */
int pq_trace_read_op( int file, void *op );

#endif
