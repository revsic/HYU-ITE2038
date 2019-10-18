#include <stdlib.h>
#include <string.h>

#include "bpt.h"
#include "dbapi.h"

struct buffer_manager_t GLOBAL_BUFFERS;

struct table_manager_t GLOBAL_TABLES;

int init_db(int num_buf) {
    return buffer_manager_init(&GLOBAL_BUFFERS, num_buf);
}

tablenum_t open_table(const char* pathname) {
    return table_manager_load(&GLOBAL_TABLES, pathname);
}

int close_table(tablenum_t table_id) {
    CHECK_SUCCESS(buffer_manager_release_table(&GLOBAL_BUFFERS, table_id));
    CHECK_SUCCESS(table_manager_remove(&GLOBAL_TABLES, table_id));
    return SUCCESS;
}

int shutdown_db() {
    CHECK_SUCCESS(buffer_manager_shutdown(&GLOBAL_BUFFERS));
    CHECK_SUCCESS(table_manager_release(&GLOBAL_TABLES));
    return SUCCESS;
}

int insert(tablenum_t table_id, int64_t key, char* value) {
    struct table_t* table = table_manager_find(&GLOBAL_TABLES, table_id);
    if (table == NULL) {
        return FAILURE;
    }

    return bpt_insert(
        key,
        value,
        strlen(value) + 1,
        &table->file_manager);
}

int find(tablenum_t table_id, int64_t key, char* ret_val) {
    struct table_t* table = table_manager_find(&GLOBAL_TABLES, table_id);
    if (table == NULL) {
        return FAILURE;
    }

    struct record_t rec;
    CHECK_SUCCESS(bpt_find(key, &rec, &table->file_manager));
    // whether return value or not
    if (ret_val != NULL) {
        memcpy(ret_val, rec.value, sizeof(struct record_t) - sizeof(prikey_t));
    }
    return SUCCESS;
}

int delete(tablenum_t table_id, int64_t key) {
    struct table_t* table = table_manager_find(&GLOBAL_TABLES, table_id);
    if (table == NULL) {
        return FAILURE;
    }
    return bpt_delete(key, &table->file_manager);
}
