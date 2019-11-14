#ifndef BPTREE_ITER_HPP
#define BPTREE_ITER_HPP

#include "bptree.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

class UbufferRecordRef {
public:
    UbufferRecordRef(int record_index, Ubuffer* buffer);

    UbufferRecordRef(UbufferRecordRef const&) = delete;

    UbufferRecordRef(UbufferRecordRef&&) noexcept;

    UbufferRecordRef& operator=(UbufferRecordRef const&) = delete;

    UbufferRecordRef& operator=(UbufferRecordRef&&) noexcept;

    ~UbufferRecordRef() = default;

    prikey_t key();

    template <typename F>
    inline Status use(RWFlag flag, F&& callback) {
        return buffer->use(flag, [&](Page& page) {
            return callback(page.records()[record_index]);
        });
    }

private:
    int record_index;
    Ubuffer* buffer;
};

class BPTreeIterator {
public:
    BPTreeIterator(BPTreeIterator const& other);

    BPTreeIterator(BPTreeIterator&& other) noexcept;

    BPTreeIterator& operator=(BPTreeIterator const& other);

    BPTreeIterator& operator=(BPTreeIterator&& other) noexcept;

    ~BPTreeIterator() = default;

    static BPTreeIterator begin(BPTree const& tree);

    static BPTreeIterator end();

    BPTreeIterator& operator++();

    bool operator!=(BPTreeIterator const& other);    

    bool operator==(BPTreeIterator const& other);

    UbufferRecordRef operator*();

private:
    BPTreeIterator(
        pagenum_t pagenum, int record_index, int num_key,
        Ubuffer buffer, BPTree const* tree);

    pagenum_t pagenum;
    int record_index;
    int num_key;
    Ubuffer buffer;
    BPTree const* tree;

#ifdef TEST_MODULE
    friend struct BPTreeIteratorTest;
#endif
};

#endif