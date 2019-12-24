#ifndef __UTIL_H__
#define __UTIL_H__

#include "types.h"
#include "file_manager.h"

#define Version "0.8"
#define QUEUE_SIZE 1000

// GLOBALS.

/* queue */
Queue* queue;



// OUTPUT AND UTILITIES

/* print */

void print_notice(void);
void print_usage(void);

void print_database(void);
void print_tables(void);


/* queue*/
void init_queue(void);
int isEmpty(void);
int isFull(void);
void enqueue(pagenum_t n);
pagenum_t dequeue(void);

/* bpt */

void print_tree(int table_id);
int cut(int length);

#endif /* __UTIL_H__*/
