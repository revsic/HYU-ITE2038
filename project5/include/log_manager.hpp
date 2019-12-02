#ifndef LOG_MANAGER_HPP
#define LOG_MANAGER_HPP

#include "xaction_manager.hpp"

using lsn_t = size_t;

constexpr lsn_t INVALID_LSN = 0;

enum class LogType {
    INVALID = 0,
    UPDATE = 1,
    COMMIT = 2,
    ABORT = 3,
    CLR = 4,
    END = 5,
};

struct Log {
    Log();

    Log(lsn_t lsn, lsn_t prev_lsn, trxid_t xid, LogType type,
        HID hid, int offset, Record const& before, Record const& after);

    lsn_t lsn;
    lsn_t prev_lsn;
    trxid_t xid;
    LogType type;
    HID hid;
    int offset;
    Record before;
    Record after;
};

class LogManager {
public:
    LogManager();

    lsn_t log_update(
        trxid_t xid, HID hid, int offset,
        Record const& before, Record const& after);

    std::list<Log> const& get_logs(trxid_t xid) const;

    Status remove_trxlog(trxid_t xid);

private:
    std::recursive_mutex mtx;
    lsn_t last_lsn;
    std::map<trxid_t, std::list<Log>> log_map;

    lsn_t get_lsn();
};

#endif