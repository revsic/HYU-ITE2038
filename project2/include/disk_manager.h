#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdint.h>

typedef uint64_t pagenum_t;

struct keyvalue_t {
    uint64_t key;
    uint8_t value[120];
};

struct page_t {
    pagenum_t parent_page_number;
    uint32_t is_leaf;
    uint32_t number_of_keys;
    uint32_t flags;
    uint8_t reserved[104];
    pagenum_t special_page_number;
    struct keyvalue_t pairs[31];
};

pagenum_t file_alloc_page();

void file_free_page(pagenum_t pagenum);

void file_read_page(pagenum_t pagenum, struct page_t* dest);

void file_write_page(pagenum_t pagenum, const struct page_t* src);

#endif
