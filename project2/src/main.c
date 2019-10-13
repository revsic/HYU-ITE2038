#include "bpt.h"
#include "dbapi.h"

// MAIN

int main(int argc, char ** argv) {
    int tid, input, range2;
    char instruction;
    char value[1024];
    struct file_manager_t* manager;

    tid = open_table("datafile");
    manager = get_file_manager(tid);

    usage_1();
    usage_2();

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'o':
            // close table
            if (tid != -1) {
                close_table(tid);
            }
            // open table
            scanf("%1023s", value);
            tid = open_table(value);
            // table id validation
            if (tid == -1) {
                printf("cannot open file %s\n", value);
                break;
            }
            manager = get_file_manager(tid);
            break;
        case 'd':
            scanf("%d", &input);
            db_delete(input);
            print_tree(manager);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            db_insert(input, value);
            print_tree(manager);
            break;
        case 'f':
            scanf("%d", &input);
            if (db_find(input, value) == SUCCESS) {
                printf("Key: %d  Value: %s\n", input, value);
            } else {
                printf("not found\n");
            }
            //find_and_print(input, manager);
            break;
        case 'r':
            scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            find_and_print_range(input, range2, manager);
            break;
        case 'l':
            print_leaves(manager);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            close_table(tid);
            return 0;
            break;
        case 't':
            print_tree(manager);
            break;
        case 'x':
            destroy_tree(manager);
            print_tree(manager);
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
