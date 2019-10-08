#include "test.h"

TEST_SUITE(file_init, {

})

TEST_SUITE(file_open, {

})

TEST_SUITE(file_close, {

})

TEST_SUITE(last_pagenum, {

})

TEST_SUITE(last_pagenum_from_size, {

})

TEST_SUITE(page_create, {

})

TEST_SUITE(page_extend_free, {

})

TEST_SUITE(page_free, {

})

TEST_SUITE(page_read, {

})

TEST_SUITE(page_write, {

})

int disk_manager_test() {
    return file_init_test()
        && file_open_test()
        && file_close_test()
        && last_pagenum_test()
        && last_pagenum_from_size_test()
        && page_create_test()
        && page_extend_free_test()
        && page_free_test()
        && page_read_test()
        && page_write_test();
}
