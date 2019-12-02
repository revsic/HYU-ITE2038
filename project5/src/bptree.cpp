#include <cstring>
#include <iostream>
#include <queue>

#include "bptree.hpp"
#include "bptree_iter.hpp"

BPTree::BPTree(FileManager* file, BufferManager* buffers) :
    leaf_order(DEFAULT_LEAF_ORDER),
    internal_order(DEFAULT_INTERNAL_ORDER),
    delayed_merge(DEFAULT_DELAYED_MERGE),
    verbose_output(false),
    file(file),
    buffers(buffers) {
    // Do nothing
}

BPTree::BPTree(BPTree&& other) noexcept
    : leaf_order(other.leaf_order)
    , internal_order(other.internal_order)
    , delayed_merge(other.delayed_merge)
    , verbose_output(other.verbose_output)
    , file(other.file)
    , buffers(other.buffers)
{
    other.leaf_order = other.internal_order = 0;
    other.delayed_merge = other.verbose_output = false;
    other.file = nullptr;
    other.buffers = nullptr;
}

BPTree& BPTree::operator=(BPTree&& other) noexcept {
    leaf_order = other.leaf_order;
    internal_order = other.internal_order;
    delayed_merge = other.delayed_merge;
    verbose_output = other.verbose_output;
    file = other.file;
    buffers = other.buffers;

    other.leaf_order = other.internal_order = 0;
    other.delayed_merge = other.verbose_output = false;
    other.file = nullptr;
    other.buffers = nullptr;

    return *this;
}

void BPTree::reset_file(FileManager* file) {
    this->file = file;
}

void BPTree::test_config(int leaf_order,
                         int internal_order,
                         bool delayed_merge) {
    this->leaf_order = leaf_order;
    this->internal_order = internal_order;
    this->delayed_merge = delayed_merge;
    verbose_output = true;
}

void BPTree::print_leaves() const {
    Ubuffer buffer = buffering(FILE_HEADER_PAGENUM);
    pagenum_t pagenum = buffer.read([&](Page const& page) {
        return page.file_header().root_page_number;
    });

    if (pagenum == INVALID_PAGENUM) {
        std::cout << "Empty tree." << std::endl;
        return;
    }

    bool is_leaf = false;
    while (!is_leaf) {
        buffer = buffering(pagenum);
        buffer.read_void([&](Page const& page) {
            is_leaf = page.page_header().is_leaf;
            pagenum = page.page_header().special_page_number;
        });
    }

    while (true) {
        buffer.read_void([&](Page const& page) {
            Record const* rec = page.records();
            int num_key = page.page_header().number_of_keys;
            pagenum = page.page_header().special_page_number;

            for (int i = 0; i < num_key; ++i) {
                std::cout << rec[i].key << ' ';
                if (verbose_output) {
                    std::cout
                        << '{'
                        << reinterpret_cast<char const*>(rec[i].value)
                        << "} ";
                }
            }
        });

        if (verbose_output) {
            std::cout << "{next " << pagenum << "} ";
        }

        if (pagenum != INVALID_PAGENUM) {
            std::cout << " | ";
            buffer = buffering(pagenum);
        } else {
            break;
        }
    }
    std::cout << std::endl;
}

