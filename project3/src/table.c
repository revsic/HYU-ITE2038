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
    return file_open(&table->file_manager, filename);
}

int table_release(struct table_t* table) {
    table->table_id = INVALID_TABLENUM;
    return file_close(&table->file_manager);
}
