#ifndef BPTREE_HPP
#define BPTREE_HPP

#include <vector>

#include "buffer_manager.hpp"
#include "disk_manager.hpp"

#ifdef TEST_MODULE
#include "test.hpp"
#endif

class BPTree {
public:
    static constexpr int DEFAULT_LEAF_ORDER = 32;

    static constexpr int DEFAULT_INTERNAL_ORDER = 249;

    static constexpr bool DEFAULT_DELAYED_MERGE = true;

    BPTree(FileManager* file, BufferManager* buffers);

    ~BPTree() = default;

    void test_config(int leaf_order, int internal_order, bool delayed_merge);

    void print_leaves() const;

    void print_tree() const;

    Status find(prikey_t key, Record* record) const;

    std::vector<Record> find_range(prikey_t start, prikey_t end) const;

    Status insert(prikey_t key, const uint8_t* value, int value_size) const;

    Status remove(prikey_t key) const;

    Status destroy_tree() const;

private:
    int leaf_order;
    int internal_order;
    bool verbose_output;
    bool delayed_merge;
    FileManager* file;
    BufferManager* buffers;

// Ubuffer macro
    Ubuffer buffering(pagenum_t pagenum) const;

    Ubuffer create_page(bool leaf) const;

    Status free_page(pagenum_t pagenum) const;

    int path_to_root(pagenum_t pagenum) const;

    static constexpr int cut(int length) {
        return length % 2 == 0
            ? length / 2
            : length / 2 + 1;
    }

// find
    pagenum_t find_leaf(prikey_t key, Ubuffer& buffer) const;

    Status find_key_from_leaf(
        prikey_t key, Ubuffer& buffer, Record* record) const;

    Status find_pagenum_from_internal(
        pagenum_t pagenum, Ubuffer& buffer, int& idx) const;

// insert
    static Status write_record(
        Record& rec, prikey_t key, const uint8_t* value, int value_size);
    
    Status insert_to_leaf(Ubuffer leaf, Record const& rec) const;

    Status insert_and_split_leaf(Ubuffer leaf, Record const& rec) const;

    Status insert_to_node(
        Ubuffer node, int index, Internal const& entry) const;

    Status insert_and_split_node(
        Ubuffer node, int index, Internal const& entry) const;

    Status insert_to_parent(Ubuffer left, prikey_t key, Ubuffer right) const;

    Status insert_new_root(Ubuffer left, prikey_t key, Ubuffer right) const;

    Status new_tree(Record const& rec) const;

// delete
    Status remove_record_from_leaf(prikey_t key, Ubuffer& node) const;

    Status remove_entry_from_internal(prikey_t key, Ubuffer& node) const;

    Status rotate_to_right(
        Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
    ) const;
    
    Status rotate_to_left(
        Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
    ) const;
    
    Status shrink_root() const;

    Status merge_nodes(
        Ubuffer left, prikey_t key, Ubuffer right, Ubuffer parent) const;
    
    Status redistribute_nodes(
        Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
    ) const;
    
    Status delete_entry(prikey_t key, Ubuffer page) const;

#ifdef TEST_MODULE
    friend struct BPTreeTest;
#endif
};

#endif