void BPTree::print_tree() const {
    Ubuffer buffer = buffering(FILE_HEADER_PAGENUM);
    pagenum_t root = buffer.read([&](Page const& page) {
        return page.file_header().root_page_number;
    });

    if (root == INVALID_PAGENUM) {
        std::cout << "Empty tree." << std::endl;
        return;
    }

    std::queue<pagenum_t> queue;
    queue.push(root);

    int rank = 0;
    while (!queue.empty()) {
        pagenum_t pagenum = queue.front();
        queue.pop();

        buffer = buffering(pagenum);
        buffer.read_void([&](Page const& page) {
            PageHeader const& pagehdr = page.page_header();
            if (pagehdr.parent_page_number != INVALID_PAGENUM) {
                Ubuffer tmp = buffering(pagehdr.parent_page_number);
                tmp.read_void([&](Page const& parent) {
                    if (parent.page_header().special_page_number == pagenum) {
                        int new_rank = path_to_root(pagenum);
                        if (rank != new_rank) {
                            rank = new_rank;
                            std::cout << std::endl;
                        }
                    }
                });
            }

            if (verbose_output) {
                std::cout << "(page " << pagenum << ") ";
                if (!pagehdr.is_leaf) {
                    std::cout << '{' << pagehdr.special_page_number << "} ";
                }
            }

            Record const* rec = page.records();
            Internal const* ent = page.entries();
            for (int i = 0; i < pagehdr.number_of_keys; ++i) {
                if (pagehdr.is_leaf) {
                    std::cout << rec[i].key << ' ';
                    if (verbose_output) {
                        std::cout
                            << '{'
                            << reinterpret_cast<char const*>(rec[i].value)
                            << "} ";
                    }
                } else {
                    std::cout << ent[i].key << ' ';
                    if (verbose_output) {
                        std::cout << '{' << ent[i].pagenum << "} ";
                    }
                }
            }

            if (!pagehdr.is_leaf) {
                if (pagehdr.special_page_number != INVALID_PAGENUM) {
                    queue.push(pagehdr.special_page_number);
                    for (int i = 0; i < pagehdr.number_of_keys; ++i) {
                        queue.push(ent[i].pagenum);
                    }
                }
            }

            if (verbose_output && pagehdr.is_leaf) {
                std::cout
                    << "(parent "
                    << pagehdr.parent_page_number
                    << ", next "
                    << pagehdr.special_page_number
                    << ") ";
            }
            std::cout << "| ";
        });
    }
    std::cout << std::endl;
}

Status BPTree::find(prikey_t key, Record* record) const {
    Ubuffer buffer(nullptr);
    pagenum_t c = find_leaf(key, buffer);
    if (c == INVALID_PAGENUM) {
        return Status::FAILURE;
    }
    return find_key_from_leaf(key, buffer, record);
}

std::vector<Record> BPTree::find_range(prikey_t start, prikey_t end) const {
    std::vector<Record> retn;

    Ubuffer buffer(nullptr);
    pagenum_t leaf = find_leaf(start, buffer);
    if (leaf == INVALID_PAGENUM) {
        return retn;
    }

    int i;
    Status res = buffer.read([&](Page const& page) {
        Record const* rec = page.records();
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
        buffer.read_void([&](Page const& page) {
            Record const* rec = page.records();
            int num_key = page.page_header().number_of_keys;
            for (; i < num_key && rec[i].key <= end; ++i) {
                retn.push_back(rec[i]);
            }

            if (i < num_key && rec[i].key > end) {
                next = INVALID_PAGENUM;
            } else {
                next = page.page_header().special_page_number;
            }
        });
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

Status BPTree::insert(
    prikey_t key, const uint8_t* value, int value_size
) const {
    Ubuffer leaf_page(nullptr);
    pagenum_t leaf = find_leaf(key, leaf_page);
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, leaf_page, nullptr) == Status::SUCCESS) {
        return Status::FAILURE;
    }

    Record record;
    CHECK_SUCCESS(write_record(record, key, value, value_size));

    pagenum_t root = buffering(FILE_HEADER_PAGENUM).read(
        [&](Page const& page) {
            return page.file_header().root_page_number;
        });

    if (root == INVALID_PAGENUM) {
        return new_tree(record);
    }

    int num_key = leaf_page.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });
    if (num_key < leaf_order - 1) {
        return insert_to_leaf(std::move(leaf_page), record);
    }

    return insert_and_split_leaf(std::move(leaf_page), record);
}

Status BPTree::remove(prikey_t key) const {
    Ubuffer leaf_page(nullptr);
    pagenum_t leaf = find_leaf(key, leaf_page);
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, leaf_page, nullptr) == Status::SUCCESS
    ) {
        return delete_entry(key, std::move(leaf_page));
    }
    return Status::FAILURE;
}

