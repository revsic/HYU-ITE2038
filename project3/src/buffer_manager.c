#include <stdlib.h>
#include <string.h>

#include "buffer_manager.h"
#include "disk_manager.h"

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
    if (buffer->is_dirty) {
        CHECK_SUCCESS(page_write(buffer->pagenum, &buffer->table, &buffer->frame));
    }
    return buffer_init(buffer);
}

int buffer_manager_init(struct buffer_manager_t* manager, int num_buffer) {
    manager->capacity = num_buffer;
    manager->num_buffer = 0;
    manager->lru = -1;
    manager->mru = -1;
    manager->buffers = malloc(sizeof(struct buffer_t) * num_buffer);
    if (manager->buffers == NULL) {
        manager->capacity = 0;
        return FAILURE;
    }
    return SUCCESS;
}

int buffer_manager_shutdown(struct buffer_manager_t* manager)
{
    int i;
    for (i = 0; i < manager->num_buffer; ++i) {
        buffer_release(&manager->buffers[i]);
    }

    free(manager->buffers);
    memset(manager, 0, sizeof(struct buffer_manager_t));
    return SUCCESS;
}

int buffer_manager_load(struct buffer_manager_t* manager,
                        struct table_manager_t* tables,
                        tablenum_t tablenum,
                        pagenum_t pagenum)
{
    int idx;
    struct buffer_t* buffer;
    struct table_t* table = table_manager_find(tables, tablenum);
    if (table == NULL) {
        return FAILURE;
    }

    if (manager->num_buffer >= manager->capacity) {
        idx = buffer_manager_release_lru(manager);
        if (idx == -1) {
            return FAILURE;
        }
    } else {
        idx = manager->num_buffer;
    }

    buffer = &manager->buffers[idx];
    CHECK_SUCCESS(buffer_load(buffer, table, pagenum));

    buffer->next_use = -1;
    buffer->prev_use = manager->mru;
    if (manager->mru == -1) {
        manager->lru = idx;
    } else {
        manager->buffers[buffer->prev_use].next_use = idx;
    }

    manager->mru = idx;
    manager->num_buffer += 1;
    return SUCCESS;
}

int buffer_manager_release_lru(struct buffer_manager_t* manager) {
    int idx = manager->lru;
    if (idx == -1) {
        return -1;
    }

    struct buffer_t* buffer = &manager->buffers[idx];
    if (buffer->next_use == -1) {
        manager->mru = -1;
    } else {
        manager->buffers[buffer->next_use].prev_use = -1;
    }
    manager->lru = buffer->next_use;
    manager->num_buffer -= 1;

    if (buffer_release(buffer) == FAILURE) {
        return -1;
    }
    return idx;
}

int buffer_manager_release_mru(struct buffer_manager_t* manager) {
    int idx = manager->mru;
    if (idx == -1) {
        return -1;
    }

    struct buffer_t* buffer = &manager->buffers[idx];
    if (buffer->prev_use == -1) {
        manager->lru = -1;
    } else {
        manager->buffers[buffer->prev_use].next_use = -1;
    }
    manager->mru = buffer->prev_use;
    manager->num_buffer -= 1;

    if (buffer_release(buffer) == FAILURE) {
        return -1;
    }
    return idx;
}
