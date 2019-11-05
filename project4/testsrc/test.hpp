#include <iostream>

#include "headers.hpp"

#define TEST(expr)                                      \
if(!(expr)) {                                           \
    printf("%s, line %d: err\n", __FILE__, __LINE__);   \
    return 0;                                           \
}

#define TEST_SUCCESS(val) TEST(val == result_t::SUCCESS);

#define TEST_SUITE(name, content)                           \
int name##_test() {                                         \
    content;                                                \
    printf("[*] %s: " #name " test success\n", __FILE__);   \
    return 1;                                               \
}

int headers_test();