#include <cstring>
#include <iostream>
#include <queue>

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
    buffer.use(RWFlag::READ, [&](Page& page) {
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
        buffer.use(RWFlag::READ, [&](Page& page) {
            is_leaf = page.page_header().is_leaf;
            pagenum = page.page_header().special_page_number;
            return Status::SUCCESS;
        });
    }

    while (true) {
        buffer.use(RWFlag::READ, [&](Page& page) {
            Record* rec = page.records();
            int num_key = page.page_header().number_of_keys;
            pagenum = page.page_header().special_page_number;

            for (int i = 0; i < num_key; ++i) {
                std::cout << rec[i].key << ' ';
                if (verbose_output) {
                    std::cout
                        << '{'
                        << reinterpret_cast<char*>(rec[i].value)
                        << "} ";
                }
            }
            return Status::SUCCESS;
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

void BPTree::print_tree() {
    pagenum_t root;
    Ubuffer buffer = buffering(FILE_HEADER_PAGENUM);
    buffer.use(RWFlag::READ, [&](Page& page) {
        root = page.file_header().root_page_number;
        return Status::SUCCESS;
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
        buffer.use(RWFlag::READ, [&](Page& page) {
            PageHeader& pagehdr = page.page_header();
            if (pagehdr.parent_page_number != INVALID_PAGENUM) {
                Ubuffer tmp = buffering(pagehdr.parent_page_number);
                tmp.use(RWFlag::READ, [&](Page& parent) {
                    if (parent.page_header().special_page_number == pagenum) {
                        int new_rank = path_to_root(pagenum);
                        if (rank != new_rank) {
                            rank = new_rank;
                            std::cout << std::endl;
                        }
                    }
                    return Status::SUCCESS;
                });
            }

            if (verbose_output) {
                std::cout << "(page " << pagenum << ") ";
                if (!pagehdr.is_leaf) {
                    std::cout << '{' << pagehdr.special_page_number << "} ";
                }
            }

            Record* rec = page.records();
            Internal* ent = page.entries();
            for (int i = 0; i < pagehdr.number_of_keys; ++i) {
                if (pagehdr.is_leaf) {
                    std::cout << rec[i].key << ' ';
                    if (verbose_output) {
                        std::cout
                            << '{'
                            << reinterpret_cast<char*>(rec[i].value)
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
                if (pagehdr.special_page_number != INVALID_FILENUM) {
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
            return Status::SUCCESS;
        });
    }
    std::cout << std::endl;
}

Status BPTree::find(prikey_t key, Record* record) {
    Ubuffer buffer(nullptr);
    pagenum_t c = find_leaf(key, buffer);
    if (c == INVALID_PAGENUM) {
        return Status::FAILURE;
    }
    return find_key_from_leaf(key, buffer, record);
}

std::vector<Record> BPTree::find_range(prikey_t start, prikey_t end) {
    std::vector<Record> retn;

    Ubuffer buffer(nullptr);
    pagenum_t leaf = find_leaf(start, buffer);
    if (leaf == INVALID_PAGENUM) {
        return retn;
    }

    int i;
    Status res = buffer.use(RWFlag::READ, [&](Page& page) {
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
        res = buffer.use(RWFlag::READ, [&](Page& page) {
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

Status BPTree::insert(prikey_t key, const uint8_t* value, int value_size) {
    Ubuffer leaf_page(nullptr);
    pagenum_t leaf = find_leaf(key, leaf_page);
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, leaf_page, nullptr) == Status::SUCCESS) {
        return Status::FAILURE;
    }

    Record record;
    CHECK_SUCCESS(write_record(record, key, value, value_size));

    pagenum_t root;
    CHECK_SUCCESS(
        buffering(FILE_HEADER_PAGENUM).use(RWFlag::READ, [&](Page& page) {
            root = page.file_header().root_page_number;
            return Status::SUCCESS;
        })
    );
    if (root == INVALID_PAGENUM) {
        return new_tree(record);
    }

    int num_key;
    CHECK_SUCCESS(leaf_page.use(RWFlag::READ, [&](Page& page) {
        num_key = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));
    if (num_key < leaf_order - 1) {
        return insert_to_leaf(std::move(leaf_page), record);
    }

    return insert_and_split_leaf(std::move(leaf_page), record);
}

Status BPTree::remove(prikey_t key) {
    Ubuffer leaf_page(nullptr);
    pagenum_t leaf = find_leaf(key, leaf_page);
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, leaf_page, nullptr) == Status::SUCCESS
    ) {
        return delete_entry(key, std::move(leaf_page));
    }
    return Status::FAILURE;
}

Status BPTree::destroy_tree() {
    pagenum_t root;
    CHECK_SUCCESS(
        buffering(FILE_HEADER_PAGENUM).use(RWFlag::WRITE, [&](Page& page) {
            root = page.file_header().root_page_number;
            page.file_header().root_page_number = INVALID_PAGENUM;
            return Status::SUCCESS;
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
            buffering(pagenum).use(RWFlag::READ, [&](Page& page) {
                if (!page.page_header().is_leaf) {
                    queue.push(page.page_header().special_page_number);
                    
                    int num_key = page.page_header().number_of_keys;
                    for (int i = 0; i < num_key; ++i) {
                        queue.push(page.entries()[i].pagenum);
                    }
                }
                return Status::SUCCESS;
            })
        );
        CHECK_SUCCESS(free_page(pagenum));
    }
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
        ubuf.use(RWFlag::WRITE, [&](Page& page) {
            return page.init(leaf);
        })
    );

    return ubuf;
}

Status BPTree::free_page(pagenum_t pagenum) {
    return buffers->free_page(*file, pagenum);
}

int BPTree::path_to_root(pagenum_t pagenum) {
    pagenum_t root;
    EXIT_ON_FAILURE(
        buffering(FILE_HEADER_PAGENUM).use(RWFlag::READ, [&](Page& page) {
            root = page.file_header().root_page_number;
            return Status::SUCCESS;
        })
    );

    int length = 0;
    for (; root != pagenum; ++length) {
        EXIT_ON_FAILURE(
            buffering(pagenum).use(RWFlag::READ, [&](Page& page) {
                pagenum = page.page_header().parent_page_number;
                return Status::SUCCESS;
            })
        );
    }
    return length;
}

// find
pagenum_t BPTree::find_leaf(prikey_t key, Ubuffer& buffer) {
    buffer = buffering(FILE_HEADER_PAGENUM);

    pagenum_t page;
    EXIT_ON_FAILURE(
        buffer.use(RWFlag::READ, [&](Page& bufpage) {
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
            buffer.use(RWFlag::READ, [&](Page& bufpage) {
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

Status BPTree::find_key_from_leaf(prikey_t key, Ubuffer& buffer, Record* record) {
    return buffer.use(RWFlag::READ, [&](Page& page) {
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

Status BPTree::find_pagenum_from_internal(pagenum_t pagenum,
                                          Ubuffer& buffer,
                                          int& idx) {
    return buffer.use(RWFlag::READ, [&](Page& page) {
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
Status BPTree::write_record(Record& rec,
                            prikey_t key,
                            const uint8_t* value,
                            int value_size) {
    rec.key = key;
    value_size = std::min(
        value_size,
        static_cast<int>(sizeof(Record) - sizeof(prikey_t)));
    std::memcpy(rec.value, value, value_size);
    return Status::SUCCESS;
}

Status BPTree::insert_to_leaf(Ubuffer leaf, Record const& rec) {
    return leaf.use(RWFlag::WRITE, [&](Page& page) {
        int num_key = page.page_header().number_of_keys;
        if (num_key >= leaf_order) {
            return Status::FAILURE;
        }

        int insertion_point;
        Record* records = page.records();
        for (insertion_point = 0;
             insertion_point < num_key && records[insertion_point].key < rec.key;
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

Status BPTree::insert_and_split_leaf(Ubuffer leaf, Record const& rec) {
    pagenum_t next, parent_node;
    Ubuffer new_page = create_page(true);
    auto temp_record = std::make_unique<Record[]>(leaf_order);
    CHECK_SUCCESS(leaf.use(RWFlag::READ, [&](Page& page) {
        Record* records = page.records();
        PageHeader& pagehdr = page.page_header();

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
        return Status::SUCCESS;
    }));

    int split_index = cut(leaf_order - 1);
    CHECK_SUCCESS(leaf.use(RWFlag::WRITE, [&](Page& page) {
        Record* records = page.records();
        PageHeader& pagehdr = page.page_header();

        pagehdr.number_of_keys = 0;
        for (int i = 0; i < split_index; ++i) {
            std::memcpy(&records[i], &temp_record[i], sizeof(Record));
            pagehdr.number_of_keys++;
        }

        pagehdr.special_page_number = new_page.safe_pagenum();
        return Status::SUCCESS;
    }));

    prikey_t key;
    CHECK_SUCCESS(new_page.use(RWFlag::WRITE, [&](Page& page) {
        Record* records = page.records();
        PageHeader& pagehdr = page.page_header();

        for (int i = split_index, j = 0; i < leaf_order; ++i, ++j) {
            std::memcpy(&records[j], &temp_record[i], sizeof(Record));
            pagehdr.number_of_keys++;
        }

        pagehdr.special_page_number = next;
        pagehdr.parent_page_number = parent_node;

        key = records[0].key;
        return Status::SUCCESS;
    }))

    return insert_to_parent(std::move(leaf), key, std::move(new_page));
}

Status BPTree::insert_to_node(Ubuffer node, int index, Internal const& entry) {
    return node.use(RWFlag::WRITE, [&](Page& page) {
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

Status BPTree::insert_and_split_node(Ubuffer node, int index, Internal const& entry) {
    pagenum_t parent_num;
    Ubuffer new_node = create_page(false);
    auto temp = std::make_unique<Internal[]>(internal_order);
    CHECK_SUCCESS(node.use(RWFlag::READ, [&](Page& page) {
        Internal* ent = page.entries();
        PageHeader& header = page.page_header();

        parent_num = header.parent_page_number;
        for (int i = 0, j = 0; j < header.number_of_keys; ++i, ++j) {
            if (i == index) {
                ++i;
            }
            temp[i] = ent[j];
        }
        temp[index] = entry;
        return Status::SUCCESS;
    }));

    int split = cut(internal_order);
    CHECK_SUCCESS(node.use(RWFlag::WRITE, [&](Page& page) {
        Internal* ent = page.entries();
        PageHeader& header = page.page_header();

        header.number_of_keys = 0;
        for (int i = 0; i < split - 1; ++i) {
            ent[i] = temp[i];
            header.number_of_keys++;
        }
        return Status::SUCCESS;
    }));

    int k_prime;
    CHECK_SUCCESS(new_node.use(RWFlag::WRITE, [&](Page& page) {
        Internal* ent = page.entries();
        PageHeader& header = page.page_header();

        k_prime = temp[split - 1].key;
        header.special_page_number = temp[split - 1].pagenum;
        for (int i = split, j = 0; i < internal_order; ++i, ++j) {
            ent[j] = temp[i];
            header.number_of_keys++;
        }

        header.parent_page_number = parent_num;
        return Status::SUCCESS;
    }));

    parent_num = new_node.safe_pagenum();
    CHECK_SUCCESS(new_node.use(RWFlag::READ, [&](Page& page) {
        Internal* ent = page.entries();
        PageHeader& header = page.page_header();

        for (int i = -1; i < static_cast<int>(header.number_of_keys); ++i) {
            pagenum_t temp_pagenum = i == -1
                ? header.special_page_number
                : ent[i].pagenum;
            
            CHECK_SUCCESS(
                buffering(temp_pagenum).use(RWFlag::WRITE, [&](Page& page) {
                    page.page_header().parent_page_number = parent_num;
                    return Status::SUCCESS;
                })
            );
        }
        return Status::SUCCESS;
    }));

    return insert_to_parent(std::move(node), k_prime, std::move(new_node));
}

Status BPTree::insert_to_parent(Ubuffer left, prikey_t key, Ubuffer right) {
    pagenum_t parent_num;
    pagenum_t left_num = left.safe_pagenum();
    pagenum_t right_num = right.safe_pagenum();
    CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
        parent_num = page.page_header().parent_page_number;
        return Status::SUCCESS;
    }));

    if (parent_num == INVALID_PAGENUM) {
        return insert_new_root(std::move(left), key, std::move(right));
    }

    int idx;
    Ubuffer parent_page = buffering(parent_num);
    CHECK_SUCCESS(find_pagenum_from_internal(left_num, parent_page, idx));

    int num_key;
    CHECK_SUCCESS(parent_page.use(RWFlag::READ, [&](Page& page) {
        num_key = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));

    Internal entry = { key, right_num };
    if (num_key < internal_order - 1) {
        return insert_to_node(std::move(parent_page), idx + 1, entry);
    }
    return insert_and_split_node(std::move(parent_page), idx + 1, entry);
}

Status BPTree::insert_new_root(Ubuffer left, prikey_t key, Ubuffer right) {
    Ubuffer root = create_page(false);
    pagenum_t root_num = root.safe_pagenum();
    CHECK_SUCCESS(root.use(RWFlag::WRITE, [&](Page& page) {
        PageHeader& header = page.page_header();
        header.number_of_keys++;
        header.special_page_number = left.safe_pagenum();

        Internal& ent = page.entries()[0];
        ent.key = key;
        ent.pagenum = right.safe_pagenum();
        
        return Status::SUCCESS;
    }));

    CHECK_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
        page.page_header().parent_page_number = root_num;
        return Status::SUCCESS;
    }));

    CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
        page.page_header().parent_page_number = root_num;
        return Status::SUCCESS;
    }));

    CHECK_SUCCESS(
        buffering(FILE_HEADER_PAGENUM).use(RWFlag::WRITE, [&](Page& page) {
            page.file_header().root_page_number = root_num;
            return Status::SUCCESS;
        })
    );
    return Status::SUCCESS;
}

Status BPTree::new_tree(Record const& rec) {
    Ubuffer root = create_page(true);
    pagenum_t root_num = root.safe_pagenum();
    CHECK_SUCCESS(root.use(RWFlag::WRITE, [&](Page& page) {
        page.page_header().number_of_keys++;
        memcpy(&page.records()[0], &rec, sizeof(Record));
        return Status::SUCCESS;
    }));

    CHECK_SUCCESS(
        buffering(FILE_HEADER_PAGENUM).use(RWFlag::WRITE, [&](Page& page) {
            page.file_header().root_page_number = root_num;
            return Status::SUCCESS;
        })
    );

    return Status::SUCCESS;
}

// delete
Status BPTree::remove_record_from_leaf(prikey_t key, Ubuffer& node) {
    return node.use(RWFlag::WRITE, [&](Page& page) {
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

Status BPTree::remove_entry_from_internal(prikey_t key, Ubuffer& node) {
    return node.use(RWFlag::WRITE, [&](Page& page) {
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

Status BPTree::rotate_to_right(Ubuffer left,
                               prikey_t key,
                               int index,
                               Ubuffer right,
                               Ubuffer parent) {
    bool is_leaf;
    int num_key;
    CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
        is_leaf = page.page_header().is_leaf;
        num_key = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }))

    if (is_leaf) {
        CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& rightpage) {
            Record* right_rec = rightpage.records();
            for (int i = num_key; i > 0; --i) {
                right_rec[i] = right_rec[i - 1];
            }
            CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& leftpage) {
                num_key = leftpage.page_header().number_of_keys;
                right_rec[0] = leftpage.records()[num_key - 1];
                return Status::SUCCESS;
            }));
            CHECK_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& parentpage) {
                parentpage.entries()[index].key = right_rec[0].key;
                return Status::SUCCESS;
            }));
            return Status::SUCCESS;
        }));
    } else {
        CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& rightpage) {
            Internal* right_ent = rightpage.entries();
            for (int i = num_key; i > 0; --i) {
                right_ent[i] = right_ent[i - 1];
            }
            right_ent[0].key = key;
            right_ent[0].pagenum = rightpage.page_header().special_page_number;
            return Status::SUCCESS;
        }));
        CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& leftpage) {
            Internal* left_ent = leftpage.entries();
            num_key = leftpage.page_header().number_of_keys;

            Internal tmp = left_ent[num_key - 1];
            CHECK_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
                page.entries()[index].key = tmp.key;
                return Status::SUCCESS;
            }));

            pagenum_t rightnum = right.safe_pagenum();
            CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
                page.page_header().special_page_number = tmp.pagenum;
                return Status::SUCCESS;
            }));
            CHECK_SUCCESS(
                buffering(tmp.pagenum).use(RWFlag::WRITE, [&](Page& page) {
                    page.page_header().parent_page_number = rightnum;
                    return Status::SUCCESS;
                })
            );
            return Status::SUCCESS;
        }));
    }

    CHECK_SUCCESS(left.use(RWFlag::WRITE, [](Page& page) {
        page.page_header().number_of_keys--;
        return Status::SUCCESS;
    }));
    CHECK_SUCCESS(right.use(RWFlag::WRITE, [](Page& page) {
        page.page_header().number_of_keys++;
        return Status::SUCCESS;
    }));

    return Status::SUCCESS;
}

Status BPTree::rotate_to_left(Ubuffer left,
                              prikey_t key,
                              int index,
                              Ubuffer right,
                              Ubuffer parent) {
    bool is_leaf;
    int num_key;
    CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
        is_leaf = page.page_header().is_leaf;
        num_key = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));

    if (is_leaf) {
        CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
            Record* right_rec = page.records();
            CHECK_SUCCESS(left.use(RWFlag::WRITE, [&](Page& leftpage) {
                leftpage.records()[num_key] = right_rec[0];
                return Status::SUCCESS;
            }));
            CHECK_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& parentpage) {
                parentpage.entries()[index].key = right_rec[1].key;
                return Status::SUCCESS;
            }));
            return Status::SUCCESS;
        }));
        CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
            Record* right_rec = page.records();
            num_key = page.page_header().number_of_keys;
            for (int i = 0; i < num_key - 1; ++i) {
                right_rec[i] = right_rec[i + 1];
            }
            return Status::SUCCESS;
        }));
    } else {
        CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& rightpage) {
            pagenum_t childnum;
            Internal* right_ent = rightpage.entries();
            CHECK_SUCCESS(left.use(RWFlag::WRITE, [&](Page& leftpage) {
                Internal* left_ent = leftpage.entries();
                left_ent[num_key].key = key;
                left_ent[num_key].pagenum =
                    rightpage.page_header().special_page_number;
                
                childnum = left_ent[num_key].pagenum;
                return Status::SUCCESS;
            }));

            CHECK_SUCCESS(
                buffering(childnum).use(RWFlag::WRITE, [&](Page& page) {
                    page.page_header().parent_page_number = left.safe_pagenum();
                    return Status::SUCCESS;
                })
            );
            CHECK_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
                page.entries()[index].key = right_ent[0].key;
                return Status::SUCCESS;
            }));
            return Status::SUCCESS;
        }));

        CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
            Internal* right_ent = page.entries();
            page.page_header().special_page_number
                = right_ent[0].pagenum;
            
            num_key = page.page_header().number_of_keys;
            for (int i = 0; i < num_key - 1; ++i) {
                right_ent[i] = right_ent[i + 1];
            }
            return Status::SUCCESS;
        }));
    }

    CHECK_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
        page.page_header().number_of_keys--;
        return Status::SUCCESS;
    }));
    CHECK_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
        page.page_header().number_of_keys++;
        return Status::SUCCESS;
    }));

    return Status::SUCCESS;
}