Status BPTree::update(prikey_t key, Record const& record) const {
    Ubuffer buffer(nullptr);
    pagenum_t c = find_leaf(key, buffer);
    if (c == INVALID_PAGENUM) {
        return Status::FAILURE;
    }
    return find_key_from_leaf<Access::WRITE>(
        key, buffer,
        [&](Record& rec) {
            std::memcpy(&rec, &record, sizeof(Record));
            return Status::SUCCESS;
        });
}

Status BPTree::destroy_tree() const {
    pagenum_t root;
    CHECK_SUCCESS(
        buffering(FILE_HEADER_PAGENUM).write_void([&](Page& page) {
            root = page.file_header().root_page_number;
            page.file_header().root_page_number = INVALID_PAGENUM;
        })
    );

    if (root == INVALID_PAGENUM) {
        return Status::SUCCESS;
    }

    std::queue<pagenum_t> queue;
    queue.push(root);

    while (!queue.empty()) {
        pagenum_t pagenum = queue.front();
        queue.pop();

        CHECK_SUCCESS(
            buffering(pagenum).read_void([&](Page const& page) {
                if (!page.page_header().is_leaf) {
                    queue.push(page.page_header().special_page_number);
                    
                    int num_key = page.page_header().number_of_keys;
                    for (int i = 0; i < num_key; ++i) {
                        queue.push(page.entries()[i].pagenum);
                    }
                }
            })
        );
        CHECK_SUCCESS(free_page(pagenum));
    }
    return Status::SUCCESS;
}


BPTreeIterator BPTree::begin() const {
    return BPTreeIterator::begin(*this);
}

BPTreeIterator BPTree::end() const {
    return BPTreeIterator::end();
}

// Ubuffer macro
Ubuffer BPTree::buffering(pagenum_t pagenum) const {
    return buffers->buffering(*file, pagenum);
}

Ubuffer BPTree::create_page(bool leaf) const {
    Ubuffer ubuf = buffers->new_page(*file);
    if (ubuf.buffer() == nullptr) {
        return ubuf;
    }

    EXIT_ON_FAILURE(ubuf.write([&](Page& page) {
        return page.init(leaf);
    }));

    return ubuf;
}

Status BPTree::free_page(pagenum_t pagenum) const {
    return buffers->free_page(*file, pagenum);
}

int BPTree::path_to_root(pagenum_t pagenum) const {
    pagenum_t root = buffering(FILE_HEADER_PAGENUM).read(
        [&](Page const& page) {
            return page.file_header().root_page_number;
        });

    int length = 0;
    for (; root != pagenum; ++length) {
        pagenum = buffering(pagenum).read([&](Page const& page) {
            return page.page_header().parent_page_number;
        });
    }
    return length;
}

// find
pagenum_t BPTree::find_leaf(prikey_t key, Ubuffer& buffer) const {
    buffer = buffering(FILE_HEADER_PAGENUM);

    pagenum_t page = buffer.read([&](Page const& page) {
        return page.file_header().root_page_number;
    });

    if (page == INVALID_PAGENUM) {
        return page;
    }

    bool runnable = true;
    while (runnable) {
        buffer = buffering(page);
        EXIT_ON_FAILURE(
            buffer.read_void([&](Page const& bufpage) {
                Internal const* ent = bufpage.entries();
                PageHeader const& header = bufpage.page_header();
                if (header.is_leaf) {
                    runnable = false;
                    return;
                }

                int i;
                for (i = 0;
                     i < header.number_of_keys && key >= ent[i].key;
                     ++i) {}

                --i;
                if (i < 0) {
                    page = header.special_page_number;
                } else {
                    page = ent[i].pagenum;
                }
            })
        );
    }

    return page;
}

