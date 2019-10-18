#include "bpt.h"
#include "dbms.h"

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

struct buffer_t* dbms_buffering(struct dbms_t* dbms,
                                struct page_uri_t* page_uri)
{
    return buffer_manager_buffering(&dbms->buffers, &dbms->tables, page_uri);
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
