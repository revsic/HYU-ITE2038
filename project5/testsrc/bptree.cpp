// #include <random>

// #include "bptree.hpp"
// #include "test.hpp"

// struct BPTreeTest {
// #define TEST_NAME(name) static int name##_test();
//     TEST_NAME(constructor)
//     TEST_NAME(destructor)
//     TEST_NAME(buffering)
//     TEST_NAME(create_page)
//     TEST_NAME(free_page)
//     TEST_NAME(test_config)
//     TEST_NAME(path_to_root)
//     TEST_NAME(cut)
//     TEST_NAME(find_leaf)
//     TEST_NAME(find_key_from_leaf)
//     TEST_NAME(find_pagenum_from_internal)
//     TEST_NAME(find)
//     TEST_NAME(find_range)
//     TEST_NAME(print_leaves)
//     TEST_NAME(print_tree)
//     TEST_NAME(write_record)
//     TEST_NAME(insert_to_leaf)
//     TEST_NAME(insert_and_split_leaf)
//     TEST_NAME(insert_to_node)
//     TEST_NAME(insert_and_split_node)
//     TEST_NAME(insert_to_parent)
//     TEST_NAME(insert_new_root)
//     TEST_NAME(new_tree)
//     TEST_NAME(insert)
//     TEST_NAME(remove_record_from_leaf)
//     TEST_NAME(remove_entry_from_internal)
//     TEST_NAME(shrink_root)
//     TEST_NAME(merge_nodes)
//     TEST_NAME(rotate_to_left)
//     TEST_NAME(rotate_to_right)
//     TEST_NAME(redistribute_nodes_leaf)
//     TEST_NAME(redistribute_nodes_internal)
//     TEST_NAME(delete_entry);
//     TEST_NAME(remove);
//     TEST_NAME(destroy_tree);
// };

// void bpt_test_postprocess(FileManager& file, BufferManager& buffers) {
//     buffers.shutdown();
//     file.~FileManager();
//     remove("testfile");
// }

// TEST_SUITE(BPTreeTest::buffering, {
//     // just porting buffer_manager_buffering
// })

// TEST_SUITE(BPTreeTest::create_page, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer buf = bpt.create_page(true);
//     PageHeader& header = buf.page().page_header();
//     TEST(header.parent_page_number == INVALID_PAGENUM);
//     TEST(header.is_leaf);
//     TEST(header.number_of_keys == 0);
//     TEST(header.special_page_number == INVALID_PAGENUM);

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::free_page, {
//     // just porting buffer_manager_free_page
// })

// TEST_SUITE(BPTreeTest::constructor, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     TEST(bpt.file == &file);
//     TEST(bpt.buffers == &buffers);
//     TEST(bpt.leaf_order == 32);
//     TEST(bpt.internal_order == 249);
//     TEST(bpt.verbose_output == false);
//     TEST(bpt.delayed_merge == true);

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::destructor, {
//     // default destructor
// })

// TEST_SUITE(BPTreeTest::test_config, {
//     BPTree bpt(nullptr, nullptr);
//     bpt.test_config(5, 4, true);
//     TEST(bpt.leaf_order == 5);
//     TEST(bpt.internal_order == 4);
//     TEST(bpt.verbose_output == true);
//     TEST(bpt.delayed_merge == true);
// })

// TEST_SUITE(BPTreeTest::path_to_root, {
//     // print method
// })

// TEST_SUITE(BPTreeTest::cut, {
//     TEST(BPTree::cut(4) == 2);
//     TEST(BPTree::cut(5) == 3);
// })

// TEST_SUITE(BPTreeTest::find_leaf, {
//     char str[] = "00";
//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;

//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     bpt.test_config(leaf_order, internal_order, true);
//     bpt.verbose_output = false;
//     for (int i = 0; i < 40; ++i) {
//         str[0] = '0' + i / 10;
//         str[1] = '0' + i % 10;
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), 3));
//     }

//     Ubuffer buf = bpt.buffering(FILE_HEADER_PAGENUM);
//     pagenum_t pagenum = buf.page().file_header().root_page_number;
//     while (true) {
//         buf = bpt.buffering(pagenum);
//         if (buf.page().page_header().is_leaf) {
//             break;
//         }
//         pagenum = buf.page().page_header().special_page_number;
//     }

//     for (int i = 0; i < 40; ++i) {
//         if (i && i % 2 == 0) {
//             TEST_SUCCESS(buf.use(RWFlag::READ, [&](Page& page) {
//                 pagenum = page.page_header().special_page_number;
//                 return Status::SUCCESS;
//             }));
//             buf = bpt.buffering(pagenum);
//         }
//         Ubuffer tmp(nullptr);
//         TEST(pagenum == bpt.find_leaf(i, tmp));
//     }
//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::find_key_from_leaf, {
//     // preproc
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     // case 0. leaf validation
//     Ubuffer node = bpt.create_page(false);
//     Page& page = node.page();
//     for (int i = 0; i < 3; ++i) {
//         page.page_header().number_of_keys++;
//         page.records()[i].key = i * 10;
//     }

//     TEST(bpt.find_key_from_leaf(10, node, nullptr) == Status::FAILURE);

//     // case 1. cannot find
//     page.page_header().is_leaf = true;
//     page.records()[1].key = 15;
//     TEST(bpt.find_key_from_leaf(10, node, nullptr) == Status::FAILURE);

//     // case 2. find and record=NULL
//     page.records()[1].key = 10;
//     TEST_SUCCESS(bpt.find_key_from_leaf(10, node, nullptr));

//     page.records()[0].key = 10;
//     page.records()[1].key = 15;
//     TEST_SUCCESS(bpt.find_key_from_leaf(10, node, nullptr));

//     page.records()[0].key = 5;
//     page.records()[1].key = 7;
//     page.records()[2].key = 10;
//     TEST_SUCCESS(bpt.find_key_from_leaf(10, node, nullptr));

//     // case 3. find and record
//     Record rec;
//     *(int*)page.records()[2].value = 40;
//     TEST_SUCCESS(bpt.find_key_from_leaf(10, node, &rec));
//     TEST(rec.key == 10);
//     TEST(*(int*)rec.value == 40);

