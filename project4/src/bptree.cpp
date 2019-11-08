#include <iostream>

#include "bptree.hpp"

BPTree::BPTree(FileManager* file, BufferManager* buffers) :
    leaf_order(DEFAULT_LEAF_ORDER),
    internal_order(DEFAULT_INTERNAL_ORDER),
    delayed_merge(DEFAULT_DELAYED_MERGE),
    verbose_output(false),
    file(file),
    buffers(buffers) {
    // Do nothing
}

void BPTree::test_config(int leaf_order,
                         int internal_order,
                         bool delayed_merge) {
    this->leaf_order = leaf_order;
    this->internal_order = internal_order;
    this->delayed_merge = delayed_merge;
    verbose_output = true;
}

void BPTree::print_leaves() {

}

void BPTree::print_tree() {

}

Status BPTree::find(prikey_t key, Record* record) {
    return Status::SUCCESS;
}

std::vector<Record> BPTree::find_range(prikey_t start, prikey_t end) {
    return std::vector<Record>();
}

Status BPTree::insert(prikey_t key, uint8_t* value, int value_size) {
    return Status::SUCCESS;
}

Status BPTree::remove(prikey_t key) {
    return Status::SUCCESS;
}

Status BPTree::destroy_tree() {
    return Status::SUCCESS;
}

// Ubuffer macro
Ubuffer BPTree::buffering(pagenum_t pagenum) {
    return buffers->buffering(*file, pagenum);
}

Ubuffer BPTree::create_page(bool leaf) {
    Ubuffer ubuf = buffers->new_page(*file);
    if (ubuf.buffer() == nullptr) {
        return ubuf;
    }

    EXIT_ON_FAILURE(
        ubuf.use(RWFlag::WRITE, [leaf](Ubuffer& buf) {
            return buf.page().init(leaf);
        })
    );

    return ubuf;
}

Status BPTree::free_page(pagenum_t pagenum) {
    return buffers->free_page(*file, pagenum);
}

constexpr int BPTree::cut(int length) {
    return length % 2 == 0
        ? length / 2
        : length / 2 + 1;
}

// find
pagenum_t BPTree::find_leaf(prikey_t key, Ubuffer buffer) {
    buffer = buffering(FILE_HEADER_PAGENUM);

    pagenum_t page;
    EXIT_ON_FAILURE(
        buffer.use(RWFlag::READ, [&page](Ubuffer& ubuf) {
            page = ubuf.page().file_header().root_page_number;
            return Status::SUCCESS;
        })
    );

    if (page == INVALID_PAGENUM) {
        if (verbose_output) {
            std::cout << "Empty tree." << std::endl;
        }
        return page;
    }

    bool runnable = true;
    while (runnable) {
        buffer = buffering(page);
        EXIT_ON_FAILURE(
            buffer.use(RWFlag::READ, [this, key, &page, &runnable](Ubuffer& ubuf) {
                Internal* ent = ubuf.page().entries();
                PageHeader& header = ubuf.page().page_header();

                if (header.is_leaf) {
                    runnable = false;
                    return Status::SUCCESS;
                }

                if (verbose_output) {
                    std::cout << '[';
                    for (int i = 0; i < header.number_of_keys - 1; ++i) {
                        std::cout << ent[i].key << ' ';
                    }
                    std::cout << ent[header.number_of_keys - 1].key << "] ";
                }

                int i;
                for (i = 0; i < header.number_of_keys && key >= ent[i].key; ++i)
                    {}
                
                --i;
                if (verbose_output) {
                    std::cout << i << " ->" << std::endl;
                }

                if (i < 0) {
                    page = header.special_page_number;
                } else {
                    page = ent[i].pagenum;
                }

                return Status::SUCCESS;
            })
        );
    }

    if (verbose_output) {
        EXIT_ON_FAILURE(
            buffer.use(RWFlag::READ, [](Ubuffer& ubuf) {
                PageHeader& header = ubuf.page().page_header();
                Record* rec = ubuf.page().records();

                std::cout << "Leaf [";
                for (int i = 0; i < header.number_of_keys - 1; ++i) {
                    std::cout << rec[i].key << ' ';
                }
                std::cout << rec[header.number_of_keys - 1].key
                          << ']'
                          << std::endl;
                return Status::SUCCESS;
            })
        );
    }

    return page;
}

Status BPTree::find_key_from_leaf(prikey_t key, Ubuffer buffer, Record* record) {
    return Status::SUCCESS;
}

Status BPTree::find_pagenum_from_internal(pagenum_t pagenum, Ubuffer buffer) {
    return Status::SUCCESS;
}

// insert
Status BPTree::write_record(Record& rec,
                            prikey_t key,
                            const uint8_t* value,
                            int value_size) {
    return Status::SUCCESS;
}

Status BPTree::insert_to_leaf(Ubuffer leaf, Record const& rec) {
    return Status::SUCCESS;
}

Status insert_and_split_leaf(Ubuffer leaf, Record const& rec) {
    return Status::SUCCESS;
}

Status insert_to_node(Ubuffer node, int index, Internal const& entry) {
    return Status::SUCCESS;
}

Status insert_and_split_node(Ubuffer node, int index, Internal const& entry) {
    return Status::SUCCESS;
}

Status insert_to_parent(Ubuffer leaf, prikey_t key, Ubuffer right) {
    return Status::SUCCESS;
}

Status insert_new_root(Ubuffer leaf, prikey_t key, Ubuffer right) {
    return Status::SUCCESS;
}

Status new_tree(Record const& rec) {
    return Status::SUCCESS;
}

// delete
Status remove_record_from_leaf(prikey_t key, Ubuffer node) {
    return Status::SUCCESS;
}

Status remove_entry_from_internal(prikey_t key, Ubuffer node) {
    return Status::SUCCESS;
}

Status rotate_to_right(Ubuffer left,
                        prikey_t key,
                        int index,
                        Ubuffer right,
                        Ubuffer parent) {
    return Status::SUCCESS;
}

Status rotate_to_left(Ubuffer left,
                        prikey_t key,
                        int index,
                        Ubuffer right,
                        Ubuffer parent) {
    return Status::SUCCESS;
}

Status shrink_root() {
    return Status::SUCCESS;
}

Status merge_nodes(Ubuffer left,
                    prikey_t key,
                    Ubuffer right,
                    Ubuffer parent) {
    return Status::SUCCESS;
}

Status redistribute_nodes(Ubuffer left,
                            prikey_t key,
                            int index,
                            Ubuffer right,
                            Ubuffer parent) {
    return Status::SUCCESS;
}

Status delete_entry(prikey_t key, Ubuffer page) {
    return Status::SUCCESS;
}
