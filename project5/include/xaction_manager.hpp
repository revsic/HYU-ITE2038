#ifndef XACTION_MANAGER_HPP
#define XACTION_MANAGER_HPP

#include "lock_manager.hpp"

#include <list>
#include <mutex>
#include <unordered_map>

using trxid_t = int;

constexpr trxid_t INVALID_TRXID = 0;

enum class TrxState {
    IDLE = 0,
    RUNNING = 1,
    WAITING = 2,
};

class Transaction {
public:
    Transaction();

    Transaction(trxid_t id);

    ~Transaction() = default;

    Transaction(Transaction&& trx) noexcept;

    Transaction(Transaction const& trx) = delete;

    Transaction& operator=(Transaction&& trx) noexcept;

    Transaction& operator=(Transaction const& trx) = delete;

    Status end_trx(LockManager& manager);

    Status require_lock(
        LockManager& manager, HierarchicalID hid, LockMode mode);

    Status release_locks(LockManager& manager);

private:
    trxid_t id;
    TrxState state;
    std::shared_ptr<Lock> wait;
    std::list<std::shared_ptr<Lock>> locks;
};

class TransactionManager {
public:
    TransactionManager();

    ~TransactionManager();

    TransactionManager(TransactionManager const&) = delete;

    TransactionManager(TransactionManager&&) = delete;

    TransactionManager& operator=(TransactionManager const&) = delete;

    TransactionManager& operator=(TransactionManager&&) = delete;

    Status shutdown();

    trxid_t new_trx();

    Status end_trx(trxid_t id, LockManager& manager);

    Status require_lock(
        trxid_t id, LockManager& manager, HierarchicalID hid, LockMode mode);

    Status release_locks(trxid_t id, LockManager& manager);

private:
    std::mutex mtx;

    trxid_t last_id;
    std::unordered_map<trxid_t, Transaction> trxs;
};

#endif