//     // postproc
//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::find_pagenum_from_internal, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer buf = bpt.create_page(false);
//     PageHeader& header = buf.page().page_header();
//     header.number_of_keys = std::min(5, bpt.internal_order - 1);
//     header.special_page_number = 10;

//     for (int i = 0; i < header.number_of_keys; ++i) {
//         buf.page().entries()[i].pagenum = (i + 2) * 10;
//     }

//     int idx;
//     TEST(bpt.find_pagenum_from_internal(5, buf, idx) == Status::FAILURE);
//     TEST(bpt.find_pagenum_from_internal(10 * (header.number_of_keys + 3), buf, idx)
//         == Status::FAILURE);

//     TEST_SUCCESS(bpt.find_pagenum_from_internal(10, buf, idx));
//     TEST(idx == -1);
//     for (int i = 0; i < header.number_of_keys; ++i) {
//         TEST_SUCCESS(bpt.find_pagenum_from_internal((i + 2) * 10, buf, idx));
//         TEST(idx == i);
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::find, {
//     char str[] = "00";
//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;

//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     bpt.test_config(leaf_order, internal_order, true);
//     bpt.verbose_output = false;
//     for (int i = 0; i < 40; ++i) {
//         str[0] = '0' + i / 10;
//         str[1] = '0' + i % 10;
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), 3));
//     }

