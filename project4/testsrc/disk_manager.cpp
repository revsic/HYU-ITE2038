#include <cstring>
#include <string>

#include "disk_manager.hpp"
#include "fileio.hpp"
#include "headers.hpp"
#include "test.hpp"

TEST_SUITE(hash_filename, {
    auto pair = FileManager::hash_filename("testfile");
    auto pair2 = FileManager::hash_filename("../project/testfile");
    TEST(pair == pair2);
})

TEST_SUITE(file_constructor, {
    FileManager manager("testfile");
    Page page;
    manager.page_read(0, page);

    FileHeader& file_header = page.file_header();
    TEST(file_header.free_page_number == 0);
    TEST(file_header.root_page_number == 0);
    TEST(file_header.number_of_pages == 0);

    manager.~FileManager();

    FileManager manager2("testfile");
    manager2.page_read(0, page);

    TEST(file_header.free_page_number == 0);
    TEST(file_header.root_page_number == 0);
    TEST(file_header.number_of_pages == 0);

    manager2.~FileManager();

    FileManager manager3("testfile");
    FileManager manager4("../project4/testfile");
    TEST(manager3.get_id() == manager4.get_id());
    TEST(manager3.get_filename() == manager4.get_filename());
    remove("testfile");
})

TEST_SUITE(file_destructor, {
    // Just free fp
})

TEST_SUITE(page_create, {
    FileManager manager("testfile");

    pagenum_t pagenum = manager.page_create();
    TEST(pagenum == 1);

    Page page;
    TEST_SUCCESS(manager.page_read(FILE_HEADER_PAGENUM, page));
    TEST(page.file_header().free_page_number == 0);
    TEST(page.file_header().number_of_pages == 1);

    pagenum = manager.page_create();
    TEST_SUCCESS(manager.page_read(FILE_HEADER_PAGENUM, page));
    TEST(pagenum == 2);
    TEST(page.file_header().free_page_number == 0);
    TEST(page.file_header().number_of_pages == 2);

    pagenum = manager.page_create();
    TEST_SUCCESS(manager.page_read(FILE_HEADER_PAGENUM, page));
    TEST(pagenum == 4);
    TEST(page.file_header().free_page_number == 3);
    TEST(page.file_header().number_of_pages == 4);

    manager.~FileManager();
    remove("testfile");
})

TEST_SUITE(page_free, {
    FileManager manager("testfile");
    pagenum_t pagenum = manager.page_create();
    TEST(pagenum == 1);

    Page page;
    TEST_SUCCESS(manager.page_read(FILE_HEADER_PAGENUM, page));
    TEST(page.file_header().free_page_number == 0);

    TEST(manager.page_create() != INVALID_PAGENUM);

    TEST_SUCCESS(manager.page_free(pagenum));
    TEST_SUCCESS(manager.page_read(FILE_HEADER_PAGENUM, page));
    TEST(page.file_header().free_page_number == pagenum);

    TEST_SUCCESS(manager.page_read(pagenum, page));
    TEST(page.free_page().next_page_number == 0);

    manager.~FileManager();
    remove("testfile");
})

TEST_SUITE(page_read_write, {
    FileManager manager("testfile");

    pagenum_t pagenum = manager.page_create();

    Page page;
    TEST_SUCCESS(manager.page_read(pagenum, page));

    PageHeader& pheader = page.page_header();
    pheader.parent_page_number = 0;
    pheader.is_leaf = 0;
    pheader.number_of_keys = 0;

    TEST_SUCCESS(manager.page_write(pagenum, page));

    TEST_SUCCESS(manager.page_read(1, page));
    TEST(pheader.parent_page_number == 0);
    TEST(pheader.is_leaf == 0);
    TEST(pheader.number_of_keys == 0);

    pagenum = manager.page_create();

    pheader.parent_page_number = 1;
    pheader.is_leaf = 1;
    pheader.number_of_keys = 1;

    TEST_SUCCESS(manager.page_write(pagenum, page));

    Page page2;
    TEST_SUCCESS(manager.page_read(pagenum, page2));

    pheader = page2.page_header();
    TEST(pheader.parent_page_number == 1);
    TEST(pheader.is_leaf == 1);
    TEST(pheader.number_of_keys == 1);

    manager.~FileManager();
    remove("testfile");
})

int disk_manager_test() {
    return hash_filename_test()
        && file_constructor_test()
        && file_destructor_test()
        && page_create_test()
        && page_free_test()
        && page_read_write_test();
}
