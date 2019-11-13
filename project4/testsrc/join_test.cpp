#include "dbms.hpp"
#include "join.hpp"

int arr1[] = { 1, 2, 3, 5, 7};
int arr2[] = { 1, 4, 5, 6, 7};

TEST_SUITE(set_merge, {
    Database dbms(5);

    tableid_t tid1 = dbms.open_table("testfile1");
    tableid_t tid2 = dbms.open_table("testfile2");

    uint8_t arr[5];
    for (int i = 0; i < 5; ++i) {
        dbms.insert(tid1, arr1[i], arr, 5);
        dbms.insert(tid2, arr2[i], arr, 5);
    }

    std::vector<int> found;
    JoinOper::set_merge(dbms[tid1], dbms[tid2],
        [&](Record& rec1, Record& rec2) {
            found.push_back(rec1.key);
            return Status::SUCCESS;
        }
    );

    TEST(found.size() == 3);
    TEST(found[0] == 1);
    TEST(found[1] == 5);
    TEST(found[2] == 7);

    dbms.~Database();
    remove("testfile1");
    remove("testfile2");
})

int join_test() {
    return set_merge_test();
}