//     Record rec;
//     for (int i = 0; i < 40; ++i) {
//         TEST_SUCCESS(bpt.find(i, &rec));
//         TEST(rec.value[0] == '0' + i / 10);
//         TEST(rec.value[1] == '0' + i % 10);
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::find_range, {
//     char str[] = "00";
//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;

//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     bpt.test_config(leaf_order, internal_order, true);
//     bpt.verbose_output = false;
//     for (int i = 0; i < 40; ++i) {
//         str[0] = '0' + i / 10;
//         str[1] = '0' + i % 10;
//         TEST_SUCCESS(bpt.insert(i, (uint8_t*)str, 3));
//     }

//     // case 0. whole range
//     std::vector<Record> vec = bpt.find_range(-100, 100);
//     TEST(vec.size() == 40);
//     for (int i = 0; i < 40; ++i) {
//         TEST(vec[i].key == i);
//     }

//     // case 1. half range
//     vec = bpt.find_range(-100, 20);
//     TEST(vec.size() == 21);
//     for (int i = 0; i < 21; ++i) {
//         TEST(vec[i].key == i);
//     }

//     vec = bpt.find_range(20, 100);
//     TEST(vec.size() == 20);
//     for (int i = 0; i < 20; ++i) {
//         TEST(vec[i].key == i + 20);
//     }

//     // case 2. in range
//     vec = bpt.find_range(10, 23);
//     TEST(vec.size() == 14);
//     for (int i = 0; i < 14; ++i) {
//         TEST(vec[i].key == i + 10);
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::print_leaves, {
//     // print method
// })

// TEST_SUITE(BPTreeTest::print_tree, {
//     // print method
// })

// TEST_SUITE(BPTreeTest::write_record, {
//     Record rec;

//     int value = 100;
//     TEST_SUCCESS(BPTree::write_record(
//         rec, 10, reinterpret_cast<uint8_t*>(&value), sizeof(int)));
//     TEST(rec.key == 10);
//     TEST(*reinterpret_cast<int*>(rec.value) == value);

//     int array[200];
//     for (int i = 0; i < 200; ++i) {
//         array[i] = i;
//     }

//     TEST_SUCCESS(BPTree::write_record(
//         rec,
//         20,
//         reinterpret_cast<const uint8_t*>(array),
//         sizeof(int) * 200));
//     TEST(rec.key == 20);

//     int* ptr = reinterpret_cast<int*>(rec.value);
//     for (int i = 0; i < 30; ++i) {
//         TEST(ptr[i] == array[i]);
//     }
// })

// TEST_SUITE(BPTreeTest::insert_to_leaf, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     bpt.test_config(5, 5, true);
//     Ubuffer buf = bpt.create_page(true);
//     pagenum_t bufnum = buf.safe_pagenum();

//     // case 0. ordered input
//     Record rec;
//     for (int i = 0; i < 5; ++i) {
//         rec.key = i;
//         TEST_SUCCESS(
//             bpt.insert_to_leaf(bpt.buffering(bufnum), rec));
//     }

//     for (int i = 0; i < 5; ++i) {
//         TEST(buf.page().records()[i].key == i);
//     }

//     // case 0.1. overflow
//     TEST(bpt.insert_to_leaf(bpt.buffering(bufnum), rec)
//         == Status::FAILURE);

//     // case 1. reversed ordered input
//     buf.page().page_header().number_of_keys = 0;
//     for (int i = 0; i < 5; ++i) {
//         rec.key = 5 - i;
//         TEST_SUCCESS(
//             bpt.insert_to_leaf(bpt.buffering(bufnum), rec));
//     }

//     for (int i = 0; i < 5; ++i) {
//         TEST(buf.page().records()[i].key == i + 1);
//     }

//     // case 2. random input
//     int arr[5];
//     arr[0] = 1;
//     arr[1] = 3;
//     arr[2] = 2;
//     arr[3] = 5;
//     arr[4] = 4;

//     buf.page().page_header().number_of_keys = 0;
//     for (int i = 0; i < 5; ++i) {
//         rec.key = arr[i];
//         TEST_SUCCESS(
//             bpt.insert_to_leaf(bpt.buffering(bufnum), rec));
//     }

//     for (int i = 0; i < 5; ++i) {
//         TEST(buf.page().records()[i].key == i + 1);
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::insert_and_split_leaf, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);
//     bpt.test_config(5, 4, true);

//     Ubuffer leaf = bpt.create_page(true);
//     pagenum_t leafnum = leaf.safe_pagenum();

//     Record record;

//     // case 0. last one
//     constexpr int size = 4;
//     TEST_SUCCESS(leaf.check());
//     for (int i = 0; i < size; ++i) {
//         record.key = i;
//         TEST_SUCCESS(
//             bpt.insert_to_leaf(bpt.buffering(leafnum), record));
//     }

//     record.key = size;
//     TEST_SUCCESS(
//         bpt.insert_and_split_leaf(bpt.buffering(leafnum), record));

//     Ubuffer filehdr = bpt.buffering(FILE_HEADER_PAGENUM);
//     pagenum_t root = filehdr.page().file_header().root_page_number;

//     Ubuffer rootpage = bpt.buffering(root);
//     Page& page = rootpage.page();

//     TEST(page.page_header().special_page_number == leafnum);
//     TEST(page.page_header().number_of_keys == 1);

//     constexpr int expected = BPTree::cut(size);
//     TEST(page.entries()[0].key == expected);
//     pagenum_t right = page.entries()[0].pagenum;

//     TEST_SUCCESS(leaf.check());
//     Page& page2 = leaf.page();
//     TEST(page2.page_header().number_of_keys == expected);
//     TEST(page2.page_header().parent_page_number == root);
//     TEST(page2.page_header().special_page_number == right);
//     for (int i = 0; i < expected; ++i) {
//         TEST(page2.records()[i].key == i);
//     }

//     Ubuffer rightbuf = bpt.buffering(right);
//     Page& page3 = rightbuf.page();
//     constexpr int right_expected = size + 1 - expected;
//     TEST(page3.page_header().number_of_keys == right_expected);
//     TEST(page3.page_header().parent_page_number == root);
//     TEST(page3.page_header().special_page_number == INVALID_PAGENUM);
//     for (int i = 0; i < right_expected; ++i) {
//         TEST(page3.records()[i].key == i + BPTree::cut(size));
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::insert_to_node, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     bpt.test_config(5, 5, true);
//     Ubuffer buf = bpt.create_page(false);
//     pagenum_t bufnum = buf.safe_pagenum();

//     // case 0. ordered input
//     Internal ent;
//     for (int i = 0; i < 5; ++i) {
//         ent.key = i;
//         TEST_SUCCESS(
//             bpt.insert_to_node(bpt.buffering(bufnum), i, ent));
//     }

//     for (int i = 0; i < 5; ++i) {
//         TEST(buf.page().entries()[i].key == i);
//     }

//     // case 0.1. overflow
//     TEST(bpt.insert_to_node(bpt.buffering(bufnum), 5, ent)
//         == Status::FAILURE);

//     // // case 1. reversed ordered input
//     buf.page().page_header().number_of_keys = 0;
//     for (int i = 0; i < 5; ++i) {
//         ent.key = 5 - i;
//         TEST_SUCCESS(
//             bpt.insert_to_node(bpt.buffering(bufnum), 0, ent));
//     }

//     for (int i = 0; i < 5; ++i) {
//         TEST(buf.page().entries()[i].key == i + 1);
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::insert_and_split_node, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;
//     bpt.test_config(leaf_order, internal_order, true);

//     Ubuffer node = bpt.create_page(false);
//     pagenum_t nodenum = node.safe_pagenum();
//     node.page().page_header().number_of_keys = internal_order - 1;

//     Ubuffer leaf(nullptr);
//     pagenum_t pages[5 + 1];
//     for (int i = -1; i < internal_order - 1; ++i) {
//         Ubuffer tmp = bpt.create_page(true);
//         pages[i + 1] = tmp.safe_pagenum();
//         TEST_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& page) {
//             page.page_header().parent_page_number = nodenum;
//             page.page_header().number_of_keys = leaf_order - 1;
//             return Status::SUCCESS;
//         }))

//         if (i != -1) {
//             TEST_SUCCESS(leaf.use(RWFlag::WRITE, [&](Page& page) {
//                 page.page_header().special_page_number = tmp.safe_pagenum();
//                 return Status::SUCCESS;
//             }))
//         }
//         leaf = std::move(tmp);

//         TEST_SUCCESS(node.use(RWFlag::WRITE, [&](Page& page) {
//             if (i == -1) {
//                 page.page_header().special_page_number = leaf.safe_pagenum();
//             } else {
//                 Internal& ent = page.entries()[i];
//                 ent.key = i * leaf_order;
//                 ent.pagenum = leaf.safe_pagenum();
//             }
//             return Status::SUCCESS;
//         }))
//     }

//     TEST_SUCCESS(node.check());
//     for (int i = -1; i < internal_order - 1; ++i) {
//         if (i == -1) {
//             TEST(pages[i + 1] == node.page().page_header().special_page_number);
//         } else {
//             TEST(pages[i + 1] == node.page().entries()[i].pagenum);
//         }
//     }

//     leaf = bpt.create_page(true);
//     pages[internal_order] = leaf.safe_pagenum();
//     TEST_SUCCESS(leaf.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().parent_page_number = nodenum;
//         page.page_header().number_of_keys = leaf_order - 1;
//         return Status::SUCCESS;
//     }))

//     Internal val;
//     val.key = (internal_order - 1) * leaf_order;
//     val.pagenum = leaf.safe_pagenum();
//     TEST_SUCCESS(
//         bpt.insert_and_split_node(bpt.buffering(nodenum), internal_order - 1, val));

//     Ubuffer filehdr = bpt.buffering(FILE_HEADER_PAGENUM);
//     pagenum_t root = filehdr.page().file_header().root_page_number;

//     Ubuffer rootpage = bpt.buffering(root);
//     TEST(rootpage.page().page_header().is_leaf == false);
//     TEST(rootpage.page().page_header().number_of_keys == 1);
//     TEST(rootpage.page().page_header().parent_page_number == INVALID_PAGENUM);
//     TEST(rootpage.page().page_header().special_page_number == nodenum);

//     constexpr int split = BPTree::cut(internal_order);
//     TEST(rootpage.page().entries()[0].key == (split - 1) * leaf_order);
//     pagenum_t rightnum = rootpage.page().entries()[0].pagenum;

//     TEST_SUCCESS(node.check());
//     TEST(node.page().page_header().is_leaf == false);
//     TEST(node.page().page_header().number_of_keys == split - 1);
//     TEST(node.page().page_header().parent_page_number == root);

//     for (int i = -1; i < split - 1; ++i) {
//         TEST_SUCCESS(node.use(RWFlag::READ, [&](Page& page) {
//             if (i == -1) {
//                 TEST_STATUS(page.page_header().special_page_number == pages[0]);
//             } else {
//                 TEST_STATUS(page.entries()[i].key == i * leaf_order);
//                 TEST_STATUS(page.entries()[i].pagenum == pages[i + 1]);
//             }
//             return Status::SUCCESS;
//         }))

//         TEST_SUCCESS(bpt.buffering(pages[i + 1]).use(RWFlag::READ, [&](Page& page) {
//             TEST_STATUS(page.page_header().parent_page_number == nodenum);
//             return Status::SUCCESS;
//         }))
//     }

//     constexpr int expected = internal_order - split;
//     Ubuffer right = bpt.buffering(rightnum);
//     TEST(right.page().page_header().is_leaf == false);
//     TEST(right.page().page_header().number_of_keys == expected);
//     TEST(right.page().page_header().parent_page_number == root);

//     for (int i = -1; i < expected; ++i) {
//         TEST_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
//             if (i == -1) {
//                 TEST_STATUS(page.page_header().special_page_number == pages[split + i + 1]);
//             } else {
//                 TEST_STATUS(page.entries()[i].key == (split + i) * leaf_order);
//                 TEST_STATUS(page.entries()[i].pagenum == pages[split + 1 + i]);
//             }
//             return Status::SUCCESS;
//         }))

//         TEST_SUCCESS(
//             bpt.buffering(pages[split + 1 + i]).use(RWFlag::READ, [&](Page& page) {
//                 TEST_STATUS(page.page_header().parent_page_number == rightnum);
//                 return Status::SUCCESS;
//             })
//         )
//     }

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::insert_to_parent, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);
//     bpt.test_config(7, 5, true);

