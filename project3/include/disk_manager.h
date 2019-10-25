#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdio.h>

#include "headers.h"

// TYPE DEFINITION

/// File level disk manager.
struct file_manager_t {
    FILE* fp;                           /// file pointer.
};


// PROCEDURE DEFINITION

/// Initialize file manager.
/// \param manager struct file_manager_t* file manager.
/// \return int, whether initializing is successful (= SUCCESS) or not (= FAILURE).
int file_init(struct file_manager_t* manager);

/// Create B+Tree based record system on file manager.
/// \param manager struct file_manager_t*, file manager.
/// \param filename const char*, name of the file.
/// \return int, whether creation is successful (= SUCCESS) or not (= FAILURE).
int file_create(struct file_manager_t* manager, const char* filename);

/// Open or create disk-based record system on file manager.
/// \param manager struct file_manager_t*, file manager.
/// \param filename const char*, name of the file.
/// \return int, whether open operation is successful (= SUCCESS) or not (= FAILURE).
int file_open(struct file_manager_t* manager, const char* filename);

/// Close the record system from the file manager.
/// \param manager struct file_manager_t*, file manager.
/// \return int, return SUCCESS (doesn't check the success).
int file_close(struct file_manager_t* manager);

/// Read file header through file manager.
/// \param manager struct file_manager_t*, file manager.
/// \param header struct file_header_t*, memory to save the read header.
/// \return int, whether writing operation is successful or not.
int file_read_header(struct file_manager_t* manager, struct file_header_t* header);

/// Write the file header through file manager.
/// \param manager struct file_manager_t*, file manager.
/// \param header struct file_header_t*, target file header.
/// \return int, whether writing operation is successful or not.
int file_write_header(struct file_manager_t* manager, struct file_header_t* header);

/// Get last page ID of current record system.
/// \param manager struct file_manager_t*, file manager.
/// \return pagenum_t, last page ID.
pagenum_t last_pagenum(struct file_manager_t* manager);

/// Get last page ID from file size.
/// \param size long, size of the file.
/// \return pagenum_t, last page ID.
pagenum_t last_pagenum_from_size(long size);

/// Create new page in file manager.
/// \param manager struct file_manager_t*, file manager.
/// \return pagenum_t, new page.
pagenum_t page_create(struct file_manager_t* manager);

/// Initialize page header.
/// \param page struct page_t*, page structure.
/// \param leaf int, whether leaf page (= 1) or not (= 0).
/// \return int, whether success to write page header or not.
int page_init(struct page_t* page, uint32_t leaf);

/// Allocate free pages.
/// \param manager struct file_manager_t*, file manager.
/// \param num int, number of free pages.
/// \return int, whether success to extend free page or not.
int page_extend_free(struct file_manager_t* manager, int num);

/// Deallocate given page.
/// \param manager struct file_manager_t*, file manager.
/// \param pagenum pagenum_t, target page ID.
/// \return int, whether success to deallocate page or not.
int page_free(struct file_manager_t* manager, pagenum_t pagenum);

/// Read page.
/// \param manager struct file_manager_t*, file manager.
/// \param pagenum pagenum_t, target page ID.
/// \param dst struct page_t*, memory for writing read page.
/// \return int, whether success to read page or not.
int page_read(struct file_manager_t* manager,
              pagenum_t pagenum,
              struct page_t* dst);

/// Write page.
/// \param manager struct file_manager_t*, file manager.
/// \param pagenum pagenum_t, target page ID.
/// \param src struct page_t*, target page data.
/// \return int, whether success to write page or not.
int page_write(struct file_manager_t* manager,
               pagenum_t pagenum,
               struct page_t* src);

#endif
