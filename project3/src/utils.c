#include "utils.h"



// GLOBALS.

extern Queue* queue;



// OUTPUT AND UTILITIES

/* print */

void print_notice(void) {
    printf("Welcome to the SQL client.\n"
           "version: %s\n\n", Version);
    printf("Type 'h' for help.\n\n");
}

void print_usage(void) {
    printf("List of all commands that my SQL client supports.\n\n"
    "insert\ti <k> <v>  -- Insert the key-value pair into the tree.\n"
    "delete\td <k>  -- Delete the key from the tree.\n"
    "find\tf <k>  -- Find the key from the free.\n"
    "open\to <f>  -- Open or change the file with its name.\n"
    "quit\tq  -- Quit SQL client.\n"
    "help\th  -- Display this help.\n"
    "tree\tt <id> -- Print all tree of the table_id.\n"
    "op1\t1  -- Print database.\n"
    "op2\t2  -- Print tables.\n"
    "op3\t3  -- Debug buffer pool.\n\n");
}

void print_database(void) {

    printf(
    "+-------------+\n"
    "| Database    |\n"
    "+-------------+\n"
    "| SQL client  |\n"
    "+-------------+\n"
    "1 row in set\n\n");
}

void print_tables(void) {

    if (num_files == 0) {
        printf("Empty set\n\n");
        return;
    }

    int i;
    printf(
    "+----------------------+\n"
    "| Tables_in_SQL_client |\n"
    "+----------------------+\n");

    for (i = 0; i < num_files; i++) {
        printf("| %-20s |\n", files[i].name);
    }
    printf("+----------------------+\n");

    if (i == 1) printf("1 row in set\n\n");
    else printf("%d rows in set\n\n", i);

}


/* queue */

void init_queue(void) {
    
    queue = (Queue*)malloc(sizeof(Queue));
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    queue->pages = (pagenum_t*)malloc(sizeof(pagenum_t) * QUEUE_SIZE);
}

int isEmpty(void) {
    if (queue->front == queue->rear + 1) return 1;
    return 0;
}

int isFull(void) {
    if (queue->size == QUEUE_SIZE) return 1;
    return 0;
}

void enqueue(pagenum_t n) {

    if (!isFull()) {
        queue->rear++;
        queue->pages[queue->rear] = n;
    }
}

pagenum_t dequeue(void) {

    if (!isEmpty()) {
        int tmp_front = queue->front;
        queue->front++;
        return queue->pages[tmp_front];
    }
}


/* bpt */

void print_tree(int table_id) {

    if (table_id == -1) {
        printf("Invalid table_id: %d\n", table_id);
        return;
    }

    page_t header;
    file_read_page(table_id, 0, &header);

    //root doesn't exist!
    if (header.header_page.root == 0) {
        printf("Empty tree.\n");
        return;
    }

    int i, max_level_key = 0;
    pagenum_t n;

    init_queue();
    enqueue(header.header_page.root);

    while (!isEmpty()) {

        n = dequeue();
        page_t* tmp_n = (page_t*)malloc(sizeof(page_t));
        file_read_page(table_id, n, tmp_n);

        if (tmp_n->node.is_leaf) {
            for (i = 0; i < tmp_n->node.num_keys; i++)
                printf("%ld ", tmp_n->node.records[i].key);
            printf("| ");

            if (tmp_n->node.records[i - 1].key >= max_level_key)
                printf("\n");
        }

        else {
            enqueue(tmp_n->node.right_or_indexing);
            for (i = 0; i < tmp_n->node.num_keys; i++) {
                printf("%ld ", tmp_n->node.internal[i].key);
                enqueue(tmp_n->node.internal[i].pointer);
            }
            printf("| ");
            if (tmp_n->node.internal[i - 1].key >= max_level_key) {
                max_level_key = tmp_n->node.internal[i - 1].key;
                printf("\n");
            }
        }
        free(tmp_n);
    }
    free(queue);
}

int cut(int length) {
    if (length % 2 == 0)
        return length / 2;
    else
        return length / 2 + 1;
}