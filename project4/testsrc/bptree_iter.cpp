#include "bptree_iter.hpp"

TEST_SUITE(copy_ctor, {

})

TEST_SUITE(copy_assign, {

})

TEST_SUITE(move_ctor, {

})

TEST_SUITE(move_assign, {

})

TEST_SUITE(begin, {

})

TEST_SUITE(end, {

})

TEST_SUITE(inc_operator, {

})

TEST_SUITE(cmp_operator, {

})

TEST_SUITE(ref_operator, {

})

int bptree_iter_test() {
    return copy_ctor_test()
        && copy_assign_test()
        && move_ctor_test()
        && move_assign_test()
        && begin_test()
        && end_test()
        && inc_operator_test()
        && cmp_operator_test()
        && ref_operator_test();
}