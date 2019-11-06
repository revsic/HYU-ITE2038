#include "test.hpp"

TEST_SUITE(size, {
    TEST(sizeof(padded_file_header_t) == PAGE_SIZE);
    TEST(sizeof(page_header_t) == 128);
    TEST(sizeof(padded_free_page_t) == 128);
    TEST(sizeof(record_t) == 128);
    TEST(sizeof(page_t) == PAGE_SIZE);
})

TEST_SUITE(page_release, {
    // TODO: Impl test
})

TEST_SUITE(page_create, {
    // TODO: Impl test

})

TEST_SUITE(page_extend_free, {
    // TODO: Impl test

})

TEST_SUITE(page_getter, {
    // TODO: Impl test

})

TEST_SUITE(headers, {
    static_assert(sizeof(page_t) == PAGE_SIZE, "the size of page_t is not PAGE_SIZE");
    return size_test()
        && page_release_test()
        && page_create_test()
        && page_extend_free_test()
        && page_getter_test();
})
