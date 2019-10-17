#include "table_manager.h"

int table_init(struct table_t* table) {
    return SUCCESS;
}

int table_load(struct table_t* table, const char* filename) {
    return SUCCESS;
}

int table_release(struct table_t* table) {
    return SUCCESS;
}

int table_vec_init(struct table_vec_t* table_vec) {
    return SUCCESS;
}

int table_vec_extend(struct table_vec_t* table_vec) {
    return SUCCESS;
}

int table_vec_append(struct table_vec_t* table_vec) {
    return SUCCESS;
}

int table_vec_remove(struct table_vec_t* table_vec) {
    return SUCCESS;
}

int table_vec_shrink(struct table_vec_t* table_vec) {
    return SUCCESS;
}

int table_vec_release(struct table_vec_t* table_vec) {
    return SUCCESS;
}

int table_manager_init(struct table_manager_t* manager) {
    return SUCCESS;
}

tablenum_t table_manager_load(struct table_manager_t* manager, const char* filename) {
    return INVALID_TABLENUM;
}

int table_manager_remove(struct table_manager_t* manager, tablenum_t table_id) {
    return SUCCESS;
}

int table_manager_release(struct table_manager_t* manager) {
    return SUCCESS;
}
