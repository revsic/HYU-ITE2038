#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>

/// Check whether given file exists.
/// \param filename const char*, name of file.
/// \return int, whether exist (= 1) or not (= 0).
int fexist(const char* filename);

/// Return a size of the file.
/// \param fp FILE*, file pointer.
/// \return long, size of the file.
long fsize(FILE* fp);

/// Resize the file with given size.
/// \param fp FILE*, file pointer.
/// \param size size_t, expected size.
/// \return int, whether success (= 1) or not (= 0).
int fresize(FILE* fp, size_t size);

/// Write the data to stream in given position.
/// \param ptr void*, data.
/// \param size size_t, size of the data.
/// \param pos long, offset from SEEK_SET (start of the file).
/// \param stream FILE*, file pointer.
/// \return int, whether success (= 1) or not (= 0).
int fpwrite(const void* ptr, size_t size, long pos, FILE* stream);


/// Read the data from stream in given position, given size.
/// \param ptr void*, memory to return data.
/// \param size size_t, size of the data.
/// \param pos long, offset from SEEK_SET (start of the file).
/// \param stream FILE*, file pointer.
/// \return int, whether success (= 1) or not (= 0).
int fpread(void* ptr, size_t size, long pos, FILE* stream);

#endif
