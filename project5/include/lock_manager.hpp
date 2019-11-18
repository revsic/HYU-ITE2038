#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

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

#endif