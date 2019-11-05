// #include <stdio.h>

// #include "table.h"
// #include "test.h"

// TEST_SUITE(create_tablenum, {
//     const char* filename = "datafile";
//     const char* fullpath = "/Users/revsic/datafile";
//     tablenum_t tablenum = create_tablenum(filename);

//     DBG(tablenum);
//     TEST(tablenum == create_tablenum(fullpath));
// })

// TEST_SUITE(rehash_tablenum, {
//     const char* filename = "1234";
//     tablenum_t previous = 0x34333231;
//     TEST(create_tablenum(filename) == rehash_tablenum(previous));
// })

// TEST_SUITE(table_init, {
//     struct table_t table;
//     TEST_SUCCESS(table_init(&table));
//     TEST(table.table_id == INVALID_TABLENUM);
// })

// TEST_SUITE(table_load, {
//     struct table_t table;
//     const char* filename = "testfile";

//     TEST_SUCCESS(table_load(&table, filename));
//     TEST(table.table_id == create_tablenum(filename));

//     TEST_SUCCESS(file_close(&table.file_manager));
//     remove("testfile");
// })

// TEST_SUITE(table_release, {
//     struct table_t table;
//     TEST_SUCCESS(table_load(&table, "testfile"));
//     TEST_SUCCESS(table_release(&table));

//     TEST(table.table_id == INVALID_TABLENUM);
//     remove("testfile");
// })

// int table_test() {
//     return create_tablenum_test()
//         && rehash_tablenum_test()
//         && table_init_test()
//         && table_load_test()
//         && table_release_test();
// }