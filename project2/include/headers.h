#ifndef HEADERS_H
#define HEADERS_H

#include <stdint.h>

#define PAGE_SIZE 4096

typedef uint64_t pagenum_t;

struct file_header_t {
    pagenum_t free_page_number;     // 8
    pagenum_t root_page_number;     // 16
    uint64_t number_of_pages;       // 24
};

struct padded_file_header_t {
    file_header_t header;
    uint8_t not_used[PAGE_SIZE - sizeof(file_header_t)];
};

struct page_header_t {
    pagenum_t parent_page_number;   // 8
    uint32_t is_leaf;               // 12
    uint32_t number_of_keys;        // 16
    uint8_t reserved[104];          // 120
    pagenum_t special_page_number;  // 128
};

struct free_page_t {
    pagenum_t next_page_number;     // 8
    uint8_t not_used[120];          // 128
};

struct record_t {
    uint64_t key;       // 8
    uint8_t value[120]; // 128
};

struct internal_t {
    uint64_t key;       // 8
    pagenum_t pagenum;  // 16
};

struct page_t {
    union {
        page_header_t page_header;
        free_page_t free_page;
    } header;                       // 128
    
    union {
        struct record_t records[31];
        struct internal_t entries[248];
    } content;                      // 4096
};

#endif