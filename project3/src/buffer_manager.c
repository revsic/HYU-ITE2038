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
    return SUCCESS;
}

int buffer_load(struct buffer_t* buffer, struct table_t* table, pagenum_t pagenum) {
    CHECK_SUCCESS(page_read(pagenum, &table->file_manager, &buffer->frame));
    buffer->table_id = table->table_id;
    buffer->pagenum = pagenum;
    buffer->is_dirty = FALSE;
    buffer->is_pinned = FALSE;
    buffer->prev_use = -1;
    buffer->next_use = -1;
    return SUCCESS;
}

int buffer_release(struct buffer_t* buffer, struct table_t* table) {
    if (buffer->is_dirty) {
        CHECK_SUCCESS(page_write(buffer->pagenum, &table->file_manager, &buffer->frame));
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

int buffer_manager_shutdown(struct buffer_manager_t* manager, struct table_manager_t* tables) {
    int i;
    struct buffer_t* buffer;
    struct table_t* table;
    for (i = 0; i < manager->num_buffer; ++i) {
        buffer = &manager->buffers[i];
        table = table_vec_find(&tables->vec, buffer->table_id);
        if (table != NULL) {
            buffer_release(buffer, table);
        }
    }

    free(manager->buffers);
    memset(manager, 0, sizeof(struct buffer_manager_t));
    return SUCCESS;
}

int buffer_manager_load(struct buffer_manager_t* manager, pagenum_t pagenum) {
    return SUCCESS;
}
