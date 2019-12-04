#include <chrono>
#include <future>

#include "dbms.hpp"

#define TEST(expr) if(!(expr)) { printf("%s, line %d: err\n", __FILE__, __LINE__); return 0; }
#define TEST_STATUS(expr) if(!(expr)) { printf("%s, line %d: err\n", __FILE__, __LINE__); return Status::FAILURE; }
#define TEST_SUCCESS(val) TEST(val == Status::SUCCESS);

using namespace std::chrono_literals;

int main() {
    LockManager manager;

    auto& module = manager.locks[HID(1, 2).make_hashable()];
    manager.detector.unit = 1h;

    Transaction trx(10);
    Transaction trx2(20);
    Transaction trx3(30);
    auto lock = manager.require_lock(&trx, HID(1, 2), LockMode::EXCLUSIVE);
    auto fut = std::async(std::launch::async, [&] {
        return manager.require_lock(&trx2, HID(1, 2), LockMode::EXCLUSIVE);
    });
    auto fut2 = std::async(std::launch::async, [&] {
        return manager.require_lock(&trx3, HID(1, 2), LockMode::SHARED);
    });

    TEST_SUCCESS(manager.release_lock(lock));
    auto lock2 = fut.get();

    auto fut3 = std::async(std::launch::async, [&] {
        return manager.require_lock(&trx, HID(1, 2), LockMode::SHARED);
    });

    TEST_SUCCESS(manager.release_lock(lock2));
    auto lock3 = fut2.get();
    auto lock4 = fut3.get();

    // auto& module = manager.locks[HID(1, 2).make_hashable()];
    TEST(module.mode == LockMode::SHARED);
    TEST(module.wait.size() == 0);
    TEST(module.run.size() == 2);
    TEST(std::set<std::shared_ptr<Lock>>(module.run.begin(), module.run.end())
        == std::set<std::shared_ptr<Lock>>({ lock3, lock4 }));
    TEST(manager.trxs.find(20) == manager.trxs.end());
    TEST(manager.trxs[10].first == &trx);
    TEST(manager.trxs[10].second == 1);
    TEST(manager.trxs[30].first == &trx3);
    TEST(manager.trxs[30].second == 1);

    TEST_SUCCESS(manager.release_lock(lock3));

    TEST(module.mode == LockMode::SHARED);
    TEST(module.wait.size() == 0);
    TEST(module.run.size() == 1);
    TEST(module.run.front() == lock4);
    TEST(manager.trxs.find(10) == manager.trxs.end());
    TEST(manager.trxs[30].first == &trx3);
    TEST(manager.trxs[30].second == 1);
}