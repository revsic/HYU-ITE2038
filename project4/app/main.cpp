#include <iostream>
#include <fstream>

#include "dbms.hpp"

int main(int argc, char* argv[]) {
    Database dbms(4);

    tableid_t tid = dbms.open_table("datafile");

    bool runnable = true;
    std::istream& in = std::cin;

    while (!in.eof() && runnable) {
        char inst;
        int input, range;
        Record record;
        std::string value;

        std::cout << '>';
        in >> inst;
        switch(inst) {
        case 'o':
            in >> value;
            dbms.close_table(tid);

            tid = dbms.open_table(value);
            dbms.print_tree(tid);
            break;
        case 'd':
            in >> input;
            dbms.remove(tid, input);
            dbms.print_tree(tid);
            break;
        case 'i':
            in >> input;
            value = std::to_string(input) + " value";
            dbms.insert(
                tid,
                input,
                reinterpret_cast<uint8_t const*>(value.c_str()),
                value.size());
            dbms.print_tree(tid);
            break;
        case 'f':
            in >> input;
            if (dbms.find(tid, input, &record) == Status::SUCCESS) {
                std::cout
                    << "Key: " << input << ' '
                    << "Value: " << record.value << std::endl;
            } else {
                std::cout << "not found" << std::endl;
            }
            break;
        case 'r':
            in >> input >> range;
            if (input > range) {
                std::swap(input, range);
            }
            for (Record const& rec : dbms.find_range(tid, input, range)) {
                std::cout
                    << "Key: " << rec.key << ' '
                    << "Value: " << rec.value << std::endl;
            }
            break;
        case 'q':
            runnable = false;
            break;
        case 't':
            dbms.print_tree(tid);
            break;
        case 'x':
            dbms.destroy_tree(tid);
            dbms.print_tree(tid);
            break;
        default:
            break;
        }
        while (getchar() != '\n');
    }

    return 0;
}
