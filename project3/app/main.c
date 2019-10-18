#include "bpt.h"
#include "dbapi.h"

// MAIN

int main(int argc, char ** argv) {
    int tid, input, range2;
    char instruction;
    char value[1024];

    init_db(4);
    tid = open_table("datafile");

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
            }
            break;
        case 'd':
            scanf("%d", &input);
            delete(tid, input);
            // print_tree(&GLOBAL_MANAGER);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            insert(tid, input, value);
            // print_tree(&GLOBAL_MANAGER);
            break;
        case 'f':
            scanf("%d", &input);
            if (find(tid, input, value) == SUCCESS) {
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
            // find_and_print_range(input, range2, &GLOBAL_MANAGER);
            break;
        case 'l':
            // print_leaves(&GLOBAL_MANAGER);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            shutdown_db();
            return 0;
            break;
        case 't':
            // print_tree(&GLOBAL_MANAGER);
            break;
        case 'x':
            // destroy_tree(&GLOBAL_MANAGER);
            // print_tree(&GLOBAL_MANAGER);
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
