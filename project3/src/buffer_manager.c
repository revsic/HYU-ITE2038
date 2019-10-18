#include <stdlib.h>
#include <string.h>

#include "buffer_manager.h"
#include "disk_manager.h"

int get_lru(struct buffer_manager_t* manager) {
    return manager->lru;
}

int next_lru(struct buffer_t* buffer) {
    return buffer->next_use;
}

int get_mru(struct buffer_manager_t* manager) {
    return manager->mru;
}

int next_mru(struct buffer_t* buffer) {
    return buffer->prev_use;
}

const struct release_policy_t RELEASE_LRU = { get_lru, next_lru };

const struct release_policy_t RELEASE_MRU = { get_mru, next_mru };

int buffer_init(struct buffer_t* buffer) {
    buffer->table_id = INVALID_TABLENUM;
    buffer->pagenum = INVALID_PAGENUM;
    buffer->is_dirty = FALSE;
    buffer->is_pinned = FALSE;
    buffer->prev_use = -1;
    buffer->next_use = -1;
    buffer->table = NULL;
    return SUCCESS;
}

int buffer_load(struct buffer_t* buffer,
                struct table_t* table,
                pagenum_t pagenum)
{
    CHECK_SUCCESS(page_read(pagenum, &table->file_manager, &buffer->frame));
    buffer->table_id = table->table_id;
    buffer->pagenum = pagenum;
    buffer->is_dirty = FALSE;
    buffer->is_pinned = FALSE;
    buffer->prev_use = -1;
    buffer->next_use = -1;
    buffer->table = table;
    return SUCCESS;
}

int buffer_release(struct buffer_t* buffer) {
    while (buffer->is_pinned)
        {}

    if (buffer->is_dirty) {
        CHECK_SUCCESS(
            page_write(
                buffer->pagenum,
                &buffer->table->file_manager,
                &buffer->frame));
    }
    return buffer_init(buffer);
}

int buffer_read_api(struct buffer_t* buffer, reader_t reader, void* param) {
    int ret;
    while (buffer->is_pinned < 0)
        {}

    ++buffer->is_pinned;
    ret = reader(&buffer->frame, param);
    --buffer->is_pinned;

    return ret;
}

int buffer_write_api(struct buffer_t* buffer, writer_t writer, void* param) {
    int ret;
    while (buffer->is_pinned != 0)
        {}
    
    --buffer->is_pinned;
    ret = writer(&buffer->frame, param);
    ++buffer->is_pinned;
    ++buffer->is_dirty;

    return ret;
}

int buffer_manager_init(struct buffer_manager_t* manager, int num_buffer) {
    int i;
    manager->capacity = num_buffer;
    manager->num_buffer = 0;
    manager->lru = -1;
    manager->mru = -1;
    
    manager->buffers = malloc(sizeof(struct buffer_t) * num_buffer);
    if (manager->buffers == NULL) {
        manager->capacity = 0;
        return FAILURE;
    }
    
    for (i = 0; i < manager->capacity; ++i) {
        CHECK_SUCCESS(buffer_init(&manager->buffers[i]));
    }
    return SUCCESS;
}

int buffer_manager_shutdown(struct buffer_manager_t* manager) {
    int i;
    for (i = 0; i < manager->capacity; ++i) {
        if (manager->buffers[i].table_id == INVALID_TABLENUM) {
            continue;
        }
        buffer_release(&manager->buffers[i]);
    }

    free(manager->buffers);
    memset(manager, 0, sizeof(struct buffer_manager_t));
    return SUCCESS;
}

int buffer_manager_load(struct buffer_manager_t* manager,
                        struct table_manager_t* tables,
                        struct page_uri_t* page_uri)
{
    int idx;
    struct buffer_t* buffer;
    struct table_t* table = table_manager_find(tables, page_uri->table_id);
    if (table == NULL) {
        return -1;
    }

    if (manager->num_buffer >= manager->capacity) {
        idx = buffer_manager_release(manager, &RELEASE_LRU);
        if (idx == -1) {
            return -1;
        }
    } else {
        for (idx = 0;
             idx < manager->capacity
                && manager->buffers[idx].table_id == INVALID_TABLENUM;
             ++idx)
            {}
    }

    buffer = &manager->buffers[idx];
    if (buffer_load(buffer, table, page_uri->pagenum) == FAILURE) {
        return -1;
    }

    buffer->next_use = -1;
    buffer->prev_use = manager->mru;
    if (manager->mru == -1) {
        manager->lru = idx;
    } else {
        manager->buffers[buffer->prev_use].next_use = idx;
    }

    manager->mru = idx;
    manager->num_buffer += 1;
    return idx;
}

int buffer_manager_release_table(struct buffer_manager_t* manager,
                                 tablenum_t table_id)
{
    int i, j;
    struct buffer_t* buffer;
    for (i = 0; i < manager->capacity; ++i) {
        buffer = &manager->buffers[i];
        if (buffer->table_id == INVALID_TABLENUM) {
            continue;
        }
        if (buffer->table_id == table_id) {
            CHECK_SUCCESS(buffer_release(buffer));
            --manager->num_buffer;
        }
    }
    return SUCCESS;
}

int buffer_manager_release(struct buffer_manager_t* manager,
                           const struct release_policy_t* policy)
{
    int idx = policy->initial_search(manager);
    while (idx != -1 && manager->buffers[idx].is_pinned) {
        idx = policy->next_search(&manager->buffers[idx]);
    }

    if (idx == -1) {
        return -1;
    }

    struct buffer_t* buffer = &manager->buffers[idx];
    if (buffer->next_use == -1) {
        manager->mru = buffer->prev_use;
    } else {
        manager->buffers[buffer->next_use].prev_use = buffer->prev_use;
    }

    if (buffer->prev_use == -1) {
        manager->lru = buffer->next_use;
    } else {
        manager->buffers[buffer->prev_use].next_use = buffer->next_use;
    }

    manager->num_buffer -= 1;
    if (buffer_release(buffer) == FAILURE) {
        return -1;
    }
    return idx;
}

int buffer_manager_find(struct buffer_manager_t* manager,
                        struct page_uri_t* page_uri)
{
    int i;
    struct buffer_t* buffer;
    for (i = 0; i < manager->capacity; ++i) {
        if (manager->buffers[i].table_id == INVALID_TABLENUM) {
            continue;
        }

        buffer = &manager->buffers[i];
        if (buffer->pagenum == page_uri->pagenum
            && buffer->table_id == page_uri->table_id)
        {
            return i;
        }
    }

    return -1;
}

struct buffer_t* buffer_manager_buffering(struct buffer_manager_t* manager,
                                          struct table_manager_t* tables,
                                          struct page_uri_t* page_uri)
{
    int idx = buffer_manager_find(manager, page_uri);
    if (idx == -1) {
        idx = buffer_manager_load(manager, tables, page_uri);
        if (idx == -1) {
            return NULL;
        }
    }
    return &manager->buffers[idx];
}
