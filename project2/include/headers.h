#ifndef HEADERS_H
#define HEADERS_H

#include <stdint.h>

typedef uint64_t pagenum_t;

struct pair_t {
    uint64_t key;       // 8
    uint8_t value[120]; // 128
};

struct file_header_t {
    pagenum_t free_page_number;     // 8
    pagenum_t root_page_number;     // 16
    uint64_t number_of_pages;       // 24
};

struct padded_file_header_t {
    file_header_t header;
    uint8_t not_used[4096 - sizeof(file_header_t)];
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

struct page_t {
    union {
        page_header_t page_header;
        free_page_t free_page;
    } header;                       // 128
    struct pair_t pairs[31];        // 4096
};

#endif