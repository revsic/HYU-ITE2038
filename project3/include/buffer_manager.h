#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "headers.h"
#include "disk_manager.h"

// Macros

/// Buffer block for save block access, buffer validation, pre, post process macro.
#define BUFFER(var, flag, cont) {           \
    EXIT_ON_FAILURE(ubuffer_check(&(var))); \
    buffer_start((var).buf, flag);          \
    cont;                                   \
    buffer_end((var).buf, flag);            \
}

/// Intercept buffer block with given action.
#define BUFFER_INTERCEPT(var, flag, action) buffer_end((var).buf, flag); action;

/// Check whether given value is true or not in buffer block.
#define BUFFER_CHECK_TRUE(var, flag, x) if (!(x)) BUFFER_INTERCEPT((var), flag, return FAILURE);

/// Check whether given value is not null in buffer block.
#define BUFFER_CHECK_NULL(var, flag, x) if ((x) == NULL) BUFFER_INTERCEPT((var), flag, return FAILURE);

/// CHeck whether given value is SUCCESS or not in buffer block.
#define BUFFER_CHECK_SUCCESS(var, flag, x) if ((x) != SUCCESS) BUFFER_INTERCEPT((var), flag, return FAILURE);


// Type definition

/// Buffer structure.
struct buffer_t {
    struct page_t frame;                /// page frame.
    pagenum_t pagenum;                  /// page ID.
    uint32_t is_dirty;                  /// whether any values are written in page frame.
    uint32_t pin;                       /// whether block is used now.
    int prev_use;                       /// previous used block, for page replacement policy.
    int next_use;                       /// next used block, for page replacement policy.
    int block_idx;                      /// index of the block in buffer manager.
    struct file_manager_t* file;        /// file pointer which current page exist.
    struct buffer_manager_t* manager;   /// buffer manager which current buffer exist.
};

/// Buffer for user provision.
struct ubuffer_t {
    struct buffer_t* buf;               /// buffer pointer.
    pagenum_t pagenum;                  /// page ID for buffer validation.
    struct file_manager_t* file;        /// file pointer for buffer validation.
};

/// Buffer manager.
struct buffer_manager_t {
    int capacity;                       /// buffer array capacity.
    int num_buffer;                     /// number of allocated buffer.
    int lru;                            /// least recently used block index.
    int mru;                            /// most recently used block index.
    struct buffer_t *buffers;           /// buffer array.
};

/// Page replacement policy.
struct release_policy_t {
    int(*initial_search)(struct buffer_manager_t* manager);     /// initial searching state.
    int(*next_search)(struct buffer_t* buffer);                 /// next buffer index.
};

/// Read, write flag for buffer handling.
enum RW_FLAG {
    READ_FLAG = 0,          /// read buffer
    WRITE_FLAG = 1          /// write buffer
};


// Global variables

/// LRU policy.
extern const struct release_policy_t RELEASE_LRU;

/// MRU policy.
extern const struct release_policy_t RELEASE_MRU;


// Procedure definition.

/// Reload ubuffer.
/// \param buffer struct ubuffer_t*, buffer for user provision.
/// \return int, whether success or not.
int ubuffer_reload(struct ubuffer_t* buffer);

/// Validate ubuffer consistency, if invalid, reload buffer based on metadata.
/// \param buffer struct ubuffer_t*, buffer for user provision.
/// \return int, whether usccess or not.
int ubuffer_check(struct ubuffer_t* buffer);

/// Get page ID from ubuffer with buffer validation.
/// \param buffer struct ubuffer_t*, buffer for user provision.
/// \return pagenum_t, page ID.
pagenum_t ubuffer_pagenum(struct ubuffer_t* buffer);

/// Get page frame pointer for buffer.
/// \param buffer struct buffer_t*, buffer.
/// \return struct page_t*, page frame pointer.
struct page_t* from_buffer(struct buffer_t* buffer);

/// Get page frame pointer for ubuffer.
/// \param buffer struct ubuffer_t*, buffer for user provision.
/// \return struct page_t*, page frame pointer.
struct page_t* from_ubuffer(struct ubuffer_t* buffer);

/// Initialize buffer with default value.
/// \param buffer struct buffer_t*, buffer.
/// \param block_idx int, block index from buffer manager.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \return whether success or not.
int buffer_init(struct buffer_t* buffer,
                int block_idx,
                struct buffer_manager_t* manager);

