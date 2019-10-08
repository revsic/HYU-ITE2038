#include "utility.h"
#include "test.h"

TEST_SUITE(max, {
    TEST(max(1, 2) == 2);
})

int utility_test() {
    return max_test();
}
