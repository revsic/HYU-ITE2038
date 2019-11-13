#include <chrono>
#include <fstream>
#include <iostream>

#include "dbapi.hpp"

template <typename T, typename F, typename... Args>
T perf(F&& func, Args&&... args) {
    auto start = std::chrono::steady_clock::now();
    auto res = func(std::forward<Args>(args)...);
    auto end = std::chrono::steady_clock::now();

    return std::chrono::duration_cast<T>(end - start);
}

int run(std::string const& filename, int bufsize) {
    init_db(bufsize);
    std::ifstream ifs(filename);

    std::vector<tableid_t> tables;
    tables.reserve(100);

    int count = 0;
    bool runnable = true;
    while (!ifs.eof() && runnable) {
        ++count;
        std::cout << '\r' << count;

        int inst, table_id;
        int input, range;
        std::string value;

        ifs >> inst;
        switch (inst) {
        case 0:
            ifs >> value;
            tables.push_back(open_table(value.c_str()));
            break;
        case 1:
            ifs >> table_id >> input >> value;
            db_insert(tables[table_id - 1], input, value.c_str());
            break;
        case 2:
            ifs >> table_id >> input;
            db_find(tables[table_id - 1], input, nullptr);
            break;
        case 3:
            ifs >> table_id >> input;
            db_delete(tables[table_id - 1], input);
            break;
        case 4:
            ifs >> table_id;
            close_table(tables[table_id - 1]);
            break;
        case 10:
            runnable = false;
            break;
        }
    }
    shutdown_db();
    std::cout << '\n';
    return 0;
}

int main() {
    auto dur = perf<std::chrono::seconds>(run, "input.txt", 100000);
    std::cout << dur.count() << " sec elapsed" << std::endl;
}