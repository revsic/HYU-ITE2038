#include "test.hpp"
#include "table_manager.hpp"

struct TableTest {
#define TEST_NAME(name) static int name##_test();
    TEST_NAME(constructor)
    TEST_NAME(assignment)
    TEST_NAME(print_tree)
    TEST_NAME(find)
    TEST_NAME(find_range)
    TEST_NAME(insert)
    TEST_NAME(remove)
    TEST_NAME(destroy_tree)
    TEST_NAME(fileid)
    TEST_NAME(rehash)
    TEST_NAME(filename)
};

TEST_SUITE(TableTest::constructor, {
    // TODO: Impl
})

TEST_SUITE(TableTest::assignment, {
    // TODO: Impl
})

TEST_SUITE(TableTest::print_tree, {
    // just wrapping
})

TEST_SUITE(TableTest::find, {
    // just wrapping
})

TEST_SUITE(TableTest::find_range, {
    // just wrapping
})

TEST_SUITE(TableTest::insert, {
    // just wrapping
})

TEST_SUITE(TableTest::remove, {
    // just wrapping
})

TEST_SUITE(TableTest::destroy_tree, {
    // just wrapping
})

TEST_SUITE(TableTest::fileid, {
    // just wrapping get_id
})

TEST_SUITE(TableTest::rehash, {
    // just wrapping
})

TEST_SUITE(TableTest::filename, {
    // just wrapping get_filename
})

struct TableManagerTest {
    TEST_NAME(load)
    TEST_NAME(find)
    TEST_NAME(remove)
};

TEST_SUITE(TableManagerTest::load, {

})

TEST_SUITE(TableManagerTest::find, {
    
})

TEST_SUITE(TableManagerTest::remove, {
    
})

int table_manager_test() {
    return TableTest::constructor_test()
        && TableTest::assignment_test()
        && TableTest::print_tree_test()
        && TableTest::find_test()
        && TableTest::find_range_test()
        && TableTest::insert_test()
        && TableTest::remove_test()
        && TableTest::destroy_tree_test()
        && TableTest::fileid_test()
        && TableTest::rehash_test()
        && TableTest::filename_test();
}
