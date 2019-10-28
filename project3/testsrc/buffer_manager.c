#include <stdlib.h>

#include "buffer_manager.h"
#include "utility.h"
#include "test.h"

TEST_SUITE(check_ubuffer, {
    struct buffer_t buf;
    struct ubuffer_t ubuf;
    ubuf.buf = &buf;
    buf.use_count = 10;

    ubuf.use_count = 10;
    TEST_SUCCESS(check_ubuffer(&ubuf));

    ubuf.use_count = 20;
    // TEST(check_ubuffer(&ubuf) == FAILURE);
})

TEST_SUITE(from_buffer, {
    struct buffer_t buf;
    TEST(from_buffer(&buf) == &buf.frame);
})

TEST_SUITE(from_ubuffer, {
    struct buffer_t buf;
    struct ubuffer_t ubuf;
    ubuf.buf = &buf;
    TEST(from_ubuffer(&ubuf) == from_buffer(&buf));
})

TEST_SUITE(buffer_init, {
    struct buffer_t buf;
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_init(&buf, FALSE, 10, &manager));

    TEST(buf.pagenum == INVALID_PAGENUM);
    TEST(buf.is_dirty == FALSE);
    TEST(buf.pin == 0);
    TEST(buf.prev_use == -1);
    TEST(buf.next_use == -1);
    TEST(buf.use_count == 0);
    TEST(buf.block_idx == 10);
    TEST(buf.file == NULL);
    TEST(buf.manager == &manager);

    TEST_SUCCESS(buffer_init(&buf, TRUE, 10, &manager));
    TEST(buf.use_count == 1);
})

TEST_SUITE(buffer_load, {
    struct buffer_t buf;
    struct file_manager_t file;
    TEST_SUCCESS(file_open(&file, "testfile"));
    TEST_SUCCESS(buffer_init(&buf, FALSE, 10, NULL));

    pagenum_t pagenum = page_create(&file);
    TEST_SUCCESS(buffer_load(&buf, &file, pagenum));

    TEST(buf.pagenum == pagenum);
    TEST(buf.file == &file);

    TEST_SUCCESS(file_close(&file));
    remove("testfile");
})

TEST_SUITE(buffer_new_page, {
    struct buffer_t buf;
    struct file_manager_t file;
    TEST_SUCCESS(file_open(&file, "testfile"));
    TEST_SUCCESS(buffer_init(&buf, FALSE, 10, NULL));
    TEST_SUCCESS(buffer_new_page(&buf, &file));

    TEST(buf.pagenum != INVALID_PAGENUM);
    TEST(buf.file == &file);

    TEST_SUCCESS(file_close(&file));
    remove("testfile");
})

TEST_SUITE(buffer_link_neighbor, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    // case 0. only node
    manager.lru = 0;
    manager.mru = 0;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = -1;

    TEST_SUCCESS(buffer_link_neighbor(&manager.buffers[0]));
    TEST(manager.lru == -1);
    TEST(manager.mru == -1);

    // case 1. least recentrly used node
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;

    TEST_SUCCESS(buffer_link_neighbor(&manager.buffers[0]));
    TEST(manager.lru == 1);
    TEST(manager.mru == 2);
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == 2);

    // case 2. most recently used node
    manager.lru = 2;
    manager.mru = 0;
    manager.buffers[0].prev_use = 1;
    manager.buffers[0].next_use = -1;
    manager.buffers[1].prev_use = 2;
    manager.buffers[1].next_use = 0;

    TEST_SUCCESS(buffer_link_neighbor(&manager.buffers[0]));
    TEST(manager.mru == 1);
    TEST(manager.lru == 2);
    TEST(manager.buffers[1].next_use == -1);
    TEST(manager.buffers[1].prev_use == 2);

    // case 3. middle node
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    TEST_SUCCESS(buffer_link_neighbor(&manager.buffers[1]));
    TEST(manager.mru == 2);
    TEST(manager.lru == 0);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == -1);

    // case 4. unconnected node
    // undefined behaviour

    TEST_SUCCESS(buffer_manager_shutdown(&manager));
})

