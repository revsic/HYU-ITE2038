#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>

#include "hashable.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

/// open literals for chrono data types.
using namespace std::chrono_literals;

/// Database system, forward declaration (ref: dbms.hpp).
class Database;

/// Transaction structure, forward declaration (ref: xaction_manager.hpp).
class Transaction;

/// Lock Mode.
enum class LockMode {
    IDLE = 0,
    SHARED = 1,
    EXCLUSIVE = 2,
};

/// Pack of tableid, pageid.
using HashableID = HashablePack<tableid_t, pagenum_t>;

/// Hierarchical ID for pointing specific page ID. 
struct HierarchicalID {
    tableid_t tid;              /// Table ID.
    pagenum_t pid;              /// Page ID.

    /// Default constructor.
    HierarchicalID();

    /// Construct hid with given table, page ID.
    /// \param tid tableid_t, table ID.
    /// \param pid pagenum_t, page ID.
    HierarchicalID(tableid_t tid, pagenum_t pid);

    /// Make ID in hashable format.
    /// \return HashableID, hashable hid.
    HashableID make_hashable() const;

    /// Define order about hid for ordered data structure.
    bool operator<(HierarchicalID const& other) const;

    /// Define as equality comparable object.
    bool operator==(HierarchicalID const& other) const;
};

/// Type alias for hierarchical ID as HID.
using HID = HierarchicalID;


/// Lock Structure.
class Lock {
public:
    /// Default constructor.
    Lock();
    /// Construct lock with given relative informations.
    Lock(HID hid, LockMode mode, Transaction* backref);
    /// Default destructor.
    ~Lock() = default;
    /// Move constructor.
    Lock(Lock&& lock) noexcept;
    /// Deleted copy constructor.
    Lock(Lock const&) = delete;
    /// Move assignment operator.
    Lock& operator=(Lock&& lock) noexcept;
    /// Deleted copy assignment operator.
    Lock& operator=(Lock const&) = delete;

    /// Return HID.
    HID get_hid() const;
    /// Return lock mode.
    LockMode get_mode() const;
    /// Return owner transaction.
    Transaction& get_backref() const;
    /// Whether this lock is waiting for others release or not.
    bool stop() const;
    /// Change lock state as waiting
    Status wait();
    /// Change lock state as runnable.
    Status run();

private:
    HID hid;                        /// hierarchical ID.
    LockMode mode;                  /// lock mode.
    Transaction* backref;           /// owner transaction.
    std::atomic<bool> wait_flag;    /// whether waiting for others or not.

#ifdef TEST_MODULE
    friend struct LockTest;
    friend struct LockManagerTest;
#endif
};


/// Lock manager.
class LockManager {
public:
    /// Default constructor.
    LockManager() = default;
    /// Default destructor.
    ~LockManager() = default;
    /// Deleted move constructor.
    LockManager(LockManager&&) = delete;
    /// Deleted copy constructor.
    LockManager(LockManager const&) = delete;
    /// Deleted move assignment.
    LockManager& operator=(LockManager&&) = delete;
    /// Deleted copy assignment.
    LockManager& operator=(LockManager const&) = delete;

    /// Acquire lock about specified page.
    /// \param backref Transaction*, lock owner.
    /// \param hid HID, hierarchical ID.
    /// \param mode LockMode, lock mode.
    /// \return std::shared_ptr<Lock>, created lock pointer.
    std::shared_ptr<Lock> require_lock(
        Transaction* backref, HID hid, LockMode mode);
    
    /// Relase lock.
    /// \param lock std::shared_ptr<Lock>, target lock.
    /// \param acquire_lock bool, acquire system lock or not.
    /// \return Status, whether success to release lock or not.
    Status release_lock(std::shared_ptr<Lock> lock, bool acquire_lock = true);

    /// Set base database structure.
    /// \param db Database&, database.
    Status set_database(Database& db);

private:
#ifdef TEST_MODULE
    friend class LockManagerTest;
#endif

    /// Unit for waiting lockable situation.
    static constexpr std::chrono::milliseconds LOCK_WAIT = 10ms;

    /// Lock struct for one specified page.
    struct LockStruct {
        LockMode mode;                              /// current lock mode.
        std::condition_variable cv;                 /// condition variable.
        std::list<std::shared_ptr<Lock>> run;       /// running locks.
        std::list<std::shared_ptr<Lock>> wait;      /// waiting locks.

        /// Default constructor.
        LockStruct();
        /// Default destructor.
        ~LockStruct() = default;
        /// Deleted copy constructor.
        LockStruct(LockStruct const&) = delete;
        /// Deleted copy assignment.
        LockStruct& operator=(LockStruct const&) = delete;

#ifdef TEST_MODULE
        friend struct LockManagerTest;
#endif
    };

    /// Type alias for lock table.
    using locktable_t = std::unordered_map<HashableID, LockStruct>;
    /// Type alias for transaction table.
    using trxtable_t = std::unordered_map<
        trxid_t,
        std::pair<Transaction*, int>>;

    /// Deadloock detector.
    struct DeadlockDetector {
        /// Graph node.
        struct Node {
            /// Reference count.
            int refcount() const;
            /// Outref count.
            int outcount() const;
            std::set<trxid_t> next_id;      /// next node.
            std::set<trxid_t> prev_id;      /// previous node.
        };

        /// Graph type.
        using graph_t = std::unordered_map<trxid_t, Node>;

        std::chrono::milliseconds unit;     /// duration unit for waiting next deadlock detection.
        std::chrono::time_point<std::chrono::steady_clock> last_use;    /// last detected point.

        /// Default constructor.
        DeadlockDetector();

        /// Scheduling deadlock detection running time.
        Status schedule();

        /// Reduce graph.
        /// \param graph graph_t&, waiting-graph.
        /// \param xid trxid_t, transaction ID.
        void reduce(graph_t& graph, trxid_t xid) const;

        /// Find cycle and return xid to aborts.
        /// \param locks locktable_t const&, lock table.
        /// \param xtable trxtable_t const&, transaction table.
        /// \return std::vector<trxid_t>, transaction IDs.
        std::vector<trxid_t> find_cycle(
            locktable_t const& locks, trxtable_t const& xtable);

        /// Choose transactions to abort.
        /// \param graph graph_t, waiting-graph.
        /// \return std::vector<trxid_t>, xid to aborts.
        std::vector<trxid_t> choose_abort(graph_t graph) const;

        /// Construct waiting-graph from lock table.
        /// \param locks locktable_t const&, lock table.
        /// \param xtable trxtable_t const&, transaction table.
        /// \return graph_t, created waiting graph.
        static graph_t construct_graph(
            locktable_t const& locks, trxtable_t const& xtable);

#ifdef TEST_MODULE
        friend struct LockManagerTest;
#endif
    };

    std::mutex mtx;                 /// system level mutex.
    locktable_t locks;              /// lock table.
    trxtable_t trxs;                /// relative transaction table.
    DeadlockDetector detector;      /// deadlock detector.
    Database* db;                   /// database system.

    /// Whether current lock is lockable on given page.
    /// \param module LockStruct const&, lock structs for target page.
    /// \param target std::shared_ptr<Lock> const&, target locks.
    /// \return bool, whether lockable or not.
    bool lockable(
        LockStruct const& module, std::shared_ptr<Lock> const& target) const;

    /// Detect deadlock and release if it is found.
    /// \return Status, whether deadlock found or not.
    Status detect_and_release();
};

#endif