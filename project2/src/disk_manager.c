#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "disk_manager.h"
#include "fileio.h"
#include "utility.h"

int file_init(struct file_manager_t* manager) {
    fresize(manager->fp, PAGE_SIZE);

    struct file_header_t* file_header = &manager->file_header;
    file_header->free_page_number = 0;
    file_header->root_page_number = 0;
    file_header->number_of_pages = 0;

    fpwrite(file_header, sizeof(struct file_header_t), 0, manager->fp);
    return 0;
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
    return 0;
}

int file_close(struct file_manager_t* manager) {
    if (manager->updated > 0) {
        fpwrite(&manager->file_header,
                sizeof(struct file_header_t),
                0,
                manager->fp);
    }
    fclose(manager->fp);
    memset(manager, 0, sizeof(struct file_manager_t));
    return 0;
}

pagenum_t last_pagenum(struct file_manager_t* manager) {
    long size = fsize(manager->fp);
    return last_pagenum_from_size(size);
}

pagenum_t last_pagenum_from_size(long size) {
    return size / PAGE_SIZE - 1;
}

pagenum_t page_create(struct file_manager_t* manager) {
    pagenum_t pagenum = manager->file_header.free_page_number;
    if (pagenum == 0) {
        page_extend_free(manager, max(1, manager->file_header.number_of_pages * 2));
        pagenum = manager->file_header.free_page_number;
    }

    struct page_t page;
    page_read(pagenum, manager, &page);

    manager->file_header.free_page_number = 
        page.header.free_page.header.next_page_number;
    manager->updated++;

    return pagenum;
}

int page_extend_free(struct file_manager_t* manager, int num) {
    if (num < 1) {
        return 1;
    }

    long size = fsize(manager->fp);
    pagenum_t last = last_pagenum_from_size(size);

    fresize(manager->fp, size + num * PAGE_SIZE);

    int i;
    struct page_t page;
    struct free_page_t* free_page = (struct free_page_t*)&page.header.free_page;
    free_page->next_page_number = manager->file_header.free_page_number;

    for (i = 1; i <= num; ++i) {
        page_write(last + i, manager, &page);
        free_page->next_page_number = last + i;
    }

    manager->file_header.free_page_number = last + num;
    manager->file_header.number_of_pages += num;
    manager->updated++;

    return 0;
}

int page_free(pagenum_t pagenum, struct file_manager_t* manager) {
    struct page_t page;
    page_read(pagenum, manager, &page);

    page.header.free_page.header.next_page_number =
        manager->file_header.free_page_number;
    page_write(pagenum, manager, &page);

    manager->file_header.free_page_number = pagenum;
    return 0;
}

int page_read(pagenum_t pagenum,
              struct file_manager_t* manager,
              struct page_t* dst)
{
    fpread(dst, sizeof(struct page_t), pagenum * PAGE_SIZE, manager->fp);
    return 0;
}

int page_write(pagenum_t pagenum,
               struct file_manager_t* manager,
               struct page_t* src)
{
    fpwrite(src, sizeof(struct page_t), pagenum * PAGE_SIZE, manager->fp);
    return 0;
}
