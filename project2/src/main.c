#include "bpt.h"
#include "disk_manager.h"

// MAIN

int main(int argc, char ** argv) {
    int input, range2;
    char instruction;
    struct file_manager_t manager;
    file_open("datafile", &manager);

    usage_1();
    usage_2();

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd':
            scanf("%d", &input);
            delete(input, &manager);
            print_tree(&manager);
            break;
        case 'i':
            scanf("%d", &input);
            insert(input, input, &manager);
            print_tree(&manager);
            break;
        case 'f':
            scanf("%d", &input);
            find_and_print(input, &manager);
            break;
        case 'r':
            scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            find_and_print_range(input, range2, &manager);
            break;
        case 'l':
            print_leaves(&manager);
            break;
        case 'q':
            while (getchar() != (int)'\n');
            return 0;
            break;
        case 't':
            print_tree(&manager);
            break;
        case 'x':
            destroy_tree(&manager);
            print_tree(&manager);
            break;
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    file_close(&manager);
    return 0;
}
