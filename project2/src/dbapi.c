#include <stdlib.h>
#include <string.h>

#include "bpt.h"
#include "dbapi.h"

int GLOBAL_TABLE_ID = -1;

struct file_manager_t GLOBAL_MANAGER;

int open_table(const char* pathname) {
    // open file
    if (file_open(pathname, &GLOBAL_MANAGER) == FAILURE) {
        return -1;
    }
    // update global table id
    return ++GLOBAL_TABLE_ID;
}

int close_table() {
    // close file
    CHECK_SUCCESS(file_close(&GLOBAL_MANAGER));
    return SUCCESS;
}

int db_insert(int64_t key, char* value) {
    // insert key value pair
    return insert(
        key,
        value,
        strlen(value) + 1,
        &GLOBAL_MANAGER);
}

int db_find(int64_t key, char* ret_val) {
    // find key
    struct record_t rec;
    CHECK_SUCCESS(find(key, &rec, &GLOBAL_MANAGER));
    // whether return value or not
    if (ret_val != NULL) {
        memcpy(ret_val, rec.value, sizeof(struct record_t) - sizeof(prikey_t));
    }
    return SUCCESS;
}

int db_delete(int64_t key) {
    // delete key.
    return delete(key, &GLOBAL_MANAGER);
}
