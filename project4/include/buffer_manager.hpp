#ifndef BUFFER_MANAGER_HPP
#define BUFFER_MANAGER_HPP

#include <memory>

#include "disk_manager.hpp"
#include "headers.hpp"
#include "status.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

/// Buffer manager.
class BufferManager;

/// Read, write flag for buffer handling.
enum class RWFlag {
    READ = 0,                   /// read buffer
    WRITE = 1,                  /// write buffer
};

/// Buffer structure.
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
    Page frame;                 /// page frame.
    pagenum_t pagenum;          /// page ID.
    bool is_dirty;              /// whether any values are written in this page frame.
    int pin;                    /// whether block is used now.
    int prev_use;               /// previous used block, for page replacement policy.
    int next_use;               /// next used block, for page replacement policy.
    int block_idx;              /// index of the block in buffer manager.
    FileManager* file;          /// file pointer which current page exist.
    BufferManager* manager;     /// buffer manager which current buffer exist.

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

    friend struct BufferManagerTest;
#endif
};

/// Buffer for user povision.
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
    Buffer* buf;                /// buffer pointer.
    pagenum_t pagenum;          /// page ID for buffer validation.
    FileManager* file;          /// file pointer for buffer validation.

#ifdef TEST_MODULE
    friend struct UbufferTest;

    friend struct BufferManagerTest;
#endif
};

/// Page replacement policy.
struct ReleasePolicy {
    /// Initial searching state.
    /// \param manager BufferManager const&, buffer manager.
    /// \return int, index of the buffer array.
    virtual int init(BufferManager const& manager) const = 0;

    /// Next buffer index.
    /// \param buffer Buffer const&, buffer.
    /// \return int, index of the buffer array.
    virtual int next(Buffer const& buffer) const = 0;
};

/// Buffer manager.
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
    int capacity;                           /// buffer array size.
    int num_buffer;                         /// number of the element.
    int lru;                                /// least recently used block index.
    int mru;                                /// most recently used block index.
    std::unique_ptr<Buffer[]> buffers;      /// buffer array.

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

struct ReleaseLRU : ReleasePolicy {
    int init(BufferManager const& manager) const override {
        return manager.lru;
    }

    int next(Buffer const& buffer) const override {
        return buffer.next_use;
    }
    static ReleaseLRU& inst() {
        static ReleaseLRU lru;
        return lru;
    }
};

struct ReleaseMRU : ReleasePolicy {
    int init(BufferManager const& manager) const override {
        return manager.mru;
    }
    int next(Buffer const& buffer) const override {
        return buffer.prev_use;
    }
    static ReleaseMRU& inst() {
        static ReleaseMRU mru;
        return mru;
    }
};

#endif