#include "test.hpp"

TEST_SUITE(size, {
    TEST(sizeof(PaddedFileHeader) == PAGE_SIZE);
    TEST(sizeof(PageHeader) == 128);
    TEST(sizeof(PaddedFreePageHeader) == 128);
    TEST(sizeof(Record) == 128);
    TEST(sizeof(Page) == PAGE_SIZE);
})

TEST_SUITE(page_init, {
    // TODO: Impl test
})

TEST_SUITE(page_release_abstraction, {
    // TODO: Impl test
})

TEST_SUITE(page_create_abstraction, {
    // TODO: Impl test

})

TEST_SUITE(page_extend_free_abstraction, {
    // TODO: Impl test

})

TEST_SUITE(page_getter, {
    // TODO: Impl test

})

TEST_SUITE(headers, {
    static_assert(sizeof(Page) == PAGE_SIZE, "the size of Page is not PAGE_SIZE");
    return size_test()
        && page_init_test()
        && page_release_abstraction_test()
        && page_create_abstraction_test()
        && page_extend_free_abstraction_test()
        && page_getter_test();
})
