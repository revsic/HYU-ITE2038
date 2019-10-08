#ifndef UTILITY_H
#define UTILITY_H

#include "headers.h"

inline int max(int a, int b) {
    return a > b ? a : b;
}

inline struct page_header_t* page_header(struct page_t* page) {
    return &page->header.page_header;
}

inline struct free_page_t* free_page(struct page_t* page) {
    return &page->header.free_page.header;
}

inline struct record_t* records(struct page_t* page) {
    return page->content.records;
}

inline struct internal_t* entries(struct page_t* page) {
    return page->content.entries;
}

#endif