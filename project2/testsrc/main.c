#include "test.h"

TEST_SUITE(integrate, {
    TEST(fileio_test());
    TEST(headers_test());
    TEST(disk_manager_test());
})

int main() {
    integrate_test();
}
