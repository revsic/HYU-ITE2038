#include "buffer_manager.hpp"
#include "disk_manager.hpp"
#include "test.hpp"

struct BufferTest {
    static int page_test();
    static int start_end_test();
    static int init_test();
    static int load_test();
    static int new_page_test();
    static int link_neighbor_test();
    static int append_mru_test();
    static int release_test();
};

struct UbufferTest {
    static int constructor_test();
    static int page_test();
    static int reload_test();
    static int check_test();
    static int safe_pagenum_test();
};

struct BufferManagerTest {
    static int constructor_test();
    static int destructor_test();
    static int shutdown_test();
    static int buffering_test();
    static int new_page_test();
    static int free_page_test();
    static int alloc_test();
    static int load_test();
    static int release_block_test();
    static int release_file_test();
    static int release_test();
    static int find_test();
};

TEST_SUITE(UbufferTest::constructor, {
    // TODO: impl test
})

TEST_SUITE(UbufferTest::reload, {
    // TODO: impl test
})

TEST_SUITE(UbufferTest::check, {
    // TODO impl test
})

TEST_SUITE(UbufferTest::page, {
    Buffer buf;
    Ubuffer ubuf(&buf, INVALID_PAGENUM, nullptr);
    TEST(&ubuf.page() == &buf.page());
})

TEST_SUITE(UbufferTest::safe_pagenum, {
    // TODO: impl test
})

TEST_SUITE(BufferTest::page, {
    Buffer buf;
    TEST(&buf.page() == &buf.frame);
})

TEST_SUITE(BufferTest::init, {
    Buffer buf;
    TEST_SUCCESS(buf.init(10, nullptr));

    TEST(buf.pagenum == INVALID_PAGENUM);
    TEST(buf.is_dirty == false);
    TEST(buf.pin == 0);
    TEST(buf.prev_use == -1);
    TEST(buf.next_use == -1);
    TEST(buf.block_idx == 10);
    TEST(buf.file == nullptr);
    TEST(buf.manager == nullptr);
})

TEST_SUITE(BufferTest::load, {
    Buffer buf;
    FileManager file("testfile");
    TEST_SUCCESS(buf.init(10, nullptr));

    pagenum_t pagenum = file.page_create();
    TEST_SUCCESS(buf.load(file, pagenum));

    TEST(buf.pagenum == pagenum);
    TEST(buf.file == &file);

    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferTest::new_page, {
    Buffer buf;
    FileManager file("testfile");
    TEST_SUCCESS(buf.init(10, nullptr));
    TEST_SUCCESS(buf.new_page(file));

    TEST(buf.pagenum != INVALID_PAGENUM);
    TEST(buf.file == &file);

    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferTest::link_neighbor, {
    BufferManager manager(5);

    // case 0. only node
    manager.lru = 0;
    manager.mru = 0;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = -1;

    TEST_SUCCESS(manager.buffers[0].link_neighbor());
    TEST(manager.lru == -1);
    TEST(manager.mru == -1);

    // case 1. least recentrly used node
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;

    TEST_SUCCESS(manager.buffers[0].link_neighbor());
    TEST(manager.lru == 1);
    TEST(manager.mru == 2);
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == 2);

    // case 2. most recently used node
    manager.lru = 2;
    manager.mru = 0;
    manager.buffers[0].prev_use = 1;
    manager.buffers[0].next_use = -1;
    manager.buffers[1].prev_use = 2;
    manager.buffers[1].next_use = 0;

    TEST_SUCCESS(manager.buffers[0].link_neighbor());
    TEST(manager.mru == 1);
    TEST(manager.lru == 2);
    TEST(manager.buffers[1].next_use == -1);
    TEST(manager.buffers[1].prev_use == 2);

    // case 3. middle node
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    TEST_SUCCESS(manager.buffers[1].link_neighbor());
    TEST(manager.mru == 2);
    TEST(manager.lru == 0);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == -1);

    // case 4. unconnected node
    // undefined behaviour
})

