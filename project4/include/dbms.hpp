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

    Status print_tree(tableid_t id);

    Status find(tableid_t id, prikey_t key, Record* record);

    std::vector<Record> find_range(tableid_t, prikey_t start, prikey_t end);

    Status insert(
        tableid_t id, prikey_t key, uint8_t const* value, int value_size);

    Status remove(tableid_t id, prikey_t key);

    Status destroy_tree(tableid_t id);

private:
    BufferManager buffers;
    TableManager tables;

    template <typename R, typename F, typename... Args>
    inline R wrapper(tableid_t id, R&& default_value, F&& callback, Args&&... args) {
        Table const* table = tables.find(id);
        if (table == nullptr) {
            return std::forward<R>(default_value);
        }
        return (table->*callback)(std::forward<Args>(args)...);
    }
};

#endif