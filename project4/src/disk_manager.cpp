// #include "disk_manager.hpp"

// file_manager_t::file_manager_t() {
//     EXIT_ON_FALSE(fresize(fp, PAGE_SIZE));
//     // zero-initialization
//     FileHeader file_header;
//     file_header.free_page_number = 0;
//     file_header.root_page_number = 0;
//     file_header.number_of_pages = 0;
//     // write file header
//     EXIT_ON_FALSE(fpwrite(&file_header, sizeof(FileHeader), 0, fp));
// }

// file_manager_t::file_manager_t(std::string const& filename) {

// }

// file_manager_t::~file_manager_t() {

// }

// pagenum_t file_manager_t::page_create() const {

// }

// status_t file_manager_t::page_free(pagenum_t pagenum) const {

// }

// status_t file_manager_t::page_read(pagenum_t pagenum, Page* dst) const {

// }

// status_t file_manager_t::page_write(pagenum_t pagenum, Page* src) const {

// }
