#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>

#include "headers.h"

// GLOBAL CONSTANTS

/// Value for pointing null page.
#define INVALID_PAGENUM 0


// TYPE DEFINITION

/// File level disk manager.
struct file_manager_t {
    FILE* fp;                           /// file pointer.
    int updated;                        /// whether current file header cache is updated or not.
    struct file_header_t file_header;   /// file header cache.
};


// PROCEDURE DEFINITION

/// Initialize file manager.
/// \param manager struct file_manager_t* file manager.
/// \return whether initializing is successful (= SUCCESS) or not (= FAILURE).
int file_init(struct file_manager_t* manager);

int file_create(char* filename, struct file_manager_t* manager);

int file_open(char* filename, struct file_manager_t* manager);

int file_close(struct file_manager_t* manager);

int file_write_header(struct file_manager_t* manager);

int file_write_update(struct file_manager_t* manager);

pagenum_t last_pagenum(struct file_manager_t* manager);

pagenum_t last_pagenum_from_size(long size);

pagenum_t page_create(struct file_manager_t* manager);

int page_init(struct page_t* page, uint32_t leaf);

int page_extend_free(struct file_manager_t* manager, int num);

int page_free(pagenum_t pagenum, struct file_manager_t* manager);

int page_read(pagenum_t pagenum,
              struct file_manager_t* manager,
              struct page_t* dst);

int page_write(pagenum_t pagenum,
               struct file_manager_t* manager,
               struct page_t* src);

#endif
