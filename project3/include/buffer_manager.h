#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "headers.h"
#include "table_manager.h"

#define BUFFER_READ(var, cont) {                        \
    EXIT_ON_FALSE((var)->table_id != INVALID_TABLENUM);   \
    buffer_start(var, READ_FLAG);                       \
    cont;                                               \
    buffer_end(var, READ_FLAG);                         \
}

#define BUFFER_INTERCEPT_READ(var, cont) buffer_end(var, READ_FLAG); cont;

#define BUFFER_READ_CHECK_TRUE(var, x) if (!(x)) BUFFER_INTERCEPT_READ(var, return FAILURE);

#define BUFFER_READ_CHECK_NULL(var, x) if ((x) == NULL) BUFFER_INTERCEPT_READ(var, return FAILURE);

#define BUFFER_READ_CHECK_SUCCESS(var, x) if ((x) != SUCCESS) BUFFER_INTERCEPT_READ(var, return FAILURE);

#define BUFFER_WRITE(var, cont) {                       \
    EXIT_ON_FALSE((var)->table_id != INVALID_TABLENUM);   \
    buffer_start(var, WRITE_FLAG);                      \
    cont;                                               \
    buffer_end(var, WRITE_FLAG);                        \
}

#define BUFFER_INTERCEPT_WRITE(var, cont) buffer_end(var, WRITE_FLAG); cont;

#define BUFFER_WRITE_CHECK_TRUE(var, x) if (!(x)) BUFFER_INTERCEPT_WRITE(var, return FAILURE);

#define BUFFER_WRITE_CHECK_NULL(var, x) if ((x) == NULL) BUFFER_INTERCEPT_WRITE(var, return FAILURE);

#define BUFFER_WRITE_CHECK_SUCCESS(var, x) if ((x) != SUCCESS) BUFFER_INTERCEPT_WRITE(var, return FAILURE);

struct page_uri_t {
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
    int block_idx;
    struct table_t* table;
    struct buffer_manager_t* manager;
};

struct buffer_manager_t {
    int capacity;
    int num_buffer;
    int lru;
    int mru;
    struct buffer_t *buffers;
};

struct release_policy_t {
    int(*initial_search)(struct buffer_manager_t* manager);
    int(*next_search)(struct buffer_t* buffer);
};

enum RW_FLAG {
    READ_FLAG = 0,
    WRITE_FLAG = 0
};

typedef int(*reader_t)(const struct page_t* page, void* param);

typedef int(*writer_t)(struct page_t* page, void* param);

extern const struct release_policy_t RELEASE_LRU;

extern const struct release_policy_t RELEASE_MRU;

struct page_t* from_buffer(struct buffer_t* buffer);

int buffer_init(struct buffer_t* buffer,
                int block_idx,
                struct block_manager_t* manager);

int buffer_load(struct buffer_t* buffer,
                struct table_t* table,
                pagenum_t pagenum);

int buffer_new_page(struct buffer_t* buffer, struct table_t* table);

int buffer_link_neighbor(struct buffer_t* buffer);

int buffer_append_mru(struct buffer_t* buffer);

int buffer_release(struct buffer_t* buffer);

int buffer_start(struct buffer_t* buffer, enum RW_FLAG rw_flag);

int buffer_end(struct buffer_t* buffer, enum RW_FLAG rw_flag);

int buffer_manager_init(struct buffer_manager_t* manager, int num_buffer);

int buffer_manager_shutdown(struct buffer_manager_t* manager);

int buffer_manager_alloc(struct buffer_manager_t* manager);

int buffer_manager_load(struct buffer_manager_t* manager,
                        struct table_manager_t* tables,
                        struct page_uri_t* page_uri);

int buffer_manager_release_table(struct buffer_manager_t* manager,
                                 tablenum_t table_id);

int buffer_manager_release(struct buffer_manager_t* manager,
                           const struct release_policy_t* policy);

int buffer_manager_find(struct buffer_manager_t* manager,
                        struct page_uri_t* page_uri);

struct buffer_t* buffer_manager_buffering(struct buffer_manager_t* manager,
                                          struct table_manager_t* tables,
                                          struct page_uri_t* page_uri);

struct buffer_t* buffer_manager_new_page(struct buffer_manager_t* manager,
                                         struct table_manager_t* tables,
                                         tablenum_t table_id);

#endif