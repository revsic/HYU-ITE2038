#include <stdio.h>
#include <time.h>

#include "dbapi.h"
#include "headers.h"

int main() {
    int instruction;
    int input;
    int table_id;
    int table_size = 0;
    char value[1024];
    tablenum_t tables[100];
    FILE* fp = fopen("input.txt", "r");

    const int buf_size = 100000;
    init_db(buf_size);

    int count = 0;
    double start_time = (double)clock() / CLOCKS_PER_SEC;
    while (fscanf(fp, "%d", &instruction) != EOF) {
        ++count;
        printf("\r%d", count);

        switch (instruction) {
        case 0:
            fscanf(fp, "%s", value);
            tables[table_size++] = open_table(value);
            break;
        case 1:
            fscanf(fp, "%d %d %s", &table_id, &input, value);
            db_insert(tables[table_id - 1], input, value);
            break;
        case 2:
            fscanf(fp, "%d %d", &table_id, &input);
            db_find(tables[table_id - 1], input, NULL);
            break;
        case 3:
            fscanf(fp, "%d %d", &table_id, &input);
            db_delete(tables[table_id - 1], input);
            break;
        case 4:
            fscanf(fp, "%d", &table_id);
            close_table(tables[table_id - 1]);
            break;
        case 10:
            shutdown_db();
            double end_time = (double)clock() / CLOCKS_PER_SEC;
            printf(" / %lf sec elapsed\n", end_time - start_time);
            return 0;
        }
    }
}