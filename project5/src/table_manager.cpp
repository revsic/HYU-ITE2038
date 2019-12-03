#include "table_manager.hpp"

Table::Table() : file(), bpt(nullptr, nullptr) {
    // Do Nothing
}

Table::Table(std::string const& filename, BufferManager& manager) :
    file(filename), bpt(&file, &manager)
{
    // Do Nothing
}

Table::Table(Table&& other) noexcept :
    file(std::move(other.file)), bpt(std::move(other.bpt))
{
    bpt.reset_file(&file);
}

Table& Table::operator=(Table&& other) noexcept {
    file = std::move(other.file);
    bpt = std::move(other.bpt);
    bpt.reset_file(&file);
    return *this;
}

Status Table::print_tree() const {
    bpt.print_tree();
    return Status::SUCCESS;
}

Status Table::find(prikey_t key, Record* record, trxid_t xid) const {
    return bpt.find(key, record, xid);
}

std::vector<Record> Table::find_range(prikey_t start, prikey_t end) const {
    return bpt.find_range(start, end);
}

Status Table::insert(
    prikey_t key, uint8_t const* value, int value_size
) const {
    return bpt.insert(key, value, value_size);
}

Status Table::update(prikey_t key, Record const& rec, trxid_t xid) const {
    return bpt.update(key, rec, xid);
}

Status Table::remove(prikey_t key) const {
    return bpt.remove(key);
}

Status Table::destroy_tree() const {
    return bpt.destroy_tree();
}

Table::RecordIterator Table::begin() const {
    return bpt.begin();
}

Table::RecordIterator Table::end() const {
    return bpt.end();
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

FileManager& Table::filemng() {
    return file;
}

TableManager::TableManager(TableManager&& other) noexcept :
    tables(std::move(other.tables))
{
    // Do Nothing
}

TableManager& TableManager::operator=(TableManager&& other) noexcept {
    tables = std::move(other.tables);
    return *this;
}

tableid_t TableManager::load(
    std::string const& filename, BufferManager& buffers
) {
    // name, hash
    auto pair = FileManager::hash_filename(filename);
    fileid_t id = pair.second;
    tableid_t tid = convert(id);
    while (tables.find(tid) != tables.end()) {
        // if name of the file is same
        if (pair.first == tables[tid].filename()) {
            return tid;
        }
        // rehash id
        id = FileManager::rehash_fileid(id);
        tid = convert(id);
    }

    tables[tid] = Table(filename, buffers);
    // set rehashed id
    tables[tid].rehash(id);
    return tid;
}

Table const* TableManager::find(tableid_t id) const {
    auto iter = tables.find(id);
    if (iter != tables.end()) {
        return &iter->second;
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
    tableid_t tableid = static_cast<tableid_t>(id);
    return tableid > 0 ? tableid : -tableid;
}

FileManager* TableManager::find_file(tableid_t id) {
    auto iter = tables.find(id);
    if (iter != tables.end()) {
        return &iter->second.filemng();
    }
    return nullptr;
}
