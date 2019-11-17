#ifndef XACTION_MANAGER_HPP
#define XACTION_MANAGER_HPP

#include "lock_manager.hpp"

#include <list>
#include <mutex>
#include <unordered_map>

using trxid_t = int;

enum class TrxState {
    IDLE = 0,
    RUNNING = 1,
    WAITING = 2,
};

class Transaction {
public:
    Transaction(trxid_t id);

private:
    trxid_t id;
    TrxState state;
    std::list<Lock> locks;
};

class TransactionManager {
public:
    TransactionManager();

    trxid_t new_trx();

    Status end_trx(trxid_t id);

private:
    std::mutex mtx;

    trxid_t last_id;
    std::unordered_map<trxid_t, Transaction> trxs;
};

#endif