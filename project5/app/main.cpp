#include <iostream>
#include <fstream>

#include "dbapi.hpp"

int main(int argc, char* argv[]) {
    init_db(4);
    int tid = open_table("datafile");

    bool runnable = true;
    std::istream& in = std::cin;
    // std::ifstream in("input.txt");

    while (!in.eof() && runnable) {
        char inst;
        char arr[1024];
        int input, range;
        std::string value;

        std::cout << '>';
        in >> inst;
        switch(inst) {
        case 'o':
            in >> value;
            tid = open_table(value.c_str());

            std::cout << "tid: " << tid << std::endl;
            GLOBAL_DB->print_tree(tid);
            break;
        case 'c':
            in >> input;
            close_table(input);
            break;
        case 'd':
            in >> input;
            db_delete(tid, input);
            GLOBAL_DB->print_tree(tid);
            break;
        case 'i':
            in >> input;
            value = std::to_string(input) + " value";
            db_insert(tid, input, value.c_str());
            GLOBAL_DB->print_tree(tid);
            break;
        case 'f':
            in >> input;
            if (db_find(tid, input, arr, INVALID_TRXID) == 0) {
                std::cout
                    << "Key: " << input << ' '
                    << "Value: " << arr << std::endl;
            } else {
                std::cout << "not found" << std::endl;
            }
            break;
        case 'r':
            in >> input >> range;
            if (input > range) {
                std::swap(input, range);
            }
            for (Record const& rec : GLOBAL_DB->find_range(tid, input, range)) {
                std::cout
                    << "Key: " << rec.key << ' '
                    << "Value: " << rec.value << std::endl;
            }
            break;
        case 'q':
            runnable = false;
            break;
        case 't':
            GLOBAL_DB->print_tree(tid);
            break;
        case 'x':
            GLOBAL_DB->destroy_tree(tid);
            GLOBAL_DB->print_tree(tid);
            break;
        case 'j':
            in >> input >> range >> value;
            join_table(input, range, value.c_str());
        default:
            break;
        }
        // while (getchar() != '\n');
    }

    shutdown_db();
    return 0;
}
