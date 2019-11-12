#include "table_manager.hpp"

Table::Table() : file(), bpt(nullptr, nullptr) {
    // Do Nothing
}

Table::Table(std::string const& filename, BufferManager& manager) :
    file(filename), bpt(&file, &manager)
{
    // Do Nothing
}

Table::Table(Table&& other) :
    file(std::move(other.file)), bpt(std::move(other.bpt))
{
    bpt.update_file(&file);
}

Table& Table::operator=(Table&& other) {
    file = std::move(other.file);
    bpt = std::move(other.bpt);
    bpt.update_file(&file);
    return *this;
}

void Table::print_tree() const {
    bpt.print_tree();
}

Status Table::find(prikey_t key, Record* record) const {
    return bpt.find(key, record);
}

std::vector<Record> Table::find_range(prikey_t start, prikey_t end) const {
    return bpt.find_range(start, end);
}

Status Table::insert(
    prikey_t key, uint8_t const* value, int value_size
) const {
    return bpt.insert(key, value, value_size);
}

Status Table::remove(prikey_t key) const {
    return bpt.remove(key);
}

Status Table::destroy_tree() const {
    return bpt.destroy_tree();
}

fileid_t Table::fileid() const {
    return file.get_id();
}

fileid_t Table::rehash() {
    return file.rehash();
}

fileid_t Table::rehash(fileid_t new_id) {
    return file.rehash(new_id);
}

std::string const& Table::filename() const {
    return file.get_filename();
}

tableid_t TableManager::load(
    std::string const& filename, BufferManager& buffers
) {
    fileid_t id = FileManager::hash_filename(filename);
    while (tables.find(convert(id)) != tables.end()) {
        id = FileManager::rehash_fileid(id);
    }

    tableid_t tid = convert(id);
    tables[tid] = Table(filename, buffers);
    tables[tid].rehash(id);
    return tid;
}

Table* TableManager::find(tableid_t id) {
    if (tables.find(id) != tables.end()) {
        return &tables[id];
    }
    return nullptr;
}

Table const* TableManager::find(tableid_t id) const {
    if (tables.find(id) != tables.end()) {
        return &tables.at(id);
    }
    return nullptr;
}

Status TableManager::remove(tableid_t id) {
    auto iter = tables.find(id);
    if (iter == tables.end()) {
        return Status::FAILURE;
    }

    tables.erase(iter);
    return Status::SUCCESS;
}

tableid_t TableManager::convert(fileid_t id) {
    return static_cast<tableid_t>(id);
}
