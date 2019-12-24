#include "bpt.h"

// MAIN

int main(int argc, char** argv) {

    int64_t key;
    char value[120];

    char filename[20];
    char instruction;

    int ret_val;
    int table_id = -1;
    
    // for join.
    int tid1 = -1, tid2 = -1;
    char pathname[] = "result_join";

    init_db(6);

    print_notice();
    printf("sql> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
            case 'd':
                scanf("%ld", &key);
                ret_val = db_delete(table_id, key);
                if (ret_val == 0) {
                    //printf("Successfully deleted.\n");
                }
                else {
                    printf("Can't delete the key %ld with tid %d\n", key, table_id);
                }
                break;
            case 'i':
                scanf("%ld %s", &key, value);
                ret_val = db_insert(table_id, key, value);
                if (ret_val == 0) {
                    //printf("Successfully inserted.\n");
                }
                else {
                    printf("Can't insert the key %ld with tid %d\n", key, table_id);
                }
                break;
            case 'f':
                scanf("%ld", &key);
                if (table_id == -1) {
                    printf("Ivalid table_id: %d\n", table_id);
                    break;
                }
                ret_val = db_find(table_id, key, value);
                if (ret_val == 0) {
                    printf("Found the key-value pair with tid %d\n", table_id);
                } else {
                    printf("Record not found under key with tid %d\n", table_id);
                }
                break;
            case 'j':
                scanf("%d %d", &tid1, &tid2);
                if (tid1 == -1 || tid2 == -1 || tid1 == tid2) {
                    printf("Invalid table_id: %d %d\n", tid1, tid2);
                    break;
                }
                ret_val = join_table(tid1, tid2, pathname);
                if (ret_val == 0) {
                    //printf("Successfully joined.\n");
                } else {
                    printf("Given tables with id %d, %d isn't opened.\n", tid1, tid2);
                }
                break;
            case 'o':
                scanf("%s", filename);
                ret_val = open_table(filename);
                
                if (ret_val == -1) {
                    printf("Can't open file\n");
                } else {
                    table_id = ret_val;
                    printf("File is opened with id %d\n", table_id);
                }
                break;
            case 'q':
                while (getchar() != (int)'\n');
                return 0;
                break;
            case 'h':
                print_usage();
                break;
            case 't':
                print_tree(table_id);
                break;
            case '1':
                print_database();
                break;
            case '2':
                print_tables();
                break;
            case '3':
                debug_pool();
                break;
        }
        while (getchar() != (int)'\n');
        printf("sql> ");
    }
    printf("\n");
    return 0;
}
