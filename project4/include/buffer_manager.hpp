#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include <memory>

#include "disk_manager.hpp"
#include "headers.hpp"
#include "status.hpp"

class BufferManager;

enum class RWFlag {
    READ = 0,
    WRITE = 1,
};

struct ReleasePolicy {
    virtual int init(BufferManager* manager) = 0;
    virtual int next(Buffer* buffer) = 0;
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

class Buffer {
public:
    Buffer();

    ~Buffer();

    Buffer(Buffer const&) = delete;

    Buffer(Buffer&&) = delete;

    Buffer& operator=(Buffer const&) = delete;

    Buffer& operator=(Buffer&&) = delete;

    Page& page();

    Status start_use(RWFlag flag);

    Status end_use(RWFlag flag);

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

    Status init(int block_idx, BufferManager* manager);

    Status load(FileManager* file, pagenum_t pagenum);

    Status new_page(FileManager* file);

    Status link_neighbor();

    Status append_mru(bool link);

    Status release();
};

class Ubuffer {
public:
    Ubuffer(Buffer* buf, pagenum_t pagenum, FileManager* file);

    ~Ubuffer() = default;

    Ubuffer(Ubuffer const& ubuffer);

    Ubuffer(Ubuffer&& ubuffer);

    Ubuffer& operator=(Ubuffer const& ubuffer);

    Ubuffer& operator=(Ubuffer&& ubuffer);

    Page& page();

    Status reload();

    Status check();

    pagenum_t safe_pagenum();

private:
    Buffer* buf;
    pagenum_t pagenum;
    FileManager* file;
};

class BufferManager {
public:
    BufferManager();

    ~BufferManager();

    BufferManager(BufferManager const&) = delete;

    BufferManager(BufferManager&&) = delete;

    BufferManager& operator=(BufferManager const&) = delete;

    BufferManager& operator=(BufferManager&&) = delete;

    Status shutdown();

    Ubuffer buffering(FileManager* file, pagenum_t pagenum);

    Ubuffer new_page(FileManager* file);

    Status free_page(FileManager* file, pagenum_t pagenum);

private:
    int capacity;
    int num_buffer;
    int lru;
    int mru;
    std::unique_ptr<Buffer> buffers;

    Status init(int num_buffer);

    Status alloc();

    int load(FileManager* file, pagenum_t pagenum);

    int release_block(int idx);

    int release_file(filenum_t fileid);

    int release(ReleasePolicy const& policy);

    int find(filenum_t fileid, pagenum_t pagenum);
};

#endif