#include "bptree_iter.hpp"

struct BPTreeIteratorTest {
#define TEST_NAME(name) static int name##_test();
    TEST_NAME(ctor);
    TEST_NAME(copy_ctor);
    TEST_NAME(copy_assign);
    TEST_NAME(move_ctor);
    TEST_NAME(move_assign);
    TEST_NAME(begin);
    TEST_NAME(end);
    TEST_NAME(inc_operator);
    TEST_NAME(cmp_operator);
    TEST_NAME(deref_operator);
};

TEST_SUITE(BPTreeIteratorTest::ctor, {
    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        2, 1, 10, Ubuffer(nullptr), &tree);

    TEST(iter.pagenum == 2);
    TEST(iter.record_index == 1);
    TEST(iter.num_key == 10);
    TEST(iter.buffer.buffer() == nullptr);
    TEST(iter.tree == &tree);
})

TEST_SUITE(BPTreeIteratorTest::copy_ctor, {
    BufferManager manager(4);
    FileManager file("testfile");

    Ubuffer buffer = manager.buffering(file, 0);

    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        2, 1, 10, std::move(buffer), &tree);
    
    BPTreeIterator copied(iter);

    TEST(iter.pagenum == 2);
    TEST(iter.record_index == 1);
    TEST(iter.num_key == 10);
    TEST(iter.buffer.safe_pagenum() == 0);
    TEST(iter.tree == &tree);

    TEST(copied.pagenum == 2);
    TEST(copied.record_index == 1);
    TEST(copied.num_key == 10);
    TEST(copied.buffer.safe_pagenum() == 0);
    TEST(copied.tree == &tree);

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BPTreeIteratorTest::copy_assign, {
    BufferManager manager(4);
    FileManager file("testfile");

    Ubuffer buffer = manager.buffering(file, 0);

    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        2, 1, 10, std::move(buffer), &tree);

    BPTreeIterator assigned(0, 0, 0, Ubuffer(nullptr), nullptr);

    assigned = iter;

    TEST(iter.pagenum == 2);
    TEST(iter.record_index == 1);
    TEST(iter.num_key == 10);
    TEST(iter.buffer.safe_pagenum() == 0);
    TEST(iter.tree == &tree);

    TEST(assigned.pagenum == 2);
    TEST(assigned.record_index == 1);
    TEST(assigned.num_key == 10);
    TEST(assigned.buffer.safe_pagenum() == 0);
    TEST(assigned.tree == &tree);

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BPTreeIteratorTest::move_ctor, {
    BufferManager manager(4);
    FileManager file("testfile");

    Ubuffer buffer = manager.buffering(file, 0);

    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        2, 1, 10, std::move(buffer), &tree);

    BPTreeIterator moved(std::move(iter));

    TEST(iter.pagenum == INVALID_PAGENUM);
    TEST(iter.record_index == 0);
    TEST(iter.num_key == 0);
    TEST(iter.buffer.buffer() == nullptr);
    TEST(iter.tree == nullptr);

    TEST(moved.pagenum == 2);
    TEST(moved.record_index == 1);
    TEST(moved.num_key == 10);
    TEST(moved.buffer.safe_pagenum() == 0);
    TEST(moved.tree == &tree);

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");

})

TEST_SUITE(BPTreeIteratorTest::move_assign, {
    BufferManager manager(4);
    FileManager file("testfile");

    Ubuffer buffer = manager.buffering(file, 0);

    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        2, 1, 10, std::move(buffer), &tree);

    BPTreeIterator assigned(0, 0, 0, Ubuffer(nullptr), nullptr);

    assigned = std::move(iter);

    TEST(iter.pagenum == INVALID_PAGENUM);
    TEST(iter.record_index == 0);
    TEST(iter.num_key == 0);
    TEST(iter.buffer.buffer() == nullptr);
    TEST(iter.tree == nullptr);

    TEST(assigned.pagenum == 2);
    TEST(assigned.record_index == 1);
    TEST(assigned.num_key == 10);
    TEST(assigned.buffer.safe_pagenum() == 0);
    TEST(assigned.tree == &tree);

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");

})

TEST_SUITE(BPTreeIteratorTest::begin, {
    BufferManager manager(100);
    FileManager file("testfile");
    
    uint8_t buf[5];
    BPTree tree(&file, &manager);
    for (int i = 0; i < 20; ++i) {
        tree.insert(i, buf, 5);
    }

    auto iter = BPTreeIterator::begin(tree);
    TEST(iter.pagenum == 1);
    TEST(iter.record_index == 0);
    TEST(iter.num_key == 20);
    TEST(iter.buffer.safe_pagenum() == 1);
    TEST(iter.tree == &tree);

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BPTreeIteratorTest::end, {
    auto iter = BPTreeIterator::end();
    TEST(iter.pagenum == INVALID_PAGENUM);
    TEST(iter.record_index == 0);
    TEST(iter.num_key == 0);
    TEST(iter.buffer.buffer() == nullptr);
    TEST(iter.tree == nullptr);
})

TEST_SUITE(BPTreeIteratorTest::inc_operator, {

})

TEST_SUITE(BPTreeIteratorTest::cmp_operator, {

})

TEST_SUITE(BPTreeIteratorTest::deref_operator, {

})

int bptree_iter_test() {
    return BPTreeIteratorTest::ctor_test()
        && BPTreeIteratorTest::copy_ctor_test()
        && BPTreeIteratorTest::copy_assign_test()
        && BPTreeIteratorTest::move_ctor_test()
        && BPTreeIteratorTest::move_assign_test()
        && BPTreeIteratorTest::begin_test()
        && BPTreeIteratorTest::end_test()
        && BPTreeIteratorTest::inc_operator_test()
        && BPTreeIteratorTest::cmp_operator_test()
        && BPTreeIteratorTest::deref_operator_test();
}