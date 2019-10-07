#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include "headers.h"

pagenum_t file_alloc_page();

void file_free_page(pagenum_t pagenum);

void file_read_page(pagenum_t pagenum, struct page_t* dest);

void file_write_page(pagenum_t pagenum, const struct page_t* src);

#endif
