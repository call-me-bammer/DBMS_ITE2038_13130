#include "buffer_manager.h"



// GLOBALS.

/* buffer pool */
buffer_frame_t pool[BUF_SIZE];

/* clock hand */
int clk_hand = 0;



/* Buffer alloc, free manager */

// Allocate an on-disk page from the free page list
buffer_frame_t* buffer_alloc_page(buffer_frame_t* header) {

    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        // find empty buffer frame space.
        if (pool[i].table_id == 0) {
            break;
        }
    }

    // fail to load because of full buffer.
    if (i == BUF_SIZE) {
        //printf("is is full, %d, so before clk_req_empty()\n", i);
        i = clock_request_empty();
    }
    memset(pool + i, 0, FRAME_SIZE);

    //buffer_frame_t* alloc = pool + i;
    pool[i].table_id = header->table_id;

    // Case 1: Header page has no free page.
    if (header->frame.header_page.free == 0) {
        pool[i].page_num = header->frame.header_page.num_pages;
        pool[i].frame.free_page.next = 0;
        header->frame.header_page.free = pool[i].page_num;
        ++header->frame.header_page.num_pages;
    }

    // Case 2: Header page has free page(s).
    //alloc = get_buf_frame(header->table_id, header->frame.header_page.free);
    pool[i].page_num = header->frame.header_page.free;
    header->frame.header_page.free = pool[i].frame.free_page.next;

    header->is_dirty = 1;

    pool[i].is_dirty = 1;
    pool[i].is_pinned = 1;
    pool[i].refbit = 1;

    return pool + i;
}

// // Allocate an on-disk page from the free page list
// buffer_frame_t* buffer_alloc_page(buffer_frame_t* header) {

//     // Case 1: Header page has no free page.
//     if (header->frame.header_page.free == 0) {

//         buffer_frame_t* empty = pool + clock_request_empty();
//         empty->table_id = header->table_id;
//         empty->page_num = header->frame.header_page.num_pages;
//         empty->frame.free_page.next = 0;

//         header->frame.header_page.free = empty->page_num;
//         ++header->frame.header_page.num_pages;
//     }

//     // Case 2: Header page has free page(s).
//     buffer_frame_t* free = get_buf_frame(header->table_id, header->frame.header_page.free);
//     header->frame.header_page.free = free->frame.free_page.next;

//     header->is_dirty = 1;
//     return free;
// }

// Free an on-disk page to the free page list
void buffer_free_page(buffer_frame_t* header, buffer_frame_t* page) { 

    memset(page, 0, PAGE_SIZE);
    page->frame.free_page.next = header->frame.header_page.free;
    page->is_dirty = 1;

    header->frame.header_page.free = page->page_num;
    header->is_dirty = 1;
}



/* DB initialization function. */


/* Allocate the buffer pool (array) with the given number of entries and
 * initialize other fileds (state info, LRU info ...) with my own desin.
 *
 * return 0 or NONZERO;
 */
int buf_init(int num_buf) {

    int i;
    //pool = (buffer_frame_t*)malloc(sizeof(buffer_frame_t) * num_buf);
    
    // if (pool == NULL) { 
    //     perror("buf_init");
    //     return -1;
    // }

    for (i = 0; i < num_buf; i++) {

        //memset(&pool[i].frame, 0, PAGE_SIZE);
        pool[i].table_id = 0;
        pool[i].is_dirty = 0;
        pool[i].is_pinned = 0;
    }
    return 0;
}

/* Open existing data file or create one if not existed. */


/* If success, return the unique table_id, witch represents
 * the own table in this DB. (ret negative value if error occurs)
 * Anyway, modify prev. open_db interface to open_table.
 *
 * You have to maintain a table id once open_table() is called,
 * which is matching file descriptor or file pointer
 * depending on your previous implementation.
 * (table id ≥ 1 and maximum allocated id is set to 10)
 */ 
int buf_open_table(char* pathname) {
    return file_open_table(pathname);
}

/* Buffer frame manager */
/*
=> page가 buffer pool에 없다, cache-miss하다면
   disk에서 page를 읽어와서 buffer block에 maintain한다.

   page modification은 in-memory buffer에서만 일어난다.
   buffer에 있는 page frame이 update되었다면,
   buffer block의 is_dirty를 mark한다.

   LRU policy에 의하면, (least recently used, 가장 과거에 사용한 게 안ㅡㅡ중요)
   LRU buffer는 page eviction의 vimtim이다.
   그래서 LRU page eviction 동안 disk에 page writing이 이루어진다.
*/
// 1. find the frame from current buffer pool.
buffer_frame_t* get_buf_frame(int table_id, pagenum_t n) {

    int i;
    //buffer_frame_t* tmp_frame;
    for (i = 0; i < BUF_SIZE; i++) {
        if (pool[i].table_id == table_id && pool[i].page_num == n) {
            break;
        }
    }

    // requested frame exists.
    if (i != BUF_SIZE) {
        pool[i].is_pinned = 1;
        pool[i].refbit = 1;
        return pool + i;
    }


    // doesn't exist.
    // buffer_load_page 안에서는.. 핀..을 꽂아줌.
    i = buffer_load_page(table_id, n);

    if (i == -1) {
        perror("get_buf_frame");
        return NULL;
    }
    return pool + i;
}

