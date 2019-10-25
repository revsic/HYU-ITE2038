#include "table.h"

// sdbm
tablenum_t create_tablenum(const char* filename) {
    char c;
    unsigned long hash = 0;
    while ((c = *filename++)) {
        if (c == '/' || c == '\\') {
            hash = 0;
            continue;
        }
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return (tablenum_t)hash;
}

// sdbm
tablenum_t rehash_tablenum(tablenum_t tablenum) {
    int i;
    unsigned long hash = 0;
    uint8_t* bytes = (uint8_t*)&tablenum;
    for (i = 0; i < sizeof(tablenum_t); ++i) {
        hash = bytes[i] + (hash << 6) + (hash << 16) - hash;
    }
    return (tablenum_t)hash;
}

int table_init(struct table_t* table) {
    table->table_id = INVALID_TABLENUM;
    return SUCCESS;
}

int table_load(struct table_t* table, const char* filename) {
    table->table_id = create_tablenum(filename);
    return file_open(filename, &table->file_manager);
}

int table_release(struct table_t* table) {
    table->table_id = INVALID_TABLENUM;
    return file_close(&table->file_manager);
}

pagenum_t table_create_page(struct table_t* table) {
    return page_create(&table->file_manager);
}

int table_free_page(struct table_t* table, pagenum_t pagenum) {
    return page_free(pagenum, &table->file_manager);
}

int table_read_page(struct table_t* table, pagenum_t pagenum, struct page_t* dst) {
    return page_read(pagenum, &table->file_manager, dst);
}

int table_write_page(struct table_t* table, pagenum_t pagenum, struct page_t* src) {
    return page_write(pagenum, &table->file_manager, src);
}