#include <stdio.h>

#define TEST(expr)                                      \
if(!(expr)) {                                           \
    printf("%s, line %d: err\n", __FILE__, __LINE__);   \
    return 0;                                           \
}

#define TEST_SUITE(name, content)                           \
int name##_test() {                                         \
    content;                                                \
    printf("[*] %s: " #name " test success\n", __FILE__);   \
    return 1;                                               \
}

int fileio_test();
int headers_test();
// int utility_test();
int disk_manager_test();
// int table_manager_test();
// int buffer_manager_test();
// int bpt_test();
// int dbms_test();
// int dbapi_test();
