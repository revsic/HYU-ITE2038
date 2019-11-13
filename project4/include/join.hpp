#ifndef JOIN_HPP
#define JOIN_HPP

#include "table_manager.hpp"

namespace JoinOper {
template <typename F>
void set_merge(Table const* table1, Table const* table2, F&& callback) {
    auto iter1 = table1->begin();
    auto iter2 = table2->begin();

    auto end1 = table1->end();
    auto end2 = table2->end();

    while (iter1 != end1 || iter2 != end2) {
        prikey_t key1;
        prikey_t key2 = (*iter2).key;
        while ((key1 = (*iter1).key()) < key2 && iter1 != end1) {
            ++iter1;
        }
        while ((key2 = (*iter2).key()) < key1 && iter2 != end2) {
            ++iter2;
        }

        if (key1 == key2) {
            callback(*iter1, *iter2);
        }
    }
}
}

#endif