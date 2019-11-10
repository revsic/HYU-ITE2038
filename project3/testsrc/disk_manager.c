#include <string.h>

#include "disk_manager.h"
#include "fileio.h"
#include "test.h"
#include "utility.h"

TEST_SUITE(create_filenum, {
    const char* filename = "datafile";
    const char* fullpath = "/Users/revsic/datafile";
    filenum_t filenum = create_filenum(filename);
    TEST(filenum == create_filenum(fullpath))
})

TEST_SUITE(file_init, {
    struct file_manager_t manager;
    manager.fp = fopen("testfile", "w+");

    TEST(file_init(&manager) == SUCCESS);
    TEST(fsize(manager.fp) == PAGE_SIZE);

    struct padded_file_header_t real_fheader;
    fpread(&real_fheader, PAGE_SIZE, 0, manager.fp);
    
    struct file_header_t* file_header = &real_fheader.header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    fclose(manager.fp);
    remove("testfile");
})

TEST_SUITE(file_create, {
    struct file_manager_t manager;

    TEST(file_create(&manager, "testfile") == SUCCESS);
    fclose(manager.fp);

    FILE* fp = fopen("testfile", "rb");
    struct padded_file_header_t padded;
    fpread(&padded, PAGE_SIZE, 0, fp);

    struct file_header_t* file_header = &padded.header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    fclose(fp);
    remove("testfile");
})

TEST_SUITE(file_open, {
    struct file_manager_t manager;
    struct padded_file_header_t padded;

    TEST_SUCCESS(file_open(&manager, "testfile"));
    fpread(&padded, PAGE_SIZE, 0, manager.fp);

    struct file_header_t* file_header = &padded.header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    fclose(manager.fp);

    TEST_SUCCESS(file_open(&manager, "testfile"));
    fpread(&padded, PAGE_SIZE, 0, manager.fp);

    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    fclose(manager.fp);
    remove("testfile");
})

TEST_SUITE(file_close, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open(&manager, "testfile"));

    TEST_SUCCESS(file_close(&manager));
    TEST(manager.fp == NULL);

    remove("testfile");
})

TEST_SUITE(file_rw_header, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open(&manager, "testfile"));

    struct file_header_t header;
    header.free_page_number = 100;
    header.root_page_number = 200;
    header.number_of_pages = 300;

    TEST_SUCCESS(file_write_header(&manager, &header));
    memset(&header, 0, sizeof(struct file_header_t));

    TEST_SUCCESS(file_read_header(&manager, &header));
    TEST(header.free_page_number == 100);
    TEST(header.root_page_number == 200);
    TEST(header.number_of_pages == 300);

    TEST_SUCCESS(file_close(&manager));
    remove("testfile");
})

TEST_SUITE(last_pagenum, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open(&manager, "testfile"));

    TEST(last_pagenum(&manager) == 0);

    fresize(manager.fp, PAGE_SIZE * (10 + 1));
    TEST(last_pagenum(&manager) == 10);

    TEST_SUCCESS(file_close(&manager));
    remove("testfile");
})

TEST_SUITE(last_pagenum_from_size, {
    const int num = 37;
    TEST(last_pagenum_from_size(PAGE_SIZE * num) == num - 1);
})

TEST_SUITE(page_create, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open( &manager, "testfile"));

    pagenum_t pagenum = page_create(&manager);
    TEST(pagenum == 1);

    struct padded_file_header_t padded;
    fpread(&padded, PAGE_SIZE, 0, manager.fp);
    TEST(padded.header.free_page_number == 0);
    TEST(padded.header.number_of_pages == 1);

    TEST(fsize(manager.fp) == PAGE_SIZE * 2);

    pagenum = page_create(&manager);
    fpread(&padded, PAGE_SIZE, 0, manager.fp);
    TEST(pagenum == 2);
    TEST(padded.header.free_page_number == 0);
    TEST(padded.header.number_of_pages == 2);

    TEST(fsize(manager.fp) == PAGE_SIZE * 3);

    pagenum = page_create(&manager);
    fpread(&padded, PAGE_SIZE, 0, manager.fp);
    TEST(pagenum == 4);
    TEST(padded.header.free_page_number == 3);
    TEST(padded.header.number_of_pages == 4);

    TEST_SUCCESS(file_close(&manager));
    remove("testfile");
})

