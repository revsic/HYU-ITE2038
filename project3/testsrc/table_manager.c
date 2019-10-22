#include "table_manager.h"
#include "test.h"

TEST_SUITE(create_tablenum, {

})

TEST_SUITE(rehash_tablenum, {

})

TEST_SUITE(searching_policy, {

})

TEST_SUITE(table_init, {

})

TEST_SUITE(table_load, {

})

TEST_SUITE(table_release, {

})

TEST_SUITE(table_vec_init, {

})

TEST_SUITE(table_vec_extend, {

})

TEST_SUITE(table_vec_append, {

})

TEST_SUITE(table_vec_find, {

})

TEST_SUITE(table_vec_remove, {

})

TEST_SUITE(table_vec_shrink, {

})

TEST_SUITE(table_manager_init, {

})

TEST_SUITE(table_manager_load, {

})

TEST_SUITE(table_manager_find, {

})

TEST_SUITE(table_manager_remove, {

})

TEST_SUITE(table_manager_release, {

})

int table_manager_test() {
    return create_tablenum_test()
        && rehash_tablenum_test()
        && searching_policy_test()
        && table_init_test()
        && table_load_test()
        && table_release_test()
        && table_vec_init_test()
        && table_vec_extend_test()
        && table_vec_append_test()
        && table_vec_find_test()
        && table_vec_remove_test()
        && table_vec_shrink_test()
        && table_manager_init_test()
        && table_manager_load_test()
        && table_manager_find_test()
        && table_manager_remove_test()
        && table_manager_release_test();
}