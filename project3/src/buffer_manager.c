#include <stdlib.h>
#include <string.h>

#include "buffer_manager.h"

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

int ubuffer_reload(struct ubuffer_t* buffer) {
    // rebuffering
    *buffer = buffer_manager_buffering(
        buffer->buf->manager, buffer->file, buffer->pagenum);
    CHECK_NULL(buffer->buf);
    return SUCCESS;
}

int ubuffer_check(struct ubuffer_t* buffer) {
    if (buffer->buf->file != NULL
        && buffer->file->id == buffer->buf->file->id
        && buffer->pagenum == buffer->buf->pagenum)
    {
        return SUCCESS;
    }
    return ubuffer_reload(buffer);
}

struct page_t* from_buffer(struct buffer_t* buffer) {
    return &buffer->frame;
}

struct page_t* from_ubuffer(struct ubuffer_t* buffer) {
    return from_buffer(buffer->buf);
}

pagenum_t ubuffer_pagenum(struct ubuffer_t* buffer) {
    // check buffer validation
    EXIT_ON_FAILURE(ubuffer_check(buffer));
    return buffer->buf->pagenum;
}

int buffer_init(struct buffer_t* buffer,
                int block_idx,
                struct buffer_manager_t* manager)
{
    buffer->pagenum = INVALID_PAGENUM;
    buffer->is_dirty = FALSE;
    buffer->pin = 0;
    buffer->prev_use = -1;
    buffer->next_use = -1;
    buffer->block_idx = block_idx;
    buffer->file = NULL;
    buffer->manager = manager;
    return SUCCESS;
}

int buffer_load(struct buffer_t* buffer,
                struct file_manager_t* file,
                pagenum_t pagenum)
{
    // buffer must be initialized by buffer_init
    CHECK_SUCCESS(page_read(file, pagenum, &buffer->frame));
    buffer->pagenum = pagenum;
    buffer->file = file;
    return SUCCESS;
}

int buffer_new_page(struct buffer_t* buffer, struct file_manager_t* file) {
    // primitive page creattion
    pagenum_t res = page_create(file);
    if (res == INVALID_PAGENUM) {
        return FAILURE;
    }
    return buffer_load(buffer, file, res);
}

int buffer_link_neighbor(struct buffer_t* buffer) {
    // don't use unconnected node from lru to mru.
    struct buffer_manager_t* manager;
    CHECK_NULL(manager = buffer->manager);
    // if mru buffer
    if (buffer->next_use == -1) {
        manager->mru = buffer->prev_use;
    } else {
        manager->buffers[buffer->next_use].prev_use = buffer->prev_use;
    }
    // if lru buffer
    if (buffer->prev_use == -1) {
        manager->lru = buffer->next_use;
    } else {
        manager->buffers[buffer->prev_use].next_use = buffer->next_use;
    }
    return SUCCESS;
}

int buffer_append_mru(struct buffer_t* buffer, int link) {
    int mru;
    struct buffer_manager_t* manager;
    CHECK_NULL(manager = buffer->manager);
    if (link) {
        CHECK_SUCCESS(buffer_link_neighbor(buffer));
    }

    buffer->prev_use = manager->mru;
    buffer->next_use = -1;
    if (manager->mru != -1) {
        manager->buffers[manager->mru].next_use = buffer->block_idx;
    }
    manager->mru = buffer->block_idx;
    if (manager->lru == -1) {
        manager->lru = buffer->block_idx;
    }

    return SUCCESS;
}

int buffer_release(struct buffer_t* buffer) {
    while (buffer->pin)
        {}

    --buffer->pin;
    CHECK_SUCCESS(buffer_link_neighbor(buffer));

    if (buffer->is_dirty) {
        CHECK_SUCCESS(
            page_write(
                buffer->file,
                buffer->pagenum,
                &buffer->frame));
    }
    return buffer_init(buffer, buffer->block_idx, buffer->manager);
}

int buffer_start_read(struct buffer_t* buffer) {
    while (buffer->pin < 0)
        {}
    ++buffer->pin;
    return SUCCESS;
}

int buffer_start_write(struct buffer_t* buffer) {
    while (buffer->pin != 0)
        {}
    --buffer->pin;
    return SUCCESS;
}

int buffer_start(struct buffer_t* buffer, enum RW_FLAG rw_flag) {
    // acquire pin based rwlock
    return rw_flag == READ_FLAG
        ? buffer_start_read(buffer)
        : buffer_start_write(buffer);
}

int buffer_end_read(struct buffer_t* buffer) {
    --buffer->pin;
    CHECK_SUCCESS(buffer_append_mru(buffer, TRUE));
    return SUCCESS;
}

int buffer_end_write(struct buffer_t* buffer) {
    ++buffer->pin;
    buffer->is_dirty = TRUE;
    CHECK_SUCCESS(buffer_append_mru(buffer, TRUE));
    return SUCCESS;
}

int buffer_end(struct buffer_t* buffer, enum RW_FLAG rw_flag) {
    // release pin and update mru
    return rw_flag == READ_FLAG
        ? buffer_end_read(buffer)
        : buffer_end_write(buffer);
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
    // initialize buffer
    for (i = 0; i < manager->capacity; ++i) {
        CHECK_SUCCESS(buffer_init(&manager->buffers[i], i, manager));
    }
    return SUCCESS;
}

