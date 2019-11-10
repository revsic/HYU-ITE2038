#include "bptree.hpp"
#include "test.hpp"

struct BPTreeTest {
#define TEST_NAME(name) static int name##_test();
    TEST_NAME(constructor)
    TEST_NAME(destructor)
    TEST_NAME(buffering)
    TEST_NAME(create_page)
    TEST_NAME(free_page)
    TEST_NAME(test_config)
    TEST_NAME(path_to_root)
    TEST_NAME(cut)
    TEST_NAME(find_leaf)
    TEST_NAME(find_key_from_leaf)
    TEST_NAME(find_pagenum_from_internal)
    TEST_NAME(find)
    TEST_NAME(find_range)
    TEST_NAME(print_leaves)
    TEST_NAME(print_tree)
    TEST_NAME(write_record)
    TEST_NAME(insert_to_leaf)
    TEST_NAME(insert_and_split_leaf)
    TEST_NAME(insert_to_node)
    TEST_NAME(insert_and_split_node)
    TEST_NAME(insert_to_parent)
    TEST_NAME(insert_new_root)
    TEST_NAME(new_tree)
    TEST_NAME(insert)
    TEST_NAME(remove_record_from_leaf)
    TEST_NAME(remove_entry_from_internal)
    TEST_NAME(shrink_root)
    TEST_NAME(merge_nodes)
    TEST_NAME(rotate_to_left)
    TEST_NAME(rotate_to_right)
    TEST_NAME(redistribute_nodes_leaf)
    TEST_NAME(redistribute_nodes_internal)
    TEST_NAME(delete_entry);
    TEST_NAME(remove);
    TEST_NAME(destroy_tree);
};

// int bpt_test_preprocess(struct bpt_t* bpt,
//                         struct file_manager_t* file,
//                         struct buffer_manager_t* buffers)
// {
//     TEST_SUCCESS(buffer_manager_init(buffers, 4));
//     TEST_SUCCESS(file_open(file, "testfile"));
//     TEST_SUCCESS(bpt_init(bpt, file, buffers));
//     return SUCCESS;
// }

// int bpt_test_postprocess(struct bpt_t* bpt,
//                          struct file_manager_t* file,
//                          struct buffer_manager_t* buffers)
// {
//     TEST_SUCCESS(bpt_release(bpt));
//     TEST_SUCCESS(buffer_manager_shutdown(buffers));
//     TEST_SUCCESS(file_close(file));
//     remove("testfile");
//     return SUCCESS;
// }

TEST_SUITE(BPTreeTest::buffering, {
    // just porting buffer_manager_buffering
})

