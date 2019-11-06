#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP

#include <cstdio>
#include <cstdint>
#include <string>

#include "headers.hpp"
#include "status.hpp"

class FileManager {
public:
    FileManager();

    FileManager(std::string const& filename);

    ~FileManager();

    pagenum_t page_create() const;

    Status page_free(pagenum_t pagenum) const;

    Status page_read(pagenum_t pagenum, Page* dst) const;

    Status page_write(pagenum_t pagenum, Page* src) const;

private:
    FILE* fp;

    Status file_init();

    Status file_create(std::string const& filename);

    template <typename T>
    Status rwcallback(pagenum_t pagenum, T&& func) const {
        Page page;
        CHECK_SUCCESS(page_read(pagenum, &page));
        CHECK_SUCCESS(func(&page));
        CHECK_SUCCESS(page_write(pagenum, &page));
        return Status::SUCCESS;
    }
};

#endif