Status BPTree::find_key_from_leaf(
    prikey_t key, Ubuffer& buffer, Record* record
) const {
    return find_key_from_leaf<Access::READ>(
        key, buffer,
        [=](Record const& rec) {
            if (record != nullptr) {
                std::memcpy(record, &rec, sizeof(Record));
            }
            return Status::SUCCESS;
        });
}

Status BPTree::find_pagenum_from_internal(
    pagenum_t pagenum, Ubuffer& buffer, int& idx
) const {
    return buffer.read([&](Page const& page) {
        if (page.page_header().special_page_number == pagenum) {
            idx = -1;
            return Status::SUCCESS;
        }
        int num_key = page.page_header().number_of_keys;
        for (idx = 0;
             idx < num_key && page.entries()[idx].pagenum != pagenum;
             ++idx)
            {}
        if (idx == num_key) {
            return Status::FAILURE;
        }
        return Status::SUCCESS;
    });
}

// insert
Status BPTree::write_record(
    Record& rec, prikey_t key, const uint8_t* value, int value_size
) {
    rec.key = key;
    value_size = std::min(
        value_size,
        static_cast<int>(sizeof(Record) - sizeof(prikey_t)));
    std::memcpy(rec.value, value, value_size);
    return Status::SUCCESS;
}

Status BPTree::insert_to_leaf(Ubuffer leaf, Record const& rec) const {
    return leaf.write([&](Page& page) {
        int num_key = page.page_header().number_of_keys;
        if (num_key >= leaf_order) {
            return Status::FAILURE;
        }

        int insertion_point;
        Record* records = page.records();
        for (insertion_point = 0;
             insertion_point < num_key
                && records[insertion_point].key < rec.key;
             ++insertion_point)
            {}
        
        for (int i = num_key; i > insertion_point; --i) {
            std::memcpy(&records[i], &records[i - 1], sizeof(Record));
        }

        page.page_header().number_of_keys++;
        std::memcpy(&records[insertion_point], &rec, sizeof(Record));
        return Status::SUCCESS;
    });
}

Status BPTree::insert_and_split_leaf(Ubuffer leaf, Record const& rec) const {
    pagenum_t next, parent_node;
    Ubuffer new_page = create_page(true);
    auto temp_record = std::make_unique<Record[]>(leaf_order);
    leaf.read_void([&](Page const& page) {
        Record const* records = page.records();
        PageHeader const& pagehdr = page.page_header();

        next = pagehdr.special_page_number;
        parent_node = pagehdr.parent_page_number;

        int insertion_index;
        for (insertion_index = 0;
            insertion_index < pagehdr.number_of_keys
                && records[insertion_index].key < rec.key;
            ++insertion_index)
            {}
        
        for (int i = 0, j = 0; i < pagehdr.number_of_keys; ++i, ++j) {
            if (j == insertion_index) {
                ++j;
            }
            std::memcpy(&temp_record[j], &records[i], sizeof(Record));
        }
        std::memcpy(&temp_record[insertion_index], &rec, sizeof(Record));
    });

    int split_index = cut(leaf_order - 1);
    leaf.write_void([&](Page& page) {
        Record* records = page.records();
        PageHeader& pagehdr = page.page_header();

        pagehdr.number_of_keys = 0;
        for (int i = 0; i < split_index; ++i) {
            std::memcpy(&records[i], &temp_record[i], sizeof(Record));
            pagehdr.number_of_keys++;
        }

        pagehdr.special_page_number = new_page.to_pagenum();
    });

    prikey_t key;
    new_page.write_void([&](Page& page) {
        Record* records = page.records();
        PageHeader& pagehdr = page.page_header();

        for (int i = split_index, j = 0; i < leaf_order; ++i, ++j) {
            std::memcpy(&records[j], &temp_record[i], sizeof(Record));
            pagehdr.number_of_keys++;
        }

        pagehdr.special_page_number = next;
        pagehdr.parent_page_number = parent_node;

        key = records[0].key;
    });

    return insert_to_parent(std::move(leaf), key, std::move(new_page));
}

