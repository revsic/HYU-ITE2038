#include "test.hpp"
#include "hashable.hpp"

TEST_SUITE(single_inst, {
    using single_t = HashablePack<int>;
    int a = 0xdeadbeef;
    char const* casted = reinterpret_cast<char const*>(&a);
    std::string str(casted, casted + sizeof(a));
    TEST(std::hash<std::string>{}(str) == std::hash<single_t>{}({pack_init, a}));
})

using multi_t = HashablePack<char, int32_t, uint64_t>;
TEST_SUITE(multi_inst, {
    char a = 'a';
    int32_t b = 0xdeadbeef;
    uint64_t c = 0x12345678abcdef;

    char const* casted = reinterpret_cast<char const*>(&a);
    std::string str(casted, casted + sizeof(char));

    casted = reinterpret_cast<char const*>(&b);
    str.insert(str.end(), casted, casted + sizeof(int32_t));

    casted = reinterpret_cast<char const*>(&c);
    str.insert(str.end(), casted, casted + sizeof(uint64_t));

    TEST(std::hash<std::string>{}(str) == std::hash<multi_t>{}({pack_init, a, b, c}));
})

int hashable_test() {
    return single_inst_test()
        && multi_inst_test();
};