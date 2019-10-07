#include "fileio.h"

#if defined(__GNUC__)
    #include <unistd.h>
#elif defined(_MSC_VER)
#endif

int fexist(char* filename) {
#if defined(__GNUC__)
    return access(filename, F_OK) != -1;
#elif defined(_MSC_VER)
#endif
}

int fpwrite(const void* ptr, size_t size, long pos, FILE* stream) {
    long cur = ftell(stream);
    if (pos == -1) {
        fseek(stream, 0, SEEK_END);
    } else {
        fseek(stream, pos, SEEK_SET);
    }
    fwrite(ptr, size, 1, stream);
    fseek(stream, cur, SEEK_SET);
    return 1;
}
