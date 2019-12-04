#ifndef DBMS_HPP
#define DBMS_HPP

#include "buffer_manager.hpp"
#include "lock_manager.hpp"
#include "log_manager.hpp"
#include "table_manager.hpp"
#include "xaction_manager.hpp"
#include "join.hpp"

/// Database management system.
class Database {
public:
    /// Construct database with the number of buffers.
    /// \param num_buffer int, number of buffers.
    Database(int num_buffer);

    /// Default destructor.
    ~Database() = default;

    /// Deleted copy constructor.
    Database(Database const&) = delete;

    /// Deleted move constructor.
    Database(Database&&) = delete;

    /// Deleted copy assignment.
    Database& operator=(Database const&) = delete;

    /// Deleted move assignment.
    Database& operator=(Database&&) = delete;

    /// Open table with given filename.
    /// \param filename std::string const&, the name of the file.
    /// \return tableid_t, created table ID.
    tableid_t open_table(std::string const& filename);

    /// Close table.
    /// \param id tableid_t, table ID.
    /// \return Status, whether success to close table or not.
    Status close_table(tableid_t id);

    /// Print tree.
    /// \param id tableid_t, table ID.
    /// \return Status, whether success to print tree or not.
    Status print_tree(tableid_t id);

    /// Find the record with given key and write the result to record pointer.
    /// \param id tableid_t, table ID.
    /// \param key prikey_t, primary key.
    /// \param record Record*, record pointer to write the result.
    /// \param xid trxid_t, transaction id, default INVALID_TRXID.
    /// \return Status, whether success to find the record or not.
    Status find(
        tableid_t id, prikey_t key,
        Record* record, trxid_t xid = INVALID_TRXID);

    /// Range based search.
    /// \param id tableid_t, table ID.
    /// \param start prikey_t, start point.
    /// \param end prikey_t, end point.
    /// \return std::vector<Record>, result records.
    std::vector<Record> find_range(tableid_t id, prikey_t start, prikey_t end);

    /// Insert the record to the tree.
    /// \param id tableid_t, table ID.
    /// \param key prikey_t, primary key.
    /// \param value uint8_t const*, byte sequence.
    /// \param value_size int, the length of the sequence.
    /// \return Status, whether success to insert the record or not.
    Status insert(
        tableid_t id, prikey_t key, uint8_t const* value, int value_size);

    /// Remove the record from the tree.
    /// \param id tableid_t, table ID.
    /// \param key prikey_t, primary key.
    /// \return Status, whether success to remove the record or not.
    Status remove(tableid_t id, prikey_t key);

    /// Update the record in the tree.
    /// \param id tableid_t, table ID.
    /// \param key prikey_t, primary key.
    /// \param record Record const&, update record.
    /// \param xid trxid_t, transaction id, default INVALID_TRXID.
    /// \return Status, whether success to update record or not.
    Status update(
        tableid_t id, prikey_t key,
        Record const& rec, trxid_t xid = INVALID_TRXID);

    /// Remove all records and clean up the tree.
    /// \param id tableid_t, table ID.
    /// \return Status, whether success to clean up the tree.
    Status destroy_tree(tableid_t id);

    /// Natural join on primary key.
    /// \tparam F typename, Status(Record&).
    /// \param id1 tableid_t, left table id.
    /// \param id2 tableid_t, right table id.
    /// \param callback F&&, callback for found keys.
    /// \return Status, whether success to join or not.
    template <typename F>
    Status prikey_join(tableid_t id1, tableid_t id2, F&& callback) {
        Table const* table1 = (*this)[id1];
        Table const* table2 = (*this)[id2];
        CHECK_NULL(table1);
        CHECK_NULL(table2);
        return JoinOper::set_merge(table1, table2, std::forward<F>(callback));
    }

    /// Get table from table manager.
    /// \param id tableid_t, table ID.
    /// \return Table const*, table structure.
    Table const* operator[](tableid_t id) const;

    /// Start transaction.
    /// \return trxid_t, created transaction id.
    trxid_t begin_trx();

    /// End transaction.
    /// \param id trxid_t, target transaction id.
    /// \return Status, whether success or not.
    Status end_trx(trxid_t id);

    /// Abort transaction.
    /// \param id trxid_t, target transaction id.
    /// \return Status, whether success or not.
    Status abort_trx(trxid_t id);

private:
    TableManager tables;            /// table manager.
    BufferManager buffers;          /// buffer manager.
    LockManager locks;              /// lock manager.
    LogManager logs;                /// log manager.
    TransactionManager trxs;        /// transaction manager.

    friend class BPTree;
    friend class BufferManager;
    friend class Transaction;

#ifdef TEST_MODULE
    friend struct LockManagerTest;
#endif

    /// \tparam typename R, return type.
    /// \tparam typename F, callback type.
    /// \tparam typename Args..., arguments type.
    /// \param tableid_t id, table ID.
    /// \param default_value R&&, default return value.
    /// \param callback F&&, callback.
    /// \param args Args&&..., arguments.
    template <typename R, typename F, typename... Args>
    inline R wrapper(tableid_t id, R&& default_value, F&& callback, Args&&... args) {
        Table const* table = tables.find(id);
        if (table == nullptr) {
            return std::forward<R>(default_value);
        }
        return (table->*callback)(std::forward<Args>(args)...);
    }
};

#endif