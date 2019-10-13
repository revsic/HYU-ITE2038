#ifndef HEADERS_H
#define HEADERS_H

#include <stdint.h>
#include <stdio.h>

// GLOBAL CONSTANTS
#define SUCCESS 0
#define FAILURE 1

#define PAGE_SIZE 4096

// VALIDATION MACRO
#define CHECK_TRUE(x) if (!x) { return FAILURE; }
#define CHECK_SUCCESS(x) if (x != SUCCESS) { return FAILURE; }
#define EXIT_ON_FAILURE(x) if (x == FAILURE) { printf("check failure: file %s, line %d\n", __FILE__, __LINE__); exit(FAILURE); }

// TYPE DEFINITION
typedef uint64_t pagenum_t;

typedef int64_t prikey_t;

// file header structure.
struct file_header_t {
    pagenum_t free_page_number;     // 8
    pagenum_t root_page_number;     // 16
    uint64_t number_of_pages;       // 24
};

// pad to page size.
struct padded_file_header_t {
    struct file_header_t header;
    uint8_t not_used[PAGE_SIZE - sizeof(struct file_header_t)];
};

// page header structure
struct page_header_t {
    pagenum_t parent_page_number;   // 8
    uint32_t is_leaf;               // 12
    uint32_t number_of_keys;        // 16
    uint8_t reserved[104];          // 120
    pagenum_t special_page_number;  // 128
};

// free page structure.
struct free_page_t {
    pagenum_t next_page_number;     // 8
};

// pad to 128B, same as page header.
struct padded_free_page_t {
    struct free_page_t header;
    uint8_t not_used[128 - sizeof(struct free_page_t)];
};

// record structure for leaf page.
struct record_t {
    prikey_t key;       // 8
    uint8_t value[120]; // 128
};

// entry structure for internal page.
struct internal_t {
    prikey_t key;       // 8
    pagenum_t pagenum;  // 16
};

// page structure.
struct page_t {
    union {
        struct page_header_t page_header;
        struct padded_free_page_t free_page;
    } header;                       // 128

    union {
        struct record_t records[31];
        struct internal_t entries[248];
    } content;                      // 4096
};

#endif
