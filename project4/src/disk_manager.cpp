#include <functional>

#include "disk_manager.hpp"

std::pair<std::string, std::size_t> FileManager::hash_filename(
    std::string const& filename
) {
    std::size_t pos = filename.rfind('/');
    std::string name = pos == std::string::npos
        ? filename
        : filename.substr(pos + 1);
    return std::make_pair(name, std::hash<std::string>{}(name));
}

fileid_t FileManager::rehash_fileid(fileid_t id) {
    constexpr std::size_t shift = 12;
    return std::hash<std::size_t>{}(
        (id << shift) | (id >> ((sizeof(std::size_t) << 3) - shift)));
}

FileManager::FileManager() : fp(nullptr), id(0) {
    // Do nothing.
}

FileManager::FileManager(std::string const& filename) {
    auto pair = hash_filename(filename);
    name = pair.first;
    id = pair.second;
    if (fexist(filename.c_str())) {
        EXIT_ON_NULL(fp = fopen(filename.c_str(), "r+"));
    } else {
        EXIT_ON_FAILURE(file_create(filename));
    }
}

FileManager::FileManager(FileManager&& other) noexcept :
    fp(other.fp), id(other.id), name(std::move(other.name))
{
    other.fp = nullptr;
    other.id = 0;
}

FileManager::~FileManager() {
    if (fp != nullptr) {
        fclose(fp);
        // for preventing double free
        fp = nullptr;
    }
}

FileManager& FileManager::operator=(FileManager&& other) noexcept {
    fp = other.fp;
    id = other.id;
    name = std::move(other.name);

    other.fp = nullptr;
    other.id = 0;

    return *this;
}

fileid_t FileManager::get_id() const {
    return id;
}

fileid_t FileManager::rehash() {
    return (id = rehash_fileid(id));
}

fileid_t FileManager::rehash(fileid_t new_id) {
    return (id = new_id);
}

std::string const& FileManager::get_filename() const {
    return name;
}

Status FileManager::file_init() {
    CHECK_TRUE(fresize(fp, PAGE_SIZE));
    // zero-initialization
    FileHeader file_header;
    file_header.free_page_number = 0;
    file_header.root_page_number = 0;
    file_header.number_of_pages = 0;
    // write file header
    CHECK_TRUE(fpwrite(&file_header, sizeof(FileHeader), 0, fp));
    return Status::SUCCESS;
}

Status FileManager::file_create(std::string const& filename) {
    CHECK_NULL(fp = fopen(filename.c_str(), "w+"));
    return file_init();
}

pagenum_t FileManager::page_create() const {
    // use page create abstraction
    return Page::create(
        [this](pagenum_t target, auto func) {
            return rwcallback(target, func);
        }, fp);
}

Status FileManager::page_free(pagenum_t pagenum) const {
    // use page release abstraction
    return Page::release(
        [this](pagenum_t target, auto&& func) { 
            return rwcallback(target, func);
        }, pagenum);
}

Status FileManager::page_read(pagenum_t pagenum, Page& dst) const {
    // low level read
    CHECK_TRUE(fpread(&dst, sizeof(Page), pagenum * PAGE_SIZE, fp));
    return Status::SUCCESS;
}

Status FileManager::page_write(pagenum_t pagenum, Page const& src) const {
    // low level write
    CHECK_TRUE(fpwrite(&src, sizeof(Page), pagenum * PAGE_SIZE, fp));
    return Status::SUCCESS;
}
