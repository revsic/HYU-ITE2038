#ifndef HEADERS_HPP
#define HEADERS_HPP

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "fileio.hpp"
#include "status.hpp"

/// Size of the page.
constexpr size_t PAGE_SIZE = 4096;

/// Type for page ID.
using pagenum_t = uint64_t;

/// Value for pointing null page.
constexpr pagenum_t INVALID_PAGENUM = 0;

/// Value for header page number.
constexpr pagenum_t FILE_HEADER_PAGENUM = 0;

/// Type for primary key.
using prikey_t = int64_t;

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
    /// Release page.
    /// \param T callback type, status_t(pagenum_t, status_t(page_t*)).
    /// \param page_proc T, callback for writing other relative pages
    /// with pagenum.
    /// \param self pagenum_t, self pagenumber.
    /// \return status_t, whether success or not.
    template <typename T>
    static status_t release(T&& page_proc, pagenum_t self) {
        pagenum_t last_free;
        CHECK_SUCCESS(page_proc(
            FILE_HEADER_PAGENUM,
            [&last_free, self](page_t* page) {
                file_header_t* filehdr = page->file_header();
                last_free = filehdr->free_page_number;
                filehdr->free_page_number = self;
                return status_t::SUCCESS;
            }
        ));

        CHECK_SUCCESS(page_proc(self, [last_free](page_t* page) {
            page->free_page()->next_page_number = last_free;
            return status_t::SUCCESS;
        }));

        return status_t::SUCCESS;
    }

    /// Create page.
    /// \param T callback type, status_t(pagenum_t, status_t(page_t*)).
    /// \param page_proc T, callback for writing other relative pages.
    /// with pagenum.
    /// \param fp FILE*, file pointer for extending free pages if there is no more free pages.
    /// \return pagenum_t, created page number.
    template <typename T>
    static pagenum_t create(T&& page_proc, FILE* fp) {
        status_t res;
        res = page_proc(FILE_HEADER_PAGENUM, [fp, &page_proc](page_t* page) {
            file_header_t* filehdr = page->file_header();
            if (filehdr->free_page_number == 0) {
                CHECK_SUCCESS(extend_free(
                    page_proc, fp,
                    std::max(1ULL, filehdr->number_of_pages)));
            }
        });

        if (res == status_t::FAILURE) {
            return INVALID_PAGENUM;
        }

        pagenum_t freepage;
        res = page_proc(FILE_HEADER_PAGENUM, [&freepage, &page_proc](page_t* page) {
            file_header_t* filehdr = page->file_header();
            freepage = filehdr->free_page_number;
            CHECK_SUCCESS(page_proc(freepage, [filehdr](page_t* freep) {
                filehdr->free_page_number = freep->free_page()->next_page_number;
            }));
        });

        if (res == status_t::FAILURE) {
            return INVALID_PAGENUM;
        }

        return freepage;
    }

    /// Extend free page list.
    /// \param T callback type, status_t(pagenum_t, status_t(page_t*)).
    /// \param page_proc T, callback for writing other relative pages
    /// with pagenum.
    /// \param fp FILE*, file pointer.
    /// \param num int, the number of requested free pages.
    /// \return status_t, whether success or failure.
    template <typename T>
    static status_t extend_free(T&& page_proc, FILE* fp, int num) {
        if (num < 1) {
            return status_t::FAILURE;
        }

        long size = fsize(fp);
        pagenum_t last = fsize(fp) / PAGE_SIZE - 1;
        CHECK_TRUE(fresize(fp, size + num * PAGE_SIZE));

        CHECK_SUCCESS(page_proc(FILE_HEADER_PAGENUM, [last, num, &page_proc](page_t* page) {
            file_header_t* header = page->file_header();
            pagenum_t prev = header->free_page_number;

            for (int i = 1; i <= num; ++i) {
                CHECK_SUCCESS(page_proc(last + i, [prev](page_t* page) {
                    page->free_page()->next_page_number = prev;
                }));
                prev = last + i;
            }

            header->free_page_number = last + num;
            header->number_of_pages += num;
        }));

        return status_t::SUCCESS;
    }

    /// Initialize page.
    /// \param leaf uint32_t, is leaf or not.
    /// \return status_t, whether success or not.
    status_t init(uint32_t leaf);

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

/// Assertion for page size 
static_assert(sizeof(page_t) == PAGE_SIZE, "the size of page_t is not PAGE_SIZE");

#endif