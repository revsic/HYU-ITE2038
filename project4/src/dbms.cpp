#include "dbms.hpp"

Database::Database(int num_buffer) : buffers(num_buffer), tables() {
    // Do Nothing
}

Database::Database(Database&& other) noexcept
    : buffers(std::move(other.buffers)), tables(std::move(other.tables))
{
    // Do Nothing
}

Database& Database::operator=(Database&& other) noexcept {
    buffers = std::move(other.buffers);
    tables = std::move(other.tables);
    return *this;
}

tableid_t Database::open_table(std::string const& filename) {
    return tables.load(filename, buffers);
}

Status Database::close_table(tableid_t id) {
    Table const* table = tables.find(id);
    CHECK_NULL(table);

    buffers.release_file(table->fileid());
    return tables.remove(id);
}

Status Database::print_tree(tableid_t id) {
    return wrapper(id, Status::FAILURE, &Table::print_tree);
}

Status Database::find(tableid_t id, prikey_t key, Record* record) {
    return wrapper(id, Status::FAILURE, &Table::find, key, record);
}

std::vector<Record> Database::find_range(
    tableid_t id, prikey_t start, prikey_t end
) {
    return wrapper(id, std::vector<Record>(), &Table::find_range, start, end);
}

Status Database::insert(
    tableid_t id, prikey_t key, uint8_t const* value, int value_size
) {
    return wrapper(id, Status::FAILURE, &Table::insert, key, value, value_size);
}

Status Database::remove(tableid_t id, prikey_t key) {
    return wrapper(id, Status::FAILURE, &Table::remove, key);
}

Status Database::destroy_tree(tableid_t id) {
    return wrapper(id, Status::FAILURE, &Table::destroy_tree);
}
