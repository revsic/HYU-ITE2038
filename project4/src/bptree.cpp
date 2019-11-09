#include <cstring>
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
    pagenum_t pagenum;
    Ubuffer buffer = buffering(FILE_HEADER_PAGENUM);
    buffer.use(RWFlag::READ, [&pagenum](Page& page) {
        pagenum = page.file_header().root_page_number;
        return Status::SUCCESS;
    });

    if (pagenum == INVALID_PAGENUM) {
        std::cout << "Empty tree." << std::endl;
        return;
    }

    bool is_leaf = false;
    while (!is_leaf) {
        buffer = buffering(pagenum);
        buffer.use(RWFlag::READ, [&is_leaf, &pagenum](Page& page) {
            is_leaf = page.page_header().is_leaf;
            pagenum = page.page_header().special_page_number;
            return Status::SUCCESS;
        });
    }

    while (true) {
        buffer.use(RWFlag::READ, [](Page& buf) {
            return Status::SUCCESS;
        });
    }
}

void BPTree::print_tree() {

}

Status BPTree::find(prikey_t key, Record* record) {
    Ubuffer buffer(nullptr);
    pagenum_t c = find_leaf(key, buffer);
    if (c == INVALID_PAGENUM) {
        return Status::FAILURE;
    }
    return find_key_from_leaf(key, std::move(buffer), record);
}

std::vector<Record> BPTree::find_range(prikey_t start, prikey_t end) {
    std::vector<Record> retn;

    Ubuffer buffer(nullptr);
    pagenum_t leaf = find_leaf(start, buffer);
    if (leaf == INVALID_PAGENUM) {
        return retn;
    }

    int i;
    Status res = buffer.use(RWFlag::READ, [start, &i](Page& page) {
        Record* rec = page.records();
        int num_key = page.page_header().number_of_keys;
        for (i = 0; i < num_key && rec[i].key < start; ++i)
            {}

        if (i == num_key) {
            return Status::FAILURE;
        }
        return Status::SUCCESS;
    });

    if (res == Status::FAILURE) {
        return retn;
    }

    while (true) {
        pagenum_t next;
        res = buffer.use(RWFlag::READ, [end, &next, &i, &retn](Page& page) {
            Record* rec = page.records();
            int num_key = page.page_header().number_of_keys;
            for (; i < num_key && rec[i].key <= end; ++i) {
                retn.push_back(rec[i]);
            }

            if (i < num_key && rec[i].key > end) {
                next = INVALID_PAGENUM;
            } else {
                next = page.page_header().special_page_number;
            }
            return Status::SUCCESS;
        });

        if (res == Status::FAILURE) {
            return std::vector<Record>();
        }
        if (next == INVALID_PAGENUM) {
            break;
        }

        i = 0;
        buffer = buffering(next);
        if (buffer.buffer() == nullptr) {
            return std::vector<Record>();
        }
    }
    return retn;
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
        ubuf.use(RWFlag::WRITE, [leaf](Page& page) {
            return page.init(leaf);
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
pagenum_t BPTree::find_leaf(prikey_t key, Ubuffer& buffer) {
    buffer = buffering(FILE_HEADER_PAGENUM);

    pagenum_t page;
    EXIT_ON_FAILURE(
        buffer.use(RWFlag::READ, [&page](Page& bufpage) {
            page = bufpage.file_header().root_page_number;
            return Status::SUCCESS;
        })
    );

    if (page == INVALID_PAGENUM) {
        return page;
    }

    bool runnable = true;
    while (runnable) {
        buffer = buffering(page);
        EXIT_ON_FAILURE(
            buffer.use(RWFlag::READ, [key, &page, &runnable](Page& bufpage) {
                Internal* ent = bufpage.entries();
                PageHeader& header = bufpage.page_header();
                if (header.is_leaf) {
                    runnable = false;
                    return Status::SUCCESS;
                }

                int i;
                for (i = 0; i < header.number_of_keys && key >= ent[i].key; ++i)
                    {}

                --i;
                if (i < 0) {
                    page = header.special_page_number;
                } else {
                    page = ent[i].pagenum;
                }

                return Status::SUCCESS;
            })
        );
    }

    return page;
}

Status BPTree::find_key_from_leaf(prikey_t key, Ubuffer buffer, Record* record) {
    return buffer.use(RWFlag::READ, [key, record](Page& page) {
        if (!page.page_header().is_leaf) {
            return Status::FAILURE;
        }

        int i, num_key = page.page_header().number_of_keys;
        for (i = 0; i < num_key && page.records()[i].key != key; ++i)
            {}
        
        if (i < num_key) {
            if (record != nullptr) {
                std::memcpy(record, &page.records()[i], sizeof(Record));
            }
            return Status::SUCCESS;
        }
        return Status::FAILURE;
    });
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
