#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include <list>
#include <mutex>
#include <unordered_map>

#include "hashable.hpp"
#include "table_manager.hpp"

class Transaction;

enum class LockMode {
    INVALID = 0,
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

    Lock(Lock&& lock) noexcept;

    Lock(Lock const&) = delete;

    Lock& operator=(Lock&& lock) noexcept;

    HierarchicalID get_hid() const;

private:
    HierarchicalID hid;
    LockMode mode;
    Transaction* backref;
};

class LockManager {
public:
    LockManager() = default;

    Status require_lock(
        Transaction* backref, HierarchicalID hid, LockMode mode);
    
    Status release_lock(HierarchicalID hid);

    Status detect_deadlock();

    Status detect_and_release();

private:
    std::mutex mtx;
    std::unordered_map<HashableID, std::list<Lock>> locks;
};

#endif