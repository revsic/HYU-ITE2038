#ifndef UTILITY_H
#define UTILITY_H

#include "utility.h"

int max(int a, int b);

struct page_header_t* page_header(struct page_t* page);

struct free_page_t* free_page(struct page_t* page);

struct record_t* records(struct page_t* page);

struct internal_t* entries(struct page_t* page);

#endif