#ifndef DBAPI_H
#define DBAPI_H

#include "disk_manager.h"

struct file_vec_t {
    int size;
    int capacity;
    struct file_manager_t* manager;
};

extern int GLOBAL_TABLE_ID;

extern struct file_vec_t GLOBAL_FILE_MANAGER;

int file_vec_init(struct file_vec_t* vec);

int file_vec_free(struct file_vec_t* vec);

int file_vec_expand(struct file_vec_t* vec);

int file_vec_append(struct file_vec_t* vec, struct file_manager_t* manager);

int open_table(char* pathname);

int close_table(int tid);

struct file_manager_t* get_file_manager(int tableid);

int db_insert(int64_t key, char* value);

int db_find(int64_t key, char* ret_val);

int db_delete(int64_t key);

#endif