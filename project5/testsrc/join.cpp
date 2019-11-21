// #include "dbms.hpp"
// #include "join.hpp"

// int arr1[] = { 1, 2, 3, 5, 7 };
// int arr2[] = { 1, 4, 5, 6, 7 };

// TEST_SUITE(set_merge, {
//     auto dbms = std::make_unique<Database>(100000);

//     tableid_t tid1 = dbms->open_table("testfile1");
//     tableid_t tid2 = dbms->open_table("testfile2");

//     uint8_t arr[5];
//     for (int i = 0; i < 5; ++i) {
//         dbms->insert(tid1, arr1[i], arr, 5);
//         dbms->insert(tid2, arr2[i], arr, 5);
//     }

//     std::vector<int> found;
//     JoinOper::set_merge((*dbms)[tid1], (*dbms)[tid2],
//         [&](Record& rec1, Record& rec2) {
//             found.push_back(rec1.key);
//             return Status::SUCCESS;
//         }
//     );

//     TEST(found.size() == 3);
//     TEST(found[0] == 1);
//     TEST(found[1] == 5);
//     TEST(found[2] == 7);

//     dbms->destroy_tree(tid1);
//     dbms->destroy_tree(tid2);

//     constexpr int num_insert =
//         BPTree::DEFAULT_LEAF_ORDER * BPTree::DEFAULT_INTERNAL_ORDER / 2;
//     for (int i = 0; i < num_insert; ++i) {
//         dbms->insert(tid1, i * 2, arr, 5);
//         dbms->insert(tid2, i * 3, arr, 5);
//         TEST_SUCCESS(dbms->find(tid1, i * 2, nullptr));
//         TEST_SUCCESS(dbms->find(tid2, i * 3, nullptr));
//     }

//     found.clear();
//     JoinOper::set_merge((*dbms)[tid1], (*dbms)[tid2],
//         [&](Record& rec1, Record&) {
//             found.push_back(rec1.key);
//             return Status::SUCCESS;
//         });

//     TEST(found.size() == num_insert / 3);

//     int i = 0;
//     for (int num : found) {
//         TEST(num == i * 6);
//         ++i;
//     }

//     dbms.reset();
//     remove("testfile1");
//     remove("testfile2");
// })

// int join_test() {
//     return set_merge_test();
// }