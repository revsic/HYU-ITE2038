#ifndef TABLE_MANAGER_HPP
#define TABLE_MANAGER_HPP

#include <vector>
#include <unordered_map>

#include "bptree.hpp"
#include "bptree_iter.hpp"
#include "buffer_manager.hpp"
#include "disk_manager.hpp"
#include "headers.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

/// Table.
class Table {
public:
    using RecordIterator = BPTreeIterator;

    /// NULL-initializing constructor.
    Table();

    /// Initializing table with given filename and buffers.
    /// \param filename std::string const&, the name of the file.
    /// \param manager BufferManager&, buffer manager.
    Table(std::string const& filename, BufferManager& manager);

    /// Default destructor.
    ~Table() = default;

    /// Deleted copy constructor.
    Table(Table const&) = delete;
    
    /// Move constructor.
    Table(Table&& other) noexcept;

    /// Deleted copy assignment.
    Table& operator=(Table const&) = delete;

    /// Move assignment.
    Table& operator=(Table&& other) noexcept;

    /// Print tree.
    Status print_tree() const;

    /// Find the key and write the record to the given pointer.
    /// \param key prikey_t, primary key.
    /// \param record Record*, pointer to write the result record.
    /// \return Status, whether success to find the record or not.
    Status find(prikey_t key, Record* record) const;

    /// Range based search.
    /// \param start prikey_t, key, start point.
    /// \param end prikey_t, key, end point.
    /// \return std::vector<Record>, found records.
    std::vector<Record> find_range(prikey_t start, prikey_t end) const;

    /// Insert the record to the tree.
    /// \param key prikey_t, primary key.
    /// \param value uint8_t const*, byte sequence.
    /// \param value_size, the length of the value.
    /// \return Status, whether success to insert the record or not.
    Status insert(prikey_t key, uint8_t const* value, int value_size) const;

    /// Remove the record from the tree.
    /// \param key prikey_t, primary key.
    /// \return Status, whether success to remove the record or not.
    Status remove(prikey_t key) const;

    /// Update record from the tree.
    /// \param key prikey_t, primary key.
    /// \param record Record const&, update record.
    /// \return Status, whether success to update record or not.
    Status update(prikey_t key, Record const& rec) const;

    /// Free all nodes and clean up the tree.
    /// \return Status, whether success to remove all.
    Status destroy_tree() const;

    /// Get the beginning of the record iterator.
    RecordIterator begin() const;

    /// Get the end of the record iterator.
    RecordIterator end() const;

    /// Get file ID.
    /// \return fileid_t, file ID.
    fileid_t fileid() const;

    /// Rehash file ID with rehashing policy.
    /// \return fileid_t, rehashed ID.
    fileid_t rehash();

    /// Set file ID with given new ID.
    /// \param new_id fileid_t, new ID.
    /// \return fileid_t, found ID.
    fileid_t rehash(fileid_t new_id);

    /// Get name of the file.
    /// \return std::string const&, filename.
    std::string const& filename() const;

private:
    FileManager file;
    BPTree bpt;

    friend class TableManager;

    FileManager& filemng();

#ifdef TEST_MODULE
    friend struct TableTest;
#endif
};

/// Table manager.
class TableManager {
public:
    /// Default constructor.
    TableManager() = default;

    /// Default destructor.
    ~TableManager() = default;

    /// Deleted copy constructor.
    TableManager(TableManager const&) = delete;

    /// Deleted move constructor.
    TableManager(TableManager&& other) noexcept;

    /// Deleted copy assignment.
    TableManager& operator=(TableManager const&) = delete;

    /// Deleted move assignment.
    TableManager& operator=(TableManager&& other) noexcept;

    /// Load table in disk based structure with given file name and buffers.
    /// \param filename std::string const&, the name of the file.
    /// \param buffers BufferManager&, buffer manager.
    /// \return tableid_t, loaded table ID.
    tableid_t load(std::string const& filename, BufferManager& buffers);

    /// Find table structure by ID (const ver).
    /// \param tableid_t, id
    /// \return Table const*, found table pointer.
    Table const* find(tableid_t id) const;

    /// Remove table from manager.
    /// \param id tableid_t, table ID.
    /// \return Status, whether success to remove the table or not.
    Status remove(tableid_t id);

    /// Convert file ID to table ID.
    /// \param id fileid_t, file ID.
    /// \return tableid_t, converted table ID.
    static tableid_t convert(fileid_t id);

private:
    std::unordered_map<tableid_t, Table> tables;

    friend class Transaction;

    FileManager* find_file(tableid_t id);

#ifdef TEST_MODULE
    friend struct TableManagerTest;
#endif
};


#endif