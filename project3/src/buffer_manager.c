#include "buffer_manager.h"

int buffer_init(struct buffer_t* buffer) {
    return SUCCESS;
}

int buffer_load(struct buffer_t* buffer, pagenum_t pagenum) {
    return SUCCESS;
}

int buffer_release(struct buffer_t* buffer) {
    return SUCCESS;
}

int buffer_manager_init(struct buffer_manager_t* manager) {
    return SUCCESS;
}

int buffer_manager_shutdown(struct buffer_manager_t* manager) {
    return SUCCESS;
}

int buffer_manager_load(struct buffer_manager_t* manager, pagenum_t pagenum) {
    return SUCCESS;
}
