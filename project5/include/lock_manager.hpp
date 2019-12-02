#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include <chrono>
#include <list>
#include <mutex>
#include <set>
#include <unordered_map>

#include "hashable.hpp"
#include "table_manager.hpp"

using namespace std::chrono_literals;

/// WARNING: assertion required about trxid_t == int in xaction_maanger.hpp
using trxid_t = int;

class Transaction;

enum class LockMode {
    IDLE = 0,
    SHARED = 1,
    EXCLUSIVE = 2,
};

// tableid, pageid, record index
using HashableID = HashablePack<tableid_t, pagenum_t>;

struct HierarchicalID {
    tableid_t tid;
    pagenum_t pid;

    HierarchicalID();

    HierarchicalID(tableid_t tid, pagenum_t pid);

    HashableID make_hashable() const;

    bool operator<(HierarchicalID const& other) const;
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

    Status detect_and_release();

private:
    static constexpr std::chrono::milliseconds LOCK_WAIT = 10ms;

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

    using locktable_t = std::unordered_map<HashableID, LockStruct>;

    using trxtable_t = std::unordered_map<
        trxid_t,
        std::pair<Transaction*, int>>;

    struct DeadlockDetector {
        struct Node {
            Node();
            ~Node() = default;

            int refcount() const;

            std::set<trxid_t> next_id;
            std::set<trxid_t> prev_id;
        };

        using graph_t = std::unordered_map<trxid_t, Node>;

        int coeff;
        bool last_found;
        std::chrono::time_point<std::chrono::steady_clock> last_use;

        DeadlockDetector();

        Status schedule();

        void reduce(
            graph_t& graph, trxid_t xid, bool chaining = false) const;

        std::vector<trxid_t> find_cycle(
            locktable_t const& locks, trxtable_t const& xtable);

        std::vector<trxid_t> choose_abort(graph_t graph) const;

        static graph_t construct_graph(
            locktable_t const& locks, trxtable_t const& xtable);
    };

    std::mutex mtx;
    locktable_t locks;
    trxtable_t trxs;
    DeadlockDetector detector;

    bool lockable(
        LockStruct const& module, std::shared_ptr<Lock> const& target) const;
};

#endif