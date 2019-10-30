#include "bpt.h"
#include "dbapi.h"

// MAIN

int main(int argc, char ** argv) {
    int input, range2;
    char instruction;
    char value[1024];

    init_db(4);
    tablenum_t table_id = open_table("datafile");
    struct bpt_t* bpt = &table_manager_find(&GLOBAL_DBMS.tables, table_id)->bpt;

    usage_1(bpt);
    usage_2(bpt);

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
                close_table(table_id);
                table_id = input;
            }
            break;
        case 'd':
            scanf("%d", &input);
            db_delete(table_id, input);
            print_tree(bpt);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            db_insert(table_id, input, value);
            print_tree(bpt);
            break;
        case 'f':
            scanf("%d", &input);
            if (db_find(table_id, input, value) == SUCCESS) {
                printf("Key: %d  Value: %s\n", input, value);
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
            find_and_print_range(bpt, input, range2);
            break;
        case 'l':
            print_leaves(bpt);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            shutdown_db();
            return 0;
            break;
        case 't':
            print_tree(bpt);
            break;
        case 'x':
            destroy_tree(bpt);
            print_tree(bpt);
            break;
        default:
            usage_2(bpt);
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    return 0;
}
