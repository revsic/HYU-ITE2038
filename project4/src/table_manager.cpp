#include "table_manager.hpp"

Table::Table(std::string const& filename, BufferManager& manager) :
    file(filename), bpt(&file, &manager)
{
    // Do Nothing
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

fileid_t Table::fileid() const {
    return file.get_id();
}

fileid_t Table::rehash() {
    return file.rehash();
}

fileid_t Table::rehash(fileid_t new_id) {
    return file.rehash(new_id);
}

tableid_t TableManager::load(
    std::string const& filename, BufferManager& buffers
) {
    fileid_t id = FileManager::hash_filename(filename);
    while (tables.find(id) != tables.end()) {
        id = FileManager::rehash_fileid(id);
    }

    tables.emplace(id, Table(filename, buffers));
    tables[id].rehash(id);
    return static_cast<tableid_t>(id);
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
