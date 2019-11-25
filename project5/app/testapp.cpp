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

    // case 0. leaf node
    Ubuffer parent = bpt.create_page(false);
    Ubuffer left = bpt.create_page(true);
    Ubuffer right = bpt.create_page(true);
    Ubuffer rightmost = bpt.create_page(true);

    pagenum_t parentnum = parent.to_pagenum();
    TEST_SUCCESS(parent.write_void([&](Page& page) {
        page.page_header().number_of_keys = 2;
        page.page_header().special_page_number = left.to_pagenum();
        page.entries()[0].key = 2;
        page.entries()[0].pagenum = right.to_pagenum();
        page.entries()[1].key = 10;
        page.entries()[1].pagenum = rightmost.to_pagenum();
    }))
    TEST_SUCCESS(left.write_void([&](Page& page) {
        page.page_header().number_of_keys = 2;
        page.page_header().parent_page_number = parentnum;
        page.page_header().special_page_number = right.to_pagenum();
        for (int i = 0; i < 2; ++i) {
            page.records()[i].key = i;
        }
    }))
    TEST_SUCCESS(right.write_void([&](Page& page) {
        page.page_header().number_of_keys = 3;
        page.page_header().parent_page_number = parentnum;
        page.page_header().special_page_number = rightmost.to_pagenum();
        for (int i = 0; i < 3; ++i) {
            page.records()[i].key = 2 + i;
        }
    }))
    
    TEST_SUCCESS(bpt.merge_nodes(
        bpt.buffering(left.to_pagenum()),
        2,
        bpt.buffering(right.to_pagenum()),
        bpt.buffering(parentnum)));

    TEST_SUCCESS(parent.read([&](Page const& page) {
        TEST_STATUS(page.page_header().number_of_keys == 1)
        TEST_STATUS(page.page_header().special_page_number == left.to_pagenum());
        TEST_STATUS(page.entries()[0].key == 10);
        TEST_STATUS(page.entries()[0].pagenum == rightmost.to_pagenum());
        return Status::SUCCESS;
    }))
    TEST_SUCCESS(left.read([&](Page const& page) {
        TEST_STATUS(page.page_header().number_of_keys == 5);
        TEST_STATUS(page.page_header().parent_page_number == parentnum);
        TEST_STATUS(page.page_header().special_page_number == rightmost.to_pagenum());
        for (int i = 0; i < 5; ++i) {
            TEST_STATUS(page.records()[i].key == i);
        }
        return Status::SUCCESS;
    }))

    // case 1. internal node
    for (auto iter : buffers.table) {
        std::cout << std::get<0>(iter.first.data) << ' '
            << std::get<1>(iter.first.data) << ' '
            << iter.second << std::endl;
    }

    parent = bpt.create_page(false);
    left = bpt.create_page(false);
    right = bpt.create_page(false);

    std::cout << parent.to_pagenum() << ' '
        << left.to_pagenum() << ' '
        << right.to_pagenum() << std::endl;
    
    // Ubuffer filehdr = bpt.buffering(FILE_HEADER_PAGENUM);
    for (auto iter : buffers.table) {
        std::cout << std::get<0>(iter.first.data) << ' '
            << std::get<1>(iter.first.data) << ' '
            << iter.second << std::endl;
    }

    rightmost = bpt.create_page(false);

    parentnum = parent.to_pagenum();
    parent.write_void([&](Page& page) {
        page.page_header().number_of_keys = 2;
        page.page_header().parent_page_number = INVALID_PAGENUM;
        page.page_header().special_page_number = left.to_pagenum();
        page.entries()[0].key = 3;
        page.entries()[0].pagenum = right.to_pagenum();
        page.entries()[1].key = 10;
        page.entries()[1].pagenum = rightmost.to_pagenum();
    });

    pagenum_t pagenums[7];
    pagenum_t leftnum = left.to_pagenum();
    left.write_void([&](Page& page) {
        page.page_header().number_of_keys = 2;
        page.page_header().parent_page_number = parent.to_pagenum();
        for (int i = 0; i < 3; ++i) {
            Ubuffer tmp = bpt.create_page(true);
            pagenums[i] = tmp.to_pagenum();
            if (i == 0) {
                page.page_header().special_page_number = pagenums[i];
            } else {
                page.entries()[i - 1].key = i;
                page.entries()[i - 1].pagenum = pagenums[i];
            }

            tmp.write_void([&](Page& tmppage) {
                tmppage.page_header().parent_page_number = leftnum;
            });
        }
    });

    pagenum_t rightnum = right.to_pagenum();
    right.write_void([&](Page& page) {
        page.page_header().number_of_keys = 3;
        page.page_header().parent_page_number = parent.to_pagenum();
        for (int i = 0; i < 4; ++i) {
            Ubuffer tmp = bpt.create_page(true);
            pagenums[3 + i] = tmp.to_pagenum();
            if (i == 0) {
                page.page_header().special_page_number = pagenums[3 + i];
            } else {
                page.entries()[i - 1].key = 3 + i;
                page.entries()[i - 1].pagenum = pagenums[3 + i];
            }

            tmp.write_void([&](Page& tmppage) {
                tmppage.page_header().parent_page_number = rightnum;
            });
        }
    });

    TEST_SUCCESS(bpt.merge_nodes(
        bpt.buffering(leftnum),
        3,
        bpt.buffering(rightnum),
        bpt.buffering(parentnum)));

    TEST_SUCCESS(parent.read([&](Page const& page) {
        TEST_STATUS(page.page_header().parent_page_number == INVALID_PAGENUM);
        TEST_STATUS(page.page_header().number_of_keys == 1);
        TEST_STATUS(page.page_header().special_page_number == left.to_pagenum());
        TEST_STATUS(page.entries()[0].key == 10);
        TEST_STATUS(page.entries()[0].pagenum == rightmost.to_pagenum());
        return Status::SUCCESS;
    }))

    TEST_SUCCESS(left.read([&](Page const& page) {
        TEST_STATUS(page.page_header().number_of_keys == 6);
        TEST_STATUS(page.page_header().parent_page_number == parentnum);
        for (int i = 0; i < 7; ++i) {
            if (i == 0) {
                TEST_STATUS(page.page_header().special_page_number == pagenums[i]);
            } else {
                TEST_STATUS(page.entries()[i - 1].key == i);
                TEST_STATUS(page.entries()[i - 1].pagenum == pagenums[i]);
            }

            Ubuffer tmp = bpt.buffering(pagenums[i]);
            CHECK_SUCCESS(tmp.read([&](Page const& tmppage) {
                TEST_STATUS(tmppage.page_header().parent_page_number == leftnum);
                return Status::SUCCESS;
            }))
        }
        return Status::SUCCESS;
    }))
}