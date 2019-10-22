#include <stdlib.h>
#include <string.h>

#include "bpt.h"
#include "dbapi.h"

struct dbms_t GLOBAL_DBMS;

int init_db(int num_buf) {
    return dbms_init(&GLOBAL_DBMS, num_buf, 10);
}

tablenum_t open_table(const char* pathname) {
    return dbms_open_table(&GLOBAL_DBMS, pathname);
}

int close_table(tablenum_t table_id) {
    return dbms_close_table(&GLOBAL_DBMS, table_id);
}

int shutdown_db() {
    return dbms_shutdown(&GLOBAL_DBMS);
}

int insert(tablenum_t table_id, int64_t key, char* value) {
    struct dbms_table_t table = { &GLOBAL_DBMS, table_id };
    return dbms_insert(&table, key, (uint8_t*)value, strlen(value) + 1);
}

int find(tablenum_t table_id, int64_t key, char* retval) {
    struct record_t rec;
    rec.key = key;

    struct dbms_table_t table = { &GLOBAL_DBMS, table_id };
    CHECK_SUCCESS(dbms_find(&table, &rec));
    if (retval != NULL) {
        memcpy(retval, rec.value, sizeof(struct record_t) - sizeof(prikey_t));
    }
    return SUCCESS;
}

int delete(tablenum_t table_id, int64_t key) {
    struct dbms_table_t table = { &GLOBAL_DBMS, table_id };
    return dbms_delete(&table, key);
}
