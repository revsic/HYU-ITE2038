#include <string.h>

#include "disk_manager.h"
#include "fileio.h"
#include "test.h"

TEST_SUITE(file_init, {
    struct file_manager_t manager;
    manager.fp = fopen("testfile", "w+");

    file_init(&manager);

    struct file_header_t* file_header = &manager.file_header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 1);
    TEST(file_header->number_of_pages == 1);

    struct padded_file_header_t real_fheader;
    fpread(&real_fheader, PAGE_SIZE, 0, manager.fp);
    
    file_header = &real_fheader.header;
    TEST(file_header->free_page_number == 0);
    TEST(file_header->root_page_number == 1);
    TEST(file_header->number_of_pages == 1);

    struct page_t page;
    page_read(1, &manager, &page);

    struct page_header_t* page_header = &page.header.page_header;
    TEST(page_header->parent_page_number == 0);
    TEST(page_header->is_leaf == 0);
    TEST(page_header->number_of_keys == 0);
    TEST(page_header->special_page_number == 0);

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
    TEST(file_header->root_page_number == 1);
    TEST(file_header->number_of_pages == 1);

    struct page_t page;
    page_read(1, &manager, &page);

    struct page_header_t* page_header = &page.header.page_header;
    TEST(page_header->parent_page_number == 0);
    TEST(page_header->is_leaf == 0);
    TEST(page_header->number_of_keys == 0);

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

    TEST(last_pagenum(&manager) == 1);

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

})

TEST_SUITE(page_extend_free, {

})

TEST_SUITE(page_free, {

})

TEST_SUITE(page_read, {

})

TEST_SUITE(page_write, {

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
        && page_read_test()
        && page_write_test();
}