int buffer_manager_shutdown(struct buffer_manager_t* manager) {
    int i;
    // release all blocks
    for (i = 0; i < manager->capacity; ++i) {
        buffer_manager_release_block(manager, i);
    }

    free(manager->buffers);
    memset(manager, 0, sizeof(struct buffer_manager_t));
    return SUCCESS;
}

int buffer_manager_alloc(struct buffer_manager_t* manager) {
    int idx;
    // if extra page exist
    if (manager->num_buffer < manager->capacity) {
        // searching page (linear)
        for (idx = 0;
             idx < manager->capacity
                && manager->buffers[idx].file != NULL;
             ++idx)
            {}
        if (idx == manager->capacity) {
            return -1;
        }
    } else {
        // release LRU page
        idx = buffer_manager_release(manager, &RELEASE_LRU);
        if (idx == -1) {
            return -1;
        }
    }
    manager->num_buffer += 1;
    return idx;
}

int buffer_manager_load(struct buffer_manager_t* manager,
                        struct file_manager_t* file,
                        pagenum_t pagenum)
{
    int idx;
    struct buffer_t* buffer;
    // alloc buffer
    idx = buffer_manager_alloc(manager);
    if (idx == -1) {
        return -1;
    }
    // load page
    buffer = &manager->buffers[idx];
    if (buffer_load(buffer, file, pagenum) == FAILURE) {
        --manager->num_buffer;
        buffer_init(buffer, idx, manager);    
        return -1;
    }
    // append loaded to mru
    CHECK_SUCCESS(buffer_append_mru(buffer, FALSE));
    return idx;
}

int buffer_manager_release_block(struct buffer_manager_t* manager, int idx) {
    CHECK_TRUE(0 <= idx && idx < manager->capacity);

    struct buffer_t* buffer = &manager->buffers[idx];
    CHECK_NULL(buffer->file);
    CHECK_SUCCESS(buffer_release(buffer));

    --manager->num_buffer;
    return SUCCESS;
}

int buffer_manager_release_file(struct buffer_manager_t* manager,
                                filenum_t file_id)
{
    int i;
    struct file_manager_t* file;
    CHECK_TRUE(file_id != INVALID_FILENUM);
    for (i = 0; i < manager->capacity; ++i) {
        file = manager->buffers[i].file;
        if (file != NULL && file->id == file_id) {
            CHECK_SUCCESS(buffer_manager_release_block(manager, i));
        }
    }
    return SUCCESS;
}

int buffer_manager_release(struct buffer_manager_t* manager,
                           const struct release_policy_t* policy)
{
    // search block with release policy
    int idx = policy->initial_search(manager);
    while (idx != -1 && manager->buffers[idx].pin) {
        idx = policy->next_search(&manager->buffers[idx]);
    }
    if (idx == -1) {
        return -1;
    }
    // release block
    if (buffer_manager_release_block(manager, idx) == FAILURE) {
        return -1;
    }
    return idx;
}

int buffer_manager_find(struct buffer_manager_t* manager,
                        filenum_t file_id,
                        pagenum_t pagenum)
{
    int i;
    struct buffer_t* buffer;
    // linear search
    for (i = 0; i < manager->capacity; ++i) {
        buffer = &manager->buffers[i];
        if (buffer->file != NULL
            && buffer->pagenum == pagenum
            && buffer->file->id == file_id)
        {
            return i;
        }
    }

    return -1;
}

struct ubuffer_t buffer_manager_buffering(struct buffer_manager_t* manager,
                                          struct file_manager_t* file,
                                          pagenum_t pagenum)
{
    struct ubuffer_t ubuf = { NULL, INVALID_PAGENUM, NULL };
    int idx = buffer_manager_find(manager, file->id, pagenum);
    if (idx == -1) {
        idx = buffer_manager_load(manager, file, pagenum);
        if (idx == -1) {
            return ubuf;
        }
    }
    ubuf.buf = &manager->buffers[idx];
    ubuf.pagenum = pagenum;
    ubuf.file = file;
    return ubuf;
}

struct ubuffer_t buffer_manager_new_page(struct buffer_manager_t* manager,
                                         struct file_manager_t* file)
{
    struct ubuffer_t ubuf = { NULL, INVALID_PAGENUM, NULL };
    int idx = buffer_manager_find(manager, file->id, FILE_HEADER_PAGENUM);
    if (idx == -1) {
        idx = buffer_manager_alloc(manager);
        if (idx == -1) {
            return ubuf;
        }
    } else {
        if (buffer_manager_release_block(manager, idx) == FAILURE) {
            return ubuf;
        }
        ++manager->num_buffer;
    }

    struct buffer_t* buffer = &manager->buffers[idx];
    if (buffer_new_page(buffer, file) == FAILURE) {
        --manager->num_buffer;
        buffer_init(buffer, idx, manager);
        return ubuf;
    }

    if (buffer_append_mru(buffer, FALSE) == FAILURE) {
        return ubuf;
    }

    ubuf.buf = buffer;
    ubuf.file = file;
    ubuf.pagenum = buffer->pagenum;
    return ubuf;
}

int buffer_manager_free_page(struct buffer_manager_t* manager,
                             struct file_manager_t* file,
                             pagenum_t pagenum)
{
    int idx = buffer_manager_find(manager, file->id, pagenum);
    if (idx != -1) {
        CHECK_SUCCESS(buffer_manager_release_block(manager, idx));
    }

    idx = buffer_manager_find(manager, file->id, FILE_HEADER_PAGENUM);
    if (idx != -1) {
        CHECK_SUCCESS(buffer_manager_release_block(manager, idx));
    }
    return page_free(file, pagenum);
}
