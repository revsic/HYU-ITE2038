#include "headers.h"

int max(int a, int b) {
    return a > b ? a : b;
}

struct page_header_t* page_header(struct page_t* page) {
    return &page->header.page_header;
}

struct free_page_t* free_page(struct page_t* page) {
    return &page->header.free_page.header;
}

struct record_t* records(struct page_t* page) {
    return page->content.records;
}

struct internal_t* entries(struct page_t* page) {
    return page->content.entries;
}
