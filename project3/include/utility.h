#ifndef UTILITY_H
#define UTILITY_H

#include "headers.h"

/// Return maximum value.
/// \param a int, integer value.
/// \param b int, integer value.
/// \return int, maximum value between two arguments.
int max(int a, int b);

/// Return page header from page structure.
/// \param page struct page_t*, page structure.
/// \return struct page_header_t*, page header.
struct page_header_t* page_header(struct page_t* page);

/// Return free page header from page structure.
/// \param page struct page_t*, page structure.
/// \return struct free_page_t*, free page header.
struct free_page_t* free_page(struct page_t* page);

/// Return records array from page structure.
/// \param page struct page_t*, page structure.
/// \return struct record_t*, page header.
struct record_t* records(struct page_t* page);

/// Return entries array from page structure.
/// \param page struct page_t*, page structure.
/// \return struct internal_t*, page header.
struct internal_t* entries(struct page_t* page);

#endif