TEST_SUITE(buffer_append_mru, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    // case 1. first node
    manager.lru = -1;
    manager.mru = -1;
    manager.buffers[0].prev_use = -1;
    manager.buffers[1].next_use = -1;
    TEST_SUCCESS(buffer_append_mru(&manager.buffers[0], FALSE));

    TEST(manager.lru == 0);
    TEST(manager.mru == 0);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == -1);

    // case 2. append
    manager.lru = 0;
    manager.mru = 1;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = -1;
    TEST_SUCCESS(buffer_append_mru(&manager.buffers[2], FALSE));

    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 1);
    TEST(manager.buffers[1].prev_use == 0);
    TEST(manager.buffers[1].next_use == 2);
    TEST(manager.buffers[2].prev_use == 1);
    TEST(manager.buffers[2].next_use == -1);

    TEST_SUCCESS(buffer_manager_shutdown(&manager));
})

TEST_SUITE(buffer_release, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    struct buffer_t* target = &manager.buffers[1];

    struct file_manager_t file;
    TEST_SUCCESS(file_open(&file, "testfile"));
    TEST_SUCCESS(buffer_init(target, FALSE, 1, &manager));
    TEST_SUCCESS(buffer_load(target, &file, FILE_HEADER_PAGENUM));

    // case 1. not dirty
    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    TEST_SUCCESS(buffer_release(target));
    // linkage
    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == -1);
    // block info
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == -1);
    TEST(manager.buffers[1].block_idx == 1);
    TEST(manager.buffers[1].manager == &manager);
    // file check
    TEST_SUCCESS(buffer_load(target, &file, FILE_HEADER_PAGENUM));
    TEST(file_header(from_buffer(target))->free_page_number == 0);
    TEST(file_header(from_buffer(target))->root_page_number == INVALID_PAGENUM);
    TEST(file_header(from_buffer(target))->number_of_pages == 0);

    // case 2. dirty
    struct ubuffer_t ubuffer;
    ubuffer.buf = target;
    ubuffer.use_count = target->use_count;
    BUFFER(ubuffer, WRITE_FLAG, {
        file_header(from_ubuffer(&ubuffer))->root_page_number = 1;
        file_header(from_ubuffer(&ubuffer))->number_of_pages = 20;
    })

    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    TEST_SUCCESS(buffer_release(target));
    // linkage
    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == -1);
    // block info
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == -1);
    TEST(manager.buffers[1].block_idx == 1);
    TEST(manager.buffers[1].manager == &manager);
    // file check
    TEST_SUCCESS(buffer_load(target, &file, FILE_HEADER_PAGENUM));
    TEST(file_header(from_buffer(target))->free_page_number == 0);
    TEST(file_header(from_buffer(target))->root_page_number == 1);
    TEST(file_header(from_buffer(target))->number_of_pages == 20);

    TEST_SUCCESS(file_close(&file));
    TEST_SUCCESS(buffer_manager_shutdown(&manager));
    remove("testfile");
})

TEST_SUITE(buffer_start_end, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    struct buffer_t* target = &manager.buffers[1];

    TEST_SUCCESS(buffer_start(target, READ_FLAG));
    TEST(target->pin == 1);
    TEST(target->next_use == -1);
    TEST(target->prev_use == 2);
    TEST(manager.lru == 0);
    TEST(manager.mru == 1);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == 1);

    TEST_SUCCESS(buffer_end(target, READ_FLAG));
    TEST(target->pin == 0);

    target = &manager.buffers[2];
    TEST_SUCCESS(buffer_start(target, WRITE_FLAG));
    TEST(target->pin == -1);
    TEST(target->next_use == -1);
    TEST(target->prev_use == 1);
    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 1);
    TEST(manager.buffers[1].prev_use == 0);
    TEST(manager.buffers[1].next_use == 2);

    TEST_SUCCESS(buffer_end(target, WRITE_FLAG));
    TEST(target->pin == 0);
    TEST_SUCCESS(buffer_manager_shutdown(&manager));
})

