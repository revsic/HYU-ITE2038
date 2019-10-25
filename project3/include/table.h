#ifndef TABLE_h
#define TABLE_H

#include "disk_manager.h"
#include "headers.h"

struct table_t {
    tablenum_t table_id;
    struct file_manager_t file_manager;
};

tablenum_t create_tablenum(const char* filename);

tablenum_t rehash_tablenum(tablenum_t tablenum);

int table_init(struct table_t* table);

int table_load(struct table_t* table, const char* filename);

int table_release(struct table_t* table);

#endif