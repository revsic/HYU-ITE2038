#include <string.h>

#include "disk_manager.h"
#include "fileio.h"
#include "test.h"

TEST_SUITE(file_init, {
    struct file_manager_t manager;
    manager.fp = fopen("testfile", "w+");

    file_init(&manager);
    TEST(fsize(manager.fp) == PAGE_SIZE);

    struct file_header_t* file_header = &manager.file_header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    struct padded_file_header_t real_fheader;
    fpread(&real_fheader, PAGE_SIZE, 0, manager.fp);
    
    file_header = &real_fheader.header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    fclose(manager.fp);
    remove("testfile");
})

TEST_SUITE(file_open, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    TEST(manager.updated == 0);

    fclose(manager.fp);
    memset(&manager, 0, sizeof(struct file_manager_t));

    file_open("testfile", &manager);

    TEST(manager.updated == 0);

    struct padded_file_header_t real_fheader;
    fpread(&real_fheader, PAGE_SIZE, 0, manager.fp);

    struct file_header_t* file_header = &real_fheader.header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    fclose(manager.fp);
    remove("testfile");
})

TEST_SUITE(file_close, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    file_close(&manager);
    TEST(manager.file_header.root_page_number == 0);

    file_open("testfile", &manager);
    manager.file_header.number_of_pages = 100;
    manager.updated += 1;

    file_close(&manager);

    file_open("testfile", &manager);
    TEST(manager.updated == 0);
    TEST(manager.file_header.number_of_pages = 100);

    struct padded_file_header_t real_fheader;
    fpread(&real_fheader, PAGE_SIZE, 0, manager.fp);

    struct file_header_t* file_header = &real_fheader.header;
    TEST(file_header->number_of_pages == 100);

    file_close(&manager);
    remove("testfile");
})

TEST_SUITE(last_pagenum, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    TEST(last_pagenum(&manager) == 0);

    fresize(manager.fp, PAGE_SIZE * (10 + 1));
    TEST(last_pagenum(&manager) == 10);

    file_close(&manager);
    remove("testfile");
})

TEST_SUITE(last_pagenum_from_size, {
    const int num = 37;
    TEST(last_pagenum_from_size(PAGE_SIZE * num) == num - 1);
})

TEST_SUITE(page_create, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    pagenum_t pagenum = page_create(&manager);
    TEST(pagenum == 1);
    TEST(manager.file_header.free_page_number == 0);

    TEST(fsize(manager.fp) == PAGE_SIZE * 2);

    struct page_t page;
    page_read(pagenum, &manager, &page);
    TEST(page.header.free_page.header.next_page_number == 0);

    pagenum = page_create(&manager);
    TEST(pagenum == 3);
    TEST(manager.file_header.free_page_number == 2);

    TEST(fsize(manager.fp) == PAGE_SIZE * 4);

    page_read(pagenum, &manager, &page);
    TEST(page.header.free_page.header.next_page_number == 2);

    file_close(&manager);
    remove("testfile");
})

TEST_SUITE(page_extend_free, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    page_extend_free(&manager, 3);
    TEST(fsize(manager.fp) == 4 * PAGE_SIZE);

    TEST(manager.file_header.free_page_number == 3);

    struct page_t page;

    int i;
    for (i = 2; i >= 0; --i) {
        page_read(i + 1, &manager, &page);
        TEST(page.header.free_page.header.next_page_number == i);
    }

    file_close(&manager);
    remove("testfile");
})

TEST_SUITE(page_free, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    pagenum_t pagenum = page_create(&manager);
    TEST(pagenum == 1);
    TEST(manager.file_header.free_page_number == 0);

    page_create(&manager);

    page_free(pagenum, &manager);
    TEST(manager.file_header.free_page_number == 1);

    struct page_t page;
    page_read(pagenum, &manager, &page);
    TEST(page.header.free_page.header.next_page_number == 2);

    file_close(&manager);
    remove("testfile");
})

TEST_SUITE(page_read_write, {
    struct file_manager_t manager;
    file_open("testfile", &manager);

    pagenum_t pagenum = page_create(&manager);

    struct page_t page;
    page_read(pagenum, &manager, &page);

    struct page_header_t* page_header = &page.header.page_header;
    page_header->parent_page_number = 0;
    page_header->is_leaf = 0;
    page_header->number_of_keys = 0;

    page_write(pagenum, &manager, &page);

    page_read(1, &manager, &page);
    TEST(page_header->parent_page_number == 0);
    TEST(page_header->is_leaf == 0);
    TEST(page_header->number_of_keys == 0);

    fresize(manager.fp, PAGE_SIZE * 3);

    page_header->parent_page_number = 1;
    page_header->is_leaf = 1;
    page_header->number_of_keys = 1;

    page_write(2, &manager, &page);

    struct page_t page2;
    page_read(2, &manager, &page2);

    page_header = &page2.header.page_header;
    TEST(page_header->parent_page_number == 1);
    TEST(page_header->is_leaf == 1);
    TEST(page_header->number_of_keys == 1);

    file_close(&manager);
    remove("testfile");
})

int disk_manager_test() {
    return file_init_test()
        && file_open_test()
        && file_close_test()
        && last_pagenum_test()
        && last_pagenum_from_size_test()
        && page_create_test()
        && page_extend_free_test()
        && page_free_test()
        && page_read_write_test();
}
