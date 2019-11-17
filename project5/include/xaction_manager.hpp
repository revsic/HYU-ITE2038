#ifndef XACTION_MANAGER_HPP
#define XACTION_MANAGER_HPP

#include "lock_manager.hpp"

#include <list>

enum class TrxState {
    IDLE = 0,
    RUNNING = 1,
    WAITING = 2,
};

class Transaction {
public:

private:
    std::size_t id;
    TrxState state;
    std::list<Lock> locks;
};

#endif