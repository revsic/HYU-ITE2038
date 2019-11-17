#ifndef XACTION_MANAGER_HPP
#define XACTION_MANAGER_HPP

#include "lock_manager.hpp"

#include <atomic>
#include <list>
#include <unordered_map>

using trxid_t = int;

enum class TrxState {
    IDLE = 0,
    RUNNING = 1,
    WAITING = 2,
};

class Transaction {
public:
    Transaction(size_t id);

private:
    size_t id;
    TrxState state;
    std::list<Lock> locks;
};

class TransactionManager {
public:
    TransactionManager();

    trxid_t new_trx();

    Status end_trx(trxid_t id);

private:
    std::unordered_map<trxid_t, Transaction> trxs;

    static std::atomic<trxid_t> next_id;
};

#endif