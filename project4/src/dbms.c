#include "bpt.h"
#include "dbms.h"

int dbms_init(struct dbms_t* dbms, int num_buffer, int table_capacity) {
    buffer_manager_init(&dbms->buffers, num_buffer);
    table_manager_init(&dbms->tables, table_capacity);
    return SUCCESS;
}

int dbms_shutdown(struct dbms_t* dbms) {
    buffer_manager_shutdown(&dbms->buffers);
    table_manager_release(&dbms->tables);
    return SUCCESS;
}

tablenum_t dbms_open_table(struct dbms_t* dbms, const char* filename) {
    return table_manager_load(&dbms->tables, filename, &dbms->buffers);
}

int dbms_close_table(struct dbms_t* dbms, tablenum_t table_id) {
    CHECK_SUCCESS(buffer_manager_release_file(&dbms->buffers, table_id));
    CHECK_SUCCESS(table_manager_remove(&dbms->tables, table_id));
    return SUCCESS;
}

int dbms_find(struct dbms_t* dbms,
              tablenum_t table_id,
              prikey_t key,
              struct record_t* record)
{
    struct table_t* table;
    CHECK_NULL(table = table_manager_find(&dbms->tables, table_id));
    return table_find(table, key, record);
}

int dbms_insert(struct dbms_t* dbms,
                tablenum_t table_id,
                prikey_t key,
                uint8_t* value,
                int value_size)
{
    struct table_t* table;
    CHECK_NULL(table = table_manager_find(&dbms->tables, table_id));
    return table_insert(table, key, value, value_size);
}

int dbms_delete(struct dbms_t* dbms, tablenum_t table_id, prikey_t key) {
    struct table_t* table;
    CHECK_NULL(table = table_manager_find(&dbms->tables, table_id));
    return table_delete(table, key);
}
