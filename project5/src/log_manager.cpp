#include <cstring>

#include "log_manager.hpp"

/// WARNING: INVALID_TRXID should be 0.
Log::Log()
    : lsn(0), prev_lsn(0), xid(0), type(LogType::INVALID)
    , hid(), offset(-1), before(), after()
{
    // Do Nothing
}

Log::Log(
    lsn_t lsn, lsn_t prev_lsn, trxid_t xid, LogType type
) : lsn(lsn), prev_lsn(prev_lsn), xid(xid), type(type), hid(), offset(-1)
{
    // Do Nothing
}

Log::Log(
    lsn_t lsn, lsn_t prev_lsn, trxid_t xid, LogType type,
    HID hid, int offset, Record const& before, Record const& after
) : lsn(lsn), prev_lsn(prev_lsn), xid(xid), type(type),
    hid(hid), offset(offset) {
    std::memcpy(&this->before, &before, sizeof(Record));
    std::memcpy(&this->after, &after, sizeof(Record));
}

LogManager::LogManager() : mtx(), last_lsn(0), log_map() {
    // Do Nothing
}

lsn_t LogManager::log_update(
    trxid_t xid, HID hid, int offset, Record const& before, Record const& after
) {
    return wrapper(xid, LogType::UPDATE, hid, offset, before, after);
}

lsn_t LogManager::log_abort(trxid_t xid) {
    return wrapper(xid, LogType::ABORT);
}

lsn_t LogManager::log_commit(trxid_t xid) {
    return wrapper(xid, LogType::COMMIT);
}

lsn_t LogManager::log_end(trxid_t xid) {
    return wrapper(xid, LogType::END);
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
