#include "headers.h"
#include "test.h"

TEST_SUITE(structure_size, {
    TEST(sizeof(struct padded_file_header_t) == PAGE_SIZE);
    TEST(sizeof(struct page_header_t) == 128);
    TEST(sizeof(struct padded_free_page_t) == 128);
    TEST(sizeof(struct page_t) == PAGE_SIZE);
})

int headers_test() {
    return structure_size_test();
}
