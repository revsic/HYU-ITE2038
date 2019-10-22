#include "bpt.h"
#include "dbapi.h"

// MAIN

int main(int argc, char ** argv) {
    int input, range2;
    char instruction;
    char value[1024];

    struct dbms_table_t table = { &GLOBAL_DBMS, INVALID_TABLENUM };

    init_db(4);
    table.table_id = open_table("datafile");

    usage_1();
    usage_2();

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'o':
            // open table
            scanf("%1023s", value);
            input = open_table(value);
            // table id validation
            if (input == -1) {
                printf("cannot open file %s\n", value);
            } else {
                close_table(table.table_id);
                table.table_id = input;
            }
            break;
        case 'd':
            scanf("%d", &input);
            delete(table.table_id, input);
            print_tree(&table);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            insert(table.table_id, input, value);
            print_tree(&table);
            break;
        case 'f':
            scanf("%d", &input);
            if (find(table.table_id, input, value) == SUCCESS) {
                printf("Key: %d  Value: %s\n", input, value);
            } else {
                printf("not found\n");
            }
            find_and_print(input, &table);
            break;
        case 'r':
            scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            find_and_print_range(input, range2, &table);
            break;
        case 'l':
            print_leaves(&table);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            shutdown_db();
            return 0;
            break;
        case 't':
            print_tree(&table);
            break;
        case 'x':
            destroy_tree(&table);
            print_tree(&table);
            break;
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return 0;
}
