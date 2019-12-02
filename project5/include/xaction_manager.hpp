#ifndef XACTION_MANAGER_HPP
#define XACTION_MANAGER_HPP

#include "lock_manager.hpp"

#include <map>
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

    Status abort_trx(LockManager& lockmng, LogManager& logmng);

    Status require_lock(
        LockManager& manager, HID hid, LockMode mode);

    Status release_locks(LockManager& manager);

    trxid_t get_id() const;

    std::map<HID, std::shared_ptr<Lock>> const& get_locks() const;

private:
    trxid_t id;
    TrxState state;
    std::shared_ptr<Lock> wait;
    std::map<HID, std::shared_ptr<Lock>> locks;

    friend class LockManager;

    Status elevate_lock(
        LockManager& manager, std::shared_ptr<Lock> lock, LockMode mode);
};

class TransactionManager {
public:
    TransactionManager(LockManager& lockmng);

    ~TransactionManager();

    TransactionManager(TransactionManager const&) = delete;

    TransactionManager(TransactionManager&&) = delete;

    TransactionManager& operator=(TransactionManager const&) = delete;

    TransactionManager& operator=(TransactionManager&&) = delete;

    Status shutdown();

    trxid_t new_trx();

    Status end_trx(trxid_t id);

    Status require_lock(trxid_t id, HID hid, LockMode mode);

    Status release_locks(trxid_t id);

private:
    std::mutex mtx;
    LockManager* lock_manager;

    trxid_t last_id;
    std::unordered_map<trxid_t, Transaction> trxs;
};

#endif