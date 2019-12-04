#include "dbms.hpp"
#include "lock_manager.hpp"
#include "xaction_manager.hpp"
#include "test.hpp"

#include <memory>
#include <thread>

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
    TEST_METHOD(async_require_lock);
    TEST_METHOD(release_lock);
    TEST_METHOD(detect_and_release);
    TEST_METHOD(set_database);
    TEST_METHOD(lockstruct_constructor);
    TEST_METHOD(deadlock_constructor);
    TEST_METHOD(deadlock_schedule);
    TEST_METHOD(deadlock_reduce);
    TEST_METHOD(deadlock_find_cycle);
    TEST_METHOD(deadlock_choose_abort);
    TEST_METHOD(deadlock_construct_graph);
    TEST_METHOD(lockable);

    struct GraphInfo {
        LockManager::locktable_t locktable;
        LockManager::trxtable_t trxtable;
        std::unique_ptr<Transaction[]> trxs;
        LockManager::DeadlockDetector::graph_t graph;
    };

    static std::unique_ptr<GraphInfo> sample_dag();
    static std::unique_ptr<GraphInfo> sample_graph();
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
    /// getter is already tested in previous tests
})

TEST_SUITE(LockTest::run, {
    Lock lock;
    lock.wait = true;

    TEST_SUCCESS(lock.run());
    TEST(!lock.is_wait());
})

TEST_SUITE(LockManagerTest::require_lock, {

})

TEST_SUITE(LockManagerTest::async_require_lock, {

})

TEST_SUITE(LockManagerTest::release_lock, {

})

TEST_SUITE(LockManagerTest::detect_and_release, {

})

TEST_SUITE(LockManagerTest::set_database, {
    Database dbms(1);
    LockManager lockmng;
    lockmng.set_database(dbms);
    TEST(&dbms == lockmng.db);
})

TEST_SUITE(LockManagerTest::lockstruct_constructor, {
    LockManager::LockStruct module;
    TEST(module.mode == LockMode::IDLE);
    TEST(module.run.size() == 0);
    TEST(module.wait.size() == 0);
})

TEST_SUITE(LockManagerTest::deadlock_constructor, {
    LockManager::DeadlockDetector detector;
    TEST(detector.unit == LockManager::LOCK_WAIT);
})

TEST_SUITE(LockManagerTest::deadlock_schedule, {
    LockManager manager;
    manager.detector.unit = 1h;
    TEST(manager.detector.schedule() == Status::FAILURE);

    manager.detector.unit = manager.LOCK_WAIT;
    std::this_thread::sleep_for(manager.LOCK_WAIT);
    TEST_SUCCESS(manager.detector.schedule());
})

TEST_SUITE(LockManagerTest::deadlock_reduce, {
    LockManager::DeadlockDetector detector;
    auto graph_info = sample_graph();
    auto& graph = graph_info->graph;
    TEST(graph.size() == 7);

    detector.reduce(graph, 0);
    TEST(graph.size() == 6);
    TEST(graph[6].refcount() == 1);

    detector.reduce(graph, 2);
    TEST(graph.size() == 2);
    TEST(graph[5].next_id == graph[5].prev_id
        && graph[5].prev_id == std::set<trxid_t>({ 4 }));
    TEST(graph[4].next_id == graph[4].prev_id
        && graph[4].prev_id == std::set<trxid_t>({ 5 }));
    
    detector.reduce(graph, 5);
    TEST(graph.size() == 0);

    graph_info = sample_dag();
    auto& graph2 = graph_info->graph;
    TEST(graph2.size() == 6);

    detector.reduce(graph2, 0);
    TEST(graph2.size() == 4);
    TEST(graph2[3].refcount() == 1);

    detector.reduce(graph2, 2);
    TEST(graph2.size() == 0);
})

TEST_SUITE(LockManagerTest::deadlock_find_cycle, {
    LockManager::DeadlockDetector detector;
    auto graph_info = sample_graph();
    auto& locktable = graph_info->locktable;
    auto& trxtable = graph_info->trxtable;
    TEST(detector.find_cycle(locktable, trxtable)
        == std::vector<trxid_t>({ 2, 5 }));
    TEST(detector.unit == LockManager::LOCK_WAIT);
    
    graph_info = sample_dag();
    auto& locktable2 = graph_info->locktable;
    auto& trxtable2 = graph_info->trxtable;
    TEST(detector.find_cycle(locktable2, trxtable2)
        == std::vector<trxid_t>());
    TEST(detector.unit == LockManager::LOCK_WAIT * 2);
})

