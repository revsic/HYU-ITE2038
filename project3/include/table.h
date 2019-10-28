#ifndef TABLE_H
#define TABLE_H

#include "disk_manager.h"
#include "bpt.h"
#include "headers.h"

struct table_t {
    tablenum_t id;
    struct bpt_t bpt;
    struct file_manager_t file;
};

int table_init(struct table_t* table);

int table_load(struct table_t* table,
               const char* filename,
               struct buffer_manager_t* buffers);

int table_release(struct table_t* table);

int table_find(struct table_t* table, prikey_t key, struct record_t* record);

int table_find_range(struct table_t* table, prikey_t start, prikey_t end, struct record_vec_t* retval);

int table_insert(struct table_t* table, prikey_t key, uint8_t* value, int value_size);

int table_delete(struct table_t* table, prikey_t key);

tablenum_t table_rehash(struct table_t* table, int update_id);

#endif