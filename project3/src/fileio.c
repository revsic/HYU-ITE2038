#include "fileio.h"

#if defined(__GNUC__)
    #include <unistd.h>
#elif defined(_MSC_VER)
#endif

int fexist(const char* filename) {
#if defined(__GNUC__)
    return access(filename, F_OK) != -1;
#elif defined(_MSC_VER)
#endif
}

long fsize(FILE* fp) {
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

int fresize(FILE* fp, size_t size) {
#if defined(__GNUC__)
    int fd = fileno(fp);
    return ftruncate(fd, size) == 0;
#elif defined(_MSC_VER)
#endif
}

int fpwrite(const void* ptr, size_t size, long pos, FILE* stream) {
    fseek(stream, pos, SEEK_SET);
    int retval = fwrite(ptr, size, 1, stream);
    fflush(stream);
#if defined(__GNUC__)
    // for synchronizing kernel level caching
    fsync(fileno(stream));
#endif
    return retval == 1;
}

int fpread(void* ptr, size_t size, long pos, FILE* stream) {
    fseek(stream, pos, SEEK_SET);
    return fread(ptr, size, 1, stream) == 1;
}
