#include <vector>
#include "buffer_manager.hpp"

int main() {
    BufferManager manager(5);
    FileManager file("testfile");

    std::vector<int> indices;
    for (int i = 0; i < 13; ++i) {
        Ubuffer ubuf = manager.new_page(file);

        // TEST(std::find(indices.begin(), indices.end(), ubuf.pagenum)
        //     == indices.end());
        indices.push_back(ubuf.to_pagenum());
    }
    // TEST_SUCCESS(manager.shutdown());
    manager.shutdown();
    file.~FileManager();
    remove("testfile");
}