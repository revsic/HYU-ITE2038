#ifndef DBAPI_HPP
#define DBAPI_HPP

#include "dbms.hpp"

extern std::unique_ptr<Database> GLOBAL_DB;

int init_db(int buf_num);

int open_table(char const* pathname);

int db_insert(int table_id, int64_t key, char const* value);

int db_find(int table_id, int64_t key, char* ret_val);

int db_delete(int table_id, int64_t key);

int close_table(int table_id);

int shutdown_db();

#endif