#include "bptree.hpp"

BPTree::BPTree(FileManager* file, BufferManager* buffers) {

}

BPTree::~BPTree() {

}

void BPTree::test_config(int leaf_order,
                         int internal_order,
                         bool delayed_merge) {

}

void BPTree::print_leaves() {

}

void BPTree::print_tree() {

}

Status BPTree::find(prikey_t key, Record* record) {

}

std::vector<Record> BPTree::find_range(prikey_t start, prikey_t end) {

}

Status BPTree::insert(prikey_t key, uint8_t* value, int value_size) {

}

Status BPTree::remove(prikey_t key) {

}

Status BPTree::destroy_tree() {

}

// Ubuffer macro
Ubuffer BPTree::buffering(pagenum_t pagenum) {

}

Ubuffer BPTree::create_page(bool leaf) {

}

Ubuffer BPTree::free_page(pagenum_t pagenum) {

}

constexpr int BPTree::cut(int length) {

}

// find
pagenum_t BPTree::find_leaf(prikey_t key, Ubuffer buffer) {

}

int BPTree::find_key_from_leaf(prikey_t key, Ubuffer buffer, Record* record) {

}

int BPTree::find_pagenum_from_internal(pagenum_t pagenum, Ubuffer buffer) {

}

// insert
Status BPTree::write_record(Record& rec,
                            prikey_t key,
                            const uint8_t* value,
                            int value_size) {

}

Status BPTree::insert_to_leaf(Ubuffer leaf, Record const& rec) {

}

Status insert_and_split_leaf(Ubuffer leaf, Record const& rec) {

}

Status insert_to_node(Ubuffer node, int index, Internal const& entry) {

}

Status insert_and_split_node(Ubuffer node, int index, Internal const& entry) {

}

Status insert_to_parent(Ubuffer leaf, prikey_t key, Ubuffer right) {

}

Status insert_new_root(Ubuffer leaf, prikey_t key, Ubuffer right) {

}

Status new_tree(Record const& rec) {

}

// delete
Status remove_record_from_leaf(prikey_t key, Ubuffer node) {

}

Status remove_entry_from_internal(prikey_t key, Ubuffer node) {

}

Status rotate_to_right(Ubuffer left,
                        prikey_t key,
                        int index,
                        Ubuffer right,
                        Ubuffer parent) {

}

Status rotate_to_left(Ubuffer left,
                        prikey_t key,
                        int index,
                        Ubuffer right,
                        Ubuffer parent) {

}

Status shrink_root() {

}

Status merge_nodes(Ubuffer left,
                    prikey_t key,
                    Ubuffer right,
                    Ubuffer parent) {

}

Status redistribute_nodes(Ubuffer left,
                            prikey_t key,
                            int index,
                            Ubuffer right,
                            Ubuffer parent) {

}

Status delete_entry(prikey_t key, Ubuffer page) {

}