Status BPTree::insert_to_node(
    Ubuffer node, int index, Internal const& entry
) const {
    return node.write([&](Page& page) {
        PageHeader& header = page.page_header();
        if (header.number_of_keys >= internal_order) {
            return Status::FAILURE;
        }

        Internal* ent = page.entries();
        for (int i = header.number_of_keys; i > index; --i) {
            ent[i] = ent[i - 1];
        }
        ent[index] = entry;
        header.number_of_keys++;
        return Status::SUCCESS;
    });
}

Status BPTree::insert_and_split_node(
    Ubuffer node, int index, Internal const& entry
) const {
    pagenum_t parent_num;
    Ubuffer new_node = create_page(false);
    auto temp = std::make_unique<Internal[]>(internal_order);
    node.read_void([&](Page const& page) {
        Internal const* ent = page.entries();
        PageHeader const& header = page.page_header();

        parent_num = header.parent_page_number;
        for (int i = 0, j = 0; j < header.number_of_keys; ++i, ++j) {
            if (i == index) {
                ++i;
            }
            temp[i] = ent[j];
        }
        temp[index] = entry;
    });

    int split = cut(internal_order);
    node.write_void([&](Page& page) {
        Internal* ent = page.entries();
        PageHeader& header = page.page_header();

        header.number_of_keys = 0;
        for (int i = 0; i < split - 1; ++i) {
            ent[i] = temp[i];
            header.number_of_keys++;
        }
    });

    int k_prime;
    new_node.write_void([&](Page& page) {
        Internal* ent = page.entries();
        PageHeader& header = page.page_header();

        k_prime = temp[split - 1].key;
        header.special_page_number = temp[split - 1].pagenum;
        for (int i = split, j = 0; i < internal_order; ++i, ++j) {
            ent[j] = temp[i];
            header.number_of_keys++;
        }

        header.parent_page_number = parent_num;
    });

    parent_num = new_node.to_pagenum();
    new_node.read_void([&](Page const& page) {
        Internal const* ent = page.entries();
        PageHeader const& header = page.page_header();

        for (int i = -1; i < static_cast<int>(header.number_of_keys); ++i) {
            pagenum_t temp_pagenum = i == -1
                ? header.special_page_number
                : ent[i].pagenum;

            buffering(temp_pagenum).write_void([&](Page& page) {
                page.page_header().parent_page_number = parent_num;
            });
        }
    });

    return insert_to_parent(std::move(node), k_prime, std::move(new_node));
}

Status BPTree::insert_to_parent(
    Ubuffer left, prikey_t key, Ubuffer right
) const {
    pagenum_t left_num = left.to_pagenum();
    pagenum_t right_num = right.to_pagenum();
    pagenum_t parent_num = left.read([&](Page const& page) {
        return page.page_header().parent_page_number;
    });

    if (parent_num == INVALID_PAGENUM) {
        return insert_new_root(std::move(left), key, std::move(right));
    }

    int idx;
    Ubuffer parent_page = buffering(parent_num);
    CHECK_SUCCESS(find_pagenum_from_internal(left_num, parent_page, idx));

    int num_key = parent_page.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });

    Internal entry = { key, right_num };
    if (num_key < internal_order - 1) {
        return insert_to_node(std::move(parent_page), idx + 1, entry);
    }
    return insert_and_split_node(std::move(parent_page), idx + 1, entry);
}

Status BPTree::insert_new_root(
    Ubuffer left, prikey_t key, Ubuffer right
) const {
    Ubuffer root = create_page(false);
    pagenum_t root_num = root.to_pagenum();
    root.write_void([&](Page& page) {
        PageHeader& header = page.page_header();
        header.number_of_keys++;
        header.special_page_number = left.to_pagenum();

        Internal& ent = page.entries()[0];
        ent.key = key;
        ent.pagenum = right.to_pagenum();
    });

    left.write_void([&](Page& page) {
        page.page_header().parent_page_number = root_num;
    });

    right.write_void([&](Page& page) {
        page.page_header().parent_page_number = root_num;
    });

    buffering(FILE_HEADER_PAGENUM).write_void([&](Page& page) {
        page.file_header().root_page_number = root_num;
    });
    return Status::SUCCESS;
}

