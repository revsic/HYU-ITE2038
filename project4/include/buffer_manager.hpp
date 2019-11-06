#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include <memory>

#include "disk_manager.hpp"
#include "headers.hpp"

class BufferManager;

class Buffer {
public:


private:
    Page frame;
    pagenum_t pagenum;
    bool is_dirty;
    int pin;
    int prev_use;
    int next_use;
    int block_idx;
    FileManager* file;
    BufferManager* manager;
};

class Ubuffer {
public:

private:
    Buffer* buf;
    pagenum_t pagenum;
    FileManager* file;
};

class BufferManager {
public:
private:
    int capacity;
    int num_buffer;
    int lru;
    int mru;
    std::unique_ptr<Buffer> buffers;
};

struct ReleasePolicy {
    int initial_search(BufferManager* manager)
};

#endif