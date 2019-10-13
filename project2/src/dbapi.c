#include <stdlib.h>
#include <string.h>

#include "bpt.h"
#include "dbapi.h"

int GLOBAL_TABLE_ID = -1;

struct file_vec_t GLOBAL_FILE_MANAGER = { 0, 0, NULL };

int file_vec_init(struct file_vec_t* vec) {
    // initialize file vector with default capacity.
    vec->size = 0;
    vec->capacity = DEFAULT_FILE_VEC_CAPACITY;
    vec->manager = malloc(sizeof(struct file_vec_t) * DEFAULT_FILE_VEC_CAPACITY);
    return SUCCESS;
}

int file_vec_free(struct file_vec_t* vec) {
    free(vec->manager);
}

int file_vec_expand(struct file_vec_t* vec) {
    // check max size constraint
    if (vec->capacity >= MAXIMUM_FILE_VEC_CAPACITY) {
        return FAILURE;
    }
    // expand capacity as twice
    vec->capacity *= 2;
    struct file_manager_t* new_vec =
        malloc(sizeof(struct file_vec_t) * vec->capacity);
    // copy file managers
    memcpy(new_vec, vec->manager, sizeof(struct file_vec_t) * vec->size);
    // update array
    vec->manager = new_vec;
    return SUCCESS;
}

int file_vec_append(struct file_vec_t* vec, struct file_manager_t* manager) {
    // if vector is not initialized
    if (vec->manager == NULL) {
        CHECK_SUCCESS(file_vec_init(vec));
    }
    // if capacity is exhausted
    if (vec->size >= vec->capacity) {
        CHECK_SUCCESS(file_vec_expand(vec));
    }
    // copy manager
    vec->manager[vec->size++] = *manager;
    // make zero
    memset(manager, 0, sizeof(struct file_manager_t));
    return SUCCESS;
}

int open_table(const char* pathname) {
    // open file
    struct file_manager_t manager;
    if (file_open(pathname, &manager) == FAILURE) {
        return -1;
    }
    // append manager
    if (file_vec_append(&GLOBAL_FILE_MANAGER, &manager) == FAILURE) {
        return -1;
    }
    // update global table id
    GLOBAL_TABLE_ID = GLOBAL_FILE_MANAGER.size - 1;
    return GLOBAL_TABLE_ID;
}

int close_table(int tableid) {
    // table id validation
    if (0 <= tableid && tableid < GLOBAL_FILE_MANAGER.size) {
        return FAILURE;
    }
    // close file
    struct file_manager_t* res = get_file_manager(tableid);
    CHECK_SUCCESS(file_close(res));
    return SUCCESS;
}

struct file_manager_t* get_file_manager(int tableid) {
    // get manager from global file manager
    if (0 <= tableid && tableid < GLOBAL_FILE_MANAGER.size) {
        return &GLOBAL_FILE_MANAGER.manager[tableid];
    }
    return NULL;
}

int db_insert(int64_t key, char* value) {
    // insert key value pair
    return insert(
        key,
        value,
        strlen(value) + 1,
        get_file_manager(GLOBAL_TABLE_ID));
}

int db_find(int64_t key, char* ret_val) {
    // find key
    struct record_t rec;
    CHECK_SUCCESS(find(key, &rec, get_file_manager(GLOBAL_TABLE_ID)));
    // whether return value or not
    if (ret_val != NULL) {
        memcpy(ret_val, rec.value, sizeof(struct record_t) - sizeof(prikey_t));
    }
    return SUCCESS;
}

int db_delete(int64_t key) {
    // delete key.
    return delete(key, get_file_manager(GLOBAL_TABLE_ID));
}
