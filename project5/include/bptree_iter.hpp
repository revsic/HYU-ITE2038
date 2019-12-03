#ifndef BPTREE_ITER_HPP
#define BPTREE_ITER_HPP

#include "bptree.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

/// Wrapper class for record based ubuffer.
class UbufferRecordRef {
public:
    /// Construct record buffer with given record index and page unit buffer.
    /// \param record_index int, target record index.
    /// \param buffer Ubuffer*, buffer where specified record exists.
    UbufferRecordRef(int record_index, Ubuffer* buffer);

    /// Deleted copy constructor.
    UbufferRecordRef(UbufferRecordRef const&) = delete;

    /// Move constructor.
    UbufferRecordRef(UbufferRecordRef&&) noexcept;

    /// Deleted copy assignment.
    UbufferRecordRef& operator=(UbufferRecordRef const&) = delete;

    /// Deleted move assignment.
    UbufferRecordRef& operator=(UbufferRecordRef&&) noexcept;

    /// Default destructor.
    ~UbufferRecordRef() = default;

    /// Get primary key from record.
    prikey_t key();

    /// Read record safely.
    /// \tparam F callback type, R(Record const&).
    /// \param callback F&&, callback for processing record.
    /// \return R, return value of the callback.
    template <typename F>
    inline auto read(F&& callback) {
        return buffer->read([&](Page const& page) {
            return callback(page.records()[record_index]);
        });
    }

    /// Write record safely.
    /// \tparam F callback type, R(Record&).
    /// \param callback F&&, callback for processing record.
    /// \return R, return value of the callback.
    template <typename F>
    inline auto write(F&& callback) {
        return buffer->write([&](Page& page) {
            return callback(page.records()[record_index]);
        });
    }

private:
    int record_index;       /// current record index.
    Ubuffer* buffer;        /// buffer which points target page.
};

class BPTreeIterator {
public:
    /// Copy constructor.
    BPTreeIterator(BPTreeIterator const& other);

    /// Move constructor.
    BPTreeIterator(BPTreeIterator&& other) noexcept;

    /// Copy assignment.
    BPTreeIterator& operator=(BPTreeIterator const& other);

    /// Move assignment.
    BPTreeIterator& operator=(BPTreeIterator&& other) noexcept;

    /// Default destructor.
    ~BPTreeIterator() = default;

    /// Get the beginning of the iterator from given b+tree structure.
    /// \param tree BPTree const&, target b+tree.
    /// \return BPTreeIterator, beginning of the iterator.
    static BPTreeIterator begin(BPTree const& tree);

    /// Get the end of the iterator.
    /// \return BPTreeIterator, end of the iterator.
    static BPTreeIterator end();

    /// Move to the next record.
    /// \return BPTreeIterator&, updated iterator.
    BPTreeIterator& operator++();

    /// Check whether this and given are not same.
    /// \param other BPTreeIterator const&, other iterator.
    /// \return bool, whether this and given are inequal. 
    bool operator!=(BPTreeIterator const& other);    

    /// Check whether this and given are same.
    /// \param other BPTreeIterator const&, other iterator.
    /// \return bool, whether this and given are equal.
    bool operator==(BPTreeIterator const& other);

    /// Dereference to the record ref.
    /// \return UbufferRecordRef, record unit buffer.
    UbufferRecordRef operator*();

private:
    /// Constructor.
    /// \param pagenum pagenum_t, page ID where pointed record exists.
    /// \param record_index int, the index of the record.
    /// \param num_key int, the number of the key.
    /// \param buffer Ubuffer, buffer where pointed record exists.
    /// \param tree BPTree const*, b+tree structure.
    BPTreeIterator(
        pagenum_t pagenum, int record_index, int num_key,
        Ubuffer buffer, BPTree const* tree);

    pagenum_t pagenum;      /// current page ID.
    int record_index;       /// current record index.
    int num_key;            /// number of keys in page.
    Ubuffer buffer;         /// buffer pointing current page.
    BPTree const* tree;     /// tree pointer.

#ifdef TEST_MODULE
    friend struct BPTreeIteratorTest;
#endif
};

#endif