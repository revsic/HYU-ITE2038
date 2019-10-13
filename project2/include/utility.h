#ifndef UTILITY_H
#define UTILITY_H

#include "headers.h"

/// Return maximum value.
/// Args:
///     a: int, integer value.
///     b: int, integer value.
/// Returns:
///     int, maximum value between two arguments.
int max(int a, int b);

/// Return page header from page structure.
/// Args:
///     page: struct page_t*, page structure.
/// Returns:
///     struct page_header_t*, page header.
struct page_header_t* page_header(struct page_t* page);

/// Return free page header from page structure.
/// Args:
///     page: struct page_t*, page structure.
/// Returns:
///     struct free_page_t*, free page header.
struct free_page_t* free_page(struct page_t* page);

/// Return records array from page structure.
/// Args:
///     page: struct page_t*, page structure.
/// Returns:
///     struct record_t*, page header.
struct record_t* records(struct page_t* page);

/// Return entries array from page structure.
/// Args:
///     page: struct page_t*, page structure.
/// Returns:
///     struct internal_t*, page header.
struct internal_t* entries(struct page_t* page);

#endif