//     // case 0. new root
//     Ubuffer left = bpt.create_page(true);
//     Ubuffer right = bpt.create_page(true);
//     pagenum_t leftnum = left.safe_pagenum();
//     pagenum_t rightnum = right.safe_pagenum();

//     TEST_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().parent_page_number = INVALID_PAGENUM;
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(bpt.insert_to_parent(
//         bpt.buffering(leftnum), 10, bpt.buffering(rightnum)));

//     Ubuffer buf = bpt.buffering(FILE_HEADER_PAGENUM);
//     pagenum_t root = buf.page().file_header().root_page_number;

//     buf = bpt.buffering(root);
//     TEST(buf.page().page_header().is_leaf == false);
//     TEST(buf.page().page_header().number_of_keys == 1);
//     TEST(buf.page().page_header().parent_page_number == INVALID_PAGENUM);
//     TEST(buf.page().page_header().special_page_number == leftnum);
//     TEST(buf.page().entries()[0].pagenum == rightnum);
//     TEST(buf.page().entries()[0].key == 10);

//     // case 1. simple insert
//     Ubuffer parent = bpt.create_page(false);
//     Ubuffer temporal = bpt.create_page(true);
//     TEST_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 1;
//         page.page_header().special_page_number = temporal.safe_pagenum();
//         page.entries()[0].key = 10;
//         page.entries()[0].pagenum = leftnum;
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(temporal.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().special_page_number = leftnum;
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().special_page_number = rightnum;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = 0; i < 3; ++i) {
//             page.records()[i].key = 10 + i;
//             page.page_header().number_of_keys++;
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = 0; i < 3; ++i) {
//             page.records()[i].key = 13 + i;
//             page.page_header().number_of_keys++;
//         }
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.insert_to_parent(
//         bpt.buffering(leftnum), 13, bpt.buffering(rightnum)));

//     TEST(parent.page().page_header().number_of_keys == 2);
//     TEST(parent.page().page_header().special_page_number == temporal.safe_pagenum());
//     TEST(parent.page().entries()[0].key == 10);
//     TEST(parent.page().entries()[0].pagenum == leftnum);
//     TEST(parent.page().entries()[1].key == 13);
//     TEST(parent.page().entries()[1].pagenum == rightnum);
//     TEST(temporal.page().page_header().special_page_number == leftnum);
//     TEST(left.page().page_header().special_page_number == rightnum);
//     TEST(right.page().page_header().special_page_number == INVALID_PAGENUM);

//     // case 2. node split
//     // TODO: impl test

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::insert_new_root, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer left = bpt.create_page(true);
//     Ubuffer right = bpt.create_page(true);
//     pagenum_t leftnum = left.safe_pagenum();
//     pagenum_t rightnum = right.safe_pagenum();

//     TEST_SUCCESS(bpt.insert_new_root(
//         bpt.buffering(leftnum), 10, bpt.buffering(rightnum)));
    
//     Ubuffer buf = bpt.buffering(FILE_HEADER_PAGENUM);
//     pagenum_t root = buf.page().file_header().root_page_number;

//     buf = bpt.buffering(root);
//     TEST(buf.page().page_header().is_leaf == false);
//     TEST(buf.page().page_header().number_of_keys == 1);
//     TEST(buf.page().page_header().parent_page_number == INVALID_PAGENUM);

//     TEST(buf.page().page_header().special_page_number == left.safe_pagenum());
//     TEST(buf.page().entries()[0].pagenum == right.safe_pagenum());

//     TEST(left.page().page_header().parent_page_number == buf.safe_pagenum());
//     TEST(right.page().page_header().parent_page_number == buf.safe_pagenum());

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::new_tree, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Record rec;
//     rec.key = 10;
//     *reinterpret_cast<int*>(rec.value) = 20;

//     TEST_SUCCESS(bpt.new_tree(rec));

//     Ubuffer ubuf = bpt.buffering(FILE_HEADER_PAGENUM);
//     pagenum_t root = ubuf.page().file_header().root_page_number;
//     TEST(root != INVALID_PAGENUM);

//     ubuf = bpt.buffering(root);
//     Page& page = ubuf.page();
//     TEST(page.page_header().is_leaf == true);
//     TEST(page.page_header().number_of_keys == 1);
//     TEST(page.page_header().parent_page_number == INVALID_PAGENUM);
//     TEST(page.records()[0].key == 10);
//     TEST(*reinterpret_cast<int*>(page.records()[0].value) == 20);

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::insert, {
//     int arr[40];
//     char str[] = "00";
//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;

//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);
//     bpt.test_config(leaf_order, internal_order, true);
//     bpt.verbose_output = false;
//     for (int i = 0; i < 40; ++i) {
//         str[0] = '0' + i / 10;
//         str[1] = '0' + i % 10;
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), 3));
//     }
//     bpt.print_tree();
//     bpt_test_postprocess(file, buffers);

//     FileManager file2("testfile");
//     BufferManager buffers2(4);
//     BPTree bpt2(&file2, &buffers2);
//     bpt2.test_config(leaf_order, internal_order, true);
//     bpt2.verbose_output = false;
//     for (int i = 40; i > 0; --i) {
//         str[0] = '0' + i / 10;
//         str[1] = '0' + i % 10;
//         TEST_SUCCESS(bpt2.insert(i, reinterpret_cast<uint8_t*>(str), 3));
//     }
//     bpt2.print_tree();
//     bpt_test_postprocess(file2, buffers);

//     FileManager file3("testfile");
//     BufferManager buffers3(4);
//     BPTree bpt3(&file3, &buffers3);
//     bpt3.test_config(leaf_order, internal_order, true);
//     bpt3.verbose_output = false;
//     for (int i = 0; i < 40; ++i) {
//         arr[i] = i;
//     }

//     std::random_device rd;
//     std::default_random_engine gen(rd());
//     for (int i = 40; i > 0; --i) {
//         int idx = gen() % i;
//         int j = arr[idx];
//         str[0] = '0' + j / 10;
//         str[1] = '0' + j % 10;
//         TEST_SUCCESS(bpt3.insert(j, reinterpret_cast<uint8_t*>(str), 3));

//         for (j = idx; j < i - 1; ++j) {
//             arr[j] = arr[j + 1];
//         }
//     }
//     bpt3.print_tree();
//     bpt_test_postprocess(file3, buffers3);
// })

// TEST_SUITE(BPTreeTest::remove_record_from_leaf, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer buf = bpt.create_page(true);
//     buf.page().page_header().number_of_keys = 5;
//     for (int i = 0; i < 5; ++i) {
//         buf.page().records()[i].key = i;
//     }

//     TEST_SUCCESS(bpt.remove_record_from_leaf(0, buf));
//     TEST(buf.page().page_header().number_of_keys == 4);
//     for (int i = 0; i < 4; ++i) {
//         TEST(buf.page().records()[i].key == i + 1);
//     }

//     TEST_SUCCESS(bpt.remove_record_from_leaf(4, buf));
//     TEST(buf.page().page_header().number_of_keys == 3);
//     for (int i = 0; i < 3; ++i) {
//         TEST(buf.page().records()[i].key == i + 1);
//     }

//     TEST_SUCCESS(bpt.remove_record_from_leaf(2, buf));
//     TEST(buf.page().page_header().number_of_keys == 2);
//     TEST(buf.page().records()[0].key == 1);
//     TEST(buf.page().records()[1].key == 3);

//     TEST_SUCCESS(bpt.remove_record_from_leaf(1, buf));
//     TEST(buf.page().page_header().number_of_keys == 1);
//     TEST(buf.page().records()[0].key == 3);

//     TEST_SUCCESS(bpt.remove_record_from_leaf(3, buf));
//     TEST(buf.page().page_header().number_of_keys == 0);

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::remove_entry_from_internal, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer buf = bpt.create_page(false);
//     Page& page = buf.page();

//     page.page_header().number_of_keys = 5;
//     for (int i = 0; i < 5; ++i) {
//         page.entries()[i].key = i;
//     }

//     TEST_SUCCESS(bpt.remove_entry_from_internal(0, buf));
//     TEST(page.page_header().number_of_keys == 4);
//     for (int i = 0; i < 4; ++i) {
//         TEST(page.entries()[i].key == i + 1);
//     }

//     TEST_SUCCESS(bpt.remove_entry_from_internal(4, buf));
//     TEST(page.page_header().number_of_keys == 3);
//     for (int i = 0; i < 3; ++i) {
//         TEST(page.entries()[i].key == i + 1);
//     }
    
//     TEST_SUCCESS(bpt.remove_entry_from_internal(2, buf));
//     TEST(page.page_header().number_of_keys == 2);
//     TEST(page.entries()[0].key == 1);
//     TEST(page.entries()[1].key == 3);

//     TEST_SUCCESS(bpt.remove_entry_from_internal(1, buf));
//     TEST(page.page_header().number_of_keys == 1);
//     TEST(page.entries()[0].key == 3);

//     TEST_SUCCESS(bpt.remove_entry_from_internal(3, buf));
//     TEST(page.page_header().number_of_keys == 0);

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::shrink_root, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     // case 0. no shrink
//     Ubuffer node = bpt.create_page(true);
//     Ubuffer filehdr = bpt.buffering(FILE_HEADER_PAGENUM);
//     TEST_SUCCESS(node.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 1;
//         page.records()[0].key = 10;
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(filehdr.use(RWFlag::WRITE, [&](Page& page) {
//         page.file_header().root_page_number = node.safe_pagenum();
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.shrink_root());
//     TEST_SUCCESS(filehdr.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.file_header().root_page_number == node.safe_pagenum());
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(node.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 1);
//         TEST_STATUS(page.records()[0].key == 10);
//         return Status::SUCCESS;
//     }))

//     // case 1. leaf root
//     TEST_SUCCESS(node.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 0;
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.shrink_root());
//     TEST_SUCCESS(filehdr.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.file_header().root_page_number == INVALID_PAGENUM);
//         return Status::SUCCESS;
//     }))

//     // case 2. internal root
//     node = bpt.create_page(true);
//     pagenum_t nodenum = node.safe_pagenum();

//     node = bpt.create_page(false);
//     TEST_SUCCESS(filehdr.use(RWFlag::WRITE, [&](Page& page) {
//         page.file_header().root_page_number = node.safe_pagenum();
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(node.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().special_page_number = nodenum;
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.shrink_root());
//     TEST_SUCCESS(filehdr.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.file_header().root_page_number == nodenum);
//         return Status::SUCCESS;
//     }))

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::merge_nodes, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     // case 0. leaf node
//     Ubuffer parent = bpt.create_page(false);
//     Ubuffer left = bpt.create_page(true);
//     Ubuffer right = bpt.create_page(true);
//     Ubuffer rightmost = bpt.create_page(true);

//     pagenum_t parentnum = parent.safe_pagenum();
//     TEST_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 2;
//         page.page_header().special_page_number = left.safe_pagenum();
//         page.entries()[0].key = 2;
//         page.entries()[0].pagenum = right.safe_pagenum();
//         page.entries()[1].key = 10;
//         page.entries()[1].pagenum = rightmost.safe_pagenum();
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 2;
//         page.page_header().parent_page_number = parentnum;
//         page.page_header().special_page_number = right.safe_pagenum();
//         for (int i = 0; i < 2; ++i) {
//             page.records()[i].key = i;
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 3;
//         page.page_header().parent_page_number = parentnum;
//         page.page_header().special_page_number = rightmost.safe_pagenum();
//         for (int i = 0; i < 3; ++i) {
//             page.records()[i].key = 2 + i;
//         }
//         return Status::SUCCESS;
//     }))
    
//     TEST_SUCCESS(bpt.merge_nodes(
//         bpt.buffering(left.safe_pagenum()),
//         2,
//         bpt.buffering(right.safe_pagenum()),
//         bpt.buffering(parentnum)));

//     TEST_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 1)
//         TEST_STATUS(page.page_header().special_page_number == left.safe_pagenum());
//         TEST_STATUS(page.entries()[0].key == 10);
//         TEST_STATUS(page.entries()[0].pagenum == rightmost.safe_pagenum());
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 5);
//         TEST_STATUS(page.page_header().parent_page_number == parentnum);
//         TEST_STATUS(page.page_header().special_page_number == rightmost.safe_pagenum());
//         for (int i = 0; i < 5; ++i) {
//             TEST_STATUS(page.records()[i].key == i);
//         }
//         return Status::SUCCESS;
//     }))

//     // case 1. internal node
//     parent = bpt.create_page(false);
//     left = bpt.create_page(false);
//     right = bpt.create_page(false);
//     rightmost = bpt.create_page(false);

//     parentnum = parent.safe_pagenum();
//     TEST_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 2;
//         page.page_header().parent_page_number = INVALID_PAGENUM;
//         page.page_header().special_page_number = left.safe_pagenum();
//         page.entries()[0].key = 3;
//         page.entries()[0].pagenum = right.safe_pagenum();
//         page.entries()[1].key = 10;
//         page.entries()[1].pagenum = rightmost.safe_pagenum();
//         return Status::SUCCESS;
//     }))

//     pagenum_t pagenums[7];
//     pagenum_t leftnum = left.safe_pagenum();
//     TEST_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 2;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = 0; i < 3; ++i) {
//             Ubuffer tmp = bpt.create_page(true);
//             pagenums[i] = tmp.safe_pagenum();
//             if (i == 0) {
//                 page.page_header().special_page_number = pagenums[i];
//             } else {
//                 page.entries()[i - 1].key = i;
//                 page.entries()[i - 1].pagenum = pagenums[i];
//             }

//             CHECK_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& tmppage) {
//                 tmppage.page_header().parent_page_number = leftnum;
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))