TEST_SUITE(page_init, {
    struct page_t page;
    TEST_SUCCESS(page_init(&page, 1));

    struct page_header_t* header = page_header(&page);
    TEST(header->is_leaf == 1);
    TEST(header->number_of_keys == 0);
    TEST(header->parent_page_number == INVALID_PAGENUM);
    TEST(header->special_page_number == INVALID_PAGENUM);
})

TEST_SUITE(page_extend_free, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open(&manager, "testfile"));

    TEST_SUCCESS(page_extend_free(&manager, 3));
    TEST(fsize(manager.fp) == 4 * PAGE_SIZE);

    struct padded_file_header_t padded;
    fpread(&padded, PAGE_SIZE, 0, manager.fp);
    TEST(padded.header.free_page_number == 3);

    struct page_t page;

    int i;
    for (i = 2; i >= 0; --i) {
        TEST_SUCCESS(page_read(&manager, i + 1, &page));
        TEST(free_page(&page)->next_page_number == i);
    }

    TEST_SUCCESS(file_close(&manager));
    remove("testfile");
})

TEST_SUITE(page_free, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open(&manager, "testfile"));

    pagenum_t pagenum = page_create(&manager);
    TEST(pagenum == 1);

    struct padded_file_header_t padded;
    fpread(&padded, PAGE_SIZE, 0, manager.fp);
    TEST(padded.header.free_page_number == 0);

    TEST(page_create(&manager) != INVALID_PAGENUM);

    TEST_SUCCESS(page_free(&manager, pagenum));
    fpread(&padded, PAGE_SIZE, 0, manager.fp);
    TEST(padded.header.free_page_number == pagenum);

    struct page_t page;
    TEST_SUCCESS(page_read(&manager, pagenum, &page));
    TEST(free_page(&page)->next_page_number == 0);

    TEST_SUCCESS(file_close(&manager));
    remove("testfile");
})

TEST_SUITE(page_read_write, {
    struct file_manager_t manager;
    TEST_SUCCESS(file_open(&manager, "testfile"));

    pagenum_t pagenum = page_create(&manager);

    struct page_t page;
    TEST_SUCCESS(page_read(&manager, pagenum, &page));

    struct page_header_t* pheader = page_header(&page);
    pheader->parent_page_number = 0;
    pheader->is_leaf = 0;
    pheader->number_of_keys = 0;

    TEST_SUCCESS(page_write(&manager, pagenum, &page));

    TEST_SUCCESS(page_read(&manager, 1, &page));
    TEST(pheader->parent_page_number == 0);
    TEST(pheader->is_leaf == 0);
    TEST(pheader->number_of_keys == 0);

    fresize(manager.fp, PAGE_SIZE * 3);

    pheader->parent_page_number = 1;
    pheader->is_leaf = 1;
    pheader->number_of_keys = 1;

    TEST_SUCCESS(page_write(&manager, 2, &page));

    struct page_t page2;
    TEST_SUCCESS(page_read(&manager, 2, &page2));

    pheader = page_header(&page2);
    TEST(pheader->parent_page_number == 1);
    TEST(pheader->is_leaf == 1);
    TEST(pheader->number_of_keys == 1);

    TEST_SUCCESS(file_close(&manager));
    remove("testfile");
})

int disk_manager_test() {
    return create_filenum_test()
        && file_init_test()
        && file_create_test()
        && file_open_test()
        && file_close_test()
        && file_rw_header_test()
        && last_pagenum_test()
        && last_pagenum_from_size_test()
        && page_init_test()
        && page_create_test()
        && page_extend_free_test()
        && page_free_test()
        && page_read_write_test();
}
