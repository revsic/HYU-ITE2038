#include <stdlib.h>

#include "table_manager.h"
#include "test.h"

TEST_SUITE(create_tablenum, {
    const char* filename = "datafile";
    const char* fullpath = "/Users/revsic/datafile";
    tablenum_t tablenum = create_tablenum(filename);

    DBG(tablenum);
    TEST(tablenum == create_tablenum(fullpath));
})

TEST_SUITE(rehash_tablenum, {
    const char* filename = "1234";
    tablenum_t previous = 0x34333231;
    TEST(create_tablenum(filename) == rehash_tablenum(previous));
})

TEST_SUITE(searching_policy, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));

    int i;
    struct table_t table;
    for (i = 0; i < 3 * TABLE_VEC_DEFAULT_CAPACITY; ++i) {
        table.table_id = i;
        TEST_SUCCESS(table_vec_append(&vec, &table));
    }

    for (i = 0; i < 3 * TABLE_VEC_DEFAULT_CAPACITY; ++i) {
        TEST(searching_policy(&vec, i) == i);
    }
    TEST(searching_policy(&vec, 100) == -1);
    TEST(searching_policy(&vec, -100) == -1);
    free(vec.array);
})

TEST_SUITE(table_init, {
    struct table_t table;
    TEST_SUCCESS(table_init(&table));
    TEST(table.table_id == INVALID_TABLENUM);
})

TEST_SUITE(table_load, {
    struct table_t table;
    const char* filename = "testfile";

    TEST_SUCCESS(table_load(&table, filename));
    TEST(table.table_id == create_tablenum(filename));

    TEST_SUCCESS(file_close(&table.file_manager));
    remove("testfile");
})

TEST_SUITE(table_release, {
    struct table_t table;
    TEST_SUCCESS(table_load(&table, "testfile"));
    TEST_SUCCESS(table_release(&table));

    TEST(table.table_id == INVALID_TABLENUM);
    remove("testfile");
})

TEST_SUITE(table_vec_init, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));
    TEST(vec.size == 0);
    TEST(vec.capacity == TABLE_VEC_DEFAULT_CAPACITY);
    TEST(vec.array != NULL);
    free(vec.array);
})

TEST_SUITE(table_vec_extend, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));

    vec.size += 2;
    vec.array[0] = malloc(sizeof(struct table_t));
    vec.array[0]->table_id = 100;

    vec.array[1] = malloc(sizeof(struct table_t));
    vec.array[1]->table_id = 200;

    struct table_t** prev = vec.array;
    TEST_SUCCESS(table_vec_extend(&vec));

    TEST(vec.capacity == 2 * TABLE_VEC_DEFAULT_CAPACITY);
    TEST(vec.size == 2);
    TEST(vec.array != prev);
    TEST(vec.array[0]->table_id == 100);
    TEST(vec.array[1]->table_id == 200);

    TEST_SUCCESS(table_vec_extend(&vec));
    TEST(vec.capacity == 4 * TABLE_VEC_DEFAULT_CAPACITY);

    free(vec.array);
})

TEST_SUITE(table_vec_append, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));

    int i;
    struct table_t table;
    for (i = 0; i < TABLE_VEC_DEFAULT_CAPACITY * 3; ++i) {
        table.table_id = i;
        TEST_SUCCESS(table_vec_append(&vec, &table));
    }

    TEST(vec.capacity == 4 * TABLE_VEC_DEFAULT_CAPACITY);
    TEST(vec.size == 3 * TABLE_VEC_DEFAULT_CAPACITY);

    for (i = 0; i < TABLE_VEC_DEFAULT_CAPACITY * 3; ++i) {
        TEST(vec.array[i]->table_id == i);
    }
    free(vec.array);
})

TEST_SUITE(table_vec_find, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));

    int i;
    struct table_t table;
    for (i = 0; i < TABLE_VEC_DEFAULT_CAPACITY * 3; ++i) {
        table.table_id = i;
        TEST_SUCCESS(table_vec_append(&vec, &table));
    }

    struct table_t* found;
    for (i = 0; i < TABLE_VEC_DEFAULT_CAPACITY * 3; ++i) {
        found = table_vec_find(&vec, i);
        TEST(found != NULL);
        TEST(found->table_id == i);
    }

    TEST(table_vec_find(&vec, -100) == NULL);
    TEST(table_vec_find(&vec, 100) == NULL);

    free(vec.array);
})

TEST_SUITE(table_vec_remove, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));

    const int size = TABLE_VEC_DEFAULT_CAPACITY * 3;

    int i;
    struct table_t table;
    for (i = 0; i < size; ++i) {
        table.table_id = i;
        TEST_SUCCESS(table_vec_append(&vec, &table));
    }

    int* origin = malloc(sizeof(int) * size);
    for (i = 0; i < size; ++i) {
        origin[i] = i;
    }

    int j;
    int idx;
    int rest = size;
    int* shuffled = malloc(sizeof(int) * size);

    srand(1024);
    for (i = 0; i < size; ++i) {
        idx = rand() % rest;
        shuffled[i] = origin[idx];
        for (j = idx; j < rest - 1; ++j) {
            origin[j] = origin[j + 1];
        }
        rest -= 1;
    }

    for (i = 0; i < size; ++i) {
        TEST_SUCCESS(table_vec_remove(&vec, shuffled[i]));
        TEST(table_vec_find(&vec, shuffled[i]) == NULL);
        TEST(vec.size == size - i - 1);
    }

    TEST(table_vec_remove(&vec, 0) == FAILURE);

    free(origin);
    free(shuffled);
    free(vec.array);
})