//     pagenum_t rightnum = right.safe_pagenum();
//     TEST_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 3;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = 0; i < 4; ++i) {
//             Ubuffer tmp = bpt.create_page(true);
//             pagenums[3 + i] = tmp.safe_pagenum();
//             if (i == 0) {
//                 page.page_header().special_page_number = pagenums[3 + i];
//             } else {
//                 page.entries()[i - 1].key = 3 + i;
//                 page.entries()[i - 1].pagenum = pagenums[3 + i];
//             }

//             CHECK_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& tmppage) {
//                 tmppage.page_header().parent_page_number = rightnum;
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(bpt.merge_nodes(
//         bpt.buffering(leftnum),
//         3,
//         bpt.buffering(rightnum),
//         bpt.buffering(parentnum)));

//     TEST_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().parent_page_number == INVALID_PAGENUM);
//         TEST_STATUS(page.page_header().number_of_keys == 1);
//         TEST_STATUS(page.page_header().special_page_number == left.safe_pagenum());
//         TEST_STATUS(page.entries()[0].key == 10);
//         TEST_STATUS(page.entries()[0].pagenum == rightmost.safe_pagenum());
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 6);
//         TEST_STATUS(page.page_header().parent_page_number == parentnum);
//         for (int i = 0; i < 7; ++i) {
//             if (i == 0) {
//                 TEST_STATUS(page.page_header().special_page_number == pagenums[i]);
//             } else {
//                 TEST_STATUS(page.entries()[i - 1].key == i);
//                 TEST_STATUS(page.entries()[i - 1].pagenum == pagenums[i]);
//             }

//             Ubuffer tmp = bpt.buffering(pagenums[i]);
//             CHECK_SUCCESS(tmp.use(RWFlag::READ, [&](Page& tmppage) {
//                 TEST_STATUS(tmppage.page_header().parent_page_number == leftnum);
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::rotate_to_right, {
//     // in redistribute_nodes test
// })

// TEST_SUITE(BPTreeTest::rotate_to_left, {
//     // in redistribute_nodes test
// })

// TEST_SUITE(BPTreeTest::redistribute_nodes_leaf, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer parent = bpt.create_page(false);
//     Ubuffer left = bpt.create_page(true);
//     Ubuffer right = bpt.create_page(true);
    
//     TEST_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 1;
//         page.page_header().parent_page_number = INVALID_PAGENUM;
//         page.page_header().special_page_number = left.safe_pagenum();
//         page.entries()[0].key = 3;
//         page.entries()[0].pagenum = right.safe_pagenum();
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {;
//         page.page_header().number_of_keys = 3;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = 0; i < 3; ++i) {
//             page.records()[i].key = i;
//             page.records()[i].value[0] = i;
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 2;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = 0; i < 2; ++i) {
//             page.records()[i].key = 3 + i;
//             page.records()[i].value[0] = 3 + i;
//         }
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.redistribute_nodes(
//         bpt.buffering(left.safe_pagenum()),
//         3,
//         0,
//         bpt.buffering(right.safe_pagenum()),
//         bpt.buffering(parent.safe_pagenum())));

//     TEST_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 1);
//         TEST_STATUS(page.page_header().parent_page_number == INVALID_PAGENUM);
//         TEST_STATUS(page.page_header().special_page_number == left.safe_pagenum());
//         TEST_STATUS(page.entries()[0].key == 2);
//         TEST_STATUS(page.entries()[0].pagenum == right.safe_pagenum());
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 2);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = 0; i < 2; ++i) {
//             TEST_STATUS(page.records()[i].key == i);
//             TEST_STATUS(page.records()[i].value[0] == i);
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 3);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = 0; i < 3; ++i) {
//             TEST_STATUS(page.records()[i].key == 2 + i);
//             TEST_STATUS(page.records()[i].value[0] == 2 + i);
//         }
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.redistribute_nodes(
//         bpt.buffering(left.safe_pagenum()),
//         3,
//         0,
//         bpt.buffering(right.safe_pagenum()),
//         bpt.buffering(parent.safe_pagenum())));

//     TEST_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 1);
//         TEST_STATUS(page.page_header().parent_page_number == INVALID_PAGENUM);
//         TEST_STATUS(page.page_header().special_page_number == left.safe_pagenum());
//         TEST_STATUS(page.entries()[0].key == 3);
//         TEST_STATUS(page.entries()[0].pagenum == right.safe_pagenum());
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 3);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = 0; i < 3; ++i) {
//             TEST_STATUS(page.records()[i].key == i);
//             TEST_STATUS(page.records()[i].value[0] == i);
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 2);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = 0; i < 2; ++i) {
//             TEST_STATUS(page.records()[i].key == 3 + i);
//             TEST_STATUS(page.records()[i].value[0] == 3 + i);
//         }
//         return Status::SUCCESS;
//     }))

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::redistribute_nodes_internal, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     Ubuffer parent = bpt.create_page(false);
//     Ubuffer left = bpt.create_page(false);
//     Ubuffer right = bpt.create_page(false);
    
//     TEST_SUCCESS(parent.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 1;
//         page.page_header().parent_page_number = INVALID_PAGENUM;
//         page.page_header().special_page_number = left.safe_pagenum();
//         page.entries()[0].key = 3;
//         page.entries()[0].pagenum = right.safe_pagenum();
//         return Status::SUCCESS;
//     }))

//     pagenum_t pagenums[7];
//     pagenum_t leftnum = left.safe_pagenum();
//     TEST_SUCCESS(left.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 3;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = -1; i < 3; ++i) {
//             Ubuffer tmp = bpt.create_page(true);
//             pagenums[i + 1] = tmp.safe_pagenum();
//             if (i == -1) {
//                 page.page_header().special_page_number = pagenums[i + 1];
//             } else {
//                 page.entries()[i].key = i;
//                 page.entries()[i].pagenum = pagenums[i + 1];
//             }

//             CHECK_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& tmppage) {
//                 tmppage.page_header().parent_page_number = leftnum;
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))
//     pagenum_t rightnum = right.safe_pagenum();
//     TEST_SUCCESS(right.use(RWFlag::WRITE, [&](Page& page) {
//         page.page_header().number_of_keys = 2;
//         page.page_header().parent_page_number = parent.safe_pagenum();
//         for (int i = -1; i < 2; ++i) {
//             Ubuffer tmp = bpt.create_page(true);
//             pagenums[i + 5] = tmp.safe_pagenum();
//             if (i == -1) {
//                 page.page_header().special_page_number = pagenums[i + 5];
//             } else {
//                 page.entries()[i].key = 4 + i;
//                 page.entries()[i].pagenum = pagenums[i + 5];
//             }

