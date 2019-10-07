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
