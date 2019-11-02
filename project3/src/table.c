#include "table.h"

int table_init(struct table_t* table) {
    table->id = INVALID_TABLENUM;
    return SUCCESS;
}

int table_copy(struct table_t* dst, struct table_t* src) {
    *dst = *src;
    dst->bpt.file = &dst->file;
    return SUCCESS;
}

int table_load(struct table_t* table,
               const char* filename,
               struct buffer_manager_t* buffers)
{
    CHECK_SUCCESS(file_open(&table->file, filename));
    CHECK_SUCCESS(bpt_init(&table->bpt, &table->file, buffers));
    table->id = table_id_from_filenum(table->file.id);
    return SUCCESS;
}

int table_release(struct table_t* table) {
    table->id = INVALID_TABLENUM;
    CHECK_SUCCESS(bpt_release(&table->bpt));
    CHECK_SUCCESS(file_close(&table->file));
    return SUCCESS;
}

int table_find(struct table_t* table, prikey_t key, struct record_t* record) {
    return bpt_find(&table->bpt, key, record);
}

int table_find_range(struct table_t* table, prikey_t start, prikey_t end, struct record_vec_t* vec) {
    return bpt_find_range(&table->bpt, start, end, vec);
}

int table_insert(struct table_t* table, prikey_t key, uint8_t* value, int value_size) {
    return bpt_insert(&table->bpt, key, value, value_size);
}

int table_delete(struct table_t* table, prikey_t key) {
    return bpt_delete(&table->bpt, key);
}

tablenum_t table_id_from_filenum(filenum_t filenum) {
    return (tablenum_t)filenum;
}

tablenum_t table_rehash(struct table_t* table, int update_id) {
    int i;
    unsigned long hash = 0;
    uint8_t* bytes = (uint8_t*)&table->id;
    for (i = 0; i < sizeof(tablenum_t); ++i) {
        hash = bytes[i] + (hash << 6) + (hash << 16) - hash;
    }

    if (update_id) {
        table->id = hash;
        table->file.id = hash;
    }

    return hash;
}