TEST_SUITE(BPTreeTest::create_page, {
    // just porting buffer_manager_new_page
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t buf = make_node(&bpt, TRUE);
    // struct page_header_t* header = page_header(from_ubuffer(&buf));
    // TEST(header->parent_page_number == INVALID_PAGENUM);
    // TEST(header->is_leaf == TRUE);
    // TEST(header->number_of_keys == 0);
    // TEST(header->special_page_number == INVALID_PAGENUM);

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::free_page, {
    // just porting buffer_manager_free_page
})

TEST_SUITE(BPTreeTest::constructor, {
    // struct bpt_t config;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;

    // TEST_SUCCESS(bpt_init(&config, &file, &buffers));
    // TEST(config.file == &file);
    // TEST(config.buffers == &buffers);
    // TEST(config.leaf_order == 32);
    // TEST(config.internal_order == 249);
    // TEST(config.verbose_output == FALSE);
    // TEST(config.delayed_merge == TRUE);
})

TEST_SUITE(BPTreeTest::destructor, {
    // struct bpt_t config;
    // TEST_SUCCESS(bpt_release(&config));
    // TEST(config.leaf_order == 0);
    // TEST(config.internal_order == 0);
    // TEST(config.verbose_output == FALSE);
    // TEST(config.delayed_merge == FALSE);
    // TEST(config.file == NULL);
    // TEST(config.buffers == NULL);
})

TEST_SUITE(BPTreeTest::test_config, {
    // struct bpt_t config;
    // TEST_SUCCESS(bpt_test_config(&config, 5, 4));
    // TEST(config.leaf_order == 5);
    // TEST(config.internal_order == 4);
    // TEST(config.verbose_output == TRUE);
    // TEST(config.delayed_merge == TRUE);
})

TEST_SUITE(BPTreeTest::path_to_root, {
    // print method
})

TEST_SUITE(BPTreeTest::cut, {
    // TEST(cut(4) == 2);
    // TEST(cut(5) == 3);
})

TEST_SUITE(BPTreeTest::find_leaf, {
    // int i;
    // char str[] = "00";
    // const int leaf_order = 4;
    // const int internal_order = 5;

    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;

    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    // for (i = 0; i < 40; ++i) {
    //     str[0] = '0' + i / 10;
    //     str[1] = '0' + i % 10;
    //     TEST_SUCCESS(bpt_insert(&bpt, i, (uint8_t*)str, 3));
    // }

    // struct ubuffer_t buf = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // pagenum_t pagenum = file_header(from_ubuffer(&buf))->root_page_number;
    // while (TRUE) {
    //     buf = bpt_buffering(&bpt, pagenum);
    //     if (page_header(from_ubuffer(&buf))->is_leaf) {
    //         break;
    //     }
    
    //     pagenum = page_header(from_ubuffer(&buf))->special_page_number;
    // }

    // for (i = 0; i < 40; ++i) {
    //     if (i && i % 2 == 0) {
    //         BUFFER(buf, READ_FLAG, {
    //             pagenum = page_header(from_ubuffer(&buf))->special_page_number;
    //         })
    //         buf = bpt_buffering(&bpt, pagenum);
    //     }

    //     TEST(pagenum == find_leaf(&bpt, i, NULL));
    // }
    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::find_key_from_leaf, {
    // // preproc
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // // case 0. leaf validation
    // struct ubuffer_t node = make_node(&bpt, FALSE);
    // struct page_t* page = from_ubuffer(&node);
    // for (i = 0; i < 3; ++i) {
    //     page_header(page)->number_of_keys++;
    //     records(page)[i].key = i * 10;
    // }

    // TEST(find_key_from_leaf(10, node, NULL) == FAILURE);

    // // case 1. cannot find
    // page_header(page)->is_leaf = TRUE;
    // records(page)[1].key = 15;
    // TEST(find_key_from_leaf(10, node, NULL) == FAILURE);

    // // case 2. find and record=NULL
    // records(page)[1].key = 10;
    // TEST_SUCCESS(find_key_from_leaf(10, node, NULL));

    // records(page)[0].key = 10;
    // records(page)[1].key = 15;
    // TEST_SUCCESS(find_key_from_leaf(10, node, NULL));

    // records(page)[0].key = 5;
    // records(page)[1].key = 7;
    // records(page)[2].key = 10;
    // TEST_SUCCESS(find_key_from_leaf(10, node, NULL));

    // // case 3. find and record
    // struct record_t rec;
    // *(int*)records(page)[2].value = 40;
    // TEST_SUCCESS(find_key_from_leaf(10, node, &rec));
    // TEST(rec.key == 10);
    // TEST(*(int*)rec.value == 40);

    // // postproc
    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::find_pagenum_from_internal, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t buf = make_node(&bpt, FALSE);
    // struct page_header_t* header = page_header(from_ubuffer(&buf));
    // header->number_of_keys = min(5, bpt.internal_order - 1);
    // header->special_page_number = 10;

    // int i;
    // for (i = 0; i < header->number_of_keys; ++i) {
    //     entries(from_ubuffer(&buf))[i].pagenum = (i + 2) * 10;
    // }

    // TEST(get_index(&buf, 5) == header->number_of_keys);
    // TEST(get_index(&buf, 10 * (header->number_of_keys + 3))
    //     == header->number_of_keys);

    // TEST(get_index(&buf, 10) == -1);
    // for (i = 0; i < header->number_of_keys; ++i) {
    //     TEST(get_index(&buf, (i + 2) * 10) == i);
    // }

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::find, {
    // int i;
    // char str[] = "00";
    // const int leaf_order = 4;
    // const int internal_order = 5;

    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;

    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    // for (i = 0; i < 40; ++i) {
    //     str[0] = '0' + i / 10;
    //     str[1] = '0' + i % 10;
    //     TEST_SUCCESS(bpt_insert(&bpt, i, (uint8_t*)str, 3));
    // }

    // struct record_t rec;
    // for (i = 0; i < 40; ++i) {
    //     TEST_SUCCESS(bpt_find(&bpt, i, &rec));
    //     TEST(rec.value[0] == '0' + i / 10);
    //     TEST(rec.value[1] == '0' + i % 10);
    // }

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::find_range, {
    // int i;
    // char str[] = "00";
    // const int leaf_order = 4;
    // const int internal_order = 5;

    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;

    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    // for (i = 0; i < 40; ++i) {
    //     str[0] = '0' + i / 10;
    //     str[1] = '0' + i % 10;
    //     TEST_SUCCESS(bpt_insert(&bpt, i, (uint8_t*)str, 3));
    // }

    // struct record_vec_t vec;

    // // case 0. whole range
    // TEST_SUCCESS(record_vec_init(&vec, 50));
    // TEST(bpt_find_range(&bpt, -100, 100, &vec) == 40);
    // TEST(vec.size == 40);
    // for (i = 0; i < 40; ++i) {
    //     TEST(vec.rec[i].key == i);
    // }
    // TEST_SUCCESS(record_vec_free(&vec));

    // // case 1. half range
    // TEST_SUCCESS(record_vec_init(&vec, 50));
    // TEST(bpt_find_range(&bpt, -100, 20, &vec) == 21);
    // TEST(vec.size == 21);
    // for (i = 0; i < 21; ++i) {
    //     TEST(vec.rec[i].key == i);
    // }
    // TEST_SUCCESS(record_vec_free(&vec));

    // TEST_SUCCESS(record_vec_init(&vec, 50));
    // TEST(bpt_find_range(&bpt, 20, 100, &vec) == 20);
    // TEST(vec.size == 20);
    // for (i = 0; i < 20; ++i) {
    //     TEST(vec.rec[i].key == i + 20);
    // }
    // TEST_SUCCESS(record_vec_free(&vec));

    // // case 2. in range
    // TEST_SUCCESS(record_vec_init(&vec, 50));
    // TEST(bpt_find_range(&bpt, 10, 23, &vec) == 14);
    // TEST(vec.size == 14);
    // for (i = 0; i < 14; ++i) {
    //     TEST(vec.rec[i].key == i + 10);
    // }
    // TEST_SUCCESS(record_vec_free(&vec));

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::print_leaves, {
    // print method
})

TEST_SUITE(BPTreeTest::print_tree, {
    // print method
})

TEST_SUITE(BPTreeTest::write_record, {
    // struct record_t rec;

    // int value = 100;
    // TEST_SUCCESS(make_record(&rec, 10, (uint8_t*)&value, sizeof(int)));
    // TEST(rec.key == 10);
    // TEST(*(int*)rec.value == value);

    // int i;
    // int array[200];
    // for (i = 0; i < 200; ++i) {
    //     array[i] = i;
    // }

    // TEST_SUCCESS(make_record(&rec, 20, (const uint8_t*)array, sizeof(int) * 200));
    // TEST(rec.key == 20);

    // int* ptr = (int*)rec.value;
    // for (i = 0; i < 30; ++i) {
    //     TEST(ptr[i] == array[i]);
    // }
})

TEST_SUITE(BPTreeTest::insert_to_leaf, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // TEST_SUCCESS(bpt_test_config(&bpt, 5, 5));
    // struct ubuffer_t buf = make_node(&bpt, TRUE);

    // // case 0. ordered input
    // int i;
    // struct record_t rec;
    // for (i = 0; i < 5; ++i) {
    //     rec.key = i;
    //     TEST_SUCCESS(insert_into_leaf(&bpt, &buf, &rec));
    // }

    // for (i = 0; i < 5; ++i) {
    //     TEST(records(from_ubuffer(&buf))[i].key == i);
    // }

    // // case 0.1. overflow
    // TEST(insert_into_leaf(&bpt, &buf, &rec) == FAILURE);

    // // case 1. reversed ordered input
    // page_header(from_ubuffer(&buf))->number_of_keys = 0;
    // for (i = 0; i < 5; ++i) {
    //     rec.key = 5 - i;
    //     TEST_SUCCESS(insert_into_leaf(&bpt, &buf, &rec));
    // }

    // for (i = 0; i < 5; ++i) {
    //     TEST(records(from_ubuffer(&buf))[i].key == i + 1);
    // }

    // // case 2. random input
    // int arr[5];
    // arr[0] = 1;
    // arr[1] = 3;
    // arr[2] = 2;
    // arr[3] = 5;
    // arr[4] = 4;

    // page_header(from_ubuffer(&buf))->number_of_keys = 0;
    // for (i = 0; i < 5; ++i) {
    //     rec.key = arr[i];
    //     TEST_SUCCESS(insert_into_leaf(&bpt, &buf, &rec));
    // }

    // for (i = 0; i < 5; ++i) {
    //     TEST(records(from_ubuffer(&buf))[i].key == i + 1);
    // }

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::insert_and_split_leaf, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, 5, 4));

    // struct ubuffer_t leaf = make_node(&bpt, TRUE);
    // struct page_t* page = from_ubuffer(&leaf);

    // int i;
    // struct record_t record;

    // // case 0. last one
    // const int size = 4;
    // CHECK_SUCCESS(ubuffer_check(&leaf));
    // for (i = 0; i < size; ++i) {
    //     record.key = i;
    //     TEST_SUCCESS(insert_into_leaf(&bpt, &leaf, &record));
    // }

    // record.key = size;
    // TEST_SUCCESS(insert_into_leaf_after_splitting(&bpt, &leaf, &record));

    // struct ubuffer_t filehdr = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // pagenum_t root = file_header(from_ubuffer(&filehdr))->root_page_number;

    // struct ubuffer_t rootpage = bpt_buffering(&bpt, root);
    // page = from_ubuffer(&rootpage);

    // TEST(page_header(page)->special_page_number == ubuffer_pagenum(&leaf));
    // TEST(page_header(page)->number_of_keys == 1);

    // int expected = cut(size);
    // TEST(entries(page)[0].key == expected);
    // pagenum_t right = entries(page)[0].pagenum;

    // TEST_SUCCESS(ubuffer_check(&leaf));
    // page = from_ubuffer(&leaf);
    // TEST(page_header(page)->number_of_keys == expected);
    // TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&rootpage));
    // TEST(page_header(page)->special_page_number == right);
    // for (i = 0; i < expected; ++i) {
    //     TEST(records(page)[i].key == i);
    // }

    // struct ubuffer_t rightbuf = bpt_buffering(&bpt, right);
    // page = from_ubuffer(&rightbuf);
    // expected = size + 1 - expected;
    // TEST(page_header(page)->number_of_keys == expected);
    // TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&rootpage));
    // TEST(page_header(page)->special_page_number == INVALID_PAGENUM);
    // for (i = 0; i < expected; ++i) {
    //     TEST(records(page)[i].key == i + cut(size));
    // }

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::insert_to_node, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // TEST_SUCCESS(bpt_test_config(&bpt, 5, 5));
    // struct ubuffer_t buf = make_node(&bpt, FALSE);

    // // case 0. ordered input
    // int i;
    // struct internal_t ent;
    // for (i = 0; i < 5; ++i) {
    //     ent.key = i;
    //     TEST_SUCCESS(insert_into_node(&bpt, &buf, i, &ent));
    // }

    // for (i = 0; i < 5; ++i) {
    //     TEST(entries(from_ubuffer(&buf))[i].key == i);
    // }

    // // case 0.1. overflow
    // TEST(insert_into_node(&bpt, &buf, 5, &ent) == FAILURE);

    // // // case 1. reversed ordered input
    // page_header(from_ubuffer(&buf))->number_of_keys = 0;
    // for (i = 0; i < 5; ++i) {
    //     ent.key = 5 - i;
    //     TEST_SUCCESS(insert_into_node(&bpt, &buf, 0, &ent));
    // }

    // for (i = 0; i < 5; ++i) {
    //     TEST(entries(from_ubuffer(&buf))[i].key == i + 1);
    // }

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::insert_and_split_node, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // const int leaf_order = 4;
    // const int internal_order = 5;
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));

    // struct ubuffer_t tmp;
    // struct ubuffer_t leaf;
    // struct ubuffer_t node = make_node(&bpt, FALSE);
    // pagenum_t nodenum = ubuffer_pagenum(&node);
    // page_header(from_ubuffer(&node))->number_of_keys = internal_order - 1;

    // int i;
    // pagenum_t pages[5 + 1];
    // struct page_t* page;
    // struct internal_t* ent;
    // for (i = -1; i < internal_order - 1; ++i) {
    //     tmp = make_node(&bpt, TRUE);
    //     pages[i + 1] = ubuffer_pagenum(&tmp);
    //     BUFFER(tmp, WRITE_FLAG, {
    //         page_header(from_ubuffer(&tmp))->parent_page_number = nodenum;
    //         page_header(from_ubuffer(&tmp))->number_of_keys = leaf_order - 1;
    //     })
    //     if (i != -1) {
    //         BUFFER(leaf, WRITE_FLAG, {
    //             page_header(from_ubuffer(&leaf))->special_page_number
    //                 = ubuffer_pagenum(&tmp);
    //         })
    //     }
    //     leaf = tmp;

    //     BUFFER(node, WRITE_FLAG, {
    //         page = from_ubuffer(&node);

    //         if (i == -1) {
    //             page_header(page)->special_page_number = ubuffer_pagenum(&leaf);
    //         } else {
    //             ent = &entries(page)[i];
    //             ent->key = i * leaf_order;
    //             ent->pagenum = ubuffer_pagenum(&leaf);
    //         }
    //     })
    // }

    // TEST_SUCCESS(ubuffer_check(&node));
    // page = from_ubuffer(&node);
    // for (i = -1; i < internal_order - 1; ++i) {
    //     if (i == -1) {
    //         TEST(pages[i + 1] == page_header(page)->special_page_number);
    //     } else {
    //         TEST(pages[i + 1] == entries(page)[i].pagenum);
    //     }
    // }

    // leaf = make_node(&bpt, TRUE);
    // pages[internal_order] = ubuffer_pagenum(&leaf);
    // BUFFER(leaf, WRITE_FLAG, {
    //     page_header(from_ubuffer(&leaf))->parent_page_number = nodenum;
    //     page_header(from_ubuffer(&leaf))->number_of_keys = leaf_order - 1;
    // })

    // struct internal_t val;
    // val.key = (internal_order - 1) * leaf_order;
    // val.pagenum = ubuffer_pagenum(&leaf);
    // TEST_SUCCESS(insert_into_node_after_splitting(&bpt, &node, internal_order - 1, &val));

    // struct ubuffer_t filehdr = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // pagenum_t root = file_header(from_ubuffer(&filehdr))->root_page_number;

    // struct ubuffer_t rootpage = bpt_buffering(&bpt, root);
    // page = from_ubuffer(&rootpage);

    // TEST(page_header(page)->is_leaf == FALSE);
    // TEST(page_header(page)->number_of_keys == 1);
    // TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    // TEST(page_header(page)->special_page_number == nodenum);

    // const int split = cut(internal_order);
    // TEST(entries(page)[0].key == (split - 1) * leaf_order);
    // pagenum_t rightnum = entries(page)[0].pagenum;

    // TEST_SUCCESS(ubuffer_check(&node));
    
    // page = from_ubuffer(&node);
    // TEST(page_header(page)->is_leaf == FALSE);
    // TEST(page_header(page)->number_of_keys == split - 1);
    // TEST(page_header(page)->parent_page_number == root);

    // for (i = -1; i < split - 1; ++i) {
    //     BUFFER(node, READ_FLAG, {
    //         page = from_ubuffer(&node);
    //         if (i == -1) {
    //             TEST(page_header(page)->special_page_number == pages[0]);
    //         } else {
    //             TEST(entries(page)[i].key == i * leaf_order);
    //             TEST(entries(page)[i].pagenum == pages[i + 1]);
    //         }
    //     })

    //     tmp = bpt_buffering(&bpt, pages[i + 1]);
    //     BUFFER(tmp, READ_FLAG, {
    //         TEST(page_header(from_ubuffer(&tmp))->parent_page_number == nodenum);
    //     })
    // }

    // const int expected = internal_order - split;
    // struct ubuffer_t right = bpt_buffering(&bpt, rightnum);

    // page = from_ubuffer(&right);
    // TEST(page_header(page)->is_leaf == FALSE);
    // TEST(page_header(page)->number_of_keys == expected);
    // TEST(page_header(page)->parent_page_number == root);

    // for (i = -1; i < expected; ++i) {
    //     BUFFER(right, READ_FLAG, {
    //         page = from_ubuffer(&right);
    //         if (i == -1) {
    //             TEST(page_header(page)->special_page_number == pages[split + i + 1]);
    //         } else {
    //             TEST(entries(page)[i].key == (split + i) * leaf_order);
    //             TEST(entries(page)[i].pagenum == pages[split + 1 + i]);
    //         }
    //     })

    //     tmp = bpt_buffering(&bpt, pages[split + 1 + i]);
    //     BUFFER(tmp, READ_FLAG, {
    //         TEST(page_header(from_ubuffer(&tmp))->parent_page_number == rightnum);
    //     })
    // }

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::insert_to_parent, {
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, 7, 5));

    // // case 0. new root
    // struct ubuffer_t left = make_node(&bpt, TRUE);
    // struct ubuffer_t right = make_node(&bpt, TRUE);

    // BUFFER(left, WRITE_FLAG, {
    //     page_header(from_ubuffer(&left))->parent_page_number = INVALID_PAGENUM;
    // })
    // TEST_SUCCESS(insert_into_parent(&bpt, &left, 10, &right));

    // struct ubuffer_t buf = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // pagenum_t root = file_header(from_ubuffer(&buf))->root_page_number;

    // buf = bpt_buffering(&bpt, root);
    // struct page_t* page = from_ubuffer(&buf);
    // TEST(page_header(page)->is_leaf == FALSE);
    // TEST(page_header(page)->number_of_keys == 1);
    // TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    // TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    // TEST(entries(page)[0].pagenum == ubuffer_pagenum(&right));
    // TEST(entries(page)[0].key == 10);

    // // case 1. simple insert
    // struct ubuffer_t parent = make_node(&bpt, FALSE);
    // struct ubuffer_t temporal = make_node(&bpt, TRUE);
    // BUFFER(parent, WRITE_FLAG, {
    //     page = from_ubuffer(&parent);
    //     page_header(page)->number_of_keys = 1;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&temporal);
    //     entries(page)[0].key = 10;
    //     entries(page)[0].pagenum = ubuffer_pagenum(&left);
    // })
    // BUFFER(temporal, WRITE_FLAG, {
    //     page_header(from_ubuffer(&temporal))->special_page_number = ubuffer_pagenum(&left);
    // })
    // BUFFER(left, WRITE_FLAG, {
    //     page_header(from_ubuffer(&left))->special_page_number = ubuffer_pagenum(&right);
    //     page_header(from_ubuffer(&left))->parent_page_number = ubuffer_pagenum(&parent);    
    //     for (i = 0; i < 3; ++i) {
    //         records(from_ubuffer(&left))[i].key = 10 + i;
    //         page_header(from_ubuffer(&left))->number_of_keys++;
    //     }
    // })
    // BUFFER(right, WRITE_FLAG, {
    //     page_header(from_ubuffer(&right))->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = 0; i < 3; ++i) {
    //         records(from_ubuffer(&right))[i].key = 13 + i;
    //         page_header(from_ubuffer(&right))->number_of_keys++;
    //     }
    // })

    // TEST_SUCCESS(insert_into_parent(&bpt, &left, 13, &right));

    // page = from_ubuffer(&parent);
    // TEST(page_header(page)->number_of_keys == 2);
    // TEST(page_header(page)->special_page_number == ubuffer_pagenum(&temporal));
    // TEST(entries(page)[0].key == 10);
    // TEST(entries(page)[0].pagenum == ubuffer_pagenum(&left));
    // TEST(entries(page)[1].key == 13);
    // TEST(entries(page)[1].pagenum == ubuffer_pagenum(&right));
    // TEST(page_header(from_ubuffer(&temporal))->special_page_number == ubuffer_pagenum(&left));
    // TEST(page_header(from_ubuffer(&left))->special_page_number == ubuffer_pagenum(&right));
    // TEST(page_header(from_ubuffer(&right))->special_page_number == INVALID_PAGENUM);

    // // case 2. node split
    // // TODO: impl test

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::insert_new_root, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t left = make_node(&bpt, TRUE);
    // struct ubuffer_t right = make_node(&bpt, TRUE);

    // TEST_SUCCESS(insert_into_new_root(&bpt, &left, 10, &right));
    
    // struct ubuffer_t buf = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // pagenum_t root = file_header(from_ubuffer(&buf))->root_page_number;

    // buf = bpt_buffering(&bpt, root);
    // struct page_t* page = from_ubuffer(&buf);
    // TEST(page_header(page)->is_leaf == FALSE);
    // TEST(page_header(page)->number_of_keys == 1);
    // TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    
    // TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    // TEST(entries(page)[0].pagenum == ubuffer_pagenum(&right));

    // TEST(page_header(from_ubuffer(&left))->parent_page_number == ubuffer_pagenum(&buf));
    // TEST(page_header(from_ubuffer(&right))->parent_page_number == ubuffer_pagenum(&buf));

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::new_tree, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct record_t rec;
    // rec.key = 10;
    // *(int*)rec.value = 20;

    // TEST_SUCCESS(start_new_tree(&bpt, &rec));

    // struct ubuffer_t ubuf = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // pagenum_t root = file_header(from_ubuffer(&ubuf))->root_page_number;
    // TEST(root != INVALID_PAGENUM);

    // ubuf = bpt_buffering(&bpt, root);
    // struct page_t* page = from_ubuffer(&ubuf);
    // TEST(page_header(page)->is_leaf == TRUE);
    // TEST(page_header(page)->number_of_keys == 1);
    // TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    // TEST(records(page)[0].key == 10);
    // TEST(*(int*)records(page)[0].value == 20);

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::insert, {
    // int i;
    // int j;
    // int idx;
    // int arr[40];
    // char str[] = "00";
    // const int leaf_order = 4;
    // const int internal_order = 5;

    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;

    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    // for (i = 0; i < 40; ++i) {
    //     str[0] = '0' + i / 10;
    //     str[1] = '0' + i % 10;
    //     TEST_SUCCESS(bpt_insert(&bpt, i, (uint8_t*)str, 3));
    // }
    // print_tree(&bpt);
    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));


    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    // for (i = 40; i > 0; --i) {
    //     str[0] = '0' + i / 10;
    //     str[1] = '0' + i % 10;
    //     TEST_SUCCESS(bpt_insert(&bpt, i, (uint8_t*)str, 3));
    // }
    // print_tree(&bpt);
    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));


    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    // for (i = 0; i < 40; ++i) {
    //     arr[i] = i;
    // }
    // for (i = 40; i > 0; --i) {
    //     idx = rand() % i;
    //     j = arr[idx];
    //     str[0] = '0' + j / 10;
    //     str[1] = '0' + j % 10;
    //     TEST_SUCCESS(bpt_insert(&bpt, j, (uint8_t*)str, 3));

    //     for (j = idx; j < i - 1; ++j) {
    //         arr[j] = arr[j + 1];
    //     }
    // }
    // print_tree(&bpt);
    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::remove_record_from_leaf, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t buf = make_node(&bpt, TRUE);
    // struct page_t* page = from_ubuffer(&buf);

    // int i;
    // page_header(page)->number_of_keys = 5;
    // for (i = 0; i < 5; ++i) {
    //     records(page)[i].key = i;
    // }

    // TEST_SUCCESS(remove_record_from_leaf(0, &buf));
    // TEST(page_header(page)->number_of_keys == 4);
    // for (i = 0; i < 4; ++i) {
    //     TEST(records(page)[i].key == i + 1);
    // }

    // TEST_SUCCESS(remove_record_from_leaf(4, &buf));
    // TEST(page_header(page)->number_of_keys == 3);
    // for (i = 0; i < 3; ++i) {
    //     TEST(records(page)[i].key == i + 1);
    // }
    
    // TEST_SUCCESS(remove_record_from_leaf(2, &buf));
    // TEST(page_header(page)->number_of_keys == 2);
    // TEST(records(page)[0].key == 1);
    // TEST(records(page)[1].key == 3);

    // TEST_SUCCESS(remove_record_from_leaf(1, &buf));
    // TEST(page_header(page)->number_of_keys == 1);
    // TEST(records(page)[0].key == 3);

    // TEST_SUCCESS(remove_record_from_leaf(3, &buf));
    // TEST(page_header(page)->number_of_keys == 0);

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::remove_entry_from_internal, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t buf = make_node(&bpt, FALSE);
    // struct page_t* page = from_ubuffer(&buf);

    // int i;
    // page_header(page)->number_of_keys = 5;
    // for (i = 0; i < 5; ++i) {
    //     entries(page)[i].key = i;
    // }

    // TEST_SUCCESS(remove_entry_from_internal(0, &buf));
    // TEST(page_header(page)->number_of_keys == 4);
    // for (i = 0; i < 4; ++i) {
    //     TEST(entries(page)[i].key == i + 1);
    // }

    // TEST_SUCCESS(remove_entry_from_internal(4, &buf));
    // TEST(page_header(page)->number_of_keys == 3);
    // for (i = 0; i < 3; ++i) {
    //     TEST(entries(page)[i].key == i + 1);
    // }
    
    // TEST_SUCCESS(remove_entry_from_internal(2, &buf));
    // TEST(page_header(page)->number_of_keys == 2);
    // TEST(entries(page)[0].key == 1);
    // TEST(entries(page)[1].key == 3);

    // TEST_SUCCESS(remove_entry_from_internal(1, &buf));
    // TEST(page_header(page)->number_of_keys == 1);
    // TEST(entries(page)[0].key == 3);

    // TEST_SUCCESS(remove_entry_from_internal(3, &buf));
    // TEST(page_header(page)->number_of_keys == 0);

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));

})

