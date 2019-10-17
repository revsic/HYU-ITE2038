#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "headers.h"

struct table_t {
    tablenum_t table_id;
    struct file_manager_t* file_manager;
};

struct table_vec_t {
    int size;
    int capacity;
    struct table_t* array;
};

struct table_manager_t {
    struct table_vec_t vec;
};

int table_init(struct table_t* table);

int table_load(struct table_t* table, const char* filename);

int table_release(struct table_t* table);

int table_vec_init(struct table_vec_t* table_vec);

int table_vec_extend(struct table_vec_t* table_vec);

int table_vec_append(struct table_vec_t* table_vec);

int table_vec_remove(struct table_vec_t* table_vec);

int table_vec_shrink(struct table_vec_t* table_vec);

int table_vec_release(struct table_vec_t* table_vec);

int table_manager_init(struct table_manager_t* manager);

tablenum_t table_manager_load(struct table_manager_t* manager, const char* filename);

int table_manager_remove(struct table_manager_t* manager, tablenum_t table_id);

int table_manager_release(struct table_manager_t* manager);

#endif