//             CHECK_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& tmppage) {
//                tmppage.page_header().parent_page_number = rightnum;
//                return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.redistribute_nodes(
//         bpt.buffering(leftnum),
//         3,
//         0,
//         bpt.buffering(rightnum),
//         bpt.buffering(parent.safe_pagenum())));

//     TEST_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 1);
//         TEST_STATUS(page.page_header().parent_page_number == INVALID_PAGENUM);
//         TEST_STATUS(page.page_header().special_page_number == leftnum);
//         TEST_STATUS(page.entries()[0].key == 2);
//         TEST_STATUS(page.entries()[0].pagenum == rightnum);
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 2);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = -1; i < 2; ++i) {
//             if (i == -1) {
//                 TEST_STATUS(page.page_header().special_page_number == pagenums[i + 1]);
//             } else {
//                 TEST_STATUS(page.entries()[i].key == i);
//                 TEST_STATUS(page.entries()[i].pagenum == pagenums[i + 1]);
//             }

//             Ubuffer tmp = bpt.buffering(pagenums[i + 1]);
//             CHECK_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& tmppage) {
//                 TEST_STATUS(tmppage.page_header().parent_page_number == leftnum);
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 3);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = -1; i < 3; ++i) {
//             if (i == -1) {
//                 TEST_STATUS(page.page_header().special_page_number == pagenums[i + 4]);
//             } else {
//                 TEST_STATUS(page.entries()[i].key == i + 3);
//                 TEST_STATUS(page.entries()[i].pagenum == pagenums[i + 4]);
//             }
//             Ubuffer tmp = bpt.buffering(pagenums[i + 4]);
//             CHECK_SUCCESS(tmp.use(RWFlag::WRITE, [&](Page& tmppage) {
//                 TEST_STATUS(tmppage.page_header().parent_page_number == rightnum);
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))

