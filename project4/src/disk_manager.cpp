#include <functional>

#include "disk_manager.hpp"

filenum_t FileManager::create_filenum(std::string const& filename) {
    std::size_t pos = filename.rfind('/');
    if (pos == std::string::npos) {
        return std::hash<std::string>{}(filename);
    } else {
        return std::hash<std::string>{}(filename.substr(pos + 1));
    }
}

FileManager::FileManager() : fp(nullptr), id(0) {
    // Do nothing.
}

FileManager::FileManager(std::string const& filename) : id(create_filenum(filename)) {
    if (fexist(filename.c_str())) {
        EXIT_ON_NULL(fp = fopen(filename.c_str(), "r+"));
    } else {
        EXIT_ON_FAILURE(file_create(filename));
    }
}

FileManager::~FileManager() {
    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

filenum_t FileManager::get_id() const {
    return id;
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
    return Page::create(
        [this](pagenum_t target, auto func) {
            return rwcallback(target, func);
        }, fp);
}

Status FileManager::page_free(pagenum_t pagenum) const {
    return Page::release(
        [this](pagenum_t target, auto&& func) { 
            return rwcallback(target, func);
        }, pagenum);
}

Status FileManager::page_read(pagenum_t pagenum, Page& dst) const {
    CHECK_TRUE(fpread(&dst, sizeof(Page), pagenum * PAGE_SIZE, fp));
    return Status::SUCCESS;
}

Status FileManager::page_write(pagenum_t pagenum, Page const& src) const {
    CHECK_TRUE(fpwrite(&src, sizeof(Page), pagenum * PAGE_SIZE, fp));
    return Status::SUCCESS;
}
