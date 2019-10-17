#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "headers.h"

struct buffer_t {
    struct page_t frame;
    int32_t table_id;
    pagenum_t page_num;
    uint32_t is_dirty;
    uint32_t is_pinned;
    size_t prev_use;
    size_t next_use;
};

struct buffer_manager_t {
    int num_buffer;
    size_t lru;
    size_t mru;
    struct buffer_t *buffers;
};

int buffer_init(struct buffer_t* buffer);

int buffer_load(struct buffer_t* buffer, pagenum_t pagenum);

int buffer_release(struct buffer_t* buffer);

int buffer_manager_init(struct buffer_manager_t* manager);

int buffer_manager_shutdown(struct buffer_manager_t* manager);

int buffer_manager_load(struct buffer_manager_t* manager, pagenum_t pagenum);

#endif