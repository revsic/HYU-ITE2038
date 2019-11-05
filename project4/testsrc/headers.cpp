#include "test.hpp"

TEST_SUITE(size, {
    TEST(sizeof(padded_file_header_t) == PAGE_SIZE);
    TEST(sizeof(page_header_t) == 128);
    TEST(sizeof(padded_free_page_t) == 128);
    TEST(sizeof(record_t) == 128);
    TEST(sizeof(page_t) == PAGE_SIZE);
})

TEST_SUITE(headers, {
    return size_test();
})