//     TEST_SUCCESS(bpt.redistribute_nodes(
//         bpt.buffering(leftnum),
//         2,
//         0,
//         bpt.buffering(rightnum),
//         bpt.buffering(parent.safe_pagenum())));


//     TEST_SUCCESS(parent.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 1);
//         TEST_STATUS(page.page_header().parent_page_number == INVALID_PAGENUM);
//         TEST_STATUS(page.page_header().special_page_number == leftnum);
//         TEST_STATUS(page.entries()[0].key == 3);
//         TEST_STATUS(page.entries()[0].pagenum == rightnum);
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(left.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 3);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = -1; i < 3; ++i) {
//             if (i == -1) {
//                 TEST_STATUS(page.page_header().special_page_number == pagenums[i + 1]);
//             } else {
//                 TEST_STATUS(page.entries()[i].key == i);
//                 TEST_STATUS(page.entries()[i].pagenum == pagenums[i + 1]);
//             }

//             Ubuffer tmp = bpt.buffering(pagenums[i + 1]);
//             CHECK_SUCCESS(tmp.use(RWFlag::READ, [&](Page& tmppage) {
//                 TEST_STATUS(tmppage.page_header().parent_page_number == leftnum);
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))
//     TEST_SUCCESS(right.use(RWFlag::READ, [&](Page& page) {
//         TEST_STATUS(page.page_header().number_of_keys == 2);
//         TEST_STATUS(page.page_header().parent_page_number == parent.safe_pagenum());
//         for (int i = -1; i < 2; ++i) {
//             if (i == -1) {
//                 TEST_STATUS(page.page_header().special_page_number == pagenums[i + 5]);
//             } else {
//                 TEST_STATUS(page.entries()[i].key == i + 4);
//                 TEST_STATUS(page.entries()[i].pagenum == pagenums[i + 5]);
//             }
//             Ubuffer tmp = bpt.buffering(pagenums[i + 5]);
//             CHECK_SUCCESS(tmp.use(RWFlag::READ, [&](Page& tmppage) {
//                 TEST_STATUS(tmppage.page_header().parent_page_number == rightnum);
//                 return Status::SUCCESS;
//             }))
//         }
//         return Status::SUCCESS;
//     }))

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::delete_entry, {
//     // TODO
// })

// TEST_SUITE(BPTreeTest::remove, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;
//     bpt.test_config(leaf_order, internal_order, true);
//     bpt.verbose_output = false;

//     // case root
//     char str[] = "00";
//     for (int i = 0; i < leaf_order - 1; ++i) {
//         TEST_SUCCESS(bpt.insert(i * 10, reinterpret_cast<uint8_t*>(str), sizeof(str)));
//     }
//     bpt.print_tree();
//     for (int i = 0; i < leaf_order - 1; ++i) {
//         TEST_SUCCESS(bpt.remove(i * 10));
//         bpt.print_tree();
//     }
//     Ubuffer buffer = bpt.buffering(FILE_HEADER_PAGENUM);
//     TEST(buffer.page().file_header().root_page_number == INVALID_PAGENUM);

//     // case simple delete
//     for (int i = 0; i < leaf_order * internal_order; ++i) {
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), sizeof(str)));
//     }
//     bpt.print_tree();

