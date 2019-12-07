#include "test.hpp"

struct TransactionTest {
TEST_METHOD(constructor);
TEST_METHOD(destructor);
TEST_METHOD(move_constructor);
TEST_METHOD(move_assignment);
TEST_METHOD(end_trx);
TEST_METHOD(abort_trx);
TEST_METHOD(require_lock);
TEST_METHOD(release_lock);
TEST_METHOD(getter);
};

struct TransactionManagerTest {
TEST_METHOD(constructor);
TEST_METHOD(new_trx);
TEST_METHOD(end_trx);
TEST_METHOD(abort_trx);
TEST_METHOD(require_lock);
TEST_METHOD(release_locks);
TEST_METHOD(trx_state);
}

TEST_SUITE(TransactionTest::constructor, {

})

TEST_SUITE(TransactionTest::destructor, {

})

TEST_SUITE(TransactionTest::move_constructor, {

})

TEST_SUITE(TransactionTest::move_assignment, {

})

TEST_SUITE(TransactionTest::end_trx, {

})

TEST_SUITE(TransactionTest::abort_trx, {

})

TEST_SUITE(TransactionTest::require_lock, {

})

TEST_SUITE(TransactionTest::release_lock, {

})

TEST_SUITE(TransactionTest::getter, {
    
})

TEST_SUITE(TransactionManagerTest::constructor, {

})

TEST_SUITE(TransactionManagerTest::new_trx, {

})

TEST_SUITE(TransactionManagerTest::end_trx, {

})

TEST_SUITE(TransactionManagerTest::abort_trx, {

})

TEST_SUITE(TransactionManagerTest::require_lock, {

})

TEST_SUITE(TransactionManagerTest::release_locks, {

})

TEST_SUITE(TransactionManagerTest::trx_state, {

})

int xaction_manager_test() {
    return TransactionTest::constructor_test()
        && TransactionTest::destructor_test()
        && TransactionTest::move_constructor_test()
        && TransactionTest::move_assignment_test()
        && TransactionTest::end_trx_test()
        && TransactionTest::abort_trx_test()
        && TransactionTest::require_lock_test()
        && TransactionTest::release_lock_test()
        && TransactionTest::getter_test()
        && TransactionManagerTest::constructor_test()
        && TransactionManagerTest::new_trx_test()
        && TransactionManagerTest::end_trx_test()
        && TransactionManagerTest::abort_trx_test()
        && TransactionManagerTest::require_lock_test()
        && TransactionManagerTest::release_locks_test()
        && TransactionManagerTest::trx_state_test();
}