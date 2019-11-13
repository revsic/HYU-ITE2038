#ifndef DBMS_HPP
#define DBMS_HPP

#include "buffer_manager.hpp"
#include "table_manager.hpp"

class Database {
public:
    Database(int num_buffer);

    ~Database() = default;

    Database(Database const&) = delete;

    Database(Database&&) = delete;

    Database& operator=(Database const&) = delete;

    Database& operator=(Database&&) = delete;

    tableid_t open_table(std::string const& filename);

    Status close_table(tableid_t id);

    Status find(tableid_t id, prikey_t key, Record* record);

    Status insert(
        tableid_t id, prikey_t key, uint8_t const* value, int value_size);

    Status remove(tableid_t id, prikey_t key);

private:
    BufferManager buffers;
    TableManager tables;
};

#endif