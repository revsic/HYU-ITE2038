#ifndef DBAPI_H
#define DBAPI_H

#include "disk_manager.h"

// GLOBAL VARIABLES

/// Table id for operating db-prefix operations.
extern int GLOBAL_TABLE_ID;

/// Global file manager.
extern struct file_manager_t GLOBAL_MANAGER;


// PROCEDURES

/// Open table and save file manager to global context.
/// \param pathname const char*, path of the file.
/// \return int, created table ID.
int open_table(const char* pathname);

/// Close global table;
/// \return int, whether success to close file or not.
int close_table();

/// Insert key and value to the most recent table.
/// \param key prikey_t, key.
/// \param value char*, value data.
/// \return int, whether success to insert data or not.
/// if key is duplicated, return FAILURE and do not update value.
int db_insert(int64_t key, char* value);

/// Find key from most recent table and return value.
/// \param key prikey_t, key.
/// \param value char*, value data, nullable.
/// \return int, whether success to find data or not.
/// if key is not found, return FAILURE.
int db_find(int64_t key, char* ret_val);

/// Delete key from table.
/// \param key prikey_t, key.
/// \return int, whether success to find data or not.
int db_delete(int64_t key);

#endif