Status BPTree::shrink_root() {
    pagenum_t root;
    Ubuffer filepage = buffering(FILE_HEADER_PAGENUM);
    CHECK_SUCCESS(filepage.use(RWFlag::READ, [&](Page& page) {
        root = page.file_header().root_page_number;
        return Status::SUCCESS;
    }));

    int num_key;
    Ubuffer rootpage = buffering(root);
    CHECK_SUCCESS(rootpage.use(RWFlag::READ, [&](Page& page) {
        num_key = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));

    if (num_key > 0) {
        return Status::SUCCESS;
    }

    pagenum_t childnum = INVALID_PAGENUM;
    CHECK_SUCCESS(rootpage.use(RWFlag::READ, [&](Page& page) {
        if (!page.page_header().is_leaf) {
            childnum = page.page_header().special_page_number;
            CHECK_SUCCESS(
                buffering(childnum).use(RWFlag::WRITE, [](Page& childpage) {
                    childpage.page_header().parent_page_number = INVALID_PAGENUM;
                    return Status::SUCCESS;
                })
            );
        }
        return Status::SUCCESS;
    }));

    CHECK_SUCCESS(filepage.use(RWFlag::WRITE, [&](Page& page) {
        page.file_header().root_page_number = childnum;
        return Status::SUCCESS;
    }));

    CHECK_SUCCESS(free_page(root));
    return Status::SUCCESS;
}

Status BPTree::merge_nodes(Ubuffer left,
                           prikey_t key,
                           Ubuffer right,
                           Ubuffer parent) {
    bool is_leaf;
    int insertion_index;
    CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
        is_leaf = page.page_header().is_leaf;
        insertion_index = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));

    if (is_leaf) {
        CHECK_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
            Record* left_rec = page.records();
            uint32_t& left_num_key = page.page_header().number_of_keys;
            CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& rightpage) {
                Record* right_rec = rightpage.records();
                uint32_t& right_num_key = rightpage.page_header().number_of_keys;
                for (int i = 0; right_num_key > 0; ++i, ++insertion_index) {
                    left_rec[insertion_index] = right_rec[i];
                    --right_num_key;
                    ++left_num_key;
                }

                page.page_header().special_page_number =
                    rightpage.page_header().special_page_number;
                return Status::SUCCESS;
            }));
            return Status::SUCCESS;
        }));
    } else {
        pagenum_t leftnum = left.safe_pagenum();
        CHECK_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
            Internal* left_ent = page.entries();
            uint32_t& left_num_key = page.page_header().number_of_keys;
            CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& rightpage) {
                Internal* right_ent = rightpage.entries();
                uint32_t& right_num_key = rightpage.page_header().number_of_keys;
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
                    CHECK_SUCCESS(
                        buffering(pagenum).use(RWFlag::WRITE, [&](Page& tmppage) {
                            tmppage.page_header().parent_page_number = leftnum;
                            return Status::SUCCESS;
                        })
                    );

                    --right_num_key;
                    ++left_num_key;
                }
                return Status::SUCCESS;
            }));
            return Status::SUCCESS;
        }));
    }

    CHECK_SUCCESS(delete_entry(key, std::move(parent)));
    CHECK_SUCCESS(free_page(right.safe_pagenum()));
    return Status::SUCCESS;
}