TEST_SUITE(BPTreeTest::shrink_root, {
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // // case 0. no shrink
    // struct ubuffer_t node = make_node(&bpt, TRUE);
    // struct ubuffer_t filehdr = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // BUFFER(node, WRITE_FLAG, {
    //     page_header(from_ubuffer(&node))->number_of_keys = 1;
    //     records(from_ubuffer(&node))[0].key = 10;
    // })
    // BUFFER(filehdr, WRITE_FLAG, {
    //     file_header(from_ubuffer(&filehdr))->root_page_number =
    //         ubuffer_pagenum(&node);
    // })

    // TEST_SUCCESS(shrink_root(&bpt));
    // BUFFER(filehdr, READ_FLAG, {
    //     TEST(
    //         file_header(from_ubuffer(&filehdr))->root_page_number =
    //             ubuffer_pagenum(&node));
    // })
    // BUFFER(node, READ_FLAG, {
    //     TEST(page_header(from_ubuffer(&node))->number_of_keys == 1);
    //     TEST(records(from_ubuffer(&node))[0].key == 10);
    // })

    // // case 1. leaf root
    // BUFFER(node, WRITE_FLAG, {
    //     page_header(from_ubuffer(&node))->number_of_keys = 0;
    // })
    // TEST_SUCCESS(shrink_root(&bpt));
    // BUFFER(filehdr, READ_FLAG, {
    //     TEST(
    //         file_header(from_ubuffer(&filehdr))->root_page_number ==
    //             INVALID_PAGENUM);
    // })

    // // case 2. internal root
    // node = make_node(&bpt, TRUE);
    // pagenum_t nodenum = ubuffer_pagenum(&node);

    // node = make_node(&bpt, FALSE);
    // BUFFER(filehdr, WRITE_FLAG, {
    //     TEST(file_header(from_ubuffer(&filehdr))->root_page_number =
    //         ubuffer_pagenum(&node));
    // })
    // BUFFER(node, WRITE_FLAG, {
    //     page_header(from_ubuffer(&node))->special_page_number = nodenum;;
    // })

    // TEST_SUCCESS(shrink_root(&bpt));
    // BUFFER(filehdr, READ_FLAG, {
    //     TEST(
    //         file_header(from_ubuffer(&filehdr))->root_page_number ==
    //             nodenum);
    // })

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::merge_nodes, {
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // // case 0. leaf node
    // struct ubuffer_t parent = make_node(&bpt, FALSE);
    // struct ubuffer_t left = make_node(&bpt, TRUE);
    // struct ubuffer_t right = make_node(&bpt, TRUE);
    // struct ubuffer_t rightmost = make_node(&bpt, TRUE);

    // pagenum_t parentnum; 
    // struct page_t* page; 
    // BUFFER(parent, WRITE_FLAG, {
    //     parentnum = ubuffer_pagenum(&parent);
    //     page = from_ubuffer(&parent);
    //     page_header(page)->number_of_keys = 2;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&left);
    //     entries(page)[0].key = 2;
    //     entries(page)[0].pagenum = ubuffer_pagenum(&right);
    //     entries(page)[1].key = 10;
    //     entries(page)[1].pagenum = ubuffer_pagenum(&rightmost);
    // })
    // BUFFER(left, WRITE_FLAG, {
    //     page = from_ubuffer(&left);
    //     page_header(page)->number_of_keys = 2;
    //     page_header(page)->parent_page_number = parentnum;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&right);
    //     for (i = 0; i < 2; ++i) {
    //         records(page)[i].key = i;
    //     }
    // })
    // TEST_SUCCESS(ubuffer_check(&right));
    // BUFFER(right, WRITE_FLAG, {
    //     page = from_ubuffer(&right);
    //     page_header(page)->number_of_keys = 3;
    //     page_header(page)->parent_page_number = parentnum;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&rightmost);
    //     for (i = 0; i < 3; ++i) {
    //         records(page)[i].key = 2 + i;
    //     }
    // })
    
    // TEST_SUCCESS(merge_nodes(&bpt, &left, 2, &right, &parent));
    // BUFFER(parent, READ_FLAG, {
    //     page = from_ubuffer(&parent);
    //     TEST(page_header(page)->number_of_keys == 1)
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    //     TEST(entries(page)[0].key == 10);
    //     TEST(entries(page)[0].pagenum == ubuffer_pagenum(&rightmost))
    // })

    // BUFFER(left, READ_FLAG, {
    //     page = from_ubuffer(&left);
    //     TEST(page_header(page)->number_of_keys == 5);
    //     TEST(page_header(page)->parent_page_number == parentnum);
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&rightmost));
    //     for (i = 0; i < 5; ++i) {
    //         TEST(records(page)[i].key == i);
    //     }
    // })

    // // case 1. internal node
    // parent = make_node(&bpt, FALSE);
    // left = make_node(&bpt, FALSE);
    // right = make_node(&bpt, FALSE);
    // rightmost = make_node(&bpt, FALSE);

    // BUFFER(parent, WRITE_FLAG, {
    //     page = from_ubuffer(&parent);
    //     page_header(page)->number_of_keys = 2;
    //     page_header(page)->parent_page_number = INVALID_PAGENUM;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&left);
    //     entries(page)[0].key = 3;
    //     entries(page)[0].pagenum = ubuffer_pagenum(&right);
    //     entries(page)[1].key = 10;
    //     entries(page)[1].pagenum = ubuffer_pagenum(&rightmost);
    // })
    // struct ubuffer_t tmp;
    // pagenum_t tmpnum;
    // pagenum_t pagenums[7];
    // BUFFER(left, WRITE_FLAG, {
    //     tmpnum = ubuffer_pagenum(&left);
    //     page = from_ubuffer(&left);
    //     page_header(page)->number_of_keys = 2;
    //     page_header(page)->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = 0; i < 3; ++i) {
    //         tmp = make_node(&bpt, TRUE);
    //         pagenums[i] = ubuffer_pagenum(&tmp);
    //         if (i == 0) {
    //             page_header(page)->special_page_number = pagenums[i];
    //         } else {
    //             entries(page)[i - 1].key = i;
    //             entries(page)[i - 1].pagenum = pagenums[i];
    //         }

    //         BUFFER(tmp, WRITE_FLAG, {
    //             page_header(from_ubuffer(&tmp))->parent_page_number = tmpnum;
    //         })
    //     }
    // })
    // BUFFER(right, WRITE_FLAG, {
    //     tmpnum = ubuffer_pagenum(&right);
    //     page = from_ubuffer(&right);
    //     page_header(page)->number_of_keys = 3;
    //     page_header(page)->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = 0; i < 4; ++i) {
    //         tmp = make_node(&bpt, TRUE);
    //         pagenums[3 + i] = ubuffer_pagenum(&tmp);
    //         if (i == 0) {
    //             page_header(page)->special_page_number = pagenums[3 + i];
    //         } else {
    //             entries(page)[i - 1].key = 3 + i;
    //             entries(page)[i - 1].pagenum = pagenums[3 + i];
    //         }

    //         BUFFER(tmp, WRITE_FLAG, {
    //             page_header(from_ubuffer(&tmp))->parent_page_number = tmpnum;
    //         })
    //     }
    // })
    // TEST_SUCCESS(merge_nodes(&bpt, &left, 3, &right, &parent));

    // BUFFER(parent, READ_FLAG, {
    //     page = from_ubuffer(&parent);
    //     TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    //     TEST(page_header(page)->number_of_keys == 1);
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    //     TEST(entries(page)[0].key == 10);
    //     TEST(entries(page)[0].pagenum == ubuffer_pagenum(&rightmost));
    // })

    // BUFFER(left, READ_FLAG, {
    //     tmpnum = ubuffer_pagenum(&left);
    //     page = from_ubuffer(&left);
    //     TEST(page_header(page)->number_of_keys == 6);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = 0; i < 7; ++i) {
    //         if (i == 0) {
    //             TEST(page_header(page)->special_page_number == pagenums[i]);
    //         } else {
    //             TEST(entries(page)[i - 1].key == i);
    //             TEST(entries(page)[i - 1].pagenum == pagenums[i]);
    //         }

    //         tmp = bpt_buffering(&bpt, pagenums[i]);
    //         BUFFER(tmp, READ_FLAG, {
    //             TEST(page_header(from_ubuffer(&tmp))->parent_page_number == tmpnum);
    //         })
    //     }
    // })

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::rotate_to_right, {
    // in redistribute_nodes test
})

