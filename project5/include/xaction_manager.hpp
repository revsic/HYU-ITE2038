#ifndef XACTION_MANAGER_HPP
#define XACTION_MANAGER_HPP

#include "lock_manager.hpp"

#include <map>
#include <mutex>
#include <unordered_map>

/// Transaction state.
enum class TrxState {
    INVALID = 0,
    IDLE = 1,
    RUNNING = 2,
    WAITING = 3,
    ABORTED = 4,
};

/// Transaction structure.
class Transaction {
public:
    /// Default constructor.
    Transaction();

    /// Construct transaction with XID.
    Transaction(trxid_t id);

    /// Default destructor.
    ~Transaction() = default;

    /// Move constructor.
    Transaction(Transaction&& trx) noexcept;

    /// Deleted copy constructor.
    Transaction(Transaction const& trx) = delete;

    /// Move assignment.
    Transaction& operator=(Transaction&& trx) noexcept;

    /// Deleted copy assignment.
    Transaction& operator=(Transaction const& trx) = delete;

    /// Finish transaction in normal status.
    /// \param manager LockManager&, lock manager.
    /// \return Status, whether success to finish trx or not.
    Status end_trx(LockManager& manager);

    /// Abort transaction.
    /// \param dbms Database&, database system.
    /// \return Status, whether success to abort trx or not.
    Status abort_trx(Database& dbms);

    /// Acquire lock from lock manager.
    /// \param manager LockManager&, lock manager.
    /// \param hid HID, hierarchical ID (tableid + pageid).
    /// \param mode LockMode, lock mode.
    /// \return Status, whether success to own lock or not.
    Status require_lock(
        LockManager& manager, HID hid, LockMode mode);

    /// Release all locks.
    /// \param manager LockManager&, lock manager.
    /// \return Status, whether success or not.
    Status release_locks(LockManager& manager);

    /// Get transaction ID.
    trxid_t get_id() const;

    /// Get transaction state.
    TrxState get_state() const;

    /// Get waiting lock.
    std::shared_ptr<Lock> get_wait() const;

    /// Get all locks.
    std::map<HID, std::shared_ptr<Lock>> const& get_locks() const;

private:
    std::unique_ptr<std::mutex> mtx;                /// mutex;
    trxid_t id;                                     /// transaction ID.
    std::atomic<TrxState> state;                    /// transaction state.
    std::shared_ptr<Lock> wait;                     /// waiting lock.
    std::map<HID, std::shared_ptr<Lock>> locks;     /// all locks which trx owned.

    friend class LockManager;

    /// Elevate lock to stronger mode.
    Status elevate_lock(
        LockManager& manager, std::shared_ptr<Lock> lock, LockMode mode);

#ifdef TEST_MODULE
    friend struct LockManagerTest;
#endif
};


/// Transaction manager.
class TransactionManager {
public:
    /// Constructor.
    /// \param lockmng LockManager&, lock manager.
    TransactionManager(LockManager& lockmng);

    /// Default destructor.
    ~TransactionManager() = default;
    /// Deleted copy constructor.
    TransactionManager(TransactionManager const&) = delete;
    /// Deleted move constructor.
    TransactionManager(TransactionManager&&) = delete;
    /// Deleted copy assignment.
    TransactionManager& operator=(TransactionManager const&) = delete;
    /// Deleted move assignment.
    TransactionManager& operator=(TransactionManager&&) = delete;

    /// Create new transaction.
    /// \return trxid_t, transaction ID.
    trxid_t new_trx();

    /// Finish transaction.
    /// \param id trxid_t, transaction ID.
    /// \return Status, whether success to end transaction or not.
    Status end_trx(trxid_t id);

    /// Abort transaction.
    /// \param id trxid_t, transaction ID.
    /// \param dbms Database&, database system.
    /// \return Status, whether success to abort transaction or not.
    Status abort_trx(trxid_t id, Database& dbms);

    /// Acquire lock.
    /// \param id trxid_t, transaction ID.
    /// \param hid HID, hierarchical ID, table ID and page ID.
    /// \param mode LockMode, lock mode.
    /// \return Status, whether success to acquire lock or not.
    Status require_lock(trxid_t id, HID hid, LockMode mode);

    /// Release all owned locks.
    /// \param id trxid_t, transaction ID.
    /// \return Status, whether success to release locks or not.
    Status release_locks(trxid_t id);

    /// Get transaction state.
    TrxState trx_state(trxid_t id) const;

private:
    mutable std::mutex mtx;         /// System level mutex.
    LockManager* lock_manager;      /// Lock manager pointer.

    trxid_t last_id;                                    /// last transaction ID.
    std::unordered_map<trxid_t, Transaction> trxs;      /// all managed transactions.

#ifdef TEST_MODULE
    friend struct LockManagerTest;
#endif
};

#endif