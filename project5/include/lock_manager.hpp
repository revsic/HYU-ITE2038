#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include <list>
#include <mutex>
#include <unordered_map>

#include "table_manager.hpp"

class Transaction;

enum class LockMode {
    SHARED = 0,
    EXCLUSIVE = 1,
};

enum class LockLevel {
    INVALID = 0,
    DATABASE = 1,
    TABLE = 2,
    PAGE = 3,
    RECORD = 4,
};

struct HashableID {
    tableid_t tid;
    pagenum_t pid;
    int record_idx;
    LockLevel level;
};

namespace std {
    template <>
    struct hash<HashableID> {
        size_t operator()(const HashableID&) const {
            return 0;
        }
    };
}

class Lock {
public:
    Lock() = default;

private:
    tableid_t tid;
    pagenum_t pid;
    int record_idx;
    LockMode mode;
    LockLevel level;
    Transaction* backref;

};

class LockManager {
public:
    LockManager() = default;

private:
    std::mutex mtx;
    std::unordered_map<HashableID, std::list<Lock>> page_locks;
};

#endif