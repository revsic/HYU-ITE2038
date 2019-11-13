#include <cstring>

#include "dbapi.hpp"

Database GLOBAL_DB(0);

int init_db(int buf_num) {
    GLOBAL_DB = Database(buf_num);
    return 0;
}

int open_table(char const* pathname) {
    return GLOBAL_DB.open_table(pathname);
}

int db_insert(int table_id, int64_t key, char const* value) {
    return static_cast<int>(GLOBAL_DB.insert(
        table_id,
        key,
        reinterpret_cast<uint8_t const*>(value),
        strlen(value) + 1));
}

int db_find(int table_id, int64_t key, char* ret_val) {
    Record rec;
    Status res = GLOBAL_DB.find(table_id, key, &rec);
    if (res == Status::FAILURE) {
        return 1;
    }

    memcpy(ret_val, rec.value, sizeof(Record) - sizeof(prikey_t));
    return 0;
}

int db_delete(int table_id, int64_t key) {
    return static_cast<int>(GLOBAL_DB.remove(table_id, key));
}

int close_table(int table_id) {
    return static_cast<int>(GLOBAL_DB.close_table(table_id));
}

int shutdown_db() {
    GLOBAL_DB.~Database();
    return 0;
}