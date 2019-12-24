#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>



// TYPES.


// means page number, not offset.
typedef uint64_t pagenum_t;



/* One record requires 128 Bytes.
     Therefore maximum 31 records per one data page
     can be contained. */

typedef struct record_t {
    int64_t key;
    char value[120];
} record_t;


typedef struct internal_t {
    int64_t key;
    pagenum_t pointer;
} internal_t;


/*
 * Type representing a node in the disk-based B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 */

/* Leaf and Internal Page */



typedef struct node_t {
    /* Page Header is different between leaf and internal page
         ,but it requires 128 Bytes
         except records / indexing key and page pointer area. */

    pagenum_t parent; //because, it doesn't need union with free_page_t.
    int is_leaf;
    int num_keys;
    char reserved[104];
    pagenum_t right_or_indexing;

    /* Leaf Page contains the key-value records
         ,otherwise Internal Page contains the key-(indexing)pointer pairs. */

    /* union, it means a single variable, so using same memory location. */

    union {
        struct {
            internal_t internal[248];
        };
        struct {
            record_t records[31];
        };
    };

} node_t;



/* Header Page requires 24 Bytes except reserved space. */
typedef struct header_page_t {

    pagenum_t free;
    pagenum_t root;
    int64_t num_pages;
    char reserved[4072];

} header_page_t;



/* Free Page requires 8 Bytes except not using space. */
typedef struct free_page_t {

    pagenum_t next;
    char reserved[4088];

} free_page_t;



typedef union page_t {

    header_page_t header_page;
    free_page_t free_page;
    node_t node;

} page_t;



typedef struct Queue Queue;
struct Queue {
    int front, rear;
    int size;
    pagenum_t* pages;
};



typedef struct buffer_frame_t {

    page_t frame;
    int table_id; // >= 1 and maximum allocated id is set to 10 per file
    pagenum_t page_num;
    int is_dirty;
    int is_pinned;
    pagenum_t next_or_prev_LRU; // TODO: type of next_or_prev_LRU...
    int refbit;
    //Other info. can be added.
} buffer_frame_t;



typedef struct file_t {
    char name[20];
    int id;
} file_t;

#endif /* __TYPES_H__ */