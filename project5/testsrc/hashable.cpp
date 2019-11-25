#include "test.hpp"
#include "hashable.hpp"

TEST_SUITE(single_inst, {
    using single_t = HashablePack<int>;
    int a = 0xdeadbeef;
    char const* casted = reinterpret_cast<char const*>(&a);
    std::string str(casted, casted + sizeof(a));
    TEST(std::hash<std::string>{}(str) == std::hash<single_t>{}(single_t(a)));
})

int hashable_test() {
    return single_inst_test();
};