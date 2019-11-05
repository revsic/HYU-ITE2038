#include "headers.hpp"

result_t page_t::init(uint32_t leaf) {
    page_header_t* header = page_header();
    header->is_leaf = leaf;
    header->number_of_keys = 0;
    header->parent_page_number = INVALID_PAGENUM;
    header->special_page_number = INVALID_PAGENUM;
    return result_t::SUCCESS;
}

file_header_t* page_t::file_header() {
    return &impl.file.header;
}

const file_header_t* page_t::file_header() const {
    return &impl.file.header;
}

page_header_t* page_t::page_header() {
    return &impl.node.header.page_header;
}

const page_header_t* page_t::page_header() const {
    return &impl.node.header.page_header;
}

free_page_t* page_t::free_page() {
    return &impl.node.header.free_page.header;
}

const free_page_t* page_t::free_page() const {
    return &impl.node.header.free_page.header;
}

record_t* page_t::records() {
    return impl.node.content.records;
}

const record_t* page_t::records() const {
    return impl.node.content.records;
}

internal_t* page_t::entries() {
    return impl.node.content.entries;
}

const internal_t* page_t::entries() const {
    return impl.node.content.entries;
}
