#include "test.hpp"

TEST_SUITE(unit, {
    TEST(utils_test());
    TEST(fileio_test());
    TEST(headers_test());
    TEST(disk_manager_test());
    TEST(buffer_manager_test());
    TEST(bptree_test());
    TEST(bptree_iter_test());
    TEST(table_manager_test());
    TEST(join_test());
    TEST(hashable_test());
    TEST(lock_manager_test());
    TEST(log_manager_test());
    TEST(xaction_manager_test());
})

int main() {
    remove("testfile");
    unit_test();
    return 0;
}
