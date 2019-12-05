#include <cstring>
#include <thread>

#include "dbms.hpp"
#include "utils.hpp"
#include "xaction_manager.hpp"

Transaction::Transaction()
    : id(INVALID_TRXID), state(TrxState::IDLE), wait(nullptr), locks()
{
    // Do Nothing
}

Transaction::Transaction(trxid_t id) :
    id(id), state(TrxState::RUNNING), wait(nullptr), locks()
{
    // Do Nothing
}

Transaction::Transaction(Transaction&& trx) noexcept
    : id(trx.id), state(trx.state), wait(trx.wait), locks(std::move(trx.locks))
{
    trx.id = INVALID_TRXID;
    trx.state = TrxState::IDLE;
    trx.wait = nullptr;
}

Transaction& Transaction::operator=(Transaction&& trx) noexcept {
    id = trx.id;
    state = trx.state;
    wait = trx.wait;
    locks = std::move(trx.locks);

    trx.id = INVALID_TRXID;
    trx.state = TrxState::IDLE;
    trx.wait = nullptr;
    return *this;
}

Status Transaction::end_trx(LockManager& manager) {
    return release_locks(manager);
}

Status Transaction::abort_trx(Database& dbms) {
    state = TrxState::ABORTED;
    for (Log const& log : dbms.logs.get_logs(id)) {
        FileManager* file = dbms.tables.find_file(log.hid.tid);
        CHECK_NULL(file);
        dbms.buffers.buffering(*file, log.hid.pid).write_void(
            [&](Page& page) {
                std::memcpy(&page.records()[log.offset], &log.before, sizeof(Record));
            });
    }

    return release_locks(dbms.locks);
}

Status Transaction::require_lock(
    LockManager& manager, HID hid, LockMode mode
) {
    CHECK_TRUE(state != TrxState::IDLE && state != TrxState::ABORTED);
    if (locks.find(hid) != locks.end()) {
        std::shared_ptr<Lock> lock = locks.at(hid);
        if (static_cast<int>(mode) > static_cast<int>(lock->get_mode())) {
            return elevate_lock(manager, std::move(lock), mode);
        }

        return Status::SUCCESS;
    }

    auto lock = manager.require_lock(this, hid, mode);
    if (state == TrxState::RUNNING) {
        locks[hid] = std::move(lock);
    }
    return state == TrxState::RUNNING ? Status::SUCCESS : Status::FAILURE;
}

Status Transaction::release_locks(LockManager& manager) {
    bool acquire_lock = state != TrxState::ABORTED;
    for (auto& pair : locks) {
        CHECK_SUCCESS(manager.release_lock(pair.second, acquire_lock));
    }
    locks.clear();
    return Status::SUCCESS;
}

trxid_t Transaction::get_id() const {
    return id;
}

TrxState Transaction::get_state() const {
    return state;
}

std::shared_ptr<Lock> Transaction::get_wait() const {
    return wait;
}

std::map<HID, std::shared_ptr<Lock>> const& Transaction::get_locks() const {
    return locks;
}

Status Transaction::elevate_lock(
    LockManager& manager, std::shared_ptr<Lock> lock, LockMode mode
) {
    HID hid = lock->get_hid();
    CHECK_SUCCESS(manager.release_lock(lock));
    locks[hid] = manager.require_lock(this, hid, mode);
    return Status::SUCCESS;
}

TransactionManager::TransactionManager(LockManager& lockmng)
    : mtx(), lock_manager(&lockmng), last_id(0), trxs()
{
    // Do Nothing
}

trxid_t TransactionManager::new_trx() {
    std::unique_lock<std::mutex> lock(mtx);

    trxid_t id = ++last_id;
    if (last_id < 0) {
        last_id = 1;
    }

    if (trxs.find(id) != trxs.end()) {
        return INVALID_TRXID;
    }

    trxs.emplace(id, Transaction(id));
    return id;
}

Status TransactionManager::end_trx(trxid_t id) {
    std::unique_lock<std::mutex> lock(mtx);
    auto iter = trxs.find(id);
    CHECK_TRUE(iter != trxs.end());
    CHECK_SUCCESS(iter->second.end_trx(*lock_manager));
    trxs.erase(iter);
    return Status::SUCCESS;
}

Status TransactionManager::abort_trx(trxid_t id, Database& dbms) {
    std::unique_lock<std::mutex> lock(mtx);
    auto iter = trxs.find(id);
    CHECK_TRUE(iter != trxs.end());
    CHECK_SUCCESS(iter->second.abort_trx(dbms));
    if (iter->second.get_wait() == nullptr) {
        trxs.erase(iter);
    }
    return Status::SUCCESS;
}

Status TransactionManager::require_lock(
    trxid_t id, HID hid, LockMode mode
) {
    auto iter = trxs.find(id);
    CHECK_TRUE(iter != trxs.end());
    return iter->second.require_lock(*lock_manager, hid, mode);
}

Status TransactionManager::release_locks(trxid_t id) {
    auto iter = trxs.find(id);
    CHECK_TRUE(iter != trxs.end());
    return iter->second.release_locks(*lock_manager);
}

TrxState TransactionManager::trx_state(trxid_t id) const {
    auto iter = trxs.find(id);
    if (iter == trxs.end()) {
        return TrxState::INVALID;
    }
    return iter->second.get_state();
}