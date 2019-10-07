#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdint.h>

typedef uint64_t pagenum_t;

struct keyvalue_t {
    uint64_t key;       // 8
    uint8_t value[120]; // 128
};

struct page_t {
    pagenum_t parent_page_number;   // 8
    uint32_t is_leaf;               // 12
    uint32_t number_of_keys;        // 16
    uint8_t reserved[104];          // 120
    pagenum_t special_page_number;  // 128
    struct keyvalue_t pairs[31];    // 4096
};

pagenum_t file_alloc_page();

void file_free_page(pagenum_t pagenum);

void file_read_page(pagenum_t pagenum, struct page_t* dest);

void file_write_page(pagenum_t pagenum, const struct page_t* src);

#endif
