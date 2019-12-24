#include "file_manager.h"



// GLOBALS.

/* filenames and table ids */
file_t files[MAX_FILE_NUM];
// char* filenames[MAX_FILE_NUM];
// int tid[10];

/* number of files */
int num_files = 0;



int file_open_table(char* pathname) {

    //char f[20] = pathname;
    int table_id = open(pathname, O_RDWR | O_CREAT, 0666);

    /* find from filenames to check the name be opened before. */
    int i;
    for (i = 0; i < num_files; i++) {
        /* Case 2: file that opened before. */
        if (!strcmp(pathname, files[i].name)) {
            table_id = files[i].id;
            break;
        }
    }

    /* Error Case: Can't open because of exceeding max. */
    if (i == MAX_FILE_NUM) {
        return -1;
    }
    
    /* Case 1: file opened initially. */
    if (i == num_files) {

        strcpy(files[i].name, pathname);
        files[i].id = table_id;
        num_files++;
    }

    page_t* header = (page_t*)malloc(sizeof(page_t));
    file_read_page(table_id, HEADER_PAGE_OFFSET, header);

    if (header->header_page.num_pages == 0) {
        init_header_page(table_id, header);
    }
    free(header);
    return table_id;
}

void init_header_page(int table_id, page_t* header) {

    memset(header, 0, PAGE_SIZE);
    header->header_page.num_pages = 1;
    file_write_page(table_id, HEADER_PAGE_OFFSET, header);
}

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest) {
    pread(table_id, dest, PAGE_SIZE, pagenum * PAGE_SIZE);
}

// Write an in-memory page(src) to the on-disk page
void file_write_page(int table_id, pagenum_t pagenum, const page_t* src) {
        pwrite(table_id, src, PAGE_SIZE, pagenum * PAGE_SIZE);
}

// void file_write_tuple(int jid, tuplenum_t tuplenum, record_t* src) {
//     pwrite(jid, src, TUPLE_SIZE, tuplenum * TUPLE_SIZE);
// }