TEST_SUITE(BPTreeTest::rotate_to_left, {
    // in redistribute_nodes test
})

TEST_SUITE(BPTreeTest::redistribute_nodes_leaf, {
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t parent = make_node(&bpt, FALSE);
    // struct ubuffer_t left = make_node(&bpt, TRUE);
    // struct ubuffer_t right = make_node(&bpt, TRUE);
    
    // struct page_t* page;
    // BUFFER(parent, WRITE_FLAG, {
    //     page = from_ubuffer(&parent);
    //     page_header(page)->number_of_keys = 1;
    //     page_header(page)->parent_page_number = INVALID_PAGENUM;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&left);
    //     entries(page)[0].key = 3;
    //     entries(page)[0].pagenum = ubuffer_pagenum(&right);
    // })
    // BUFFER(left, WRITE_FLAG, {
    //     page = from_ubuffer(&left);
    //     page_header(page)->number_of_keys = 3;
    //     page_header(page)->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = 0; i < 3; ++i) {
    //         records(page)[i].key = i;
    //         records(page)[i].value[0] = i;
    //     }
    // })
    // BUFFER(right, WRITE_FLAG, {
    //     page = from_ubuffer(&right);
    //     page_header(page)->number_of_keys = 2;
    //     page_header(page)->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = 0; i < 2; ++i) {
    //         records(page)[i].key = 3 + i;
    //         records(page)[i].value[0] = 3 + i;
    //     }
    // })

    // TEST_SUCCESS(redistribute_nodes(&bpt, &left, 3, 0, &right, &parent));

    // BUFFER(parent, READ_FLAG, {
    //     page = from_ubuffer(&parent);
    //     TEST(page_header(page)->number_of_keys == 1);
    //     TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    //     TEST(entries(page)[0].key == 2);
    //     TEST(entries(page)[0].pagenum == ubuffer_pagenum(&right));
    // })
    // BUFFER(left, READ_FLAG, {
    //     page = from_ubuffer(&left);
    //     TEST(page_header(page)->number_of_keys == 2);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = 0; i < 2; ++i) {
    //         TEST(records(page)[i].key == i);
    //         TEST(records(page)[i].value[0] == i);
    //     }
    // })
    // BUFFER(right, READ_FLAG, {
    //     page = from_ubuffer(&right);
    //     TEST(page_header(page)->number_of_keys == 3);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = 0; i < 3; ++i) {
    //         TEST(records(page)[i].key == 2 + i);
    //         TEST(records(page)[i].value[0] == 2 + i);
    //     }
    // })

    // TEST_SUCCESS(redistribute_nodes(&bpt, &left, 3, 0, &right, &parent));

    // BUFFER(parent, READ_FLAG, {
    //     page = from_ubuffer(&parent);
    //     TEST(page_header(page)->number_of_keys == 1);
    //     TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    //     TEST(entries(page)[0].key == 3);
    //     TEST(entries(page)[0].pagenum == ubuffer_pagenum(&right));
    // })
    // BUFFER(left, READ_FLAG, {
    //     page = from_ubuffer(&left);
    //     TEST(page_header(page)->number_of_keys == 3);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = 0; i < 3; ++i) {
    //         TEST(records(page)[i].key == i);
    //         TEST(records(page)[i].value[0] == i);
    //     }
    // })
    // BUFFER(right, READ_FLAG, {
    //     page = from_ubuffer(&right);
    //     TEST(page_header(page)->number_of_keys == 2);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = 0; i < 2; ++i) {
    //         TEST(records(page)[i].key == 3 + i);
    //         TEST(records(page)[i].value[0] == 3 + i);
    //     }
    // })

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::redistribute_nodes_internal, {
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // struct ubuffer_t parent = make_node(&bpt, FALSE);
    // struct ubuffer_t left = make_node(&bpt, FALSE);
    // struct ubuffer_t right = make_node(&bpt, FALSE);
    
    // struct page_t* page;
    // BUFFER(parent, WRITE_FLAG, {
    //     page = from_ubuffer(&parent);
    //     page_header(page)->number_of_keys = 1;
    //     page_header(page)->parent_page_number = INVALID_PAGENUM;
    //     page_header(page)->special_page_number = ubuffer_pagenum(&left);
    //     entries(page)[0].key = 3;
    //     entries(page)[0].pagenum = ubuffer_pagenum(&right);
    // })
    // pagenum_t tmpnum;
    // pagenum_t pagenums[7];
    // struct ubuffer_t tmp;
    // BUFFER(left, WRITE_FLAG, {
    //     tmpnum = ubuffer_pagenum(&left);
    //     page = from_ubuffer(&left);
    //     page_header(page)->number_of_keys = 3;
    //     page_header(page)->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = -1; i < 3; ++i) {
    //         tmp = make_node(&bpt, TRUE);
    //         pagenums[i + 1] = ubuffer_pagenum(&tmp);
    //         if (i == -1) {
    //             page_header(page)->special_page_number = pagenums[i + 1];
    //         } else {
    //             entries(page)[i].key = i;
    //             entries(page)[i].pagenum = pagenums[i + 1];
    //         }

    //         BUFFER(tmp, WRITE_FLAG, {
    //             page_header(from_ubuffer(&tmp))->parent_page_number = tmpnum;
    //         })
    //     }
    // })
    // BUFFER(right, WRITE_FLAG, {
    //     tmpnum = ubuffer_pagenum(&right);
    //     page = from_ubuffer(&right);
    //     page_header(page)->number_of_keys = 2;
    //     page_header(page)->parent_page_number = ubuffer_pagenum(&parent);
    //     for (i = -1; i < 2; ++i) {
    //         tmp = make_node(&bpt, TRUE);
    //         pagenums[i + 5] = ubuffer_pagenum(&tmp);
    //         if (i == -1) {
    //             page_header(page)->special_page_number = pagenums[i + 5];
    //         } else {
    //             entries(page)[i].key = 4 + i;
    //             entries(page)[i].pagenum = pagenums[i + 5];
    //         }

    //         BUFFER(tmp, WRITE_FLAG, {
    //             page_header(from_ubuffer(&tmp))->parent_page_number = tmpnum;
    //         })
    //     }
    // })

    // TEST_SUCCESS(redistribute_nodes(&bpt, &left, 3, 0, &right, &parent));

    // BUFFER(parent, READ_FLAG, {
    //     page = from_ubuffer(&parent);
    //     TEST(page_header(page)->number_of_keys == 1);
    //     TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    //     TEST(entries(page)[0].key == 2);
    //     TEST(entries(page)[0].pagenum == ubuffer_pagenum(&right));
    // })
    // BUFFER(left, READ_FLAG, {
    //     tmpnum = ubuffer_pagenum(&left);
    //     page = from_ubuffer(&left);
    //     TEST(page_header(page)->number_of_keys == 2);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = -1; i < 2; ++i) {
    //         if (i == -1) {
    //             TEST(page_header(page)->special_page_number == pagenums[i + 1]);
    //         } else {
    //             TEST(entries(page)[i].key == i);
    //             TEST(entries(page)[i].pagenum == pagenums[i + 1]);
    //         }

    //         tmp = bpt_buffering(&bpt, pagenums[i + 1]);
    //         BUFFER(tmp, READ_FLAG, {
    //             TEST(page_header(from_ubuffer(&tmp))->parent_page_number == tmpnum);
    //         })
    //     }
    // })
    // BUFFER(right, READ_FLAG, {
    //     tmpnum = ubuffer_pagenum(&right);
    //     page = from_ubuffer(&right);
    //     TEST(page_header(page)->number_of_keys == 3);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = -1; i < 3; ++i) {
    //         if (i == -1) {
    //             TEST(page_header(page)->special_page_number == pagenums[i + 4]);
    //         } else {
    //             TEST(entries(page)[i].key == i + 3);
    //             TEST(entries(page)[i].pagenum == pagenums[i + 4]);
    //         }
    //         tmp = bpt_buffering(&bpt, pagenums[i + 4]);
    //         BUFFER(tmp, READ_FLAG, {
    //             TEST(page_header(from_ubuffer(&tmp))->parent_page_number == tmpnum);
    //         })
    //     }
    // })

    // TEST_SUCCESS(redistribute_nodes(&bpt, &left, 2, 0, &right, &parent));

    // BUFFER(parent, READ_FLAG, {
    //     page = from_ubuffer(&parent);
    //     TEST(page_header(page)->number_of_keys == 1);
    //     TEST(page_header(page)->parent_page_number == INVALID_PAGENUM);
    //     TEST(page_header(page)->special_page_number == ubuffer_pagenum(&left));
    //     TEST(entries(page)[0].key == 3);
    //     TEST(entries(page)[0].pagenum == ubuffer_pagenum(&right));
    // })
    // BUFFER(left, READ_FLAG, {
    //     tmpnum = ubuffer_pagenum(&left);
    //     page = from_ubuffer(&left);
    //     TEST(page_header(page)->number_of_keys == 3);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = -1; i < 3; ++i) {
    //         if (i == -1) {
    //             TEST(page_header(page)->special_page_number == pagenums[i + 1]);
    //         } else {
    //             TEST(entries(page)[i].key == i);
    //             TEST(entries(page)[i].pagenum == pagenums[i + 1]);
    //         }

    //         tmp = bpt_buffering(&bpt, pagenums[i + 1]);
    //         BUFFER(tmp, READ_FLAG, {
    //             TEST(page_header(from_ubuffer(&tmp))->parent_page_number == tmpnum);
    //         })
    //     }
    // })
    // BUFFER(right, READ_FLAG, {
    //     tmpnum = ubuffer_pagenum(&right);
    //     page = from_ubuffer(&right);
    //     TEST(page_header(page)->number_of_keys == 2);
    //     TEST(page_header(page)->parent_page_number == ubuffer_pagenum(&parent));
    //     for (i = -1; i < 2; ++i) {
    //         if (i == -1) {
    //             TEST(page_header(page)->special_page_number == pagenums[i + 5]);
    //         } else {
    //             TEST(entries(page)[i].key == i + 4);
    //             TEST(entries(page)[i].pagenum == pagenums[i + 5]);
    //         }
    //         tmp = bpt_buffering(&bpt, pagenums[i + 5]);
    //         BUFFER(tmp, READ_FLAG, {
    //             TEST(page_header(from_ubuffer(&tmp))->parent_page_number == tmpnum);
    //         })
    //     }
    // })

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::delete_entry, {
    // TODO
})

TEST_SUITE(BPTreeTest::remove, {
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // const int leaf_order = 4;
    // const int internal_order = 5;
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;

    // // case root
    // char str[] = "00";
    // for (i = 0; i < leaf_order - 1; ++i) {
    //     TEST_SUCCESS(bpt_insert(&bpt, i * 10, str, sizeof(str)));
    // }
    // print_tree(&bpt);
    // for (i = 0; i < leaf_order - 1; ++i) {
    //     TEST_SUCCESS(bpt_delete(&bpt, i * 10));
    //     print_tree(&bpt);
    // }
    // struct ubuffer_t buffer = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // TEST(file_header(from_ubuffer(&buffer))->root_page_number == INVALID_PAGENUM);

    // // case simple delete
    // for (i = 0; i < leaf_order * internal_order; ++i) {
    //     TEST_SUCCESS(bpt_insert(&bpt, i, str, sizeof(str)));
    // }
    // print_tree(&bpt);

    // for (i = 10; i <= 11; ++i) {
    //     TEST_SUCCESS(bpt_delete(&bpt, i));
    // }
    // print_tree(&bpt);

    // // case merge
    // for (i = 8; i <= 9; ++i) {
    //     TEST_SUCCESS(bpt_delete(&bpt, i));
    // }
    // print_tree(&bpt);

    // // case rotate to right
    // for (i = -1; i >= -2; --i) {
    //     TEST_SUCCESS(bpt_insert(&bpt, i, str, sizeof(str)));
    // }
    // print_tree(&bpt);

    // for (i = 14; i <= 19; ++i) {
    //     TEST_SUCCESS(bpt_delete(&bpt, i));
    // }
    // print_tree(&bpt);

    // // case left distribute
    // for (i = 14; i <= 19; ++i) {
    //     TEST_SUCCESS(bpt_insert(&bpt, i, str, sizeof(str)));
    // }
    // print_tree(&bpt);

    // for (i = -2; i <= 3; ++i) {
    //     TEST_SUCCESS(bpt_delete(&bpt, i));
    // }
    // print_tree(&bpt);

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

TEST_SUITE(BPTreeTest::destroy_tree, {
    // int i;
    // struct bpt_t bpt;
    // struct file_manager_t file;
    // struct buffer_manager_t buffers;
    // TEST_SUCCESS(bpt_test_preprocess(&bpt, &file, &buffers));

    // const int leaf_order = 4;
    // const int internal_order = 5;
    // TEST_SUCCESS(bpt_test_config(&bpt, leaf_order, internal_order));
    // bpt.verbose_output = FALSE;
    
    // char str[] = "00";
    // for (i = 0; i < leaf_order * internal_order; ++i) {
    //     TEST_SUCCESS(bpt_insert(&bpt, i, str, sizeof(str)));
    // }

    // TEST_SUCCESS(destroy_tree(&bpt));
    // struct ubuffer_t buffer = bpt_buffering(&bpt, FILE_HEADER_PAGENUM);
    // TEST(file_header(from_ubuffer(&buffer))->root_page_number == INVALID_PAGENUM);

    // TEST_SUCCESS(bpt_test_postprocess(&bpt, &file, &buffers));
})

int bptree_test() {
    srand(time(NULL));
    return BPTreeTest::constructor_test()
        && BPTreeTest::destructor_test()
        && BPTreeTest::buffering_test()
        && BPTreeTest::create_page_test()
        && BPTreeTest::free_page_test()
        && BPTreeTest::test_config_test()
        && BPTreeTest::path_to_root_test()
        && BPTreeTest::cut_test()
        && BPTreeTest::find_leaf_test()
        && BPTreeTest::find_key_from_leaf_test()
        && BPTreeTest::find_pagenum_from_internal_test()
        && BPTreeTest::find_test()
        && BPTreeTest::find_range_test()
        && BPTreeTest::print_leaves_test()
        && BPTreeTest::print_tree_test()
        && BPTreeTest::write_record_test()
        && BPTreeTest::insert_to_leaf_test()
        && BPTreeTest::insert_and_split_leaf_test()
        && BPTreeTest::insert_to_node_test()
        && BPTreeTest::insert_and_split_node_test()
        && BPTreeTest::insert_to_parent_test()
        && BPTreeTest::insert_new_root_test()
        && BPTreeTest::new_tree_test()
        && BPTreeTest::insert_test()
        && BPTreeTest::remove_record_from_leaf_test()
        && BPTreeTest::remove_entry_from_internal_test()
        && BPTreeTest::shrink_root_test()
        && BPTreeTest::merge_nodes_test()
        && BPTreeTest::rotate_to_left_test()
        && BPTreeTest::rotate_to_right_test()
        && BPTreeTest::redistribute_nodes_leaf_test()
        && BPTreeTest::redistribute_nodes_internal_test()
        && BPTreeTest::delete_entry_test()
        && BPTreeTest::remove_test()
        && BPTreeTest::destroy_tree_test();
}