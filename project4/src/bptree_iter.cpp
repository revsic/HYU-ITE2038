#include <limits>

#include "bptree_iter.hpp"

UbufferRecordRef::UbufferRecordRef(int record_index, Ubuffer* buffer) :
    record_index(record_index), buffer(buffer)
{
    // Do Nothing
}

UbufferRecordRef::UbufferRecordRef(UbufferRecordRef&& other) noexcept :
    record_index(other.record_index), buffer(other.buffer)
{
        other.record_index = 0;
        other.buffer = nullptr;
}

UbufferRecordRef& UbufferRecordRef::operator=(
    UbufferRecordRef&& other
) noexcept {
    record_index = other.record_index;
    buffer = other.buffer;

    other.record_index = 0;
    other.buffer = nullptr;
    return *this;
}

prikey_t UbufferRecordRef::key() {
    prikey_t res;
    EXIT_ON_FAILURE(use(RWFlag::READ, [&](Record& record) {
        res = record.key;
    }));
    return res;
}

BPTreeIterator::BPTreeIterator(
    pagenum_t pagenum, int record_index, Ubuffer buffer, BPTree const* tree
) : pagenum(pagenum), record_index(record_index),
    buffer(std::move(buffer)), tree(tree)
{
    // Do Nothing
}

BPTreeIterator::BPTreeIterator(BPTreeIterator const& other) :
    pagenum(other.pagenum), record_index(other.record_index),
    buffer(other.buffer.clone()), tree(other.tree)
{
    // Do Nothing
}

BPTreeIterator::BPTreeIterator(BPTreeIterator&& other) noexcept :
    pagenum(other.pagenum), record_index(other.record_index),
    buffer(std::move(other.buffer)), tree(other.tree)
{
    other.pagenum = INVALID_PAGENUM;
    other.record_index = 0;
    other.buffer = Ubuffer(nullptr);
    other.tree = nullptr;
}

BPTreeIterator& BPTreeIterator::operator=(BPTreeIterator const& other) {
    pagenum = other.pagenum;
    record_index = other.record_index;
    buffer = other.buffer.clone();
    tree = other.tree;
    return *this;
}

BPTreeIterator& BPTreeIterator::operator=(BPTreeIterator&& other) noexcept {
    pagenum = other.pagenum;
    record_index = other.record_index;
    buffer = std::move(buffer);
    tree = other.tree;

    other.pagenum = INVALID_PAGENUM;
    other.record_index = 0;
    other.buffer = Ubuffer(nullptr);
    other.tree = nullptr;
    return *this;
}

BPTreeIterator BPTreeIterator::begin(BPTree const& tree) {
    Ubuffer buffer(nullptr);
    pagenum_t leafnum = tree.find_leaf(
        std::numeric_limits<prikey_t>::min(), buffer);
    
    return BPTreeIterator(leafnum, 0, std::move(buffer), &tree);
}

BPTreeIterator BPTreeIterator::end() {
    return BPTreeIterator(INVALID_PAGENUM, 0, Ubuffer(nullptr), nullptr);
}

BPTreeIterator& BPTreeIterator::operator++() {
    record_index++;
    if (record_index < tree->leaf_order - 1) {
        return *this;
    }

    EXIT_ON_FAILURE(buffer.use(RWFlag::READ, [&](Page& page) {
        pagenum = page.page_header().special_page_number;
        return Status::SUCCESS;
    }));

    record_index = 0;
    if (pagenum == INVALID_PAGENUM) {
        buffer = Ubuffer(nullptr);
    } else {
        buffer = tree->buffering(pagenum);
    }

    return *this;
}

bool BPTreeIterator::operator!=(BPTreeIterator const& other) {
    return pagenum != other.pagenum || record_index != other.record_index;
}

UbufferRecordRef BPTreeIterator::operator*() {
    return UbufferRecordRef(record_index, &buffer);
}
