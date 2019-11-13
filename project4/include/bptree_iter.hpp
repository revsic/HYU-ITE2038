#ifndef BPTREE_ITER_HPP
#define BPTREE_ITER_HPP

#include "bptree.hpp"

class UbufferRecordRef {
public:
    UbufferRecordRef(int record_index, Ubuffer& buffer);

    template <typename F>
    inline Status use(RWFlag flag, F&& callback) {
        return buffer.use(flag, [&](Page& page) {
            return callback(page.records()[record_index]);
        });
    }
private:
    int record_index;
    Ubuffer& buffer;
};

class BPTreeIterator {
public:
    static BPTreeIterator begin(BPTree const& tree);

    static BPTreeIterator end();

    BPTreeIterator& operator++();

    bool operator!=(BPTreeIterator const& other);    

    UbufferRecordRef operator*();

private:
    BPTreeIterator(
        pagenum_t pagenum, int record_index,
        Ubuffer buffer, BPTree const* tree);

    pagenum_t pagenum;
    int record_index;
    Ubuffer buffer;
    BPTree const* tree;
};

#endif