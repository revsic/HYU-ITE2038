#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdint.h>

typedef uint64_t pagenum_t;

struct pair_t {
    uint64_t key;       // 8
    uint8_t value[120]; // 128
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

pagenum_t file_alloc_page();

void file_free_page(pagenum_t pagenum);

void file_read_page(pagenum_t pagenum, struct page_t* dest);

void file_write_page(pagenum_t pagenum, const struct page_t* src);

#endif
