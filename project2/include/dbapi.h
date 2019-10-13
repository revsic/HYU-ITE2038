#ifndef DBAPI_H
#define DBAPI_H

#include "disk_manager.h"

// TYPE DEFINTION

/// Vector class for file manager.
struct file_vec_t {
    int size;                       /// size of the vector.
    int capacity;                   /// capacity of the vector.
    struct file_manager_t* manager; /// file manager array.
};


// GLOBAL CONSTANTS

/// Default file manager vector capacity
#define DEFAULT_FILE_VEC_CAPACITY 4

/// Maximum file manager vector capacity
#define MAXIMUM_FILE_VEC_CAPACITY 8


// GLOBAL VARIABLES

/// Table id for operating db-prefix operations.
extern int GLOBAL_TABLE_ID;

/// Global file manager vector.
extern struct file_vec_t GLOBAL_FILE_MANAGER;


// PROCEDURES

/// Initialize file manager vector.
/// \param vec struct file_vec_t*, file manager vector.
/// \return int, whether success to initialize or not.
int file_vec_init(struct file_vec_t* vec);

/// Deallocate file manager vector.
/// \param vec struct file_vec_t*, file manager vector.
/// \return int, whether success to deallocate or not.
int file_vec_free(struct file_vec_t* vec);

/// Expand capacity of file manager array.
/// \param vec struct file_vec_t*, file manager vector.
/// \return int, whether expansion success or not.
int file_vec_expand(struct file_vec_t* vec);

/// Append file manager to vector.
/// \param vec struct file_vec_t*, file manager vector.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to append or not.
int file_vec_append(struct file_vec_t* vec, struct file_manager_t* manager);

/// Open table and save file manager to global context.
/// \param pathname const char*, path of the file.
/// \return int, created table ID.
int open_table(const char* pathname);

/// Close table with given table ID.
/// \param tid int, table ID.
/// \return int, whether success to close file or not.
int close_table(int tid);

/// Get file manager from table ID.
/// \param tableid int, table ID.
/// \return struct file_manager_t*, file manager.
struct file_manager_t* get_file_manager(int tableid);

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