TEST_SUITE(buffer_macro, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    struct buffer_t* target = &manager.buffers[0];
    struct ubuffer_t ubuf;
    ubuf.buf = target;
    ubuf.use_count = target->use_count;

    BUFFER(ubuf, READ_FLAG, {
        ;
    })

    TEST(target->pin == 0);
    TEST(target->next_use == -1);
    TEST(target->prev_use == 2);
    TEST(target->is_dirty == FALSE);
    TEST(manager.lru == 1);
    TEST(manager.mru == 0);
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == 2);
    TEST(manager.buffers[2].prev_use == 1);
    TEST(manager.buffers[2].next_use == 0);

    BUFFER(ubuf, WRITE_FLAG, {
        ;
    })

    TEST(target->pin == 0);
    TEST(target->next_use == -1);
    TEST(target->prev_use == 2);
    TEST(target->is_dirty == TRUE);
    TEST(manager.lru == 1);
    TEST(manager.mru == 0);
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == 2);
    TEST(manager.buffers[2].prev_use == 1);
    TEST(manager.buffers[2].next_use == 0);

    TEST_SUCCESS(buffer_manager_shutdown(&manager));
})

// TEST_SUITE(buffer_manager_init, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     TEST(manager.capacity == 5);
//     TEST(manager.num_buffer == 0);
//     TEST(manager.lru == -1);
//     TEST(manager.mru == -1);
//     TEST(manager.buffers != NULL);

//     int i;
//     for (i = 0; i < 5; ++i) {
//         TEST(manager.buffers[i].table_id == INVALID_TABLENUM);
//     }

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
// })

// TEST_SUITE(buffer_manager_shutdown, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));
    
//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct page_uri_t uri;
//     uri.table_id = tid;
//     uri.pagenum = FILE_HEADER_PAGENUM;

//     int i;
//     for (i = 0; i < 3; ++i) {
//         buffer_manager_load(&manager, &tables, &uri);
//     }

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     TEST(manager.num_buffer == 0);
//     TEST(manager.capacity == 0);
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_alloc, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     int i;
//     for (i = 0; i < 13; ++i) {
//         TEST(i % 5 == buffer_manager_alloc(&manager));
//         manager.buffers[i % 5].table_id = 10;
//         TEST_SUCCESS(buffer_append_mru(&manager.buffers[i % 5], FALSE));
//     }

//     for (i = 0; i < 5; ++i) {
//         manager.buffers[i].table_id = INVALID_TABLENUM;
//     }
//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
// })

// TEST_SUITE(buffer_manager_load, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));
    
//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct page_uri_t uri;
//     uri.table_id = tid;
//     uri.pagenum = FILE_HEADER_PAGENUM;

//     int idx = buffer_manager_load(&manager, &tables, &uri);
//     TEST(idx == 0);
//     TEST(manager.num_buffer == 1);
//     TEST(manager.mru == 0);
//     TEST(manager.lru == 0);

//     struct buffer_t* buffer = &manager.buffers[idx];
//     TEST(buffer->block_idx == 0);
//     TEST(buffer->prev_use == -1);
//     TEST(buffer->next_use == -1);
//     TEST(buffer->table_id == tid);
//     TEST(buffer->pagenum == FILE_HEADER_PAGENUM);

//     idx = buffer_manager_load(&manager, &tables, &uri);
//     TEST(idx == 1);
//     TEST(manager.num_buffer == 2);
//     TEST(manager.mru == 1);
//     TEST(manager.lru == 0);
//     TEST(buffer->prev_use == -1);
//     TEST(buffer->next_use == 1);

//     buffer = &manager.buffers[idx];
//     TEST(buffer->block_idx == 1);
//     TEST(buffer->prev_use == 0);
//     TEST(buffer->next_use == -1);

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_release_block, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));
    
