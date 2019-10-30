#ifndef DBAPI_H
#define DBAPI_H

#include "dbms.h"

// GLOBAL VARIABLES

extern struct dbms_t GLOBAL_DBMS;


// PROCEDURES

/// Initialize database system with given buffer size.
/// \param num_buf int, the number of the buffers in caching pool.
/// \return int, whether success to initialize db or not.
int init_db(int num_buf);

/// Open table from given data file path.
/// \param pathname const char*, data file path.
/// \return tablenum_t, table id of the opened data file.
tablenum_t open_table(const char* pathname);

/// Close global table;
/// \param table_id tablenum_t, target table id.
/// \return int, whether success to close file or not.
int close_table(tablenum_t table_id);

/// Close database system, destroy all buffers and close files.
/// \return int, whether success to close file or not.
int shutdown_db();

/// Insert key and value to the most recent table.
/// \param table_id tablenum_t, id of the table.
/// \param key prikey_t, key.
/// \param value char*, value data.
/// \return int, whether success to insert data or not.
/// if key is duplicated, return FAILURE and do not update value.
int db_insert(tablenum_t table_id, int64_t key, char* value);

/// Find key from most recent table and return value.
/// \param table_id tablenum_t, id of the table.
/// \param key prikey_t, key.
/// \param retval char*, value data, nullable.
/// \return int, whether success to find data or not.
/// if key is not found, return FAILURE.
int db_find(tablenum_t table_id, int64_t key, char* retval);

/// Delete key from table.
/// \param table_id tablenum_t, id of the table.
/// \param key prikey_t, key.
/// \return int, whether success to find data or not.
int db_delete(tablenum_t table_id, int64_t key);

#endif
