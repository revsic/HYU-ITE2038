#include "bpt.h"
#include "dbapi.h"

#define DEFAULT_FILE_VEC_CAPACITY 4

int GLOBAL_TABLE_ID = -1;

struct file_vec_t GLOBAL_FILE_MANAGER = { 0, 0, NULL };

int file_vec_init(struct file_vec_t* vec) {
    vec->size = 0;
    vec->capacity = DEFAULT_FILE_VEC_CAPACITY;
    vec->manager = malloc(sizeof(struct file_vec_t) * DEFAULT_FILE_VEC_CAPACITY);
    return SUCCESS;
}

int file_vec_free(struct file_vec_t* vec) {
    free(vec->manager);
}

int file_vec_expand(struct file_vec_t* vec) {
    vec->capacity *= 2;
    struct file_manager_t* new_vec =
        malloc(sizeof(struct file_vec_t) * vec->capacity);
    
    memcpy(new_vec, vec->manager, sizeof(struct file_vec_t) * vec->size);
    vec->manager = new_vec;

    return SUCCESS;
}

int file_vec_append(struct file_vec_t* vec, struct file_manager_t* manager) {
    if (vec->manager == NULL) {
        CHECK_SUCCESS(file_vec_init(vec));
    }

    if (vec->size >= vec->capacity) {
        CHECK_SUCCESS(file_vec_expand(vec));
    }
    
    vec->manager[vec->size++] = *manager;
    memset(manager, 0, sizeof(struct file_manager_t));

    return SUCCESS;
}

int open_table(char* pathname) {
    struct file_manager_t manager;
    if (file_open(pathname, &manager) == FAILURE) {
        return -1;
    }

    EXIT_ON_FAILURE(file_vec_append(&GLOBAL_FILE_MANAGER, &manager));
    GLOBAL_TABLE_ID = GLOBAL_FILE_MANAGER.size - 1;
    return GLOBAL_TABLE_ID;
}

int db_insert(int64_t key, char* value) {
    return insert(key, value, strlen(value) + 1, &GLOBAL_FILE_MANAGER);
}

int db_find(int64_t key, char* ret_val) {
    struct record_t rec;
    CHECK_SUCCESS(find(key, &rec, &GLOBAL_FILE_MANAGER));
    memcpy(ret_val, rec.value, sizeof(struct record_t) - sizeof(prikey_t));
    return SUCCESS;
}

int db_delete(int64_t key) {
    return delete(key, &GLOBAL_FILE_MANAGER);
}
