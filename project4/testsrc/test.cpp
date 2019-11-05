#include "test.hpp"

TEST_SUITE(unit, {
    TEST(headers_test());
})

int main() {
    remove("testfile");
    unit_test();
    return 0;
}