Status BPTree::new_tree(Record const& rec) const {
    Ubuffer root = create_page(true);
    pagenum_t root_num = root.to_pagenum();
    root.write_void([&](Page& page) {
        page.page_header().number_of_keys++;
        memcpy(&page.records()[0], &rec, sizeof(Record));
    });

    buffering(FILE_HEADER_PAGENUM).write_void([&](Page& page) {
        page.file_header().root_page_number = root_num;
    });

    return Status::SUCCESS;
}

// delete
Status BPTree::remove_record_from_leaf(prikey_t key, Ubuffer& node) const {
    return node.write([&](Page& page) {
        Record* rec = page.records();
        int num_key = page.page_header().number_of_keys;

        int i;
        for (i = 0; i < num_key && rec[i].key != key; ++i)
            {}
        
        if (i == num_key) {
            return Status::FAILURE;
        }
        for (++i; i < num_key; ++i) {
            rec[i - 1] = rec[i];
        }
        page.page_header().number_of_keys--;
        return Status::SUCCESS;
    });
}

Status BPTree::remove_entry_from_internal(prikey_t key, Ubuffer& node) const {
    return node.write([&](Page& page) {
        Internal* ent = page.entries();
        int num_key = page.page_header().number_of_keys;

        int i;
        for (i = 0; i < num_key && ent[i].key != key; ++i)
            {}
        
        if (i == num_key) {
            return Status::FAILURE;
        }
        for(++i; i < num_key; ++i) {
            ent[i - 1] = ent[i];
        }
        page.page_header().number_of_keys--;
        return Status::SUCCESS;
    });
}

Status BPTree::rotate_to_right(
    Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
) const {
    bool is_leaf;
    int num_key;
    right.read_void([&](Page const& page) {
        is_leaf = page.page_header().is_leaf;
        num_key = page.page_header().number_of_keys;
    });

    if (is_leaf) {
        right.write_void([&](Page& rightpage) {
            Record* right_rec = rightpage.records();
            for (int i = num_key; i > 0; --i) {
                right_rec[i] = right_rec[i - 1];
            }
            left.read_void([&](Page const& leftpage) {
                num_key = leftpage.page_header().number_of_keys;
                right_rec[0] = leftpage.records()[num_key - 1];
            });
            parent.write_void([&](Page& parentpage) {
                parentpage.entries()[index].key = right_rec[0].key;
            });
        });
    } else {
        right.write_void([&](Page& rightpage) {
            Internal* right_ent = rightpage.entries();
            for (int i = num_key; i > 0; --i) {
                right_ent[i] = right_ent[i - 1];
            }
            right_ent[0].key = key;
            right_ent[0].pagenum = rightpage.page_header().special_page_number;
        });
        left.read_void([&](Page const& leftpage) {
            Internal const* left_ent = leftpage.entries();
            num_key = leftpage.page_header().number_of_keys;

            Internal tmp = left_ent[num_key - 1];
            parent.write_void([&](Page& page) {
                page.entries()[index].key = tmp.key;
            });

            pagenum_t rightnum = right.to_pagenum();
            right.write_void([&](Page& page) {
                page.page_header().special_page_number = tmp.pagenum;
            });
            buffering(tmp.pagenum).write_void([&](Page& page) {
                page.page_header().parent_page_number = rightnum;
            });
        });
    }

    left.write_void([](Page& page) {
        page.page_header().number_of_keys--;
    });
    right.write_void([](Page& page) {
        page.page_header().number_of_keys++;
    });

    return Status::SUCCESS;
}

