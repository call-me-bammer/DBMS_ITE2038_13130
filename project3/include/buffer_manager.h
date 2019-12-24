#ifndef __BUF_M__
#define __BUF_M__

#include "file_manager.h"

#define BUF_SIZE 6
#define FRAME_SIZE 4136

// GLOBALS.

/* buffer pool */
extern buffer_frame_t pool[BUF_SIZE];

/* clock hand */
extern int clk_hand;


/* Buffer alloc, free manager */
buffer_frame_t* buffer_alloc_page(buffer_frame_t* header);
void buffer_free_page(buffer_frame_t* header, buffer_frame_t* page);

/* DB initialization function. */
int buf_init(int num_buf);
int buf_open_table(char* pathname);

/* Buffer frame manager */
buffer_frame_t* get_buf_frame(int table_id, pagenum_t n);
int buffer_load_page(int table_id, pagenum_t n);
buffer_frame_t* get_buf_empty(buffer_frame_t* header);

/* Flush all dirty frames. */
int close_table(int table_id);
/* Destroy buffer manager. */
int shutdown_db(void);

/* Clock page replacement policy */
int clock_request_page(int table_id, pagenum_t pg_num);
int clock_request_empty(void);
void unpin(buffer_frame_t* f);

/* Debug pool */
void debug_pool(void);

#endif /* __BUF_M__ */