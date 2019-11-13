#include "dbms.hpp"

Database::Database(int num_buffer) : buffers(num_buffer), tables() {
    // Do Nothing
}

tableid_t Database::open_table(std::string const& filename) {
    return tables.load(filename, buffers);
}

Status Database::close_table(tableid_t id) {
    return tables.remove(id);
}

Status Database::find(tableid_t id, prikey_t key, Record* record) {
    Table const* table = tables.find(id);
    CHECK_NULL(table);

    return table->find(key, record);
}

Status Database::insert(tableid_t id, prikey_t key, uint8_t const* value, int value_size) {
    Table const* table = tables.find(id);
    CHECK_NULL(table);

    return table->insert(key, value, value_size);
}

Status Database::remove(tableid_t id, prikey_t key) {
    Table const* table = tables.find(id);
    CHECK_NULL(table);

    return table->remove(key);
}
