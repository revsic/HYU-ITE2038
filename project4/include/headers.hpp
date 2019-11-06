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
struct FileHeader {
    pagenum_t free_page_number;     /// 0~8, ID of first free page.
    pagenum_t root_page_number;     /// 8~16, ID of root page.
    uint64_t number_of_pages;       /// 16~24, the number of total pages.
};

/// File header padded with page size.
struct PaddedFileHeader {
    FileHeader header;    /// 0~24, file header.
    uint8_t not_used[PAGE_SIZE - sizeof(FileHeader)]; /// 24~4096, padding.
};

/// Page header
struct PageHeader {
    pagenum_t parent_page_number;   /// 0~8, ID of parent page.
    uint32_t is_leaf;               /// 8~12, bool, whether leaf page or not.
    uint32_t number_of_keys;        /// 12~16, the number of keys.
    uint8_t reserved[104];          /// 16~120, reserved space.
    pagenum_t special_page_number;  /// 120~128, sibling pointer for leaf page, leftmost page ID for internal page.
};

/// Free page header.
struct FreePageHeader {
    pagenum_t next_page_number;     /// 0~8, ID of next free page.
};

/// Free page header padded with to 128 bytes, same as page header.
struct PaddedFreePageHeader {
    FreePageHeader header;      /// 0~8, free page header.
    uint8_t not_used[128 - sizeof(struct FreePageHeader)]; /// 120, padding.
};

/// Record structure for leaf page.
struct Record {
    prikey_t key;       /// 0~8, key for given record.
    uint8_t value[120]; /// 8~128, value for given record.
};

/// Entry structure for internal page.
struct Internal {
    prikey_t key;       /// 0~8, key for given entry.
    pagenum_t pagenum;  /// 8~16, child page ID.
};

/// Page structure.
class Page {
public:
    /// Release page.
    /// \param T callback type, Status(pagenum_t, Status(Page&)).
    /// \param page_proc T, callback for writing other relative pages
    /// with pagenum.
    /// \param self pagenum_t, self pagenumber.
    /// \return Status, whether success or not.
    template <typename T>
    static Status release(T&& page_proc, pagenum_t self) {
        pagenum_t last_free;
        CHECK_SUCCESS(page_proc(
            FILE_HEADER_PAGENUM,
            [&last_free, self](Page& page) {
                FileHeader& filehdr = page.file_header();
                last_free = filehdr.free_page_number;
                filehdr.free_page_number = self;
                return Status::SUCCESS;
            }
        ));

        CHECK_SUCCESS(page_proc(self, [last_free](Page& page) {
            page.free_page().next_page_number = last_free;
            return Status::SUCCESS;
        }));

        return Status::SUCCESS;
    }

    /// Create page.
    /// \param T callback type, Status(pagenum_t, Status(Page&)).
    /// \param page_proc T, callback for writing other relative pages
    /// with pagenum.
    /// \param fp FILE*, file pointer for extending free pages if there is no more free pages.
    /// \return pagenum_t, created page number.
    template <typename T>
    static pagenum_t create(T&& page_proc, FILE* fp) {
        Status res;
        res = page_proc(FILE_HEADER_PAGENUM, [fp, &page_proc](Page& page) {
            FileHeader& filehdr = page.file_header();
            if (filehdr.free_page_number == 0) {
                CHECK_SUCCESS(extend_free(
                    page_proc, fp,
                    std::max(1ULL, filehdr.number_of_pages)));
            }
            return Status::SUCCESS;
        });

        if (res == Status::FAILURE) {
            return INVALID_PAGENUM;
        }

        pagenum_t freepage;
        res = page_proc(FILE_HEADER_PAGENUM, [&freepage, &page_proc](Page& page) {
            FileHeader& filehdr = page.file_header();
            freepage = filehdr.free_page_number;
            CHECK_SUCCESS(page_proc(freepage, [filehdr](Page& freep) {
                filehdr.free_page_number = freep.free_page().next_page_number;
                return Status::SUCCESS;
            }));
            return Status::SUCCESS;
        });

        if (res == Status::FAILURE) {
            return INVALID_PAGENUM;
        }

        return freepage;
    }

    /// Extend free page list.
    /// \param T callback type, Status(pagenum_t, Status(Page&)).
    /// \param page_proc T, callback for writing other relative pages
    /// with pagenum.
    /// \param fp FILE*, file pointer.
    /// \param num int, the number of requested free pages.
    /// \return Status, whether success or failure.
    template <typename T>
    static Status extend_free(T&& page_proc, FILE* fp, int num) {
        if (num < 1) {
            return Status::FAILURE;
        }

        long size = fsize(fp);
        pagenum_t last = fsize(fp) / PAGE_SIZE - 1;
        CHECK_TRUE(fresize(fp, size + num * PAGE_SIZE));

        CHECK_SUCCESS(page_proc(FILE_HEADER_PAGENUM, [last, num, &page_proc](Page& page) {
            FileHeader& header = page.file_header();
            pagenum_t prev = header.free_page_number;

            for (int i = 1; i <= num; ++i) {
                CHECK_SUCCESS(page_proc(last + i, [prev](Page& page) {
                    page.free_page().next_page_number = prev;
                    return Status::SUCCESS;
                }));
                prev = last + i;
            }

            header.free_page_number = last + num;
            header.number_of_pages += num;
            return Status::SUCCESS;
        }));

        return Status::SUCCESS;
    }

    /// Initialize page.
    /// \param leaf uint32_t, is leaf or not.
    /// \return Status, whether success or not.
    Status init(uint32_t leaf);

    /// Get file header.
    /// \return FileHeader&, file header.
    FileHeader& file_header();

    /// Get file header.
    /// \return FileHeader const&, file header.
    FileHeader const& file_header() const;

    /// Get page header.
    /// \return PageHeader&, page header.
    PageHeader& page_header();

    /// Get page header.
    /// \return PageHeader const&, page header.
    PageHeader const& page_header() const;

    /// Get free page header.
    /// \return FreePageHeader&, free page header.
    FreePageHeader& free_page();

    /// Get free page header.
    /// \return FreePageHeader const&, free page header.
    FreePageHeader const& free_page() const;

    /// Get records from leaf node.
    /// \return Record&, record array.
    Record& records();

    /// Get records from leaf node.
    /// \return Record const&, record array.
    Record const& records() const;

    /// Get entries from internal node.
    /// \return Internal&, entry array.
    Internal& entries();

    /// Get entries from internal node.
    /// \return Internal const&, entry array.
    Internal const& entries() const;

private:
    union {
        struct {
            union {
                struct PageHeader page_header;
                struct PaddedFreePageHeader free_page;
            } header;                       /// 0~128, page or free page header.

            union {
                struct Record records[31];
                struct Internal entries[248];
            } content;                      /// 128~4096, contents.
        } node;

        struct PaddedFileHeader file;
    } impl;
};

/// Assertion for page size 
static_assert(sizeof(Page) == PAGE_SIZE, "the size of Page is not PAGE_SIZE");

#endif