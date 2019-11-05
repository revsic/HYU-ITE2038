#ifndef HEADERS_HPP
#define HEADERS_HPP

#include <cstddef>
#include <cstdint>

constexpr size_t PAGE_SIZE = 4096;

/// Type for page ID.
using pagenum_t = uint64_t;

/// Type for primary key.
using prikey_t = int64_t;

/// Either Success or Failure
enum class result_t { SUCCESS = 0, FAILURE = 1 };

/// File header.
struct file_header_t {
    pagenum_t free_page_number;     /// 0~8, ID of first free page.
    pagenum_t root_page_number;     /// 8~16, ID of root page.
    uint64_t number_of_pages;       /// 16~24, the number of total pages.
};

/// File header padded with page size.
struct padded_file_header_t {
    struct file_header_t header;    /// 0~24, file header.
    uint8_t not_used[PAGE_SIZE - sizeof(struct file_header_t)]; /// 24~4096, padding.
};

/// Page header
struct page_header_t {
    pagenum_t parent_page_number;   /// 0~8, ID of parent page.
    uint32_t is_leaf;               /// 8~12, bool, whether leaf page or not.
    uint32_t number_of_keys;        /// 12~16, the number of keys.
    uint8_t reserved[104];          /// 16~120, reserved space.
    pagenum_t special_page_number;  /// 120~128, sibling pointer for leaf page, leftmost page ID for internal page.
};

/// Free page header.
struct free_page_t {
    pagenum_t next_page_number;     /// 0~8, ID of next free page.
};

/// Free page header padded with to 128 bytes, same as page header.
struct padded_free_page_t {
    struct free_page_t header;      /// 0~8, free page header.
    uint8_t not_used[128 - sizeof(struct free_page_t)]; /// 120, padding.
};

/// Record structure for leaf page.
struct record_t {
    prikey_t key;       /// 0~8, key for given record.
    uint8_t value[120]; /// 8~128, value for given record.
};

/// Entry structure for internal page.
struct internal_t {
    prikey_t key;       /// 0~8, key for given entry.
    pagenum_t pagenum;  /// 8~16, child page ID.
};

/// Page structure.
class page_t {
public:
    /// Get file header.
    /// \return file_header_t*, file header.
    file_header_t* file_header();

    /// Get file header.
    /// \return const file_header_t*, file header.
    const file_header_t* file_header() const;

    /// Get page header.
    /// \return page_header_t*, page header.
    page_header_t* page_header();

    /// Get page header.
    /// \return const page_header_t*, page header.
    const page_header_t* page_header() const;

    /// Get free page header.
    /// \return free_page_t*, free page header.
    free_page_t* free_page();

    /// Get free page header.
    /// \return const free_page_t*, free page header.
    const free_page_t* free_page() const;

    /// Get records from leaf node.
    /// \return record_t*, record array.
    record_t* records();

    /// Get records from leaf node.
    /// \return const record_t*, record array.
    const record_t* records() const;

    /// Get entries from internal node.
    /// \return internal_t*, entry array.
    internal_t* entries();

    /// Get entries from internal node.
    /// \return const internal_t*, entry array.
    const internal_t* entries() const;

private:
    union {
        struct {
            union {
                struct page_header_t page_header;
                struct padded_free_page_t free_page;
            } header;                       /// 0~128, page or free page header.

            union {
                struct record_t records[31];
                struct internal_t entries[248];
            } content;                      /// 128~4096, contents.
        } node;

        struct padded_file_header_t file;
    } impl;
};

#endif