TEST_SUITE(LockManagerTest::deadlock_choose_abort, {
    LockManager::DeadlockDetector detector;
    auto graph_info = sample_graph();
    auto& graph = graph_info->graph;
    TEST(graph.size() == 7);

    detector.reduce(graph, 0);
    TEST(detector.choose_abort(std::move(graph))
        == std::vector<trxid_t>({ 2, 5 }));
})

TEST_SUITE(LockManagerTest::deadlock_construct_graph, {
    auto graph_info = sample_graph();
    auto& graph = graph_info->graph;
    TEST(graph[0].next_id == std::set<trxid_t>({ 6 }));
    TEST(graph[0].prev_id == std::set<trxid_t>());

    TEST(graph[1].next_id == std::set<trxid_t>({ 2, 5 }));
    TEST(graph[1].prev_id == std::set<trxid_t>({ 3 }));

    TEST(graph[2].next_id == std::set<trxid_t>({ 3, 6 }));
    TEST(graph[2].prev_id == std::set<trxid_t>({ 1, 4 }));

    TEST(graph[3].next_id == std::set<trxid_t>({ 1 }));
    TEST(graph[3].prev_id == std::set<trxid_t>({ 2 }));

    TEST(graph[4].next_id == std::set<trxid_t>({ 2, 5 }));
    TEST(graph[4].prev_id == std::set<trxid_t>({ 5 }));

    TEST(graph[5].next_id == std::set<trxid_t>({ 4 }));
    TEST(graph[5].prev_id == std::set<trxid_t>({ 1, 4 }));

    TEST(graph[6].next_id == std::set<trxid_t>());
    TEST(graph[6].prev_id == std::set<trxid_t>({ 0, 2 }));

    graph_info = sample_dag();
    auto& graph2 = graph_info->graph;
    TEST(graph2[0].next_id == std::set<trxid_t>({ 1 }));
    TEST(graph2[0].prev_id == std::set<trxid_t>());

    TEST(graph2[1].next_id == std::set<trxid_t>({ 3 }));
    TEST(graph2[1].prev_id == std::set<trxid_t>({ 0 }));

    TEST(graph2[2].next_id == std::set<trxid_t>({ 3 }));
    TEST(graph2[2].prev_id == std::set<trxid_t>());

    TEST(graph2[3].next_id == std::set<trxid_t>({ 4, 5 }));
    TEST(graph2[3].prev_id == std::set<trxid_t>({ 1, 2 }));

    TEST(graph2[4].next_id == std::set<trxid_t>());
    TEST(graph2[4].prev_id == std::set<trxid_t>({ 3 }));

    TEST(graph2[5].next_id == std::set<trxid_t>());
    TEST(graph2[5].prev_id == std::set<trxid_t>({ 3 }));
})

TEST_SUITE(LockManagerTest::lockable, {
    LockManager manager;
    LockManager::LockStruct module;
    module.mode = LockMode::IDLE;

    HID hid(10, 20);
    auto lock = std::make_shared<Lock>(hid, LockMode::SHARED, nullptr);
    TEST(manager.lockable(module, lock));

    module.mode = LockMode::SHARED;
    TEST(manager.lockable(module, lock));

    lock->mode = LockMode::EXCLUSIVE;
    TEST(!manager.lockable(module, lock));

    module.mode = LockMode::EXCLUSIVE;
    TEST(!manager.lockable(module, lock));
})

std::unique_ptr<LockManagerTest::GraphInfo> LockManagerTest::sample_dag() {
    auto graph_info = std::make_unique<GraphInfo>();
    auto& locktable = graph_info->locktable;
    auto& trxtable = graph_info->trxtable;
    
    graph_info->trxs = std::make_unique<Transaction[]>(6);
    auto& trxs = graph_info->trxs;

    for (int i = 0; i < 6; ++i) {
        trxs[i] = Transaction(i);
        trxtable[i] = std::make_pair(&trxs[i], -1);
    }

    auto& module01 = locktable[HID(0, 1).make_hashable()];
    module01.mode = LockMode::EXCLUSIVE;
    module01.run.push_back(std::make_shared<Lock>(HID(0, 1), LockMode::EXCLUSIVE, &trxs[1]));
    module01.wait.push_back(std::make_shared<Lock>(HID(0, 1), LockMode::EXCLUSIVE, &trxs[0]));
    trxs[0].wait = module01.wait.back();

    auto& module11 = locktable[HID(1, 1).make_hashable()];
    module11.mode = LockMode::EXCLUSIVE;
    module11.run.push_back(std::make_shared<Lock>(HID(1, 1), LockMode::EXCLUSIVE, &trxs[3]));
    module11.wait.push_back(std::make_shared<Lock>(HID(1, 1), LockMode::SHARED, &trxs[1]));
    trxs[1].wait = module11.wait.back();
    module11.wait.push_back(std::make_shared<Lock>(HID(1, 1), LockMode::SHARED, &trxs[2]));
    trxs[2].wait = module11.wait.back();

    auto& module21 = locktable[HID(2, 1).make_hashable()];
    module21.mode = LockMode::SHARED;
    module21.run.push_back(std::make_shared<Lock>(HID(2, 1), LockMode::SHARED, &trxs[4]));
    module21.run.push_back(std::make_shared<Lock>(HID(2, 1), LockMode::SHARED, &trxs[5]));
    module21.wait.push_back(std::make_shared<Lock>(HID(2, 1), LockMode::EXCLUSIVE, &trxs[3]));
    trxs[3].wait = module21.wait.back();
    
    graph_info->graph = LockManager::DeadlockDetector::construct_graph(locktable, trxtable);
    return std::move(graph_info);
}