//     for (int i = 10; i <= 11; ++i) {
//         TEST_SUCCESS(bpt.remove(i));
//     }
//     bpt.print_tree();

//     // case merge
//     for (int i = 8; i <= 9; ++i) {
//         TEST_SUCCESS(bpt.remove(i));
//     }
//     bpt.print_tree();

//     // case rotate to right
//     for (int i = -1; i >= -2; --i) {
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), sizeof(str)));
//     }
//     bpt.print_tree();

//     for (int i = 14; i <= 19; ++i) {
//         TEST_SUCCESS(bpt.remove(i));
//     }
//     bpt.print_tree();

//     // case left distribute
//     for (int i = 14; i <= 19; ++i) {
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), sizeof(str)));
//     }
//     bpt.print_tree();

//     for (int i = -2; i <= 3; ++i) {
//         TEST_SUCCESS(bpt.remove(i));
//     }
//     bpt.print_tree();

//     bpt_test_postprocess(file, buffers);
// })

// TEST_SUITE(BPTreeTest::destroy_tree, {
//     FileManager file("testfile");
//     BufferManager buffers(4);
//     BPTree bpt(&file, &buffers);

//     constexpr int leaf_order = 4;
//     constexpr int internal_order = 5;
//     bpt.test_config(leaf_order, internal_order, true);
//     bpt.verbose_output = false;
    
//     char str[] = "00";
//     for (int i = 0; i < leaf_order * internal_order; ++i) {
//         TEST_SUCCESS(bpt.insert(i, reinterpret_cast<uint8_t*>(str), sizeof(str)));
//     }

//     TEST_SUCCESS(bpt.destroy_tree());
//     Ubuffer buffer = bpt.buffering(FILE_HEADER_PAGENUM);
//     TEST(buffer.page().file_header().root_page_number == INVALID_PAGENUM);

//     bpt_test_postprocess(file, buffers);
// })

// int bptree_test() {
//     srand(time(NULL));
//     return BPTreeTest::constructor_test()
//         && BPTreeTest::destructor_test()
//         && BPTreeTest::buffering_test()
//         && BPTreeTest::create_page_test()
//         && BPTreeTest::free_page_test()
//         && BPTreeTest::test_config_test()
//         && BPTreeTest::path_to_root_test()
//         && BPTreeTest::cut_test()
//         && BPTreeTest::find_leaf_test()
//         && BPTreeTest::find_key_from_leaf_test()
//         && BPTreeTest::find_pagenum_from_internal_test()
//         && BPTreeTest::find_test()
//         && BPTreeTest::find_range_test()
//         && BPTreeTest::print_leaves_test()
//         && BPTreeTest::print_tree_test()
//         && BPTreeTest::write_record_test()
//         && BPTreeTest::insert_to_leaf_test()
//         && BPTreeTest::insert_and_split_leaf_test()
//         && BPTreeTest::insert_to_node_test()
//         && BPTreeTest::insert_and_split_node_test()
//         && BPTreeTest::insert_to_parent_test()
//         && BPTreeTest::insert_new_root_test()
//         && BPTreeTest::new_tree_test()
//         && BPTreeTest::insert_test()
//         && BPTreeTest::remove_record_from_leaf_test()
//         && BPTreeTest::remove_entry_from_internal_test()
//         && BPTreeTest::shrink_root_test()
//         && BPTreeTest::merge_nodes_test()
//         && BPTreeTest::rotate_to_left_test()
//         && BPTreeTest::rotate_to_right_test()
//         && BPTreeTest::redistribute_nodes_leaf_test()
//         && BPTreeTest::redistribute_nodes_internal_test()
//         && BPTreeTest::delete_entry_test()
//         && BPTreeTest::remove_test()
//         && BPTreeTest::destroy_tree_test();
// }
