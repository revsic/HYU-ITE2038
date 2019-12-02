#include <cstring>

#include "log_manager.hpp"

Log::Log()
    : lsn(0), prev_lsn(0), xid(INVALID_TRXID), type(LogType::INVALID)
    , hid(), offset(-1), before(), after()
{
    // Do Nothing
}

Log::Log(
    size_t lsn, size_t prev_lsn, trxid_t xid, LogType type,
    HID hid, int offset, Record const& before, Record const& after
) : lsn(lsn), prev_lsn(prev_lsn), xid(xid), type(type),
    hid(hid), offset(offset) {
    std::memcpy(&this->before, &before, sizeof(Record));
    std::memcpy(&this->after, &after, sizeof(Record));
}

LogManager::LogManager() : last_lsn(0) {
    // Do Nothing
}

lsn_t LogManager::log_update(
    trxid_t xid, HID hid, int offset, Record const& before, Record const& after
) {
    std::unique_lock<std::recursive_mutex> own(mtx);
    std::list<Log>& log_list = log_map[xid];

    lsn_t lsn = get_lsn();
    lsn_t last_lsn = INVALID_LSN;
    if (log_list.size() > 0) {
        last_lsn = log_list.back().lsn;
    }

    log_list.emplace_back(
        lsn, last_lsn, xid, LogType::UPDATE, hid, offset, before, after);
    return lsn;
}

std::list<Log> const& LogManager::get_logs(trxid_t xid) const {
    return log_map.at(xid);
}

Status LogManager::remove_trxlog(trxid_t xid) {
    log_map.erase(xid);
    return Status::SUCCESS;
}

lsn_t LogManager::get_lsn() {
    std::unique_lock<std::recursive_mutex> own(mtx);
    if (++last_lsn == 0) {
        ++last_lsn;
    }
    return last_lsn;
}