TEST_SUITE(BufferTest::append_mru, {
    BufferManager manager(5);

    // case 1. first node
    manager.lru = -1;
    manager.mru = -1;
    manager.buffers[0].prev_use = -1;
    manager.buffers[1].next_use = -1;
    TEST_SUCCESS(manager.buffers[0].append_mru(false));

    TEST(manager.lru == 0);
    TEST(manager.mru == 0);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == -1);

    // case 2. append
    manager.lru = 0;
    manager.mru = 1;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = -1;
    TEST_SUCCESS(manager.buffers[2].append_mru(false));

    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 1);
    TEST(manager.buffers[1].prev_use == 0);
    TEST(manager.buffers[1].next_use == 2);
    TEST(manager.buffers[2].prev_use == 1);
    TEST(manager.buffers[2].next_use == -1);
})

TEST_SUITE(BufferTest::release, {
    BufferManager manager(5);

    Buffer& target = manager.buffers[1];

    FileManager file("testfile");
    TEST_SUCCESS(target.init(1, &manager));
    TEST_SUCCESS(target.load(file, FILE_HEADER_PAGENUM));

    // case 1. not dirty
    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    TEST_SUCCESS(target.release());
    // linkage
    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == -1);
    // block info
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == -1);
    TEST(manager.buffers[1].block_idx == 1);
    TEST(manager.buffers[1].manager == &manager);
    // file check
    TEST_SUCCESS(target.load(file, FILE_HEADER_PAGENUM));
    TEST(target.page().file_header().free_page_number == 0);
    TEST(target.page().file_header().root_page_number == INVALID_PAGENUM);
    TEST(target.page().file_header().number_of_pages == 0);

    // case 2. dirty
    Ubuffer(&target, target.pagenum, target.file).use(
        RWFlag::WRITE, [](Ubuffer& buf) {
            buf.page().file_header().root_page_number = 1;
            buf.page().file_header().number_of_pages = 20;
        });

    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    TEST_SUCCESS(target.release());
    // linkage
    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == -1);
    // block info
    TEST(manager.buffers[1].prev_use == -1);
    TEST(manager.buffers[1].next_use == -1);
    TEST(manager.buffers[1].block_idx == 1);
    TEST(manager.buffers[1].manager == &manager);
    // file check
    TEST_SUCCESS(target.load(file, FILE_HEADER_PAGENUM));
    TEST(target.page().file_header().free_page_number == 0);
    TEST(target.page().file_header().root_page_number == 1);
    TEST(target.page().file_header().number_of_pages == 20);

    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferTest::start_end, {
    BufferManager manager(5);

    manager.num_buffer = 3;
    manager.lru = 0;
    manager.mru = 2;
    manager.buffers[0].prev_use = -1;
    manager.buffers[0].next_use = 1;
    manager.buffers[1].prev_use = 0;
    manager.buffers[1].next_use = 2;
    manager.buffers[2].prev_use = 1;
    manager.buffers[2].next_use = -1;

    Buffer* target = &manager.buffers[1];

    TEST_SUCCESS(target->start_use(RWFlag::READ));
    TEST(target->pin == 1);

    TEST_SUCCESS(target->end_use(RWFlag::READ));
    TEST(target->pin == 0);
    TEST(target->next_use == -1);
    TEST(target->prev_use == 2);
    TEST(manager.lru == 0);
    TEST(manager.mru == 1);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 2);
    TEST(manager.buffers[2].prev_use == 0);
    TEST(manager.buffers[2].next_use == 1);

    target = &manager.buffers[2];
    TEST_SUCCESS(target->start_use(RWFlag::WRITE));
    TEST(target->pin == -1);

    TEST_SUCCESS(target->end_use(RWFlag::WRITE));
    TEST(target->pin == 0);
    TEST(target->next_use == -1);
    TEST(target->prev_use == 1);
    TEST(manager.lru == 0);
    TEST(manager.mru == 2);
    TEST(manager.buffers[0].prev_use == -1);
    TEST(manager.buffers[0].next_use == 1);
    TEST(manager.buffers[1].prev_use == 0);
    TEST(manager.buffers[1].next_use == 2);
})

TEST_SUITE(BufferManagerTest::constructor, {
    BufferManager manager(5);

    TEST(manager.capacity == 5);
    TEST(manager.num_buffer == 0);
    TEST(manager.lru == -1);
    TEST(manager.mru == -1);
    TEST(manager.buffers != NULL);

    int i;
    for (i = 0; i < 5; ++i) {
        TEST(manager.buffers[i].file == nullptr);
    }
})