std::unique_ptr<LockManagerTest::GraphInfo> LockManagerTest::sample_graph() {
    auto graph_info = std::make_unique<GraphInfo>();
    auto& locktable = graph_info->locktable;
    auto& trxtable = graph_info->trxtable;
    
    graph_info->trxs = std::make_unique<Transaction[]>(7);
    auto& trxs = graph_info->trxs;

    for (int i = 0; i < 7; ++i) {
        trxs[i] = Transaction(i);
        trxtable[i] = std::make_pair(&trxs[i], -1);
    }

    LockManager::LockStruct& module12 = locktable[HID(1, 2).make_hashable()];
    module12.mode = LockMode::EXCLUSIVE;
    module12.run.push_back(std::make_shared<Lock>(HID(1, 2), LockMode::EXCLUSIVE, &trxs[1]));
    module12.wait.push_back(std::make_shared<Lock>(HID(1, 2), LockMode::EXCLUSIVE, &trxs[3]));
    trxs[3].wait = module12.wait.back();

    LockManager::LockStruct& module32 = locktable[HID(3, 2).make_hashable()];
    module32.mode = LockMode::SHARED;
    module32.run.push_back(std::make_shared<Lock>(HID(3, 2), LockMode::SHARED, &trxs[3]));
    module32.run.push_back(std::make_shared<Lock>(HID(3, 2), LockMode::SHARED, &trxs[6]));
    module32.wait.push_back(std::make_shared<Lock>(HID(3, 2), LockMode::EXCLUSIVE, &trxs[2]));
    trxs[2].wait = module32.wait.back();

    LockManager::LockStruct& module22 = locktable[HID(2, 2).make_hashable()];
    module22.mode = LockMode::SHARED;
    module22.run.push_back(std::make_shared<Lock>(HID(2, 2), LockMode::SHARED, &trxs[2]));
    module22.run.push_back(std::make_shared<Lock>(HID(2, 2), LockMode::SHARED, &trxs[5]));
    module22.wait.push_back(std::make_shared<Lock>(HID(2, 2), LockMode::EXCLUSIVE, &trxs[1]));
    trxs[1].wait = module22.wait.back();
    module22.wait.push_back(std::make_shared<Lock>(HID(2, 2), LockMode::EXCLUSIVE, &trxs[4]));
    trxs[4].wait = module22.wait.back();

    LockManager::LockStruct& module42 = locktable[HID(4, 2).make_hashable()];
    module42.mode = LockMode::EXCLUSIVE;
    module42.run.push_back(std::make_shared<Lock>(HID(4, 2), LockMode::EXCLUSIVE, &trxs[4]));
    module42.wait.push_back(std::make_shared<Lock>(HID(4, 2), LockMode::EXCLUSIVE, &trxs[5]));
    trxs[5].wait = module42.wait.back();

    LockManager::LockStruct& module52 = locktable[HID(5, 2).make_hashable()];
    module52.mode = LockMode::EXCLUSIVE;
    module52.run.push_back(std::make_shared<Lock>(HID(5, 2), LockMode::EXCLUSIVE, &trxs[6]));
    module52.wait.push_back(std::make_shared<Lock>(HID(5, 2), LockMode::EXCLUSIVE, &trxs[0]));
    trxs[0].wait = module52.wait.back();

    graph_info->graph = LockManager::DeadlockDetector::construct_graph(locktable, trxtable);
    return std::move(graph_info);
}

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
        && LockManagerTest::async_require_lock_test()
        && LockManagerTest::release_lock_test()
        && LockManagerTest::detect_and_release_test()
        && LockManagerTest::set_database_test()
        && LockManagerTest::lockstruct_constructor_test()
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