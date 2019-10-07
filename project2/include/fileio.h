#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>

int fexist(char* filename);

long fsize(FILE* fp);

int fresize(FILE* fp, size_t size);

int fpwrite(const void* ptr, size_t size, long pos, FILE* stream);

int fpread(void* ptr, size_t size, long pos, FILE* stream);

#endif