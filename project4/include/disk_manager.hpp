// #ifndef DISK_MANAGER_HPP
// #define DISK_MANAGER_HPP

// #include <cstdio>
// #include <cstdint>
// #include <string>

// #include "headers.hpp"
// #include "status.hpp"

// using filenum_t = int32_t;

// constexpr filenum_t INVALID_FILENUM = -1;

// class FileManager {
// public:
//     file_manager_t();

//     file_manager_t(std::string const& filename);

//     ~file_manager_t();

//     pagenum_t page_create() const;

//     status_t page_free(pagenum_t pagenum) const;

//     status_t page_read(pagenum_t pagenum, Page* dst) const;

//     status_t page_write(pagenum_t pagenum, Page* src) const;

// private:
//     FILE* fp;
//     filenum_t id;

//     status_t file_create(std::string const& filename);
// };

// #endif