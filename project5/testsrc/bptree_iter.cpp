#include <random>
#include <vector>

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
    TEST_NAME(integrate);
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
    TEST(iter.buffer.to_pagenum() == 0);
    TEST(iter.tree == &tree);

    TEST(copied.pagenum == 2);
    TEST(copied.record_index == 1);
    TEST(copied.num_key == 10);
    TEST(copied.buffer.to_pagenum() == 0);
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
    TEST(iter.buffer.to_pagenum() == 0);
    TEST(iter.tree == &tree);

    TEST(assigned.pagenum == 2);
    TEST(assigned.record_index == 1);
    TEST(assigned.num_key == 10);
    TEST(assigned.buffer.to_pagenum() == 0);
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
    TEST(moved.buffer.to_pagenum() == 0);
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
    TEST(assigned.buffer.to_pagenum() == 0);
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
    TEST(iter.buffer.to_pagenum() == 1);
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
    BufferManager manager(100);
    FileManager file("testfile");
    
    uint8_t tmp[5];
    BPTree tree(&file, &manager);
    tree.test_config(4, 5, true);
    for (int i = 0; i < 20; ++i) {
        tree.insert(i, tmp, 5);
    }

    auto iter = BPTreeIterator::begin(tree);
    Ubuffer buf = manager.buffering(file, 1);

    int num_key = buf.read([](Page const& page) {
        return page.page_header().number_of_keys;
    });

    for (int i = 0; i < 20; ++i) {
        if (i != 0 && i % 2 == 0) {
            pagenum_t next = buf.read([](Page const& page) {
                return page.page_header().special_page_number;
            });

            buf = manager.buffering(file, next);
            num_key = buf.read([](Page const& page) {
                return page.page_header().number_of_keys;
            });
        }

        TEST(iter.pagenum == buf.to_pagenum());
        TEST(iter.record_index == i % 2);
        TEST(iter.num_key == num_key);
        TEST(iter.buffer.to_pagenum() == buf.to_pagenum());
        TEST(iter.tree == &tree);
        
        ++iter;
    }

    TEST(iter == BPTreeIterator::end());

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BPTreeIteratorTest::cmp_operator, {
    BufferManager manager(4);
    FileManager file("testfile");

    Ubuffer buffer = manager.buffering(file, 0);

    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        2, 1, 10, std::move(buffer), &tree);

    BPTreeIterator iter2(
        2, 1, 10, std::move(buffer), &tree);
    
    TEST(iter == iter2);

    BPTreeIterator iter3(
        3, 1, 10, std::move(buffer), &tree);
    TEST(iter != iter3);

    BPTreeIterator iter4(
        2, 2, 10, std::move(buffer), &tree);
    TEST(iter != iter4);

    BPTreeIterator iter5(
        3, 2, 10, std::move(buffer), &tree);
    TEST(iter != iter5);

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BPTreeIteratorTest::deref_operator, {
    BufferManager manager(4);
    FileManager file("testfile");

    Ubuffer buffer = manager.buffering(file, 0);
    buffer.write_void([&](Page& page) {
        page.page_header().number_of_keys = 3;
        page.page_header().special_page_number = INVALID_PAGENUM;
        for (int i = 0; i < 3; ++i) {
            page.records()[i].key = i;
        }
    });

    BPTree tree(nullptr, nullptr);
    BPTreeIterator iter(
        0, 0, 3, std::move(buffer), &tree);

    for (int i = 0; i < 3; ++i) {
        TEST((*iter).key() == i);
        ++iter;
    }

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BPTreeIteratorTest::integrate, {
    BufferManager manager(100);
    FileManager file("testfile");
    
    uint8_t tmp[5];
    BPTree tree(&file, &manager);
    tree.test_config(4, 5, true);

    std::vector<int> values;
    for (int i = 0; i < 20; ++i) {
        values.push_back(i);
    }

    std::random_device rd;
    std::default_random_engine gen(rd());
    for (int i = 0; i < 20; ++i) {
        int idx = gen() % values.size();
        tree.insert(values[idx], tmp, 5);
        values.erase(values.begin() + idx);
    }

    int i = 0;
    for (auto rec : tree) {
        TEST(i++ == rec.key());
    }

    manager.~BufferManager();
    file.~FileManager();
    remove("testfile");
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
        && BPTreeIteratorTest::deref_operator_test()
        && BPTreeIteratorTest::integrate_test();
}