#include <stdlib.h>
#include <string.h>

#include "table_manager.h"

int table_searching_policy(struct table_vec_t* table_vec, tablenum_t table_id) {
    int i;
    for (i = 0; i < table_vec->size; ++i) {
        if (table_vec->array[i]->id == table_id) {
            return i;
        }
    }
    return -1;
}

int table_vec_init(struct table_vec_t* table_vec, int capacity) {
    table_vec->size = 0;
    table_vec->capacity = capacity;
    table_vec->array = malloc(sizeof(struct table_t*) * table_vec->capacity);
    if (table_vec->array == NULL) {
        table_vec->capacity = 0;
        return FAILURE;
    }
    return SUCCESS;
}

int table_vec_extend(struct table_vec_t* table_vec) {
    int i;
    struct table_t** vec;

    table_vec->capacity *= 2;
    vec = malloc(sizeof(struct table_t*) * table_vec->capacity);
    if (vec == NULL) {
        table_vec->capacity /= 2;
        return FAILURE;
    }

    for (i = 0; i < table_vec->size; ++i) {
        vec[i] = table_vec->array[i];
    }
    free(table_vec->array);

    table_vec->array = vec;
    return SUCCESS;
}

int table_vec_append(struct table_vec_t* table_vec, struct table_t* table) {
    if (table_vec->size >= table_vec->capacity) {
        CHECK_SUCCESS(table_vec_extend(table_vec));
    }

    table_vec->array[table_vec->size] = malloc(sizeof(struct table_t));
    CHECK_NULL(table_vec->array[table_vec->size]);

    *table_vec->array[table_vec->size++] = *table;
    memset(table, 0, sizeof(struct table_t));
    return SUCCESS;
}

struct table_t* table_vec_find(struct table_vec_t* table_vec,
                               tablenum_t table_id)
{
    int idx = table_searching_policy(table_vec, table_id);
    if (idx == -1) {
        return NULL;
    }
    return table_vec->array[idx];
}

int table_vec_remove(struct table_vec_t* table_vec, tablenum_t table_id) {
    int idx = table_searching_policy(table_vec, table_id);
    if (idx == -1) {
        return FAILURE;
    }

    CHECK_SUCCESS(table_release(table_vec->array[idx]));
    free(table_vec->array[idx]);
    for (; idx < table_vec->size - 1; ++idx) {
        table_vec->array[idx] = table_vec->array[idx + 1];
    }
    table_vec->size -= 1;
    return SUCCESS;
}

int table_vec_shrink(struct table_vec_t* table_vec) {
    int i;
    struct table_t** vec;
    if (table_vec->size == table_vec->capacity) {
        return SUCCESS;
    }

    vec = malloc(sizeof(struct table_t*) * table_vec->size);
    CHECK_NULL(vec);

    for (i = 0; i < table_vec->size; ++i) {
        vec[i] = table_vec->array[i];
    }
    free(table_vec->array);

    table_vec->capacity = table_vec->size;
    table_vec->array = vec;
    return SUCCESS;
}

int table_vec_release(struct table_vec_t* table_vec) {
    int i;
    for (i = 0; i < table_vec->size; ++i) {
        CHECK_SUCCESS(table_release(table_vec->array[i]));
        free(table_vec->array[i]);
    }

    free(table_vec->array);
    table_vec->size = 0;
    table_vec->capacity = 0;
    table_vec->array = NULL;
    return SUCCESS;
}

int table_manager_init(struct table_manager_t* manager, int capacity) {
    return table_vec_init(&manager->vec, capacity);
}

tablenum_t table_manager_load(struct table_manager_t* manager,
                              const char* filename,
                              struct buffer_manager_t* buffers)
{
    tablenum_t id;
    struct table_t table;
    if (table_load(&table, filename, buffers) == FAILURE) {
        return INVALID_TABLENUM;
    }

    while (table_vec_find(&manager->vec, table.id) != NULL) {
        if (table_rehash(&table, TRUE) == INVALID_TABLENUM) {
            table_release(&table);
            return INVALID_TABLENUM;
        }
    }

    id = table.id;
    if (table_vec_append(&manager->vec, &table) == FAILURE) {
        table_release(&table);
        return INVALID_TABLENUM;
    }
    return id;
}

struct table_t* table_manager_find(struct table_manager_t* manager,
                                   tablenum_t table_id)
{
    return table_vec_find(&manager->vec, table_id);
}

int table_manager_remove(struct table_manager_t* manager, tablenum_t table_id) {
    return table_vec_remove(&manager->vec, table_id);
}

int table_manager_release(struct table_manager_t* manager) {
    return table_vec_release(&manager->vec);
}
