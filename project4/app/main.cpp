#include <iostream>
#include <fstream>

#include "buffer_manager.hpp"
#include "table_manager.hpp"

int main(int argc, char* argv[]) {
    TableManager tables;
    BufferManager buffers(4);

    tableid_t tid = tables.load("datafile", buffers);
    Table* table = tables.find(tid);

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
            buffers.release_file(table->fileid());
            tables.remove(tid);
            tid = tables.load(value, buffers);
            table = tables.find(tid);
            table->print_tree();
            break;
        case 'd':
            in >> input;
            table->remove(input);
            table->print_tree();
            break;
        case 'i':
            in >> input;
            value = std::to_string(input) + " value";
            table->insert(
                input,
                reinterpret_cast<const uint8_t*>(value.c_str()),
                value.size());
            table->print_tree();
            break;
        case 'f':
            in >> input;
            if (table->find(input, &record) == Status::SUCCESS) {
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
            for (Record const& rec : table->find_range(input, range)) {
                std::cout
                    << "Key: " << rec.key << ' '
                    << "Value: " << rec.value << std::endl;
            }
            break;
        case 'q':
            runnable = false;
            break;
        case 't':
            table->print_tree();
            break;
        case 'x':
            table->destroy_tree();
            table->print_tree();
            break;
        default:
            break;
        }
        while (getchar() != '\n');
    }

    return 0;
}
