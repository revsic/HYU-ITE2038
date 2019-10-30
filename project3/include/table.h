#ifndef TABLE_H
#define TABLE_H

#include "disk_manager.h"
#include "bpt.h"
#include "headers.h"

// Type definition

/// Table structure.
struct table_t {
    tablenum_t id;                  /// table ID.
    struct bpt_t bpt;               /// B+Tree configuration.
    struct file_manager_t file;     /// file manager.
};


// Procedure definition.

/// Initialize table.
/// \param table struct table_t*, table.
/// \return int, whether success or not.
int table_init(struct table_t* table);

/// Load table from given filename.
/// \param table struct table_t*, table.
/// \param filename const char*, name of the file.
/// \param buffers struct buffer_manager_t*, buffer manager.
/// \return int, whether success or not.
int table_load(struct table_t* table,
               const char* filename,
               struct buffer_manager_t* buffers);

/// Release and cleam the table structure.
/// \param table struct table_t*, table.
/// \return int, whether success or not.
int table_release(struct table_t* table);

/// Find key value from table.
/// \param table struct table_t*, table.
/// \param key prikey_t, target key.
/// \param record struct record_t*, write record to given memory.
/// \return int, whether success or not.
int table_find(struct table_t* table, prikey_t key, struct record_t* record);

/// Range based search from table.
/// \param table struct table_t*, table.
/// \param start prikey_t, start position.
/// \param end prikey_t, end position.
/// \param retval struct record_vec_t*, record vector for writing result.
/// \return int, whether success or not.
int table_find_range(struct table_t* table, prikey_t start, prikey_t end, struct record_vec_t* retval);

/// Insert key, value pair to table.
/// \param table struct table_t*, table.
/// \param key prikey_t, input key.
/// \param value uint8_t*, input value.
/// \param value_size int, the size of the value in byte unit.
/// \return int, whether success or not.
int table_insert(struct table_t* table, prikey_t key, uint8_t* value, int value_size);

/// Delete key from table.
/// \param table struct table_t*, table.
/// \param key prikey_t, target key.
/// \return int, whether success or not.
int table_delete(struct table_t* table, prikey_t key);

/// Convert file ID to table ID.
/// \param filenum filenum_t, file ID.
/// \return tablenum_t, table ID.
tablenum_t table_id_from_filenum(filenum_t filenum);

/// Re-hash table ID and update.
/// \param table struct table_t*, target table.
/// \param update_id, boolean, whether update table ID and file ID or not.
/// \return tablenum_t, table ID.
tablenum_t table_rehash(struct table_t* table, int update_id);

#endif