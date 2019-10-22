#include "buffer_manager.h"
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

})

TEST_SUITE(buffer_load, {

})

TEST_SUITE(buffer_new_page, {

})

TEST_SUITE(buffer_link_neighbor, {

})

TEST_SUITE(buffer_append_mru, {

})

TEST_SUITE(buffer_release, {

})

TEST_SUITE(buffer_start, {

})

TEST_SUITE(buffer_end, {

})

TEST_SUITE(buffer_manager_init, {

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
