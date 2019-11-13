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
    /// Default constructor, init with block idx -1, null buffer manager.
    Buffer();

    /// Default destructor, call release.
    ~Buffer();

    /// Deleted copy constructor.
    Buffer(Buffer const&) = delete;

    /// Deleted move constructor.
    Buffer(Buffer&&) = delete;

    /// Deleted copy assignment.
    Buffer& operator=(Buffer const&) = delete;

    /// Deleted move assignment.
    Buffer& operator=(Buffer&&) = delete;

    /// Get page frame.
    /// \return Page&, page frame.
    Page& page();

    /// Start to use buffer.
    /// \param flag RWFlag, read write flag.
    /// \return Status, whether success or not.
    Status start_use(RWFlag flag);

    /// Finish to use buffer.
    /// \param flag RWFlag, read write flag.
    /// \return Status, whether success or not.
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

    /// Start to read buffer.
    /// \return Status, whether success or not.
    Status start_read();

    /// Start to write buffer.
    /// \return Status, whether success or not.
    Status start_write();

    /// Finish to read buffer.
    /// \return Status, whether success or not.
    Status end_read();
    
    /// Finish to write buffer.
    /// \return Status, whether success or not.
    Status end_write();

    /// Initialize buffer with given block index and manager.
    /// \param block_idx int, the index of the block in manager.
    /// \param manager BufferManager*, buffer manager.
    /// \return Status, whether success or not.
    Status init(int block_idx, BufferManager* manager);

    /// Load page frame from file manager.
    /// \param file FileManager&, file manager.
    /// \param pagenum pagenum_t, page ID.
    /// \return Status, whether success or not.
    Status load(FileManager& file, pagenum_t pagenum);

    /// Create new page frame from file manager.
    /// \param file FileManager&, file manager.
    /// \return Status, whether success or not.
    Status new_page(FileManager& file);

    /// Link neighbor blocks as prev_use and next_use.
    /// \return Status, whether success or not.
    Status link_neighbor();

    /// Append block to MRU.
    /// \param link bool, link neighbors or not.
    /// \return Status, whether success or not.
    Status append_mru(bool link);

    /// Release page frame.
    /// \return Status, whether success or not.
    Status release();

#ifdef TEST_MODULE
    friend struct BufferTest;

    friend struct BufferManagerTest;
#endif
};

/// Buffer for user povision.
class Ubuffer {
public:
    /// Construct ubuffer.
    /// \param buf Buffer*, target buffer.
    Ubuffer(Buffer& buf);

    /// Construct null buffer.
    /// \param nullptr, null pointer.
    Ubuffer(std::nullptr_t);

    /// Default destructor.
    ~Ubuffer() = default;

    /// Deleted copy constructor.
    Ubuffer(Ubuffer const& ubuffer) = delete;

    /// Move constructor.
    Ubuffer(Ubuffer&& ubuffer) noexcept;

    /// Deleted move assignment.
    Ubuffer& operator=(Ubuffer const& ubuffer) = delete;

    /// Move assignment.
    Ubuffer& operator=(Ubuffer&& ubuffer) noexcept;

    /// Get buffer pointer.
    /// \return Buffer*, buffer pointer.
    Buffer* buffer();

    /// Get page frame.
    /// \return Page&, page frame.
    Page& page();

    /// Reload buffer.
    /// \return Status, whether success or not.
    Status reload();

    /// Check buffer consistentcy.
    /// \return Status, whether success or not.
    Status check();

    /// Get page ID safely.
    /// \return pagenum_t, page ID.
    pagenum_t safe_pagenum();

    /// Use buffer frame safely.
    /// \param flag RWFlag, read write flag.
    /// \param callback void(Page&), callback.
    /// \return Status, whether success or not.
    template <typename F>
    inline Status use(RWFlag flag, F&& callback) {
        CHECK_SUCCESS(check());
        CHECK_SUCCESS(buf->start_use(flag));
        Status res = callback(page());
        CHECK_SUCCESS(buf->end_use(flag));
        return res;
    }

private:
    Buffer* buf;                /// buffer pointer.
    pagenum_t pagenum;          /// page ID for buffer validation.
    FileManager* file;          /// file pointer for buffer validation.