Status BPTree::redistribute_nodes(Ubuffer left,
                                  prikey_t key,
                                  int index,
                                  Ubuffer right,
                                  Ubuffer parent) {
    int left_nkey, right_nkey;
    CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
        left_nkey = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));
    CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
        right_nkey = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }))

    if (left_nkey < right_nkey) {
        CHECK_SUCCESS(rotate_to_left(
            std::move(left), key, index, std::move(right), std::move(parent)));
    } else {
        CHECK_SUCCESS(rotate_to_right(
            std::move(left), key, index, std::move(right), std::move(parent)));
    }
    return Status::SUCCESS;
}

Status BPTree::delete_entry(prikey_t key, Ubuffer buffer) {
    bool is_leaf;
    int num_key;
    pagenum_t parent_num, pagenum = buffer.safe_pagenum();
    CHECK_SUCCESS(buffer.use(RWFlag::READ, [&](Page& page) {
        is_leaf = page.page_header().is_leaf;
        num_key = page.page_header().number_of_keys;
        parent_num = page.page_header().parent_page_number;
        return Status::SUCCESS;
    }));

    if (is_leaf) {
        CHECK_SUCCESS(remove_record_from_leaf(key, buffer));
    } else {
        CHECK_SUCCESS(remove_entry_from_internal(key, buffer));
    }

    pagenum_t root_num;
    CHECK_SUCCESS(
        buffering(FILE_HEADER_PAGENUM).use(RWFlag::READ, [&](Page& page) {
            root_num = page.file_header().root_page_number;
            return Status::SUCCESS;
        })
    );

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
    CHECK_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
        k_prime = page.entries()[k_prime_index].key;
        left_num = index == -1
            ? page.entries()[0].pagenum
            : index == 0 ? page.page_header().special_page_number
                         : page.entries()[index - 1].pagenum;

        return Status::SUCCESS;
    }));

    Ubuffer right = std::move(buffer);
    Ubuffer left = buffering(left_num);

    if (index == -1) {
        std::swap(left, right);
    }

    int right_nkey, left_nkey;
    CHECK_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
        right_nkey = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));
    CHECK_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
        left_nkey = page.page_header().number_of_keys;
        return Status::SUCCESS;
    }));

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
