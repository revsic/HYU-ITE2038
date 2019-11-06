#include <unistd.h>

#include "fileio.hpp"

int fexist(const char* filename) {
    return access(filename, F_OK) != -1;
}

long fsize(FILE* fp) {
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

int fresize(FILE* fp, size_t size) {
    int fd = fileno(fp);
    return ftruncate(fd, size) == 0;
}

int fpwrite(const void* ptr, size_t size, long pos, FILE* stream) {
    fseek(stream, pos, SEEK_SET);
    int retval = fwrite(ptr, size, 1, stream);
    fflush(stream);
    // for synchronizing kernel level caching
    fsync(fileno(stream));
    return retval == 1;
}

int fpread(void* ptr, size_t size, long pos, FILE* stream) {
    fseek(stream, pos, SEEK_SET);
    return fread(ptr, size, 1, stream) == 1;
}
