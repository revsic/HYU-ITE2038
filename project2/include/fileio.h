#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>

int fexist(char* filename);

int fpwrite(void* ptr, size_t size, long pos, FILE* stream);

#endif