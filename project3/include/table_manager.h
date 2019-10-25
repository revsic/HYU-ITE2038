#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "headers.h"
#include "table.h"

struct table_vec_t {
    int size;
    int capacity;
    struct table_t** array;
};

struct table_manager_t {
    struct table_vec_t vec;
};

int table_searching_policy(struct table_vec_t* table_vec, tablenum_t table_id);

int table_vec_init(struct table_vec_t* table_vec, int capacity);

int table_vec_extend(struct table_vec_t* table_vec);

int table_vec_append(struct table_vec_t* table_vec, struct table_t* table);

struct table_t* table_vec_find(struct table_vec_t* table_vec,
                               tablenum_t table_id);

int table_vec_remove(struct table_vec_t* table_vec, tablenum_t table_id);

int table_vec_shrink(struct table_vec_t* table_vec);

int table_vec_release(struct table_vec_t* table_vec);

int table_manager_init(struct table_manager_t* manager, int capacity);

tablenum_t table_manager_load(struct table_manager_t* manager,
                              const char* filename);

struct table_t* table_manager_find(struct table_manager_t* manager,
                                   tablenum_t table_id);

int table_manager_remove(struct table_manager_t* manager, tablenum_t table_id);

int table_manager_release(struct table_manager_t* manager);

#endif