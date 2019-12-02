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

Status Transaction::abort_trx(
    BufferManager& bufmng, LockManager& lockmng, LogManager& logmng
) {
    for (Log const& log : logmng.get_logs(id)) {

    }

    /// TODO: recovery
    return release_locks(lockmng);
}

Status Transaction::require_lock(
    LockManager& manager, HID hid, LockMode mode
) {
    CHECK_TRUE(state != TrxState::IDLE);
    if (locks.find(hid) != locks.end()) {
        std::shared_ptr<Lock> lock = locks.at(hid);
        if (static_cast<int>(mode) > static_cast<int>(lock->get_mode())) {
            return elevate_lock(manager, std::move(lock), mode);
        }

        return Status::SUCCESS;
    }

    locks[hid] = manager.require_lock(this, hid, mode);
    return Status::SUCCESS;
}

Status Transaction::release_locks(LockManager& manager) {
    for (auto& pair : locks) {
        CHECK_SUCCESS(manager.release_lock(pair.second));
    }
    return Status::SUCCESS;
}

trxid_t Transaction::get_id() const {
    return id;
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

Status TransactionManager::abort_trx(
    trxid_t id, BufferManager& bufmng, LogManager& logmng
) {
    std::unique_lock<std::mutex> lock(mtx);
    auto iter = trxs.find(id);
    CHECK_TRUE(iter != trxs.end());
    CHECK_SUCCESS(iter->second.abort_trx(*lock_manager, bufmng, logmng));
    trxs.erase(iter);
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
