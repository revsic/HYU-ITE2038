#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "disk_manager.h"
#include "fileio.h"

int file_init(struct file_manager_t* manager) {
    struct padded_file_header_t* padded_header =
        (struct padded_file_header_t*)&manager->file_header;
    struct file_header_t* file_header = &padded_header->header;
    file_header->free_page_number = 0;
    file_header->root_page_number = 1;
    file_header->number_of_pages = 1;

    fpwrite(&padded_header,
            sizeof(struct padded_file_header_t),
            0,
            manager->fp);

    struct page_t page;
    struct page_header_t* page_header = &page.header.page_header;
    page_header->parent_page_number = 0;
    page_header->is_leaf = 0;
    page_header->number_of_keys = 0;
    page_header->special_page_number = 0;

    fpwrite(&page, sizeof(struct page_t), -1, manager->fp);
    return 1;
}

int file_open(char* filename, struct file_manager_t* manager) {
    if (fexist(filename)) {
        manager->fp = fopen(filename, "r+");
        manager->updated = 0;
        fread(&manager->file_header,
              sizeof(struct file_header_t),
              1,
              manager->fp);
    } else {
        manager->fp = fopen(filename, "w+");
        manager->updated = 0;
        file_init(manager);
    }
    return 1;
}

int file_close(struct file_manager_t* manager) {
    if (manager->updated != 0) {
        fpwrite(&manager->file_header,
                sizeof(struct page_header_t),
                0,
                manager->fp);
    }
    fclose(manager->fp);
    memset(manager, 0, sizeof(struct file_manager_t));
}

pagenum_t page_create(struct file_manager_t* manager) {

}

int page_extend_free(struct file_manager_t* manager) {

}

int page_free(pagenum_t pagenum, struct file_manager_t* manager);

int page_read(pagenum_t pagenum,
              struct file_manager_t* manager,
              struct page_t* dst);

int page_write(pagenum_t pagenum,
               struct file_manager_t* manager,
               struct page_t* src);
