#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include <memory>

#include "disk_manager.hpp"
#include "headers.hpp"
#include "status.hpp"

class BufferManager;

class Buffer {
public:
    Status init(int block_idx, BufferManager* manager);

    Page& page();

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
    Status reload();

    Status check();

    pagenum_t safe_pagenum();

    Page& page();

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
    virtual int init(BufferManager* manager) = 0;
    virtual int next(Buffer* buffer) = 0;
};

enum class RWFlag {
    READ = 0,
    WRITE = 1,
};

struct ReleaseLRU : ReleasePolicy {
    int init(BufferManager* manager) override;
    int next(Buffer* buffer) override;

    static ReleaseLRU& inst() {
        static ReleaseLRU lru;
        return lru;
    }
};

struct ReleaseMRU : ReleasePolicy {
    int init(BufferManager* manager) override;
    int next(Buffer* buffer) override;

    static ReleaseMRU& inst() {
        static ReleaseMRU mru;
        return mru;
    }
};

#endif