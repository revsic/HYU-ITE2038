#include "log_manager.hpp"
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
    Log log;
    TEST(log.lsn == INVALID_LSN);
    TEST(log.prev_lsn == INVALID_LSN);
    TEST(log.xid == INVALID_TRXID);
    TEST(log.type == LogType::INVALID);
    TEST(log.hid == HID());
    TEST(log.offset == -1);

    Log log2(3, 1, 2, LogType::END);
    TEST(log2.lsn == 3);
    TEST(log2.prev_lsn == 1);
    TEST(log2.xid == 2);
    TEST(log2.type == LogType::END);
    TEST(log2.hid == HID());
    TEST(log2.offset == -1);

    Record before;
    before.key = 300;
    before.value[0] = 'a';

    Record after;
    after.key = 400;
    after.value[0] = 'b';

    Log log3(4, 2, 3, LogType::UPDATE, HID(10, 20), 100, before, after);
    TEST(log3.lsn == 4);
    TEST(log3.prev_lsn == 2);
    TEST(log3.xid == 3);
    TEST(log3.type == LogType::UPDATE);
    TEST(log3.hid == HID(10, 20));
    TEST(log3.offset == 100);
    TEST(log3.before.key == 300);
    TEST(log3.before.value[0] == 'a');
    TEST(log3.after.key == 400);
    TEST(log3.after.value[0] == 'b');
})

TEST_SUITE(LogManagerTest::constructor, {
    LogManager logmng;
    TEST(logmng.last_lsn == 0);
    TEST(logmng.log_map.size() == 0);
})

TEST_SUITE(LogManagerTest::get_lsn, {
    LogManager logmng;
    TEST(1 == logmng.get_lsn());

    logmng.last_lsn = static_cast<size_t>(-1);
    TEST(1 == logmng.get_lsn());
})

TEST_SUITE(LogManagerTest::get_logs, {
    LogManager logmng;
    logmng.log_map[10].emplace_back();
    TEST(&logmng.get_logs(10) == &logmng.log_map[10]);
})

TEST_SUITE(LogManagerTest::log_update, {
    Record before;
    before.key = 300;
    before.value[0] = 'a';

    Record after;
    after.key = 400;
    after.value[0] = 'b';

    LogManager logmng;
    lsn_t lsn = logmng.log_update(10, HID(20, 30), 40, before, after);
    TEST(lsn == 1);
    TEST(logmng.log_map[10].size() == 1);
    TEST(logmng.log_map[10].front().lsn == 1);
    TEST(logmng.log_map[10].front().prev_lsn == 0);
    TEST(logmng.log_map[10].front().xid == 10);
    TEST(logmng.log_map[10].front().type == LogType::UPDATE);
    TEST(logmng.log_map[10].front().hid == HID(20, 30));
    TEST(logmng.log_map[10].front().offset == 40);
    TEST(logmng.log_map[10].front().before.key == 300);
    TEST(logmng.log_map[10].front().before.value[0] == 'a');
    TEST(logmng.log_map[10].front().after.key == 400);
    TEST(logmng.log_map[10].front().after.value[0] == 'b');

    lsn = logmng.log_update(10, HID(60, 70), 80, before, after);
    TEST(lsn == 2);
    TEST(logmng.log_map[10].size() == 2);
    TEST(logmng.log_map[10].front().lsn == 2);
    TEST(logmng.log_map[10].front().prev_lsn == 1);

    lsn = logmng.log_update(10, HID(90, 100), 110, before, after);
    TEST(lsn == 3);
    TEST(logmng.log_map[10].size() == 3);
    TEST(logmng.log_map[10].front().lsn == 3);
    TEST(logmng.log_map[10].front().prev_lsn == 2);
})

TEST_SUITE(LogManagerTest::log_abort, {
    LogManager logmng;
    lsn_t lsn = logmng.log_abort(10);
    TEST(lsn == 1);
    TEST(logmng.log_map[10].size() == 1);
    TEST(logmng.log_map[10].front().lsn == 1);
    TEST(logmng.log_map[10].front().prev_lsn == 0);
    TEST(logmng.log_map[10].front().xid == 10);
    TEST(logmng.log_map[10].front().type == LogType::ABORT);
})

TEST_SUITE(LogManagerTest::log_commit, {
    LogManager logmng;
    lsn_t lsn = logmng.log_abort(10);
    TEST(lsn == 1);
    TEST(logmng.log_map[10].size() == 1);
    TEST(logmng.log_map[10].front().lsn == 1);
    TEST(logmng.log_map[10].front().prev_lsn == 0);
    TEST(logmng.log_map[10].front().xid == 10);
    TEST(logmng.log_map[10].front().type == LogType::COMMIT);
})

TEST_SUITE(LogManagerTest::log_end, {
    LogManager logmng;
    lsn_t lsn = logmng.log_abort(10);
    TEST(lsn == 1);
    TEST(logmng.log_map[10].size() == 1);
    TEST(logmng.log_map[10].front().lsn == 1);
    TEST(logmng.log_map[10].front().prev_lsn == 0);
    TEST(logmng.log_map[10].front().xid == 10);
    TEST(logmng.log_map[10].front().type == LogType::END);
})

TEST_SUITE(LogManagerTest::remove_trxlog, {
    LogManager logmng;
    logmng.log_map[10].emplace_back();
    TEST(logmng.log_map[10].size() == 1);
    TEST_SUCCESS(logmng.remove_trxlog(10));
    TEST(logmng.remove_trxlog(10) == Status::FAILURE);
    TEST(logmng.log_map.find(10) == logmng.log_map.end());
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
