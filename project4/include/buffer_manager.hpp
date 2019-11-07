#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include <memory>

#include "disk_manager.hpp"
#include "headers.hpp"
#include "status.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

class BufferManager;

enum class RWFlag {
    READ = 0,
    WRITE = 1,
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

    friend class ReleaseLRU;

    friend class ReleaseMRU;

    friend class Ubuffer;

    friend class BufferManager;

    Status start_read();

    Status start_write();

    Status end_read();
    
    Status end_write();

    Status init(int block_idx, BufferManager* manager);

    Status load(FileManager& file, pagenum_t pagenum);

    Status new_page(FileManager& file);

    Status link_neighbor();

    Status append_mru(bool link);

    Status release();

#ifdef TEST_MODULE
    friend struct BufferTest;
#endif
};

class Ubuffer {
public:
    Ubuffer(Buffer* buf, pagenum_t pagenum, FileManager* file);

    ~Ubuffer() = default;

    Ubuffer(Ubuffer const& ubuffer) = delete;

    Ubuffer(Ubuffer&& ubuffer);

    Ubuffer& operator=(Ubuffer const& ubuffer) = delete;

    Ubuffer& operator=(Ubuffer&& ubuffer);

    Page& page();

    Status reload();

    Status check();

    pagenum_t safe_pagenum();

    template <typename F>
    inline Status use(RWFlag flag, F&& callback) {
        EXIT_ON_FAILURE(check());
        buf->start_use(flag);
        callback(*this);
        buf->end_use(flag);
        return Status::SUCCESS;
    }

private:
    Buffer* buf;
    pagenum_t pagenum;
    FileManager* file;

#ifdef TEST_MODULE
    friend struct UbufferTest;
#endif
};

struct ReleasePolicy {
    virtual int init(BufferManager const& manager) const = 0;
    virtual int next(Buffer const& buffer) const = 0;
};

struct ReleaseLRU : ReleasePolicy {
    int init(BufferManager const& manager) const override;
    int next(Buffer const& buffer) const override;

    static ReleaseLRU& inst() {
        static ReleaseLRU lru;
        return lru;
    }
};

struct ReleaseMRU : ReleasePolicy {
    int init(BufferManager const& manager) const override;
    int next(Buffer const& buffer) const override;

    static ReleaseMRU& inst() {
        static ReleaseMRU mru;
        return mru;
    }
};

class BufferManager {
public:
    BufferManager(int num_buffer);

    ~BufferManager();

    BufferManager(BufferManager const&) = delete;

    BufferManager(BufferManager&&) = delete;

    BufferManager& operator=(BufferManager const&) = delete;

    BufferManager& operator=(BufferManager&&) = delete;

    Status shutdown();

    Ubuffer buffering(FileManager& file, pagenum_t pagenum);

    Ubuffer new_page(FileManager& file);

    Status free_page(FileManager& file, pagenum_t pagenum);

private:
    int capacity;
    int num_buffer;
    int lru;
    int mru;
    std::unique_ptr<Buffer[]> buffers;

    friend class ReleaseLRU;

    friend class ReleaseMRU;

    friend class Buffer;

    int alloc();

    int load(FileManager& file, pagenum_t pagenum);

    Status release_block(int idx);

    Status release_file(filenum_t fileid);

    int release(ReleasePolicy const& policy);

    int find(filenum_t fileid, pagenum_t pagenum);

#ifdef TEST_MODULE
    friend struct BufferManagerTest;

    friend struct BufferTest;
#endif
};

#endif