Status BPTree::rotate_to_left(
    Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
) const {
    bool is_leaf;
    int num_key;
    left.read_void([&](Page const& page) {
        is_leaf = page.page_header().is_leaf;
        num_key = page.page_header().number_of_keys;
    });

    if (is_leaf) {
        right.read_void([&](Page const& page) {
            Record const* right_rec = page.records();
            left.write_void([&](Page& leftpage) {
                leftpage.records()[num_key] = right_rec[0];
            });
            parent.write_void([&](Page& parentpage) {
                parentpage.entries()[index].key = right_rec[1].key;
            });
        });
        right.write_void([&](Page& page) {
            Record* right_rec = page.records();
            num_key = page.page_header().number_of_keys;
            for (int i = 0; i < num_key - 1; ++i) {
                right_rec[i] = right_rec[i + 1];
            }
        });
    } else {
        right.read_void([&](Page const& rightpage) {
            pagenum_t childnum;
            Internal const* right_ent = rightpage.entries();
            left.write_void([&](Page& leftpage) {
                Internal* left_ent = leftpage.entries();
                left_ent[num_key].key = key;
                left_ent[num_key].pagenum =
                    rightpage.page_header().special_page_number;

                childnum = left_ent[num_key].pagenum;
            });

            buffering(childnum).write_void([&](Page& page) {
                page.page_header().parent_page_number =
                    left.to_pagenum();
            });
            parent.write_void([&](Page& page) {
                page.entries()[index].key = right_ent[0].key;
            });
        });

        right.write_void([&](Page& page) {
            Internal* right_ent = page.entries();
            page.page_header().special_page_number
                = right_ent[0].pagenum;
            
            num_key = page.page_header().number_of_keys;
            for (int i = 0; i < num_key - 1; ++i) {
                right_ent[i] = right_ent[i + 1];
            }
        });
    }

    right.write_void([&](Page& page) {
        page.page_header().number_of_keys--;
    });
    left.write_void([&](Page& page) {
        page.page_header().number_of_keys++;
    });
    return Status::SUCCESS;
}

Status BPTree::shrink_root() const {
    Ubuffer filepage = buffering(FILE_HEADER_PAGENUM);
    pagenum_t root = filepage.read([&](Page const& page) {
        return page.file_header().root_page_number;
    });

    Ubuffer rootpage = buffering(root);
    int num_key = rootpage.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });

    if (num_key > 0) {
        return Status::SUCCESS;
    }

    pagenum_t childnum = INVALID_PAGENUM;
    rootpage.read_void([&](Page const& page) {
        if (!page.page_header().is_leaf) {
            childnum = page.page_header().special_page_number;
            buffering(childnum).write_void([](Page& childpage) {
                childpage.page_header().parent_page_number =
                    INVALID_PAGENUM;
            });
        }
    });

    filepage.write_void([&](Page& page) {
        page.file_header().root_page_number = childnum;
    });

    CHECK_SUCCESS(free_page(root));
    return Status::SUCCESS;
}

