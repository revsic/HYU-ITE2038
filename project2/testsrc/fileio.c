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
    FILE* fp = fopen("testfile", "w");

    fresize(fp, 10);
    fpwrite("01234", 5, 3, fp);

    fclose(fp);
    fp = fopen("testfile", "r");

    char arr[10];
    fread(arr, 1, 10, fp);

    int i;
    for (i = 0; i < 3; ++i) {
        TEST(arr[i] == 0);
    }

    for (i = 0; i < 5; ++i) {
        TEST(arr[3 + i] == i + '0');
    }

    for (i = 8; i < 10; ++i) {
        TEST(arr[i] == 0);
    }

    fclose(fp);
    remove("testfile");
})

TEST_SUITE(fpread, {
    FILE* fp = fopen("testfile", "w+");

    fresize(fp, 10);
    fpwrite("01234", 5, 3, fp);

    char arr[5];
    fpread(arr, 5, 3, fp);

    int i;
    for (i = 0; i < 5; ++i) {
        TEST(arr[i] == i + '0');
    }

    fclose(fp);
    remove("testfile");
})

int fileio_test() {
    return fexist_test()
        && fsize_test()
        && fresize_test()
        && fpwrite_test()
        && fpread_test();
}