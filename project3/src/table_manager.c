#include <stdlib.h>
#include <string.h>

#include "disk_manager.h"
#include "table_manager.h"

// sdbm
tablenum_t create_tablenum(const char* filename) {
    char c;
    unsigned long hash = 0;
    while (c = *filename++) {
        if (c == '/' || c == '\\') {
            hash = 0;
            continue;
        }
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return (tablenum_t)hash;
}

// sdbm
tablenum_t rehash_tablenum(tablenum_t tablenum) {
    int i;
    unsigned long hash = 0;
    uint8_t* bytes = (uint8_t*)&tablenum;
    for (i = 0; i < sizeof(tablenum_t); ++i) {
        hash = bytes[i] + (hash << 6) + (hash << 16) - hash;
    }
    return (tablenum_t)hash;
}

int searching_policy(struct table_vec_t* table_vec, tablenum_t table_id) {
    int i;
    for (i = 0; i < table_vec->size; ++i) {
        if (table_vec->array[i].table_id == table_id) {
            return i;
        }
    }
    return -1;
}

int table_init(struct table_t* table) {
    table->table_id = INVALID_TABLENUM;
    return SUCCESS;
}

int table_load(struct table_t* table, const char* filename) {
    table->table_id = create_tablenum(filename);
    return file_open(filename, &table->file_manager);
}

int table_release(struct table_t* table) {
    table->table_id = INVALID_PAGENUM;
    return file_close(&table->file_manager);
}

int table_vec_init(struct table_vec_t* table_vec) {
    table_vec->size = 0;
    table_vec->capacity = TABLE_VEC_DEFAULT_CAPACITY;
    table_vec->array = malloc(sizeof(struct table_t) * table_vec->capacity);
    if (table_vec->array == NULL) {
        table_vec->capacity = 0;
        return FAILURE;
    }
    return SUCCESS;
}

int table_vec_extend(struct table_vec_t* table_vec) {
    int i;
    struct table_t* vec;
    
    table_vec->capacity *= 2;
    vec = malloc(sizeof(struct table_t) * table_vec->capacity);
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

    table_vec->array[table_vec->size++] = *table;
    memset(table, 0, sizeof(struct table_t));
    return SUCCESS;
}

struct table_t* table_vec_find(struct table_vec_t* table_vec,
                               tablenum_t table_id)
{
    int idx = searching_policy(table_vec, table_id);
    if (idx == -1) {
        return NULL;
    }
    return &table_vec->array[idx];
}

int table_vec_remove(struct table_vec_t* table_vec, tablenum_t table_id) {
    int idx = searching_policy(table_vec, table_id);
    if (idx == -1) {
        return FAILURE;
    }

    for (; idx < table_vec->size - 1; ++idx) {
        table_vec->array[idx] = table_vec->array[idx + 1];
    }
    table_vec->size -= 1;
    return SUCCESS;
}

int table_vec_shrink(struct table_vec_t* table_vec) {
    int i;
    struct table_t* vec = malloc(sizeof(struct table_t) * table_vec->size);
    if (vec == NULL) {
        return FAILURE;
    }

    for (i = 0; i < table_vec->size; ++i) {
        vec[i] = table_vec->array[i];
    }
    free(table_vec->array);

    table_vec->capacity = table_vec->size;
    table_vec->array = vec;
    return SUCCESS;
}

int table_vec_release(struct table_vec_t* table_vec) {
    free(table_vec->array);
    table_vec->size = 0;
    table_vec->capacity = 0;
    table_vec->array = NULL;
    return SUCCESS;
}

int table_manager_init(struct table_manager_t* manager) {
    table_vec_init(&manager->vec);
    return SUCCESS;
}

tablenum_t table_manager_load(struct table_manager_t* manager,
                              const char* filename)
{
    tablenum_t id;
    struct table_t table;
    if (table_load(&table, filename) == FAILURE) {
        return INVALID_PAGENUM;
    }

    while (table_vec_find(&manager->vec, table.table_id) != NULL) {
        table.table_id = rehash_tablenum(table.table_id);
    }

    id = table.table_id;
    if (table_vec_append(&manager->vec, &table) == FAILURE) {
        return INVALID_PAGENUM;
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
