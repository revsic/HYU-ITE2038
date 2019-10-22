#include <stdlib.h>

#include "buffer_manager.h"
#include "utility.h"
#include "test.h"

TEST_SUITE(check_ubuffer, {
    struct buffer_t buf;
    struct page_uri_t uri;
    struct ubuffer_t ubuf;
    ubuf.buf = &buf;
    buf.table_id = 10;
    buf.pagenum = 30;

    uri.table_id = 10;
    uri.pagenum = 30;
    ubuf.uri = uri;
    TEST_SUCCESS(check_ubuffer(&ubuf));

    uri.table_id = 20;
    ubuf.uri = uri;
    TEST(check_ubuffer(&ubuf) == FAILURE);
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
    TEST_SUCCESS(buffer_init(&buf, 10, &manager));

    TEST(buf.table_id == INVALID_TABLENUM);
    TEST(buf.pagenum == INVALID_PAGENUM);
    TEST(buf.is_dirty == FALSE);
    TEST(buf.pin == 0);
    TEST(buf.prev_use == -1);
    TEST(buf.next_use == -1);
    TEST(buf.block_idx == 10);
    TEST(buf.table == NULL);
    TEST(buf.manager == &manager);
})

TEST_SUITE(buffer_load, {
    struct buffer_t buf;
    struct table_t table;
    TEST_SUCCESS(table_load(&table, "testfile"));
    TEST_SUCCESS(buffer_init(&buf, 10, NULL));
    
    pagenum_t pagenum = page_create(&table.file_manager);
    TEST_SUCCESS(buffer_load(&buf, &table, pagenum));

    TEST(buf.table_id == table.table_id);
    TEST(buf.pagenum == pagenum);
    TEST(buf.table == &table);

    TEST_SUCCESS(table_release(&table));
    remove("testfile");
})

TEST_SUITE(buffer_new_page, {
    struct buffer_t buf;
    struct table_t table;
    TEST_SUCCESS(table_load(&table, "testfile"));
    TEST_SUCCESS(buffer_init(&buf, 10, NULL));
    TEST_SUCCESS(buffer_new_page(&buf, &table));

    TEST(buf.table_id == table.table_id);
    TEST(buf.pagenum != INVALID_PAGENUM);
    TEST(buf.table == &table);

    TEST_SUCCESS(table_release(&table));
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

    free(manager.buffers);
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
})

TEST_SUITE(buffer_release, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    struct buffer_t* target = &manager.buffers[1];

    struct table_t table;
    TEST_SUCCESS(table_init(&table));
    TEST_SUCCESS(table_load(&table, "testfile"));
    TEST_SUCCESS(buffer_init(target, 1, &manager));
    TEST_SUCCESS(buffer_load(target, &table, FILE_HEADER_PAGENUM));

    struct page_uri_t uri;
    uri.table_id = table.table_id;
    uri.pagenum = FILE_HEADER_PAGENUM;

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
    TEST(manager.buffers[1].table == NULL);
    // file check
    TEST_SUCCESS(buffer_load(target, &table, FILE_HEADER_PAGENUM));
    TEST(file_header(from_buffer(target))->free_page_number == 0);
    TEST(file_header(from_buffer(target))->root_page_number == INVALID_PAGENUM);
    TEST(file_header(from_buffer(target))->number_of_pages == 0);

    // case 2. dirty
    struct ubuffer_t ubuffer;
    ubuffer.buf = target;
    ubuffer.uri = uri;
    BUFFER_WRITE(ubuffer, {
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
    TEST(manager.buffers[1].table == NULL);
    // file check
    TEST_SUCCESS(buffer_load(target, &table, FILE_HEADER_PAGENUM));
    TEST(file_header(from_buffer(target))->free_page_number == 0);
    TEST(file_header(from_buffer(target))->root_page_number == 1);
    TEST(file_header(from_buffer(target))->number_of_pages == 20);

    remove("testfile");
})

TEST_SUITE(buffer_start, {

})

TEST_SUITE(buffer_end, {

})

TEST_SUITE(buffer_manager_init, {
    struct buffer_manager_t manager;
    TEST_SUCCESS(buffer_manager_init(&manager, 5));

    TEST(manager.capacity == 5);
    TEST(manager.num_buffer == 0);
    TEST(manager.lru == -1);
    TEST(manager.mru == -1);
    TEST(manager.buffers != NULL);

    int i;
    for (i = 0; i < 5; ++i) {
        TEST(manager.buffers[i].table_id == INVALID_TABLENUM);
    }

    free(manager.buffers);
})

TEST_SUITE(buffer_manager_shutdown, {

})

TEST_SUITE(buffer_manager_alloc, {

})

TEST_SUITE(buffer_manager_load, {

})

TEST_SUITE(buffer_manager_release_block, {

})

TEST_SUITE(buffer_manager_release_table, {

})

TEST_SUITE(buffer_manager_release, {

})

TEST_SUITE(buffer_manager_find, {

})

TEST_SUITE(buffer_manager_buffering, {

})

TEST_SUITE(buffer_manager_new_page, {

})

TEST_SUITE(buffer_manager_free_page, {

})

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
        && buffer_start_test()
        && buffer_end_test()
        && buffer_manager_init_test()
        && buffer_manager_shutdown_test()
        && buffer_manager_alloc_test()
        && buffer_manager_load_test()
        && buffer_manager_release_block_test()
        && buffer_manager_release_table_test()
        && buffer_manager_release_test()
        && buffer_manager_find_test()
        && buffer_manager_buffering_test()
        && buffer_manager_new_page_test()
        && buffer_manager_free_page_test();
}
