#include "headers.hpp"

Status Page::init(uint32_t leaf) {
    PageHeader& header = page_header();
    header.is_leaf = leaf;
    header.number_of_keys = 0;
    header.parent_page_number = INVALID_PAGENUM;
    header.special_page_number = INVALID_PAGENUM;
    return Status::SUCCESS;
}

FileHeader& Page::file_header() {
    return impl.file.header;
}

FileHeader const& Page::file_header() const {
    return impl.file.header;
}

PageHeader& Page::page_header() {
    return impl.node.header.page_header;
}

PageHeader const& Page::page_header() const {
    return impl.node.header.page_header;
}

FreePageHeader& Page::free_page() {
    return impl.node.header.free_page.header;
}

FreePageHeader const& Page::free_page() const {
    return impl.node.header.free_page.header;
}

Record* Page::records() {
    return impl.node.content.records;
}

Record const* Page::records() const {
    return impl.node.content.records;
}

Internal* Page::entries() {
    return impl.node.content.entries;
}

const Internal* Page::entries() const {
    return impl.node.content.entries;
}
