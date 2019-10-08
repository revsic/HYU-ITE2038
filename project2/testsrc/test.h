#include <stdio.h>

#define TEST(expr)                                      \
if(!(expr)) {                                           \
    printf("%s, line %d: err\n", __FILE__, __LINE__);   \
    return 0;                                           \
}

#define TEST_SUIT(name, content)                           \
int name##_test() {                                         \
    content;                                                \
    printf("[*] %s: " #name " test success\n", __FILE__);   \
    return 1;                                               \
}

int fileio_test();
int headers_test();
int disk_manager_test();