/// Load buffer from file with page ID.
/// \param buffer struct buffer_t*, buffer.
/// \param file struct file_manager_t*, file manager.
/// \param pagenum pagenum_t, page ID.
/// \return whether success or not.
int buffer_load(struct buffer_t* buffer,
                struct file_manager_t* file,
                pagenum_t pagenum);

/// Allocate new page on given file and load to buffer.
/// \param buffer struct buffer_t*, buffer.
/// \param file struct file_manager_t*, file manager.
/// \return whether success or not.
int buffer_new_page(struct buffer_t* buffer, struct file_manager_t* file);

/// Connect previous and next used buffer as linked list.
/// \param buffer struct buffer_t*, buffer.
/// \return whether success or not.
int buffer_link_neighbor(struct buffer_t* buffer);

/// Append current buffer to list as most recently used buffer.
/// \param buffer struct buffer_t*, buffer.
/// \param link boolean, whether link neighbor or not.
/// \return whether success or not.
int buffer_append_mru(struct buffer_t* buffer, int link);

/// Release page frame from buffer.
/// \param buffer struct buffer_t*, buffer.
/// \return whether success or not.
int buffer_release(struct buffer_t* buffer);

/// Start to read or write buffer.
/// \param buffer struct buffer_t*, buffer.
/// \param rw_floag enum RW_FLAG, whether read mode or write mode.
/// \return whether success or not.
int buffer_start(struct buffer_t* buffer, enum RW_FLAG rw_flag);

/// End reading or writing buffer.
/// \param buffer struct buffer_t*, buffer.
/// \param rw_floag enum RW_FLAG, whether read mode or write mode.
/// \return whether success or not.
int buffer_end(struct buffer_t* buffer, enum RW_FLAG rw_flag);

/// Initialize buffer manager.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param num_buffer int, the number of buffer.
/// \return whether success or not.
int buffer_manager_init(struct buffer_manager_t* manager, int num_buffer);

/// Shutdown buffer manager.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \return whether success or not.
int buffer_manager_shutdown(struct buffer_manager_t* manager);

/// Allocate buffer, only allocation not initialization.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \return int, index of allocated buffer.
int buffer_manager_alloc(struct buffer_manager_t* manager);

/// Load page frame from file with pagenum.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param file struct file_manager_t*, file manager.
/// \param pagenum pagenum_t, page ID.
/// \return int, index of allocated buffer.
int buffer_manager_load(struct buffer_manager_t* manager,
                        struct file_manager_t* file,
                        pagenum_t pagenum);

/// Release page frame and clean buffer.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param idx, index of buffer.
/// \return whether success or not.
int buffer_manager_release_block(struct buffer_manager_t* manager, int idx);

/// Release all buffers related to given file ID.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param file_id filenum_t, file ID.
/// \return whether success or not.
int buffer_manager_release_file(struct buffer_manager_t* manager,
                                filenum_t file_id);

/// Release page frame with page replacement policy.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param policy const struct release_policy_t*, page replacement policy.
/// \return int, index of relaased buffer.
int buffer_manager_release(struct buffer_manager_t* manager,
                           const struct release_policy_t* policy);

/// Find buffer with given file, page ID.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param file_id filenum_t, file ID.
/// \param pagenum pagenum_t, page ID.
/// \return whether success or not.
int buffer_manager_find(struct buffer_manager_t* manager,
                        filenum_t file_id,
                        pagenum_t pagenum);

/// Get buffer from manager.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param file struct file_manager_t*, file.
/// \return struct ubuffer_t, buffer for user provision.
struct ubuffer_t buffer_manager_buffering(struct buffer_manager_t* manager,
                                          struct file_manager_t* file,
                                          pagenum_t pagenum);

/// Allocate new page from given file.
/// \param mamnager struct buffer_manager_t*, buffer manager.
/// \param file struct file_manager_t*, file manager.
/// \return struct ubuffer_t, buffer for user provision.
struct ubuffer_t buffer_manager_new_page(struct buffer_manager_t* manager,
                                         struct file_manager_t* file);

/// Free page frame from buffer manager.
/// \param manager struct buffer_manager_t*, buffer manager.
/// \param file struct file_manager_t*, file manager.
/// \param pagenum pagenum_t, target page ID.
/// \return int, whether success or not. 
int buffer_manager_free_page(struct buffer_manager_t* manager,
                             struct file_manager_t* file,
                             pagenum_t pagenum);

#endif