// 2. load page from disk to current buffer pool.
//    return index of its pool, or -1 if failed.
int buffer_load_page(int table_id, pagenum_t n) {

    int i;
    page_t* tmp = (page_t*)malloc(sizeof(page_t));
    for (i = 0; i < BUF_SIZE; i++) {
        // find empty buffer frame space.
        if (pool[i].table_id == 0) {

            // read page from disk.
            file_read_page(table_id, n, tmp);

            memcpy((pool + i), tmp, PAGE_SIZE);
            pool[i].table_id = table_id;
            pool[i].page_num = n;
            pool[i].is_pinned = 1; // because of "load", not ref.
            pool[i].refbit = 1;
            break;
        }
    }

    // fail to load because of full buffer.
    if (i == BUF_SIZE) {

        // TODO: page replacement in buffer.
        i = clock_request_page(table_id, n);
    }

    // successfully loaded.
    free(tmp);
    return i;
}

buffer_frame_t* get_buf_empty(buffer_frame_t* header) {

    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        // find empty buffer frame space.
        if (pool[i].table_id == 0) {
            break;
        }
    }

    // fail to load because of full buffer.
    if (i == BUF_SIZE) {
        i = clock_request_empty();
    }

    if (header->frame.header_page.free == 0) {
        pool[i].page_num = header->frame.header_page.num_pages;
        ++header->frame.header_page.num_pages;
    } else {
        pool[i].page_num = header->frame.header_page.free;
        buffer_frame_t* free = get_buf_frame(header->table_id, header->frame.header_page.free);
        header->frame.header_page.free = free->frame.free_page.next;
    }

    pool[i].table_id = header->table_id;
    pool[i].is_dirty = 1;
    pool[i].is_pinned = 1;
    pool[i].refbit = 1;
    return pool + i;
}

/* Flush all dirty frames. */

/* Write all dirty pages of the corresponding table_id
 * from buffer to disk and discard the table id,
 * then close the table.
 */
int close_table(int table_id) {

    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        if (pool[i].table_id == table_id && pool[i].is_dirty == 1) {
            file_write_page(table_id, pool[i].page_num, &pool[i].frame);
            //printf("FLUSH: tid %d, frame %d, page_num %ld\n", table_id, i, pool[i].page_num);
            pool[i].is_dirty = 0;
            pool[i].table_id = 0;
        }
    }
    
    close(table_id);
    return 0;
}

/* Destroy buffer manager. */

/* Flush all data from buffer and
 * destroy allocated frames.
 */
int shutdown_db(void) {
    
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        close_table(files[i].id);
    }
    return 0;
}

/* Clock page replacement policy */

/* give second change.
 * 
 * TODO: define clk_hand as a global variable.
 * -> to indicate 0 to BUF_SIZE - 1.
 */
int clock_request_page(int table_id, pagenum_t pg_num) {

    int retval = -1;
    buffer_frame_t* current = pool + clk_hand;

    while (retval == -1) {

        current = pool + clk_hand;
        
        // the happy case: replace current page
        if (current->is_pinned == 0 && current->refbit == 0) {
            if (current->is_dirty == 1) {
                file_write_page(current->table_id, current->page_num, &(current->frame));
                //printf("WRITE: tid %d, frame %d, page_num %ld\n", current->table_id, clk_hand, current->page_num);
            }
            file_read_page(table_id, pg_num, &(current->frame));
            current->page_num = pg_num;
            current->table_id = table_id;
            current->is_dirty = 0;
            current->is_pinned = 1;
            current->refbit = 1; // referenced!
            retval = clk_hand;
        }
        // second change: unset reference bit
        else if (current->is_pinned == 0 && current->refbit == 1) {
            current->refbit = 0;
        }
        // else is_pinned >= 1, so skip

        clk_hand = (clk_hand + 1) % BUF_SIZE; // advance clock hand
    }
    return retval;
}

int clock_request_empty(void) {

    int retval = -1;
    buffer_frame_t* current = pool + clk_hand;

    while (retval == -1) {

        current = pool + clk_hand;
        
        // the happy case: replace current page
        if (current->is_pinned == 0 && current->refbit == 0) {
            if (current->is_dirty == 1) {
                file_write_page(current->table_id, current->page_num, &(current->frame));
                //printf("WRITE: tid %d, frame %d, page_num %ld\n", current->table_id, clk_hand, current->page_num);
            }
            current->table_id = 0;


            //current->is_pinned = 1;
            //current->refbit = 1; // referenced!
            retval = clk_hand;
        }
        // second change: unset reference bit
        else if (current->is_pinned == 0 && current->refbit == 1) {
            current->refbit = 0;
        }
        // else is_pinned >= 1, so skip

        clk_hand = (clk_hand + 1) % BUF_SIZE; // advance clock hand
    }
    return retval;
}

void unpin(buffer_frame_t* f) {
    f->is_pinned = 0;
}

/* Debug pool */
void debug_pool(void) {

    printf("| buf_id | tid-page_num | pin-dirty-ref ||\n\n");

    int i, j, unit_factor = 3;
    
    // printf("+");
    // for (j = 0; j < unit_factor; j++) printf("--------------------");
    // printf("+\n");
    printf("+-------------------------------------------------------------+\n");
    
    for (i = 0; i < BUF_SIZE; i++) {
        printf("| %2d | %d-%2ld | %d-%d-%d |"
                 , i, pool[i].table_id, pool[i].page_num, pool[i].is_pinned, pool[i].is_dirty, pool[i].refbit);
        
        if (i % unit_factor == unit_factor - 1) {
            // printf("\n+");
            // for (j = 0; j < unit_factor; j++) printf("--------------------");
            // printf("+\n");
            printf("\n+-------------------------------------------------------------+\n");
        }
    }
    printf("\n");
}

// void buffer_write_tuple(int jid, buffer_frame_t* l1, int r, buffer_frame_t* l2, int s, char* pathname, tuplenum_t tuplenum) {

//     printf("jid: %d\n", jid);

//     file_write_tuple(jid, tuplenum, l1->frame.node.records + r);
//     file_write_tuple(jid, tuplenum + 1, l2->frame.node.records + s);
// }