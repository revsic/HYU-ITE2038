#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>

#include "headers.h"

#define MAX_FILE_PATH 512

struct file_manager_t {
    FILE* fp;
    int updated;
    struct file_header_t file_header;
};

int file_init(struct file_manager_t* manager);

int file_open(char* filename, struct file_manager_t* manager);

int file_close(struct file_manager_t* manager);

pagenum_t page_create(struct file_manager_t* manager);

int page_extend_free(struct file_manager_t* manager);

int page_free(pagenum_t pagenum, struct file_manager_t* manager);

int page_read(pagenum_t pagenum,
              struct file_manager_t* manager,
              struct page_t* dst);

int page_write(pagenum_t pagenum,
               struct file_manager_t* manager,
               struct page_t* src);

#endif
