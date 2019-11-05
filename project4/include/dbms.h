#ifndef DBMS_H
#define DBMS_H

#include "buffer_manager.h"
#include "table_manager.h"

// Type definition

/// Database management system.
struct dbms_t {
    struct buffer_manager_t buffers;    /// buffer manager.
    struct table_manager_t tables;      /// table manager.
};


// Procedure definition

/// Initialize database system.
/// \param dbms struct dbms_t*, database management system.
/// \param num_buffer int, the number of the buffer.
/// \param table_capacity int, initial vector capacity for table array.
/// \return int, whether success or not.
int dbms_init(struct dbms_t* dbms, int num_buffer, int table_capacity);

/// Shutdown database system.
/// \param dbms struct dbms_t*, database management system.
/// \return int, whether success or not.
int dbms_shutdown(struct dbms_t* dbms);

/// Open table and register to dbms.
/// \param dbms struct dbms_t*, database management system.
/// \param filename const char*, the name of the file.
/// \return int, whether success or not.
tablenum_t dbms_open_table(struct dbms_t* dbms, const char* filename);

/// Close table from dbms.
/// \param dbms struct dbms_t*, database management system.
/// \param table_id tablenum_t, table ID.
/// \return int, whether success or not.
int dbms_close_table(struct dbms_t* dbms, tablenum_t table_id);

/// Find given key from table registered in dbms.
/// \param dbms struct dbms_t*, database management system.
/// \param table_id tablenum_t, table ID.
/// \param key prikey_t, target key.
/// \param record struct record_t*, write record to given memory.
/// \return int, whether success or not.
int dbms_find(struct dbms_t* dbms,
              tablenum_t table_id,
              prikey_t key,
              struct record_t* record);

/// Insert key value pair to table registered in dbms.
/// \param dbms struct dbms_t*, database management system.
/// \param table_id tablenum_t, table ID.
/// \param key prikey_t, input key.
/// \param value uint8_t*, input value.
/// \param value_size int, the size of the value in byte unit.
/// \return int, whether success or not.
int dbms_insert(struct dbms_t* dbms,
                tablenum_t table_id,
                prikey_t key,
                uint8_t* value,
                int value_size);

/// Delete key from table registered in dbms.
/// \param dbms struct dbms_t*, database management system.
/// \param table_id tablenum_t, table ID.
/// \param key prikey_t, target key.
/// \return int, whether success or not.
int dbms_delete(struct dbms_t* table, tablenum_t table_id, prikey_t key);

#endif