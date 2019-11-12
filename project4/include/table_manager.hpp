#ifndef TABLE_MANAGER_HPP
#define TABLE_MANAGER_HPP

#include <vector>
#include <unordered_map>

#include "bptree.hpp"
#include "buffer_manager.hpp"
#include "disk_manager.hpp"
#include "headers.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

using tableid_t = int;

class Table {
public:
    Table();

    Table(std::string const& filename, BufferManager& manager);

    ~Table() = default;

    Table(Table const&) = delete;
    
    Table(Table&& other);

    Table& operator=(Table const&) = delete;

    Table& operator=(Table&& other);

    void print_tree() const;

    Status find(prikey_t key, Record* record) const;

    std::vector<Record> find_range(prikey_t start, prikey_t end) const;

    Status insert(prikey_t key, uint8_t const* value, int value_size) const;

    Status remove(prikey_t key) const;

    Status destroy_tree() const;

    fileid_t fileid() const;

    fileid_t rehash();

    fileid_t rehash(fileid_t new_id);

    std::string const& filename() const;

private:
    FileManager file;
    BPTree bpt;

#ifdef TEST_MODULE
    friend struct TableTest;
#endif
};

class TableManager {
public:
    TableManager() = default;

    ~TableManager() = default;

    TableManager(TableManager const&) = delete;

    TableManager(TableManager&&) = delete;

    TableManager& operator=(TableManager const&) = delete;

    TableManager& operator=(TableManager&&) = delete;

    tableid_t load(std::string const& filename, BufferManager& buffers);

    Table* find(tableid_t id);

    Table const* find(tableid_t id) const;

    Status remove(tableid_t id);

private:
    std::unordered_map<tableid_t, Table> tables;

    static tableid_t convert(fileid_t id);

#ifdef TEST_MODULE
    friend struct TableManagerTest;
#endif
};


#endif