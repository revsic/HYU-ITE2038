#ifndef LOCK_MANAGER_HPP
#define LOCK_MANAGER_HPP

#include "table_manager.hpp"

class Transaction;

enum class LockMode {
    SHARED = 0,
    EXCLUSIVE = 1,
};

class Lock {
public:

private:
    tableid_t tid;
    pagenum_t pid;
    int record_idx;
    LockMode mode;
    Transaction* backref;

};

#endif