#include "test.hpp"

TEST_SUITE(unit, {
    TEST(fileio_test());
    TEST(headers_test());
})

int main() {
    remove("testfile");
    unit_test();
    return 0;
}
