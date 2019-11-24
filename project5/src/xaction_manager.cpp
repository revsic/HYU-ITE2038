#include "xaction_manager.hpp"

Transaction::Transaction(trxid_t id) :
    id(id), state(TrxState::IDLE), wait(nullptr), locks()
{
    // Do Nothing
}

TransactionManager::TransactionManager() : mtx(), last_id(0), trxs() {
    // Do Nothing
}

trxid_t TransactionManager::new_trx() {
    std::unique_lock<std::mutex> lock(mtx);

    trxid_t id = last_id++;
    if (last_id < 0) {
        last_id = 0;
    }

    if (trxs.find(id) == trxs.end()) {
        return INVALID_TRXID;
    }

    trxs.emplace(id, Transaction(id));
    return id;
}

Status TransactionManager::end_trx(trxid_t id) {
    std::unique_lock<std::mutex> lock(mtx);
    if (trxs.find(id) != trxs.end()) {
        trxs.erase(id);
        return Status::SUCCESS;
    }
    return Status::FAILURE;
}
