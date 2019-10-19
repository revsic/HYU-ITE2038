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

struct ubuffer_t dbms_buffering(struct dbms_table_t* table,
                                pagenum_t pagenum)
{
    struct page_uri_t uri = { table->table_id, pagenum };
    return buffer_manager_buffering(&table->dbms->buffers,
                                    &table->dbms->tables,
                                    &uri);
}

struct ubuffer_t dbms_new_page(struct dbms_table_t* table)
{
    return buffer_manager_new_page(&table->dbms->buffers,
                                   &table->dbms->tables,
                                   table->table_id);
}

int dbms_free_page(struct dbms_table_t* table, pagenum_t pagenum) {
    struct page_uri_t uri = { table->table_id, pagenum };
    return buffer_manager_free_page(&table->dbms->buffers,
                                    &table->dbms->tables,
                                    &uri);
}

int dbms_find(struct dbms_table_t* table, struct record_t* record)
{
    return bpt_find(record->key, record, table);
}

int dbms_insert(struct dbms_table_t* table,
                prikey_t key,
                uint8_t* value,
                int value_size)
{
    return bpt_insert(key,
                      value,
                      value_size,
                      table);
}

int dbms_delete(struct dbms_table_t* table, prikey_t key) {
    return bpt_delete(key, table);
}
