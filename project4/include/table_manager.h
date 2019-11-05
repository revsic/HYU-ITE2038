#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "headers.h"
#include "table.h"

// Type definition

/// Vector structure for table.
struct table_vec_t {
    int size;                   /// the number of the allocated elements.
    int capacity;               /// capacity of array.
    struct table_t** array;     /// array for storing table structure.
};

/// Table manager.
struct table_manager_t {
    struct table_vec_t vec;     /// table vector.
};


// Procedure definition.

/// Search table from table vector with ID.
/// \param table_vec struct table_vec_t*, table vector.
/// \param table_id tablenum_t, table ID.
/// \return int, index of the found table, -1 for not found.
int table_searching_policy(struct table_vec_t* table_vec, tablenum_t table_id);

/// Initialize table vector.
/// \param table_vec struct table_vec_t*, table vector.
/// \param capacity int, initial capacity of the vector.
/// \return whether success or not.
int table_vec_init(struct table_vec_t* table_vec, int capacity);

/// Extend vector array in doubled size.
/// \param table_vec struct table_vec_t*, table vector.
/// \return whether success or not.
int table_vec_extend(struct table_vec_t* table_vec);

/// Append element to table vector.
/// \param table_vec struct table_vec_t*, table vector.
/// \param table struct table_t*, target table.
/// \return whether success or not.
int table_vec_append(struct table_vec_t* table_vec, struct table_t* table);

/// Find table from table vector with speicifying ID.
/// \param table_vec struct table_vec_t*, table vector.
/// \param table_id tablenum_t, target table ID.
/// \return whether success or not.
struct table_t* table_vec_find(struct table_vec_t* table_vec,
                               tablenum_t table_id);

/// Remove table from vector.
/// \param table_vec struct table_vec_t*, table vector.
/// \param table_id tablenum_t, target table ID.
/// \return whether success or not.
int table_vec_remove(struct table_vec_t* table_vec, tablenum_t table_id);

/// Shrink array capacity to adapt the real size.
/// \param table_vec struct table_vec_t*, table vector.
/// \return whether success or not.
int table_vec_shrink(struct table_vec_t* table_vec);

/// Free table vector.
/// \param table_vec struct table_vec_t*, table vector.
/// \return whether success or not.
int table_vec_release(struct table_vec_t* table_vec);

/// Initialize table manager.
/// \param manager table_manager_t*, table manager.
/// \param capacity int, initial capacity.
/// \return whether success or not.
int table_manager_init(struct table_manager_t* manager, int capacity);

/// Load table to manager.
/// \param manager table_manager_t*, table manager.
/// \param filename const char*, name of the file.
/// \param buffers struct buffer_manager_t*, buffer manager.
/// \return whether success or not.
tablenum_t table_manager_load(struct table_manager_t* manager,
                              const char* filename,
                              struct buffer_manager_t* buffers);

/// Find table from manager.
/// \param manager table_manager_t*, table manager.
/// \param table_id tablenum_t, target table ID.
/// \return whether success or not.
struct table_t* table_manager_find(struct table_manager_t* manager,
                                   tablenum_t table_id);

/// Remove table from manager.
/// \param manager table_manager_t*, table manager.
/// \param table_id tablenum_t, target table ID.
/// \return whether success or not.
int table_manager_remove(struct table_manager_t* manager, tablenum_t table_id);

/// Free table manager.
/// \param manager table_manager_t*, table manager.
/// \return whether success or not.
int table_manager_release(struct table_manager_t* manager);

#endif