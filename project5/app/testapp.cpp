#include <functional>
#include <vector>
#include "buffer_manager.hpp"
#include "bptree.hpp"

#define TEST(expr) if(!(expr)) { printf("%s, line %d: err\n", __FILE__, __LINE__); return 0; }
#define TEST_STATUS(expr) if(!(expr)) { printf("%s, line %d: err\n", __FILE__, __LINE__); return Status::FAILURE; }
#define TEST_SUCCESS(val) TEST(val == Status::SUCCESS);

struct Defer {
    std::function<void()> callback;

    template <typename F>
    Defer(F&& callback) : callback(std::forward<F>(callback)) { }

    ~Defer() {
        callback();
    }
};

int main() {
    FileManager file("testfile");
    BufferManager buffers(4);
    BPTree bpt(&file, &buffers);

    Defer deferer([&] {
        buffers.shutdown();
        file.~FileManager();
        remove("testfile");
    });

    Ubuffer node = bpt.create_page(true);
    Ubuffer filehdr = bpt.buffering(FILE_HEADER_PAGENUM);
    node.write_void([&](Page& page) {
        page.page_header().number_of_keys = 1;
        page.records()[0].key = 10;
        return Status::SUCCESS;
    });
    filehdr.write_void([&](Page& page) {
        page.file_header().root_page_number = node.to_pagenum();
    });

    TEST_SUCCESS(bpt.shrink_root());
    TEST_SUCCESS(filehdr.read([&](Page const& page) {
        TEST_STATUS(page.file_header().root_page_number == node.to_pagenum());
        return Status::SUCCESS;
    }))

    TEST_SUCCESS(node.read([&](Page const& page) {
        TEST_STATUS(page.page_header().number_of_keys == 1);
        TEST_STATUS(page.records()[0].key == 10);
        return Status::SUCCESS;
    }))

    node.write_void([&](Page& page) {
        page.page_header().number_of_keys = 0;
    });

    TEST_SUCCESS(bpt.shrink_root());
    TEST_SUCCESS(filehdr.read([&](Page const& page) {
        TEST_STATUS(page.file_header().root_page_number == INVALID_PAGENUM);
        return Status::SUCCESS;
    }))

    // case 2. internal root
    node = bpt.create_page(true);
    pagenum_t nodenum = node.to_pagenum();

    node = bpt.create_page(false);
    filehdr.write_void([&](Page& page) {
        page.file_header().root_page_number = node.to_pagenum();
    });
    node.write_void([&](Page& page) {
        page.page_header().special_page_number = nodenum;
    });

    TEST_SUCCESS(bpt.shrink_root());
    TEST_SUCCESS(filehdr.read([&](Page const& page) {
        TEST_STATUS(page.file_header().root_page_number == nodenum);
        return Status::SUCCESS;
    }))
}