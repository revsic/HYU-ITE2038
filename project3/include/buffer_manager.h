#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "headers.h"
#include "table_manager.h"

struct record_id_t {
    tablenum_t table_id;
    pagenum_t pagenum;
};

struct buffer_t {
    struct page_t frame;
    tablenum_t table_id;
    pagenum_t pagenum;
    uint32_t is_dirty;
    uint32_t is_pinned;
    int prev_use;
    int next_use;
    struct table_t* table;
};

struct buffer_manager_t {
    int capacity;
    int num_buffer;
    int lru;
    int mru;
    struct buffer_t *buffers;
};

typedef int(*reader_t)(const struct page_t* page, void* param);

typedef int(*writer_t)(struct page_t* page, void* param);

int buffer_init(struct buffer_t* buffer);

int buffer_load(struct buffer_t* buffer,
                struct table_t* table,
                pagenum_t pagenum);

int buffer_release(struct buffer_t* buffer);

int buffer_read_api(struct buffer_t* buffer, reader_t reader, void* param);

int buffer_write_api(struct buffer_t* buffer, writer_t writer, void* param);

int buffer_manager_init(struct buffer_manager_t* manager, int num_buffer);

int buffer_manager_shutdown(struct buffer_manager_t* manager);

int buffer_manager_load(struct buffer_manager_t* manager,
                        struct table_manager_t* tables,
                        struct record_id_t* record_id);

int buffer_manager_release_lru(struct buffer_manager_t* manager);

int buffer_manager_release_mru(struct buffer_manager_t* manager);

int buffer_manager_find(struct buffer_manager_t* manager,
                        struct record_id_t* record_id);

struct buffer_t* buffer_manager_get(struct buffer_manager_t* manager,
                                    struct table_manager_t* tables,
                                    struct record_id_t* record_id);

int buffer_manager_read(struct buffer_manager_t* manager, 
                        struct table_manager_t* tables,
                        struct record_id_t* record_id,
                        reader_t reader,
                        void* param);

int buffer_manager_write(struct buffer_manager_t* manager,
                         struct table_manager_t* tables,
                         struct record_id_t* record_id,
                         writer_t writer,
                         void* param);

#endif