    /// Construct ubuffer with specified buffer infos.
    /// \param buf Buffer*, target buffer.
    /// \param pagenum pagenum_t, page ID.
    /// \param file FileManager*, file manager.
    Ubuffer(Buffer* buf, pagenum_t pagenum, FileManager* file);

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
    /// Consturct buffer manager with pool size.
    BufferManager(int num_buffer);

    /// Destructor.
    ~BufferManager();

    /// Deleted copy constructor.
    BufferManager(BufferManager const&) = delete;

    /// Deleted move constructor.
    BufferManager(BufferManager&&) noexcept;

    /// Deleted copy assignment.
    BufferManager& operator=(BufferManager const&) = delete;

    /// Deleted move assignment.
    BufferManager& operator=(BufferManager&&) noexcept;

    /// Shutdown manager.
    /// \return Status, whether success or not.
    Status shutdown();

    /// Return requested buffer.
    /// \param file FileManager&, file manager.
    /// \param pagenum pagenum_t, page ID.
    /// \return Ubuffer, buffer for user provision.
    Ubuffer buffering(FileManager& file, pagenum_t pagenum);

    /// Create page with given file manager.
    /// \param file FileManager&, file manager.
    /// \return Ubuffer, buffer for user provision.
    Ubuffer new_page(FileManager& file);

    /// Release page from file manager.
    /// \param file FileManager&, file manager.
    /// \param pagenum pagenum_t, page ID.
    /// \return Status, whether success or not.
    Status free_page(FileManager& file, pagenum_t pagenum);

    /// Release all buffer blocks relative to given file id.
    /// \param fileid fileid_t, file ID.
    /// \return Status, whether success or not.
    Status release_file(fileid_t fileid);

private:
    int capacity;                           /// buffer array size.
    int num_buffer;                         /// number of the element.
    int lru;                                /// least recently used block index.
    int mru;                                /// most recently used block index.
    std::unique_ptr<Buffer[]> buffers;      /// buffer array.

    friend class ReleaseLRU;

    friend class ReleaseMRU;

    friend class Buffer;

    /// Allocate buffer frame.
    /// \return int, index value.
    int alloc();

    /// Load page frame to buffer arrays.
    /// \param file FileManager&, file manager.
    /// \param pagenum pagenum_t, page ID.
    /// \return int, buffer index.
    int load(FileManager& file, pagenum_t pagenum);

    /// Release buffer block.
    /// \param idx int, buffer index.
    /// \return Status, whether sucess or not.
    Status release_block(int idx);

    /// Release buffer with given policy.
    /// \return int, released index.
    int release(ReleasePolicy const& policy);

    /// Find buffers with given file ID and page ID.
    /// \param fileid fileid_t, file ID.
    /// \param pagenum pagenum_t, page ID.
    /// \return int, found index.
    int find(fileid_t fileid, pagenum_t pagenum);

#ifdef TEST_MODULE
    friend struct BufferManagerTest;

    friend struct BufferTest;
#endif
};

/// Release least recently used buffer.
struct ReleaseLRU : ReleasePolicy {
    /// Initialize searching state.
    /// \return int, index of lru block.
    int init(BufferManager const& manager) const override {
        return manager.lru;
    }
    /// Next searhing state.
    /// \return int, next usage.
    int next(Buffer const& buffer) const override {
        return buffer.next_use;
    }
    /// Get instance.
    static ReleaseLRU& inst() {
        static ReleaseLRU lru;
        return lru;
    }
};

/// Release most recenly used buffer.
struct ReleaseMRU : ReleasePolicy {
    /// Initialize searching state.
    /// \return int, idnex of mru block.
    int init(BufferManager const& manager) const override {
        return manager.mru;
    }
    /// Next searching state.
    /// \return int, previous usage.
    int next(Buffer const& buffer) const override {
        return buffer.prev_use;
    }
    /// Get instance.
    static ReleaseMRU& inst() {
        static ReleaseMRU mru;
        return mru;
    }
};

#endif