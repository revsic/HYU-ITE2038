#ifndef DBAPI_HPP
#define DBAPI_HPP

#include "dbms.hpp"
#include "xaction_manager.hpp"

/// Global DBMS for db api.
extern std::unique_ptr<Database> GLOBAL_DB;

/// Global transaction manager.
extern TransactionManager GLOBAL_TRXMNG;

/// Initialize database.
/// \param buf_num int, the number of the buffer.
/// \return int, 0 for success, 1 for failure.
int init_db(int buf_num);

/// Load table to table manager.
/// \param pathname char const*, the name of the file.
/// \return int, table ID.
int open_table(char const* pathname);

/// Insert items to the specified table.
/// \param table_id int, table ID.
/// \param key int64_t, primary key.
/// \param value char const*, null-terminated byte sequence.
/// \return int, 0 for success, 1 for failure.
int db_insert(int table_id, int64_t key, char const* value);

/// Find items by key and write the result to ret_val.
/// \param table_id, int, table ID.
/// \param key int64_t, primary key.
/// \param ret_val char*, nullable, at least 120 bytes array for writing result.
/// \param trx_id, int, transaction id.
/// \return int, 0 for success to find, 1 for not exists.
int db_find(int table_id, int64_t key, char* ret_val, int trx_id);

/// Delete the records by key.
/// \param table_id int, table ID.
/// \param key int64_t, primary key.
/// \return int, whether success to delete the record (=0) or not (=1).
int db_delete(int table_id, int64_t key);

/// Release the table structure from table manager.
/// \param table_id int, tableID.
/// \return int, whether success to release the table (=0) or not (=1).
int close_table(int table_id);

/// Shutdown the database.
/// \return int, whether success to shutdown the db or not.
int shutdown_db();

/// Join two table naturally based on primary key.
/// \param table_id_1 int, left table ID.
/// \param table_id_2 int, right table ID.
/// \param pathname char const*, output path name.
/// \return int, whether success to join tables or not.
int join_table(int table_id_1, int table_id_2, char const* pathname);

/// Start transaction.
/// \return int, transaction ID if success else 0.
int begin_trx();

/// Finish transaction.
/// \param tid int, transaction ID.
/// \return given tid if success else 0.
int end_trx(int tid);

#endif