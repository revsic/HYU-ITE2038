#include "utils.hpp"
#include "test.hpp"

TEST_SUITE(defer, {
    bool flag = false;
    {
        auto defer = utils::defer([&] {
            flag = true;
        });
    }
    TEST(flag);
})

int utils_test() {
    return defer_test();
}
