#include "test.hpp"

struct LogManagerTest {
#define TEST_METHOD(x) static int x##_test();
    TEST_METHOD(constructor)
    TEST_METHOD(get_lsn)
    TEST_METHOD(get_logs)
    TEST_METHOD(log_update)
    TEST_METHOD(log_abort)
    TEST_METHOD(log_commit)
    TEST_METHOD(log_end)
    TEST_METHOD(remove_trxlog)
};

TEST_SUITE(log_constructor, {

})

TEST_SUITE(LogManagerTest::constructor, {

})

TEST_SUITE(LogManagerTest::get_lsn, {

})

TEST_SUITE(LogManagerTest::get_logs, {
    
})

TEST_SUITE(LogManagerTest::log_update, {

})

TEST_SUITE(LogManagerTest::log_abort, {

})

TEST_SUITE(LogManagerTest::log_commit, {

})

TEST_SUITE(LogManagerTest::log_end, {

})

TEST_SUITE(LogManagerTest::remove_trxlog, {

})

int log_manager_test() {
    return log_constructor_test()
        && LogManagerTest::constructor_test()
        && LogManagerTest::get_lsn_test()
        && LogManagerTest::get_logs_test()
        && LogManagerTest::log_update_test()
        && LogManagerTest::log_abort_test()
        && LogManagerTest::log_commit_test()
        && LogManagerTest::log_end_test()
        && LogManagerTest::remove_trxlog_test();
}