//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct page_uri_t uri;
//     uri.table_id = tid;
//     uri.pagenum = FILE_HEADER_PAGENUM;

//     int idx = buffer_manager_load(&manager, &tables, &uri);
//     TEST(idx == 0);

//     idx = buffer_manager_load(&manager, &tables, &uri);
//     TEST(idx == 1);

//     TEST_SUCCESS(buffer_manager_release_block(&manager, 1));
//     TEST(manager.num_buffer == 1);
//     TEST(manager.lru == 0);
//     TEST(manager.mru == 0);
//     TEST(manager.buffers[0].next_use == -1);
//     TEST(manager.buffers[0].prev_use == -1);

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_release_table, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));
    
//     tablenum_t tid1 = table_manager_load(&tables, "testfile");
//     tablenum_t tid2 = table_manager_load(&tables, "testfile2");
//     TEST(tid1 != INVALID_TABLENUM);
//     TEST(tid2 != INVALID_TABLENUM);

//     struct page_uri_t uri;
//     uri.pagenum = FILE_HEADER_PAGENUM;

//     uri.table_id = tid1;
//     TEST(0 == buffer_manager_load(&manager, &tables, &uri));

//     uri.table_id = tid2;
//     TEST(1 == buffer_manager_load(&manager, &tables, &uri));
//     TEST(2 == buffer_manager_load(&manager, &tables, &uri));

//     TEST_SUCCESS(buffer_manager_release_table(&manager, tid2));
//     TEST(manager.num_buffer == 1);
//     TEST(manager.buffers[0].table_id == tid1);
//     TEST(manager.buffers[1].table_id == INVALID_TABLENUM);
//     TEST(manager.buffers[2].table_id == INVALID_TABLENUM);

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
//     remove("testfile2");
// })

// TEST_SUITE(buffer_manager_release, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));

//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct page_uri_t uri;
//     uri.table_id = tid;
//     uri.pagenum = FILE_HEADER_PAGENUM;

//     int i;
//     for (i = 0; i < 4; ++i) {
//         TEST(-1 != buffer_manager_load(&manager, &tables, &uri));
//     }
//     TEST(manager.lru == 0);

//     // case 1. lru
//     // 0 -> 1 -> 2 -> 3
//     TEST(0 == buffer_manager_release(&manager, &RELEASE_LRU));
//     TEST(manager.lru == 1);
//     TEST(manager.buffers[1].next_use == 2);

//     // case 2. lru is pinned
//     // 1 -> 2 -> 3
//     manager.buffers[1].pin++;
//     TEST(2 == buffer_manager_release(&manager, &RELEASE_LRU));
//     TEST(manager.lru == 1);

//     // case 3. mru
//     // 1 -> 3 -> 0 
//     manager.buffers[3].pin++;
//     TEST(0 == buffer_manager_load(&manager, &tables, &uri));
//     TEST(0 == buffer_manager_release(&manager, &RELEASE_LRU));
//     TEST(manager.lru == 1);

//     // case 1. mru
//     // 1 -> 3 -> 0
//     manager.buffers[1].pin = 0;
//     manager.buffers[3].pin = 0;
//     TEST(0 == buffer_manager_load(&manager, &tables, &uri));
//     TEST(0 == buffer_manager_release(&manager, &RELEASE_MRU));

//     // case 2. mru is pinned
//     // 1 -> 3 -> 0
//     TEST(0 == buffer_manager_load(&manager, &tables, &uri));
//     manager.buffers[0].pin++;

//     TEST(3 == buffer_manager_release(&manager, &RELEASE_MRU));

//     // case 3. lru
//     // 1 -> 0 -> 2
//     TEST(2 == buffer_manager_load(&manager, &tables, &uri));
//     manager.buffers[2].pin++;

//     TEST(1 == buffer_manager_release(&manager, &RELEASE_MRU));

//     manager.buffers[0].pin = 0;
//     manager.buffers[2].pin = 0;

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_find, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));