TEST_SUITE(table_vec_shrink, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));

    int i;
    struct table_t table;
    for (i = 0; i < TABLE_VEC_DEFAULT_CAPACITY * 3; ++i) {
        table.table_id = i;
        TEST_SUCCESS(table_vec_append(&vec, &table));
    }

    TEST(vec.size != vec.capacity);

    struct table_t** prev = vec.array;
    TEST_SUCCESS(table_vec_shrink(&vec));
    TEST(vec.size == TABLE_VEC_DEFAULT_CAPACITY * 3);
    TEST(vec.size == vec.capacity);
    TEST(vec.array != prev);

    for (i = 0; i < TABLE_VEC_DEFAULT_CAPACITY * 3; ++i) {
        TEST(vec.array[i]->table_id == i);
    }

    free(vec.array);
})

TEST_SUITE(table_vec_release, {
    struct table_vec_t vec;
    TEST_SUCCESS(table_vec_init(&vec));
    TEST_SUCCESS(table_vec_release(&vec));
    TEST(vec.size == 0);
    TEST(vec.capacity == 0);
    TEST(vec.array == NULL);
})

TEST_SUITE(table_manager_init, {
    struct table_manager_t manager;
    TEST_SUCCESS(table_manager_init(&manager));

    TEST(manager.vec.size == 0);
    TEST(manager.vec.capacity == TABLE_VEC_DEFAULT_CAPACITY);
    TEST(manager.vec.array != NULL);

    TEST_SUCCESS(table_vec_release(&manager.vec));
})

TEST_SUITE(table_manager_load, {
    struct table_manager_t manager;
    TEST_SUCCESS(table_manager_init(&manager));

    tablenum_t res = table_manager_load(&manager, "testfile");
    TEST(res != INVALID_TABLENUM);

    struct table_t* table = table_vec_find(&manager.vec, res);
    TEST(table != NULL);
    TEST(table->table_id == res);

    TEST_SUCCESS(table_vec_release(&manager.vec));
    remove("testfile");
})

TEST_SUITE(table_manager_find, {
    struct table_manager_t manager;
    TEST_SUCCESS(table_manager_init(&manager));

    int i;
    char str[] = "testfile0";
    tablenum_t arr[10];
    for (i = 0; i < 10; ++i) {
        str[8] = i + 0x30;
        arr[i] = table_manager_load(&manager, str);
        TEST(arr[i] != INVALID_TABLENUM);
    }

    for (i = 0; i < 10; ++i) {
        TEST(table_manager_find(&manager, arr[i]) != NULL);
    }

    TEST_SUCCESS(table_vec_release(&manager.vec));
    for (i = 0; i < 10; ++i) {
        str[8] = i + 0x30;
        remove(str);
    }
})

TEST_SUITE(table_manager_remove, {
    struct table_manager_t manager;
    TEST_SUCCESS(table_manager_init(&manager));

    int i;
    char str[] = "testfile0";
    tablenum_t arr[10];
    for (i = 0; i < 10; ++i) {
        str[8] = i + 0x30;
        arr[i] = table_manager_load(&manager, str);
        TEST(arr[i] != INVALID_TABLENUM);
    }

    for (i = 0; i < 10; ++i) {
        TEST_SUCCESS(table_manager_remove(&manager, arr[i]));
        TEST(table_manager_find(&manager, arr[i]) == NULL);
    }

    TEST_SUCCESS(table_vec_release(&manager.vec));
    for (i = 0; i < 10; ++i) {
        str[8] = i + 0x30;
        remove(str);
    }
})

TEST_SUITE(table_manager_release, {
    struct table_manager_t manager;
    TEST_SUCCESS(table_manager_init(&manager));
    TEST_SUCCESS(table_manager_release(&manager));
    TEST(manager.vec.size == 0);
    TEST(manager.vec.capacity == 0);
    TEST(manager.vec.array == NULL);
})

int table_manager_test() {
    return create_tablenum_test()
        && rehash_tablenum_test()
        && searching_policy_test()
        && table_init_test()
        && table_load_test()
        && table_release_test()
        && table_vec_init_test()
        && table_vec_extend_test()
        && table_vec_append_test()
        && table_vec_find_test()
        && table_vec_remove_test()
        && table_vec_shrink_test()
        && table_vec_release_test()
        && table_manager_init_test()
        && table_manager_load_test()
        && table_manager_find_test()
        && table_manager_remove_test()
        && table_manager_release_test();
}