Status BPTree::merge_nodes(
    Ubuffer left, prikey_t key, Ubuffer right, Ubuffer parent
) const {
    bool is_leaf;
    int insertion_index;
    left.read_void([&](Page const& page) {
        is_leaf = page.page_header().is_leaf;
        insertion_index = page.page_header().number_of_keys;
    });

    if (is_leaf) {
        left.write_void([&](Page& page) {
            Record* left_rec = page.records();
            uint32_t& left_num_key = page.page_header().number_of_keys;
            right.write_void([&](Page& rightpage) {
                Record* right_rec = rightpage.records();
                uint32_t& right_num_key =
                    rightpage.page_header().number_of_keys;
                for (int i = 0; right_num_key > 0; ++i, ++insertion_index) {
                    left_rec[insertion_index] = right_rec[i];
                    --right_num_key;
                    ++left_num_key;
                }

                page.page_header().special_page_number =
                    rightpage.page_header().special_page_number;
            });
        });
    } else {
        pagenum_t leftnum = left.to_pagenum();
        left.write_void([&](Page& page) {
            Internal* left_ent = page.entries();
            uint32_t& left_num_key = page.page_header().number_of_keys;
            right.write_void([&](Page& rightpage) {
                Internal* right_ent = rightpage.entries();
                uint32_t& right_num_key =
                    rightpage.page_header().number_of_keys;
                for (int i = -1;
                     static_cast<int>(right_num_key) >= 0;
                     ++i, ++insertion_index
                ) {
                    if (i == -1) {
                        left_ent[insertion_index].key = key;
                        left_ent[insertion_index].pagenum =
                            rightpage.page_header().special_page_number;
                    } else {
                        left_ent[insertion_index] = right_ent[i];
                    }

                    pagenum_t pagenum = left_ent[insertion_index].pagenum;
                    buffering(pagenum).write_void([&](Page& tmppage) {
                        tmppage.page_header().parent_page_number = leftnum;
                    });

                    --right_num_key;
                    ++left_num_key;
                }
            });
        });
    }

    CHECK_SUCCESS(delete_entry(key, std::move(parent)));
    CHECK_SUCCESS(free_page(right.to_pagenum()));
    return Status::SUCCESS;
}

Status BPTree::redistribute_nodes(
    Ubuffer left, prikey_t key, int index, Ubuffer right, Ubuffer parent
) const {
    int left_nkey = left.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });
    int right_nkey = right.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });

    if (left_nkey < right_nkey) {
        return rotate_to_left(
            std::move(left), key, index, std::move(right), std::move(parent));
    } else {
        return rotate_to_right(
            std::move(left), key, index, std::move(right), std::move(parent));
    }
}

Status BPTree::delete_entry(prikey_t key, Ubuffer buffer) const {
    bool is_leaf;
    int num_key;
    pagenum_t parent_num, pagenum = buffer.to_pagenum();
    buffer.read_void([&](Page const& page) {
        is_leaf = page.page_header().is_leaf;
        num_key = page.page_header().number_of_keys;
        parent_num = page.page_header().parent_page_number;
    });

    if (is_leaf) {
        CHECK_SUCCESS(remove_record_from_leaf(key, buffer));
    } else {
        CHECK_SUCCESS(remove_entry_from_internal(key, buffer));
    }

    pagenum_t root_num = buffering(FILE_HEADER_PAGENUM).read(
        [&](Page const& page) {
            return page.file_header().root_page_number;
        });

    if (pagenum == root_num) {
        return shrink_root();
    }

    int min_key = delayed_merge ? 1 : (
        is_leaf ? cut(leaf_order - 1) : cut(internal_order) - 1);
    if (num_key - 1 >= min_key) {
        return Status::SUCCESS;
    }

    Ubuffer parent = buffering(parent_num);

    int index;
    CHECK_SUCCESS(find_pagenum_from_internal(pagenum, parent, index));
    int k_prime_index = index == -1 ? 0 : index;

    prikey_t k_prime;
    pagenum_t left_num;
    parent.read_void([&](Page const& page) {
        k_prime = page.entries()[k_prime_index].key;
        left_num = index == -1
            ? page.entries()[0].pagenum
            : index == 0 ? page.page_header().special_page_number
                         : page.entries()[index - 1].pagenum;
    });

    Ubuffer right = std::move(buffer);
    Ubuffer left = buffering(left_num);

    if (index == -1) {
        std::swap(left, right);
    }

    int right_nkey = right.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });
    int left_nkey = left.read([&](Page const& page) {
        return page.page_header().number_of_keys;
    });

    int capacity = is_leaf ? leaf_order : internal_order - 1;
    if (left_nkey + right_nkey < capacity) {
        return merge_nodes(
            std::move(left), k_prime, std::move(right), std::move(parent));
    } else {
        return redistribute_nodes(std::move(left),
                                  k_prime,
                                  k_prime_index,
                                  std::move(right),
                                  std::move(parent));
    }
}
