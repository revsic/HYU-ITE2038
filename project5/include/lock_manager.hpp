#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include <list>
#include <mutex>
#include <unordered_map>

#include "hashable.hpp"
#include "table_manager.hpp"

class Transaction;

enum class LockMode {
    IDLE = 0,
    SHARED = 1,
    EXCLUSIVE = 2,
};

// tableid, pageid, record index
using HashableID = HashablePack<tableid_t, pagenum_t, int>;

struct HierarchicalID {
    tableid_t tid;
    pagenum_t pid;
    int rid;

    HierarchicalID();

    HierarchicalID(tableid_t tid, pagenum_t pid, int rid);

    HashableID make_hashable() const;
};

class Lock {
public:
    Lock();

    Lock(HierarchicalID hid, LockMode mode, Transaction* backref);

    ~Lock();

    Lock(Lock&& lock) noexcept;

    Lock(Lock const&) = delete;

    Lock& operator=(Lock&& lock) noexcept;

    Lock& operator=(Lock const&) = delete;

    HierarchicalID get_hid() const;

    LockMode get_mode() const;

    Transaction& get_backref() const;

    bool is_wait() const;

    Status run();

private:
    HierarchicalID hid;
    LockMode mode;
    Transaction* backref;
    std::atomic<bool> wait; 
};

class LockManager {
public:
    LockManager() = default;

    ~LockManager();

    LockManager(LockManager&&) = delete;

    LockManager(LockManager const&) = delete;

    LockManager& operator=(LockManager&&) = delete;

    LockManager& operator=(LockManager const&) = delete;

    std::shared_ptr<Lock> require_lock(
        Transaction* backref, HierarchicalID hid, LockMode mode);
    
    Status release_lock(std::shared_ptr<Lock> lock);

    Transaction* detect_deadlock();

    Status detect_and_release();

private:
    struct LockStruct {
        LockMode mode;
        std::condition_variable cv;
        std::list<std::shared_ptr<Lock>> run;
        std::list<std::shared_ptr<Lock>> wait;

        LockStruct();
        ~LockStruct() = default;
        LockStruct(LockStruct const&) = delete;
        LockStruct& operator=(LockStruct const&) = delete;
    };

    std::mutex mtx;
    std::unordered_map<HashableID, LockStruct> locks;
    /// WARNING: assert required about trxid_t == int.
    std::unordered_map<int, Transaction*> trxs;

    bool lockable(
        LockStruct const& module, std::shared_ptr<Lock> const& target) const;

    Status schedule_detection();

    struct DeadlockDetector {
        struct Node {
            Transaction* backref;
        };
        using graph_t = std::unique_ptr<std::unique_ptr<Node[]>[]>;

        graph_t graph;

        DeadlockDetector(std::unordered_map<HashableID, LockStruct> const& locks);

        Transaction* find_cycle() const;

        static graph_t construct_graph(
            std::unordered_map<HashableID, LockStruct> const& locks);
    };
};

#endif