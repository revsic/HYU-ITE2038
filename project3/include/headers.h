#ifndef HEADERS_H
#define HEADERS_H

#include <stdint.h>
#include <stdio.h>

/// GLOBAL CONSTANTS

/// Return value for successful execution.
#define SUCCESS 0

/// Return value for failure execution.
#define FAILURE 1

/// Size of page.
#define PAGE_SIZE 4096

/// Value for pointing null page.
#define INVALID_PAGENUM 0

/// Value for header page number.
#define FILE_HEADER_PAGENUM 0

/// Value for invalid table id.
#define INVALID_TABLENUM -1

// Value for invalid file id.
#define INVALID_FILENUM -1

/// True value.
#define TRUE 1

/// False value.
#define FALSE 0


/// VALIDATION MACRO

/// Return FAILURE if given is false or 0.
#define CHECK_TRUE(x) if (!(x)) { return FAILURE; }

/// Return FAILURE if given is NULL.
#define CHECK_NULL(x) if ((x) == NULL) { return FAILURE; }

/// Return FAILURE if given is not SUCCESS.
#define CHECK_SUCCESS(x) if ((x) != SUCCESS) { return FAILURE; }

/// Exit process if given is FALSE.
#define EXIT_ON_FALSE(x) if ((x) == FALSE) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(FAILURE); }

/// Exit process if given is NULL.
#define EXIT_ON_NULL(x) if ((x) == NULL) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(FAILURE); }

/// Exit process if given is FAILURE.
#define EXIT_ON_FAILURE(x) if ((x) == FAILURE) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(FAILURE); }

/// Debug variable
#define DBG(x) printf("line %d, " #x ": %d\n", __LINE__, x);


/// TYPE DEFINITION

/// Type for page ID.
typedef uint64_t pagenum_t;

/// Type for primary key.
typedef int64_t prikey_t;

/// Type for file ID.
typedef int32_t filenum_t;

/// Type for table ID.
typedef int32_t tablenum_t;

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
struct page_t {
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