TEST_SUITE(BufferManagerTest::shutdown, {
    BufferManager manager(5);
    FileManager file("testfile");

    for (int i = 0; i < 3; ++i) {
        manager.load(file, FILE_HEADER_PAGENUM);
    }

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();

    TEST(manager.num_buffer == 0);
    TEST(manager.capacity == 0);
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::alloc, {
    BufferManager manager(5);
    FileManager file;

    for (int i = 0; i < 13; ++i) {
        TEST(i % 5 == manager.alloc());
        manager.buffers[i % 5].file = &file;
        TEST_SUCCESS(manager.buffers[i % 5].append_mru(false));
    }

    for (int i = 0; i < 5; ++i) {
        manager.buffers[i].file = nullptr;
    }
})

TEST_SUITE(BufferManagerTest::load, {
    BufferManager manager(5);
    FileManager file("testfile");

    int idx = manager.load(file, FILE_HEADER_PAGENUM);
    TEST(idx == 0);
    TEST(manager.num_buffer == 1);
    TEST(manager.mru == 0);
    TEST(manager.lru == 0);

    Buffer* buffer = &manager.buffers[idx];
    TEST(buffer->block_idx == 0);
    TEST(buffer->prev_use == -1);
    TEST(buffer->next_use == -1);
    TEST(buffer->pagenum == FILE_HEADER_PAGENUM);

    idx = manager.load(file, FILE_HEADER_PAGENUM);
    TEST(idx == 1);
    TEST(manager.num_buffer == 2);
    TEST(manager.mru == 1);
    TEST(manager.lru == 0);
    TEST(buffer->prev_use == -1);
    TEST(buffer->next_use == 1);

    buffer = &manager.buffers[idx];
    TEST(buffer->block_idx == 1);
    TEST(buffer->prev_use == 0);
    TEST(buffer->next_use == -1);

    manager.shutdown();
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::release_block, {
    BufferManager manager(5);
    FileManager file("testfile");

    int idx = manager.load(file, FILE_HEADER_PAGENUM);
    TEST(idx == 0);

    idx = manager.load(file, FILE_HEADER_PAGENUM);
    TEST(idx == 1);

    TEST_SUCCESS(manager.release_block(1));
    TEST(manager.num_buffer == 1);
    TEST(manager.lru == 0);
    TEST(manager.mru == 0);
    TEST(manager.buffers[0].next_use == -1);
    TEST(manager.buffers[0].prev_use == -1);

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::release_file, {
    BufferManager manager(5);

    FileManager file1("testfile");
    FileManager file2("testfile2");

    TEST(0 == manager.load(file1, FILE_HEADER_PAGENUM));

    TEST(1 == manager.load(file2, FILE_HEADER_PAGENUM));
    TEST(2 == manager.load(file2, FILE_HEADER_PAGENUM));

    TEST_SUCCESS(manager.release_file(file2.get_id()));
    TEST(manager.num_buffer == 1);
    TEST(manager.buffers[0].file == &file1);
    TEST(manager.buffers[1].file == nullptr);
    TEST(manager.buffers[2].file == nullptr);

    TEST_SUCCESS(manager.shutdown());
    file1.~FileManager();
    file2.~FileManager();
    remove("testfile");
    remove("testfile2");
})

TEST_SUITE(BufferManagerTest::release, {
    BufferManager manager(5);
    FileManager file("testfile");

    for (int i = 0; i < 4; ++i) {
        TEST(-1 != manager.load(file, FILE_HEADER_PAGENUM));
    }
    TEST(manager.lru == 0);

    // case 1. lru
    // 0 -> 1 -> 2 -> 3
    TEST(0 == manager.release(ReleaseLRU::inst()));
    TEST(manager.lru == 1);
    TEST(manager.buffers[1].next_use == 2);

    // case 2. lru is pinned
    // 1 -> 2 -> 3
    manager.buffers[1].pin++;
    TEST(2 == manager.release(ReleaseLRU::inst()));
    TEST(manager.lru == 1);

    // case 3. mru
    // 1 -> 3 -> 0 
    manager.buffers[3].pin++;
    TEST(0 == manager.load(file, FILE_HEADER_PAGENUM));
    TEST(0 == manager.release(ReleaseLRU::inst()));
    TEST(manager.lru == 1);

    // case 1. mru
    // 1 -> 3 -> 0
    manager.buffers[1].pin = 0;
    manager.buffers[3].pin = 0;
    TEST(0 == manager.load(file, FILE_HEADER_PAGENUM));
    TEST(0 == manager.release(ReleaseMRU::inst()));

    // case 2. mru is pinned
    // 1 -> 3 -> 0
    TEST(0 == manager.load(file, FILE_HEADER_PAGENUM));
    manager.buffers[0].pin++;

    TEST(3 == manager.release(ReleaseMRU::inst()));

    // case 3. lru
    // 1 -> 0 -> 2
    TEST(2 == manager.load(file, FILE_HEADER_PAGENUM));
    manager.buffers[2].pin++;

    TEST(1 == manager.release(ReleaseMRU::inst()));

    manager.buffers[0].pin = 0;
    manager.buffers[2].pin = 0;

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::find, {
    BufferManager manager(5);
    FileManager file("testfile");

    pagenum_t pagenum[3];
    for (int i = 0; i < 3; ++i) {
        pagenum[i] = file.page_create();
        TEST(pagenum[i] != INVALID_PAGENUM);
        TEST(i == manager.load(file, pagenum[i]));
    }

    for (int i = 0; i < 3; ++i) {
        TEST(i == manager.find(file.get_id(), pagenum[i]));
    }

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::buffering, {
    BufferManager manager(5);
    FileManager file("testfile");

    pagenum_t pagenum[10];
    for (int i = 0; i < 10; ++i) {
        pagenum[i] = file.page_create();
        TEST(pagenum[i] != INVALID_PAGENUM);

        Ubuffer ubuf = manager.buffering(file, pagenum[i]);
        TEST(ubuf.pagenum == ubuf.buf->pagenum);
        TEST(ubuf.file == ubuf.buf->file);
        TEST(pagenum[i] == ubuf.buf->pagenum);
        TEST(&manager.buffers[i % 5] == ubuf.buf);
    }

    for (int i = 9; i >= 5; --i) {
        Ubuffer ubuf = manager.buffering(file, pagenum[i]);
        TEST(ubuf.pagenum == ubuf.buf->pagenum);
        TEST(ubuf.file == ubuf.buf->file);
        TEST(pagenum[i] == ubuf.buf->pagenum);
        TEST(&manager.buffers[i % 5] == ubuf.buf);
    }

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::new_page, {
    BufferManager manager(5);
    FileManager file("testfile");

    for (int i = 0; i < 13; ++i) {
        Ubuffer ubuf = manager.new_page(file);
        TEST(&manager.buffers[i % 5] == ubuf.buf);
    }

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();
    remove("testfile");
})

TEST_SUITE(BufferManagerTest::free_page, {
    BufferManager manager(5);
    FileManager file("testfile");

    Ubuffer ubuf = manager.new_page(file);
    TEST_SUCCESS(manager.free_page(file, ubuf.buf->pagenum));

    TEST_SUCCESS(manager.shutdown());
    file.~FileManager();
    remove("testfile");
})

int buffer_manager_test() {
    return UbufferTest::constructor_test()
        && UbufferTest::reload_test()
        && UbufferTest::check_test()
        && UbufferTest::page_test()
        && UbufferTest::safe_pagenum_test()
        && BufferTest::page_test()
        && BufferTest::init_test()
        && BufferTest::load_test()
        && BufferTest::new_page_test()
        && BufferTest::link_neighbor_test()
        && BufferTest::append_mru_test()
        && BufferTest::release_test()
        && BufferTest::start_end_test()
        && BufferManagerTest::constructor_test()
        && BufferManagerTest::shutdown_test()
        && BufferManagerTest::alloc_test()
        && BufferManagerTest::load_test()
        && BufferManagerTest::release_block_test()
        && BufferManagerTest::release_file_test()
        && BufferManagerTest::release_test()
        && BufferManagerTest::find_test()
        && BufferManagerTest::buffering_test()
        && BufferManagerTest::new_page_test()
        && BufferManagerTest::free_page_test();
}
