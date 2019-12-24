/*
 * bpt: disk-based B+Tree Implementation
 * Copyright (C) 2019 Hyeonseo Jo     https://github.com/jhcse5189
 * All rights reserved.
 *
 * Author:    Hyeonseo Jo
 *        https://github.com/jhcse5189
 * Original Date:     5 Oct 2019
 * Last modified:     4 Nov 2019
 *
 * This Implementation demonstrates the in-memory B+ tree DBMS
 * that contains disk space manager
 * for educational purposes, including binary file I/O, insertion, search, and deletion
 * with one column.
 *
 */

#ifndef __BPT_H__
#define __BPT_H__

#include "buffer_manager.h"
#include "utils.h"

// Default order is 4.
#define DEFAULT_ORDER 4



// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node. Every node has at most order - 1 keys and
 * and can have (roughly speaking) half that number
 * because it uses delayed merger when all keys in
 * the (leaf or internal) page have deleted.
 * ...
 */

/* order of tree */
extern int order;



// FUNCTION PROTOTYPES.

// OPEN AND EXIT.
int init_db(int num_buf);
int open_table( char * pathname );
void db_exit( void );



// SEARCH.
int db_find(int table_id, int64_t key, char * ret_val);
pagenum_t find_leaf(int table_id, pagenum_t root, int64_t key);



// INSERTION.
int db_insert(int table_id, int64_t key, char* value);

int insert_into_node(buffer_frame_t* node, int left_index, int64_t key, pagenum_t right);
int insert_into_node_after_splitting(buffer_frame_t* header, buffer_frame_t* old_node, int left_index, int64_t key, pagenum_t right);
int insert_into_new_root(buffer_frame_t* header, buffer_frame_t* left, int64_t key, buffer_frame_t* right);
int insert_into_parent(buffer_frame_t* header, buffer_frame_t* left, int64_t key, buffer_frame_t* right);
int get_left_index(buffer_frame_t* parent, pagenum_t left);
int insert_into_leaf(buffer_frame_t* leaf, int64_t key, char * value);
int insert_into_leaf_after_splitting(buffer_frame_t* header, buffer_frame_t* leaf, int64_t key, char* value);
int start_new_tree(buffer_frame_t* header, int64_t key, char* value);



// DELETION.
int db_delete( int table_id, int64_t key );

int delete_entry(buffer_frame_t* header, buffer_frame_t* node, int64_t key);
void remove_entry_from_node(buffer_frame_t* header, buffer_frame_t* node, int64_t key);
int adjust_root(buffer_frame_t* header, pagenum_t r);
int get_neighbor_index(buffer_frame_t* header, buffer_frame_t* node, buffer_frame_t* parent);
int coalesce_nodes(buffer_frame_t* header, buffer_frame_t* node, buffer_frame_t* neighbor, int neighbor_index, int64_t k_prime);
int redistribute_nodes(buffer_frame_t* header, buffer_frame_t* node, buffer_frame_t* neighbor, int neighbor_index, int k_prime_index, int64_t k_prime);



// JOIN
int join_table(int table_id_1, int table_id_2, char* pathname);
buffer_frame_t* get_buf_next_leaf(int table_id, pagenum_t n);

#endif /* __BPT_H__*/
