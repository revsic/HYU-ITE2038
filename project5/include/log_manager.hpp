#ifndef LOG_MANAGER_HPP
#define LOG_MANAGER_HPP

#include <list>
#include <map>

#include "headers.hpp"
#include "lock_manager.hpp"

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

    Log(lsn_t lsn, lsn_t prev_lsn, trxid_t xid, LogType type);

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
    
    lsn_t log_abort(trxid_t xid);

    lsn_t log_commit(trxid_t xid);

    lsn_t log_end(trxid_t xid);

    std::list<Log> const& get_logs(trxid_t xid) const;

    Status remove_trxlog(trxid_t xid);

private:
    std::recursive_mutex mtx;
    lsn_t last_lsn;
    std::map<trxid_t, std::list<Log>> log_map;

    lsn_t get_lsn();

    template <typename... Args>
    lsn_t wrapper(trxid_t xid, Args&&... args) {
        std::unique_lock<std::recursive_mutex> own(mtx);
        std::list<Log>& log_list = log_map[xid];

        lsn_t lsn = get_lsn();
        lsn_t last_lsn = INVALID_LSN;
        if (log_list.size() > 0) {
            last_lsn = log_list.back().lsn;
        }

        log_list.emplace_front(lsn, last_lsn, xid, std::forward<Args>(args)...);
        return lsn;
    }
};

#endif