#include "xaction_manager.hpp"

Transaction::Transaction()
    : id(INVALID_TRXID), state(TrxState::IDLE), wait(nullptr), locks()
{
    // Do Nothing
}

Transaction::Transaction(trxid_t id) :
    id(id), state(TrxState::IDLE), wait(nullptr), locks()
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
    // return release_locks(manager);
    return Status::SUCCESS;
}

// Status Transaction::require_lock(
//     LockManager& manager, HierarchicalID hid, LockMode mode
// ) {
//     return manager.require_lock(this, hid, mode);
// }

// Status Transaction::release_locks(LockManager& manager) {
//     for (Lock* lock : locks) {
//         CHECK_SUCCESS(manager.release_lock(lock->get_hid()));
//     }
//     return Status::SUCCESS;
// }

TransactionManager::TransactionManager() : mtx(), last_id(0), trxs() {
    // Do Nothing
}

TransactionManager::~TransactionManager() {
    shutdown();
}

Status TransactionManager::shutdown() {
    // TODO: impl shutdown method
    return Status::SUCCESS;
}

trxid_t TransactionManager::new_trx() {
    std::unique_lock<std::mutex> lock(mtx);

    trxid_t id = last_id++;
    if (last_id < 0) {
        last_id = 0;
    }

    if (trxs.find(id) == trxs.end()) {
        return INVALID_TRXID;
    }

    trxs.emplace(id, Transaction(id));
    return id;
}

Status TransactionManager::end_trx(trxid_t id, LockManager& manager) {
    std::unique_lock<std::mutex> lock(mtx);
    auto iter = trxs.find(id);
    if (iter != trxs.end()) {
        (*iter).second.end_trx(manager);
        trxs.erase(iter);
        return Status::SUCCESS;
    }
    return Status::FAILURE;
}

// Status TransactionManager::require_lock(
//     trxid_t id, LockManager& manager, HierarchicalID hid, LockMode mode
// ) {
//     auto iter = trxs.find(id);
//     CHECK_TRUE(iter != trxs.end());
//     return (*iter).second.require_lock(manager, hid, mode);
// }

// Status TransactionManager::release_locks(
//     trxid_t id, LockManager& manager
// ) {
//     auto iter = trxs.find(id);
//     CHECK_TRUE(iter != trxs.end());
//     return (*iter).second.release_locks(manager);
// }
