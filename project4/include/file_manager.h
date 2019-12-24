#ifndef __DISK_H__
#define __DISK_H__

#include "types.h"



// One page size is 4096 Bytes.
#define PAGE_SIZE 4096
#define TUPLE_SIZE 128

#define HEADER_PAGE_OFFSET 0

// Maximum open tables number.
#define MAX_FILE_NUM 10



// GLOBALS.

/* filenames and table ids */
extern file_t files[MAX_FILE_NUM];
// extern char* filenames[MAX_FILE_NUM];
// extern int tid[10];

/* number of files */
extern int num_files;



int file_open_table(char* pathname);

void init_header_page(int table_id, page_t* tmp_h);

/* file space manager (same as disk space manager) API */

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page( void );
// Free an on-disk page to the free page list
void file_free_page(pagenum_t pagenum);
// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int table_id, pagenum_t pagenum, page_t * dest);
// Write an in-memory page(src) to the on-disk page
void file_write_page(int table_id, pagenum_t pagenum, const page_t * src);
// void file_write_tuple(int jid, tuplenum_t tuplenum, record_t* src);

#endif /* __DISK_H__*/
