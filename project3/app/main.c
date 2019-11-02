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
    struct table_t table;
    struct buffer_manager_t buffers;
    buffer_manager_init(&buffers, 10);
    table_load(&table, "datafile", &buffers);

    usage_1(&table.bpt);
    usage_2(&table.bpt);

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
            table_delete(&table, input);
            print_tree(&table.bpt);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            table_insert(&table, input, value, strlen(value) + 1);
            print_tree(&table.bpt);
            break;
        case 'f':
            scanf("%d", &input);
            if (table_find(&table, input, &rec) == SUCCESS) {
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
            find_and_print_range(&table.bpt, input, range2);
            break;
        case 'l':
            print_leaves(&table.bpt);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            buffer_manager_shutdown(&buffers);
            table_release(&table);
            return 0;
            break;
        case 't':
            print_tree(&table.bpt);
            break;
        case 'x':
            destroy_tree(&table.bpt);
            print_tree(&table.bpt);
            break;
        default:
            usage_2(&table.bpt);
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return 0;
}
