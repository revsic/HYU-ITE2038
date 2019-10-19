#include "bpt.h"
#include "dbms.h"

uint64_t create_checksum(tablenum_t table_id, pagenum_t pagenum) {
    return ((uint64_t)table_id << 32) + pagenum;
}

uint64_t create_checksum_from_uri(struct page_uri_t uri) {
    return create_checksum(uri.table_id, uri.pagenum);
}

int check_ubuffer(struct ubuffer_t* buf) {
    CHECK_NULL(buf);
    uint64_t checksum = create_checksum(buf->buf->table_id, buf->buf->pagenum);
    CHECK_TRUE(buf->checksum == checksum);
    return SUCCESS;
}

struct page_t* from_ubuffer(struct ubuffer_t* buffer) {
    return from_buffer(buffer->buf);
}

int dbms_init(struct dbms_t* dbms, int num_buffer) {
    buffer_manager_init(&dbms->buffers, num_buffer);
    table_manager_init(&dbms->tables);
    return SUCCESS;
}

int dbms_shutdown(struct dbms_t* dbms) {
    buffer_manager_shutdown(&dbms->buffers);
    table_manager_release(&dbms->tables);
    return SUCCESS;
}

tablenum_t dbms_open_table(struct dbms_t* dbms, const char* filename) {
    return table_manager_load(&dbms->tables, filename);
}

int dbms_close_table(struct dbms_t* dbms, tablenum_t table_id) {
    CHECK_SUCCESS(buffer_manager_release_table(&dbms->buffers, table_id));
    CHECK_SUCCESS(table_manager_remove(&dbms->tables, table_id));
    return SUCCESS;
}

struct ubuffer_t dbms_buffering(struct dbms_t* dbms,
                                struct page_uri_t* page_uri)
{
    struct buffer_t* buf = buffer_manager_buffering(&dbms->buffers, &dbms->tables, page_uri);
    struct ubuffer_t ret = { buf, create_checksum_from_uri(*page_uri) };
    return ret;
}

struct ubuffer_t dbms_buffering_from_table(struct dbms_table_t* table,
                                           pagenum_t pagenum)
{
    struct page_uri_t uri = { table->table_id, pagenum };
    return dbms_buffering(table->dbms, &uri);
}

struct ubuffer_t dbms_new_page(struct dbms_t* dbms,
                               tablenum_t table_id)
{
    struct buffer_t* buf = buffer_manager_new_page(&dbms->buffers, &dbms->tables, table_id);
    struct ubuffer_t ret = { buf, create_checksum(table_id, buf->pagenum) };
    return ret;
}

struct ubuffer_t dbms_new_page_from_table(struct dbms_table_t* table) {
    return dbms_new_page(table->dbms, table->table_id);
}

int dbms_find(struct dbms_t* dbms,
              tablenum_t table_id,
              struct record_t* record)
{
    struct table_t* table = table_manager_find(&dbms->tables, table_id);
    CHECK_NULL(table);

    return bpt_find(record->key, record, &table->file_manager);
}

int dbms_insert(struct dbms_t* dbms,
                tablenum_t table_id,
                prikey_t key,
                uint8_t* value,
                int value_size)
{
    struct table_t* table = table_manager_find(&dbms->tables, table_id);
    CHECK_NULL(table);

    return bpt_insert(key,
                      value,
                      value_size,
                      &table->file_manager);
}

int dbms_delete(struct dbms_t* dbms,
                tablenum_t table_id,
                prikey_t key)
{
    struct table_t* table = table_manager_find(&dbms->tables, table_id);
    CHECK_NULL(table);

    return bpt_delete(key, &table->file_manager);
}
