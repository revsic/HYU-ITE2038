#include "lock_manager.hpp"
#include "xaction_manager.hpp"
#include "test.hpp"

struct HierarchicalTest {
    TEST_METHOD(constructor);
    TEST_METHOD(make_hashable);
    TEST_METHOD(comparison);
};

struct LockTest {
    TEST_METHOD(constructor);
    TEST_METHOD(move_constructor);
    TEST_METHOD(move_assignment);
    TEST_METHOD(getter);
    TEST_METHOD(run);
};

struct LockManagerTest {
    TEST_METHOD(require_lock);
    TEST_METHOD(release_lock);
    TEST_METHOD(detect_and_release);
    TEST_METHOD(set_database);
    TEST_METHOD(lockstruct_constructor);
    TEST_METHOD(deadlock_node_constructor);
    TEST_METHOD(deadlock_constructor);
    TEST_METHOD(deadlock_schedule);
    TEST_METHOD(deadlock_reduce);
    TEST_METHOD(deadlock_find_cycle);
    TEST_METHOD(deadlock_choose_abort);
    TEST_METHOD(deadlock_construct_graph);
    TEST_METHOD(lockable);
};

TEST_SUITE(HierarchicalTest::constructor, {
    HID hid;
    TEST(hid.tid == INVALID_TABLEID);
    TEST(hid.pid == INVALID_PAGENUM);

    hid = HID(10, 20);
    TEST(hid.tid == 10);
    TEST(hid.pid == 20);
})

TEST_SUITE(HierarchicalTest::make_hashable, {
    HID hid(10, 20);
    TEST(hid.tid == 10);
    TEST(hid.pid == 20);

    HashableID hashable = hid.make_hashable();
    TEST(std::get<0>(hashable.data) == 10);
    TEST(std::get<1>(hashable.data) == 20);
})

TEST_SUITE(HierarchicalTest::comparison, {
    TEST(!(HID(10, 10) < HID(10, 10)));
    TEST(HID(10, 10) < HID(10, 20));
    TEST(HID(10, 20) < HID(20, 20));
    TEST(!(HID(10, 20) < HID(10, 10)));
    TEST(!(HID(20, 20) < HID(10, 20)));

    TEST(HID(10, 10) == HID(10, 10));
    TEST(!(HID(10, 10) == HID(10, 20)));
})

TEST_SUITE(LockTest::constructor, {
    Lock lock;
    TEST(lock.get_hid() == HID());
    TEST(lock.get_mode() == LockMode::IDLE);
    TEST(lock.backref == nullptr);
    TEST(!lock.is_wait());

    Transaction xaction;
    Lock lock2(HID(10, 20), LockMode::SHARED, &xaction);
    TEST(lock2.get_hid() == HID(10, 20));
    TEST(lock2.get_mode() == LockMode::SHARED);
    TEST(&lock2.get_backref() == &xaction);
    TEST(!lock2.is_wait());
})

TEST_SUITE(LockTest::move_constructor, {
    Transaction xaction;
    Lock lock(HID(10, 20), LockMode::SHARED, &xaction);
    Lock lock2(std::move(lock));

    TEST(lock.get_hid() == HID());
    TEST(lock.get_mode() == LockMode::IDLE);
    TEST(lock.backref == nullptr);
    TEST(!lock.is_wait());

    TEST(lock2.get_hid() == HID(10, 20));
    TEST(lock2.get_mode() == LockMode::SHARED);
    TEST(&lock2.get_backref() == &xaction);
    TEST(!lock2.is_wait());
})

TEST_SUITE(LockTest::move_assignment, {
    Transaction xaction;
    Lock lock(HID(10, 20), LockMode::SHARED, &xaction);
    Lock lock2;
    
    lock2 = std::move(lock);

    TEST(lock.get_hid() == HID());
    TEST(lock.get_mode() == LockMode::IDLE);
    TEST(lock.backref == nullptr);
    TEST(!lock.is_wait());

    TEST(lock2.get_hid() == HID(10, 20));
    TEST(lock2.get_mode() == LockMode::SHARED);
    TEST(&lock2.get_backref() == &xaction);
    TEST(!lock2.is_wait());

})

TEST_SUITE(LockTest::getter, {

})

TEST_SUITE(LockTest::run, {

})

TEST_SUITE(LockManagerTest::require_lock, {

})

TEST_SUITE(LockManagerTest::release_lock, {

})

TEST_SUITE(LockManagerTest::detect_and_release, {

})

TEST_SUITE(LockManagerTest::set_database, {

})

TEST_SUITE(LockManagerTest::lockstruct_constructor, {

})

TEST_SUITE(LockManagerTest::deadlock_node_constructor, {

})

TEST_SUITE(LockManagerTest::deadlock_constructor, {

})

TEST_SUITE(LockManagerTest::deadlock_schedule, {

})

TEST_SUITE(LockManagerTest::deadlock_reduce, {

})

TEST_SUITE(LockManagerTest::deadlock_find_cycle, {

})

TEST_SUITE(LockManagerTest::deadlock_choose_abort, {

})

TEST_SUITE(LockManagerTest::deadlock_construct_graph, {

})

TEST_SUITE(LockManagerTest::lockable, {

})

int lock_manager_test() {
    return HierarchicalTest::constructor_test()
        && HierarchicalTest::make_hashable_test()
        && HierarchicalTest::comparison_test()
        && LockTest::constructor_test()
        && LockTest::move_constructor_test()
        && LockTest::move_assignment_test()
        && LockTest::getter_test()
        && LockTest::run_test()
        && LockManagerTest::require_lock_test()
        && LockManagerTest::release_lock_test()
        && LockManagerTest::detect_and_release_test()
        && LockManagerTest::set_database_test()
        && LockManagerTest::lockstruct_constructor_test()
        && LockManagerTest::deadlock_node_constructor_test()
        && LockManagerTest::deadlock_constructor_test()
        && LockManagerTest::deadlock_schedule_test()
        && LockManagerTest::deadlock_reduce_test()
        && LockManagerTest::deadlock_find_cycle_test()
        && LockManagerTest::deadlock_schedule_test()
        && LockManagerTest::deadlock_reduce_test()
        && LockManagerTest::deadlock_find_cycle_test()
        && LockManagerTest::deadlock_choose_abort_test()
        && LockManagerTest::deadlock_construct_graph_test()
        && LockManagerTest::lockable_test();
}