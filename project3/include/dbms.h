#ifndef DBMS_H
#define DBMS_H

#include "buffer_manager.h"
#include "table_manager.h"

struct dbms_t {
    struct buffer_manager_t buffers;  
    struct table_manager_t tables;
};

struct dbms_table_t {
    struct dbms_t* dbms;
    tablenum_t table_id;
};

int dbms_init(struct dbms_t* dbms, int num_buffer);

int dbms_shutdown(struct dbms_t* dbms);

tablenum_t dbms_open_table(struct dbms_t* dbms, const char* filename);

int dbms_close_table(struct dbms_t* dbms, tablenum_t table_id);

struct buffer_t* dbms_buffering(struct dbms_t* dbms,
                                struct page_uri_t* page_uri);

struct buffer_t* dbms_buffering_from_table(struct dbms_table_t* table,
                                           pagenum_t pagenum);

int dbms_find(struct dbms_t* dbms,
              tablenum_t table_id,
              struct record_t* record);

int dbms_insert(struct dbms_t* dbms,
                tablenum_t table_id,
                prikey_t key,
                uint8_t* value,
                int value_size);

int dbms_delete(struct dbms_t* dbms,
                tablenum_t table_id,
                prikey_t key);

#endif