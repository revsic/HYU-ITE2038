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
    BufferManager manager(5);
    FileManager file("testfile");

    // BPTree bpt(&file, &buffers);

    Defer deferer([&] {
        manager.shutdown();
        file.~FileManager();
        remove("testfile");
    });

    int idx = manager.load(file, FILE_HEADER_PAGENUM);
    TEST(idx == 0);

    idx = manager.load(file, 1, true);
    TEST(idx == 1);

    TEST_SUCCESS(manager.release_block(1));
    TEST(manager.num_buffer == 1);
    TEST(manager.lru == manager.buffers[0]);
    TEST(manager.mru == manager.buffers[0]);
    TEST(manager.buffers[0]->next_use == nullptr);
    TEST(manager.buffers[0]->prev_use == nullptr);
}