#include <cstring>
#include <string>

#include "disk_manager.hpp"
#include "fileio.hpp"
#include "headers.hpp"
#include "test.hpp"

TEST_SUITE(file_constructor, {
    FileManager manager("testfile");
    Page page;
    manager.page_read(0, &page);

    FileHeader* file_header = page.file_header();
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    manager.~FileManager();

    FileManager manager2("testfile");
    manager2.page_read(0, &page);

    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 0);
    TEST(file_header->number_of_pages == 0);

    manager2.~FileManager();
    remove("testfile");
})

// TEST_SUITE(file_destructor, {
//     FileManager manager;
//     TEST_SUCCESS(file_open(&manager, "testfile"));

//     TEST_SUCCESS(file_close(&manager));
//     TEST(manager.fp == NULL);

//     remove("testfile");
// })

// TEST_SUITE(page_create, {
//     FileManager manager;
//     TEST_SUCCESS(file_open( &manager, "testfile"));

//     pagenum_t pagenum = page_create(&manager);
//     TEST(pagenum == 1);

//     PaddedFileHeader padded;
//     fpread(&padded, PAGE_SIZE, 0, manager.fp);
//     TEST(padded.header.free_page_number == 0);
//     TEST(padded.header.number_of_pages == 1);

//     TEST(fsize(manager.fp) == PAGE_SIZE * 2);

//     pagenum = page_create(&manager);
//     fpread(&padded, PAGE_SIZE, 0, manager.fp);
//     TEST(pagenum == 2);
//     TEST(padded.header.free_page_number == 0);
//     TEST(padded.header.number_of_pages == 2);

//     TEST(fsize(manager.fp) == PAGE_SIZE * 3);

//     pagenum = page_create(&manager);
//     fpread(&padded, PAGE_SIZE, 0, manager.fp);
//     TEST(pagenum == 4);
//     TEST(padded.header.free_page_number == 3);
//     TEST(padded.header.number_of_pages == 4);

//     TEST_SUCCESS(file_close(&manager));
//     remove("testfile");
// })

// TEST_SUITE(page_free, {
//     FileManager manager;
//     TEST_SUCCESS(file_open(&manager, "testfile"));

//     pagenum_t pagenum = page_create(&manager);
//     TEST(pagenum == 1);

//     PaddedFileHeader padded;
//     fpread(&padded, PAGE_SIZE, 0, manager.fp);
//     TEST(padded.header.free_page_number == 0);

//     TEST(page_create(&manager) != INVALID_PAGENUM);

//     TEST_SUCCESS(page_free(&manager, pagenum));
//     fpread(&padded, PAGE_SIZE, 0, manager.fp);
//     TEST(padded.header.free_page_number == pagenum);

//     struct page_t page;
//     TEST_SUCCESS(page_read(&manager, pagenum, &page));
//     TEST(free_page(&page)->next_page_number == 0);

//     TEST_SUCCESS(file_close(&manager));
//     remove("testfile");
// })

// TEST_SUITE(page_read_write, {
//     FileManager manager;
//     TEST_SUCCESS(file_open(&manager, "testfile"));

//     pagenum_t pagenum = page_create(&manager);

//     struct page_t page;
//     TEST_SUCCESS(page_read(&manager, pagenum, &page));

//     struct page_header_t* pheader = page_header(&page);
//     pheader->parent_page_number = 0;
//     pheader->is_leaf = 0;
//     pheader->number_of_keys = 0;

//     TEST_SUCCESS(page_write(&manager, pagenum, &page));

//     TEST_SUCCESS(page_read(&manager, 1, &page));
//     TEST(pheader->parent_page_number == 0);
//     TEST(pheader->is_leaf == 0);
//     TEST(pheader->number_of_keys == 0);

//     fresize(manager.fp, PAGE_SIZE * 3);

//     pheader->parent_page_number = 1;
//     pheader->is_leaf = 1;
//     pheader->number_of_keys = 1;

//     TEST_SUCCESS(page_write(&manager, 2, &page));

//     struct page_t page2;
//     TEST_SUCCESS(page_read(&manager, 2, &page2));

//     pheader = page_header(&page2);
//     TEST(pheader->parent_page_number == 1);
//     TEST(pheader->is_leaf == 1);
//     TEST(pheader->number_of_keys == 1);

//     TEST_SUCCESS(file_close(&manager));
//     remove("testfile");
// })

int disk_manager_test() {
    return file_constructor_test()
        // && file_destructor_test()
        // && page_create_test()
        // && page_free_test()
        // && page_read_write_test();
        ;
}
