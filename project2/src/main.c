#include "bpt.h"
#include "dbapi.h"

#define INVOKE(method, tid)                     \
if (tid != -1) {                                \
    method(get_file_manager(tid));              \
} else {                                        \
    printf("invalid tid\n");                    \
}

#define INVOKE_VARIADIC(method, tid, ...)       \
if (tid != -1) {                                \
    method(__VA_ARGS__, get_file_manager(tid)); \
} else {                                        \
    printf("invalid tid\n");                    \
}

// MAIN

int main(int argc, char ** argv) {
    int tid, input, range2;
    char instruction;
    char value[1024];

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
            db_delete(input);
            INVOKE(print_tree, tid);
            break;
        case 'i':
            scanf("%d", &input);
            snprintf(value, 100, "%d value", input);
            db_insert(input, value);
            INVOKE(print_tree, tid);
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
            INVOKE_VARIADIC(find_and_print_range, tid, input, range2);
            break;
        case 'l':
            INVOKE(print_leaves, tid);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            close_table(tid);
            file_vec_free(&GLOBAL_FILE_MANAGER);
            return 0;
            break;
        case 't':
            INVOKE(print_tree, tid);
            break;
        case 'x':
            INVOKE(destroy_tree, tid);
            INVOKE(print_tree, tid);
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
