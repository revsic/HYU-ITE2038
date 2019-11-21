// #include "test.hpp"
// #include "table_manager.hpp"

// struct TableTest {
// #define TEST_NAME(name) static int name##_test();
//     TEST_NAME(constructor)
//     TEST_NAME(assignment)
//     TEST_NAME(print_tree)
//     TEST_NAME(find)
//     TEST_NAME(find_range)
//     TEST_NAME(insert)
//     TEST_NAME(remove)
//     TEST_NAME(destroy_tree)
//     TEST_NAME(fileid)
//     TEST_NAME(rehash)
//     TEST_NAME(filename)
// };

// TEST_SUITE(TableTest::constructor, {
//     // TODO: Impl
// })

// TEST_SUITE(TableTest::assignment, {
//     // TODO: Impl
// })

// TEST_SUITE(TableTest::print_tree, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::find, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::find_range, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::insert, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::remove, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::destroy_tree, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::fileid, {
//     // just wrapping get_id
// })

// TEST_SUITE(TableTest::rehash, {
//     // just wrapping
// })

// TEST_SUITE(TableTest::filename, {
//     // just wrapping get_filename
// })

// struct TableManagerTest {
//     TEST_NAME(load)
//     TEST_NAME(find)
//     TEST_NAME(remove)
// };

// TEST_SUITE(TableManagerTest::load, {
//     TableManager tables;
//     BufferManager buffers(4);

//     tableid_t res = tables.load("testfile", buffers);
//     TEST(res == tables.load("./testfile", buffers));
//     TEST(res != tables.load("./testfile2", buffers));

//     remove("testfile");
//     remove("testfile2");
// })

// TEST_SUITE(TableManagerTest::find, {
//     TableManager tables;
//     BufferManager buffers(4);

//     tableid_t res = tables.load("./testfile", buffers);
//     tableid_t res2 = tables.load("./testfile2", buffers);
//     tableid_t res3 = tables.load("./testfile3", buffers);

//     TEST(tables.find(res) != nullptr);
//     TEST(tables.find(res2) != nullptr);
//     TEST(tables.find(res3) != nullptr);
//     TEST(tables.find(res)->filename() == "testfile");
//     TEST(tables.find(res2)->filename() == "testfile2");
//     TEST(tables.find(res3)->filename() == "testfile3");

//     remove("testfile");
//     remove("testfile2");
//     remove("testfile3");
// })

// TEST_SUITE(TableManagerTest::remove, {
//     TableManager tables;
//     BufferManager buffers(4);

//     tableid_t res = tables.load("./testfile", buffers);
//     tableid_t res2 = tables.load("./testfile2", buffers);
//     tableid_t res3 = tables.load("./testfile3", buffers);

//     tableid_t arr[3];
//     arr[0] = res; arr[1] = res2; arr[2] = res3;

//     for (auto const& pair : tables.tables) {
//         bool found = false;
//         for (int i = 0; i < 3; ++i) {
//             if (arr[i] == pair.first) {
//                 found = true;
//                 break;
//             }
//         }
//         TEST(found);
//     }

//     tables.remove(res3);
//     TEST(tables.tables.size() == 2);

//     for (auto const& pair : tables.tables) {
//         bool found = false;
//         for (int i = 0; i < 2; ++i) {
//             if (arr[i] == pair.first) {
//                 found = true;
//                 break;
//             }
//         }
//         TEST(found);
//     }

//     tables.remove(res2);
//     TEST(tables.tables.size() == 1);

//     for (auto const& pair : tables.tables) {
//         TEST(arr[0] == pair.first);
//     }

//     tables.remove(res);
//     TEST(tables.tables.size() == 0);

//     remove("testfile");
//     remove("testfile2");
//     remove("testfile3");
// })

// int table_manager_test() {
//     return TableTest::constructor_test()
//         && TableTest::assignment_test()
//         && TableTest::print_tree_test()
//         && TableTest::find_test()
//         && TableTest::find_range_test()
//         && TableTest::insert_test()
//         && TableTest::remove_test()
//         && TableTest::destroy_tree_test()
//         && TableTest::fileid_test()
//         && TableTest::rehash_test()
//         && TableTest::filename_test()
//         && TableManagerTest::load_test()
//         && TableManagerTest::find_test()
//         && TableManagerTest::remove_test();
// }
