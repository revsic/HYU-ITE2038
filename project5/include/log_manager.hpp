#ifndef LOG_MANAGER_HPP
#define LOG_MANAGER_HPP

#include <list>
#include <map>

#include "headers.hpp"
#include "lock_manager.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

/// Log types.
enum class LogType {
    INVALID = 0,
    UPDATE = 1,
    COMMIT = 2,
    ABORT = 3,
    CLR = 4,
    END = 5,
};

/// Log structure.
struct Log {
    /// Default constructor.
    Log();
    /// Constructor for recordless log.
    /// \param lsn lsn_t, log sequence number.
    /// \param prev_lsn lsn_t, previous lsn.
    /// \param xid trxid_t, transaction ID.
    /// \param type LogType, log type.
    Log(lsn_t lsn, lsn_t prev_lsn, trxid_t xid, LogType type);

    /// Constructor for record relative log.
    /// \param lsn lsn_t, log sequence number.
    /// \param prev_lsn lsn_t, previous lsn.
    /// \param xid trxid_t, transaction ID.
    /// \param type LogType, log type.
    /// \param hid HID, hierarchical ID.
    /// \param offset int, record offset in target page.
    /// \param before Record const&, before record.
    /// \param after Record const&, after record.
    Log(lsn_t lsn, lsn_t prev_lsn, trxid_t xid, LogType type,
        HID hid, int offset, Record const& before, Record const& after);

    lsn_t lsn;          /// log sequence number.
    lsn_t prev_lsn;     /// previous lsn.
    trxid_t xid;        /// transaction ID.
    LogType type;       /// log type.
    HID hid;            /// hierarchical ID.
    int offset;         /// record offset in target page.
    Record before;      /// before record.
    Record after;       /// after record.
};


/// Log manager.
class LogManager {
public:
    /// Default constructor.
    LogManager();

    /// Write log about record update.
    /// \param xid trxid_t, transaction ID.
    /// \param hid HID, hierarchical ID.
    /// \param offset int, record index.
    /// \param before Record const&, before record.
    /// \param after Record const&, after record.
    /// \return lsn_t, allocated log sequence number.
    lsn_t log_update(
        trxid_t xid, HID hid, int offset,
        Record const& before, Record const& after);
    
    /// Write log about transaction abort.
    /// \param xid trxid_t, transaction ID.
    /// \return lsn_t, allocated log sequence number.
    lsn_t log_abort(trxid_t xid);

    /// Write log about commited transaction.
    /// \param xid trxid_t, transaction ID.
    /// \return lsn_t, allocated log sequence number.
    lsn_t log_commit(trxid_t xid);

    /// Write log about end transaction.
    /// \param xid trxid_t, transaction ID.
    /// \return lsn_t, allocated log sequence number.
    lsn_t log_end(trxid_t xid);

    /// Get all logs about given transaction ID.
    /// \return std::list<Log>, log list in reverse chronological order.
    std::list<Log> get_logs(trxid_t xid);

    /// Remove transaction log about given transaction ID.
    /// \param xid trxid_t, transaction ID.
    /// \return Status, whether success to remove all logs about xid or not.
    Status remove_trxlog(trxid_t xid);

private:
    std::mutex mtx;                                 /// System level mutex.
    lsn_t last_lsn;                                 /// last log sequence number.
    std::unordered_map<trxid_t, std::list<Log>> log_map;      /// logs.

    /// Get possible log sequence number.
    /// \return lsn_t, monotonically increasing lsn.
    lsn_t get_lsn();

    /// Log writer.
    /// \tparam typename... Args, arguments type.
    /// \param xid trxid_t, transaction ID.
    /// \param args Args&&..., arguments for log constructor.
    /// \return lsn_t, allocated log sequence number.
    template <typename... Args>
    lsn_t wrapper(trxid_t xid, Args&&... args) {
        std::unique_lock<std::mutex> own(mtx);
        std::list<Log>& log_list = log_map[xid];

        lsn_t lsn = get_lsn();
        lsn_t prev_lsn = INVALID_LSN;
        if (log_list.size() > 0) {
            prev_lsn = log_list.front().lsn;
        }

        log_list.emplace_front(lsn, prev_lsn, xid, std::forward<Args>(args)...);
        return lsn;
    }

#ifdef TEST_MODULE
    friend struct LogManagerTest;
#endif
};

#endif