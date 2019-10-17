#include "headers.h"
#include "utility.h"

int max(int a, int b) {
    return a > b ? a : b;
}

struct file_header_t* file_header(struct page_t* page) {
    return &page->impl.file.header;
}

struct page_header_t* page_header(struct page_t* page) {
    return &page->impl.node.header.page_header;
}

struct free_page_t* free_page(struct page_t* page) {
    return &page->impl.node.header.free_page.header;
}

struct record_t* records(struct page_t* page) {
    return page->impl.node.content.records;
}

struct internal_t* entries(struct page_t* page) {
    return page->impl.node.content.entries;
}
