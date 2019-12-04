#include <chrono>
#include <future>

#include "dbms.hpp"

#define TEST(expr) if(!(expr)) { printf("%s, line %d: err\n", __FILE__, __LINE__); return 0; }
#define TEST_STATUS(expr) if(!(expr)) { printf("%s, line %d: err\n", __FILE__, __LINE__); return Status::FAILURE; }
#define TEST_SUCCESS(val) TEST(val == Status::SUCCESS);

using namespace std::chrono_literals;

int main() {
    Database dbms(4);
    LockManager& manager = dbms.locks;

    trxid_t xid = dbms.begin_trx();
    trxid_t xid2 = dbms.begin_trx();
    TEST_SUCCESS(dbms.trxs.require_lock(xid, HID(1, 2), LockMode::EXCLUSIVE));
    TEST_SUCCESS(dbms.trxs.require_lock(xid2, HID(2, 1), LockMode::EXCLUSIVE));

    Status res;
    std::thread thread([&] {
        res = dbms.trxs.require_lock(xid, HID(2, 1), LockMode::EXCLUSIVE);
        return 0;
    });

    Status res2 = dbms.trxs.require_lock(xid2, HID(1, 2), LockMode::EXCLUSIVE);
}