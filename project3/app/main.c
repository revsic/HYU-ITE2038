#include <string.h>

#include "bpt.h"
#include "dbapi.h"

// MAIN

int main(int argc, char ** argv) {
    int input, range2;
    char instruction;
    char value[1024];

    // init_db(4);
    // tablenum_t table_id = open_table("datafile");
    struct record_t rec;
    struct bpt_t bpt;
    struct buffer_manager_t buffers;
    struct file_manager_t file;
    file_open(&file, "datafile");
    buffer_manager_init(&buffers, 10);
    bpt_init(&bpt, &file, &buffers);

    usage_1(&bpt);
    usage_2(&bpt);

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'o':
            // open table
            // scanf("%1023s", value);
            // input = open_table(value);
            // table id validation
            // if (input == -1) {
            //     printf("cannot open file %s\n", value);
            // } else {
            //     close_table(table_id);
            //     table_id = input;
            // }
            break;
        case 'd':
            scanf("%d", &input);
            bpt_delete(&bpt, input);
            print_tree(&bpt);
            // db_delete(table_id, input);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            bpt_insert(&bpt, input, value, strlen(value) + 1);
            print_tree(&bpt);
            // db_insert(table_id, input, value);bpt);
            break;
        case 'f':
            scanf("%d", &input);
            // if (db_find(table_id, input, value) == SUCCESS) {
            if (bpt_find(&bpt, input, &rec) == SUCCESS) {
                printf("Key: %d  Value: %s\n", input, rec.value);
            } else {
                printf("not found\n");
            }
            break;
        case 'r':
            scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            find_and_print_range(&bpt, input, range2);
            break;
        case 'l':
            print_leaves(&bpt);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            // shutdown_db();
            bpt_release(&bpt);
            buffer_manager_shutdown(&buffers);
            file_close(&file);
            return 0;
            break;
        case 't':
            print_tree(&bpt);
            break;
        case 'x':
            destroy_tree(&bpt);
            print_tree(&bpt);
            break;
        default:
            usage_2(&bpt);
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return 0;
}
