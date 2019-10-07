#include "fileio.h"
#include "test.h"

TEST_SUITE(fexist, {
    TEST(!fexist("testfile"));

    fclose(fopen("testfile", "w"));
    TEST(fexist("testfile"));

    remove("testfile");
    TEST(!fexist("testfile"));
})

TEST_SUITE(fsize, {
    FILE* fp = fopen("testfile", "w");
    fwrite("asdf", 1, 4, fp);

    TEST(fsize(fp) == 4);

    fclose(fp);
    remove("testfile");

})

TEST_SUITE(fresize, {
    FILE* fp = fopen("testfile", "w");

    fresize(fp, 10);
    TEST(fsize(fp) == 10);

    fclose(fp);
    remove("testfile");
})

TEST_SUITE(fpwrite, {

})

TEST_SUITE(fpread, {

})

int fileio_test() {
    return fexist_test()
        && fsize_test()
        && fresize_test()
        && fpwrite_test()
        && fpread_test();
}