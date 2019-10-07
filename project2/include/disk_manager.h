#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>

#include "headers.h"

struct file_manager_t {
    FILE* fp;
    file_header_t file_header;
};

int create_file(struct file_manager_t* manager);

int open_file(char* filename, struct file_manager_t* manager);

int close_file(struct file_manager_t* manager);

pagenum_t create_page(struct file_manager_t* manager);

int free_page(pagenum_t pagenum, struct file_manager_t* manager);

int read_page(pagenum_t pagenum, struct file_manager_t* manager, struct page_t* dst);

int write_page(pagenum_t pagenum, struct file_manager_t* manager, struct page_t* src);

#endif