//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct table_t* table = table_manager_find(&tables, tid);
//     TEST(table != NULL);

//     struct page_uri_t uri;
//     uri.table_id = tid;

//     int i;
//     pagenum_t pagenum[3];
//     for (i = 0; i < 3; ++i) {
//         pagenum[i] = page_create(&table->file_manager);
//         TEST(pagenum != INVALID_PAGENUM);

//         uri.pagenum = pagenum[i];
//         TEST(i == buffer_manager_load(&manager, &tables, &uri));
//     }

//     for (i = 0; i < 3; ++i) {
//         uri.pagenum = pagenum[i];
//         TEST(i == buffer_manager_find(&manager, &uri));
//     }

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_buffering, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));

//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct table_t* table = table_manager_find(&tables, tid);
//     TEST(table != NULL);

//     struct page_uri_t uri;
//     uri.table_id = tid;

//     int i;
//     pagenum_t pagenum[10];
//     struct ubuffer_t ubuf;
//     for (i = 0; i < 10; ++i) {
//         pagenum[i] = page_create(&table->file_manager);
//         TEST(pagenum != INVALID_PAGENUM);

//         uri.pagenum = pagenum[i];
//         ubuf = buffer_manager_buffering(&manager, &tables, &uri);
//         TEST(tid == ubuf.uri.table_id);
//         TEST(pagenum[i] == ubuf.uri.pagenum);
//         TEST(&manager.buffers[i % 5] == ubuf.buf);
//     }

//     for (i = 9; i >= 5; --i) {
//         uri.pagenum = pagenum[i];
//         ubuf = buffer_manager_buffering(&manager, &tables, &uri);
//         TEST(tid == ubuf.uri.table_id);
//         TEST(pagenum[i] == ubuf.uri.pagenum);
//         TEST(&manager.buffers[i % 5] == ubuf.buf);
//     }

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_new_page, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));

//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     int i;
//     struct ubuffer_t ubuf;
//     for (i = 0; i < 13; ++i) {
//         ubuf = buffer_manager_new_page(&manager, &tables, tid);
//         TEST(&manager.buffers[i % 5] == ubuf.buf);
//     }
    
//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

// TEST_SUITE(buffer_manager_free_page, {
//     struct buffer_manager_t manager;
//     TEST_SUCCESS(buffer_manager_init(&manager, 5));

//     struct table_manager_t tables;
//     TEST_SUCCESS(table_manager_init(&tables, 5));

//     tablenum_t tid = table_manager_load(&tables, "testfile");
//     TEST(tid != INVALID_TABLENUM);

//     struct ubuffer_t ubuf = buffer_manager_new_page(&manager, &tables, tid);

//     struct page_uri_t uri;
//     uri.table_id = tid;
//     uri.pagenum = ubuf.uri.pagenum;
//     TEST_SUCCESS(buffer_manager_free_page(&manager, &tables, &uri));

//     TEST_SUCCESS(buffer_manager_shutdown(&manager));
//     TEST_SUCCESS(table_manager_release(&tables));
//     remove("testfile");
// })

int buffer_manager_test() {
    return check_ubuffer_test()
        && from_buffer_test()
        && from_ubuffer_test()
        && buffer_init_test()
        && buffer_load_test()
        && buffer_new_page_test()
        && buffer_link_neighbor_test()
        && buffer_append_mru_test()
        && buffer_release_test()
        && buffer_start_end_test()
        && buffer_macro_test()
        // && buffer_manager_init_test()
        // && buffer_manager_shutdown_test()
        // && buffer_manager_alloc_test()
        // && buffer_manager_load_test()
        // && buffer_manager_release_block_test()
        // && buffer_manager_release_table_test()
        // && buffer_manager_release_test()
        // && buffer_manager_find_test()
        // && buffer_manager_buffering_test()
        // && buffer_manager_new_page_test()
        // && buffer_manager_free_page_test();
        ;
}
