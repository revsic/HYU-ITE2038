#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP

#include <cstdio>
#include <cstdint>

#include <string>

using filenum_t = int32_t;

constexpr filenum_t INVALID_FILENUM = -1;

class file_manager_t {
public:
    file_manager_t();

    file_manager_t(std::string const& filename);

    ~file_manager_t();

private:
    FILE* fp;
    filenum_t id;
};

#endif