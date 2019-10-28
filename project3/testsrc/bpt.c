// #include "bpt.h"
// #include "test.h"

// TEST_SUITE(swap_ubuffer, {
//     struct buffer_t buf1;
//     struct buffer_t buf2;
//     buf1.table_id = 10;
//     buf2.table_id = 20;

//     struct page_uri_t uri1;
//     struct page_uri_t uri2;
//     uri1.table_id = 10;
//     uri2.table_id = 20;

//     struct ubuffer_t ubuf1;
//     struct ubuffer_t ubuf2;
//     ubuf1.buf = &buf1;
//     ubuf1.uri = uri1;
//     ubuf2.buf = &buf2;
//     ubuf2.uri = uri2;

//     swap_ubuffer(&ubuf1, &ubuf2);
//     TEST(ubuf1.uri.table_id == 20);
//     TEST(ubuf1.buf->table_id == 20);
//     TEST(ubuf2.uri.table_id == 10);
//     TEST(ubuf2.buf->table_id == 10);
// })

// TEST_SUITE(queue, {
//     int i;
//     struct queue_t* queue = NULL;
//     for (i = 0; i < 10; ++i) {
//         queue = enqueue(queue, i);
//     }

//     struct queue_t* tmp = queue;
//     for (i = 0; i < 10; ++i) {
//         TEST(tmp->pagenum == i);
//         tmp = tmp->next;
//     }

//     pagenum_t n;
//     for (i = 0; i < 5; ++i) {
//         queue = dequeue(queue, &n);
//         TEST(n == i);
//     }

//     for (i = 10; i < 15; ++i) {
//         queue = enqueue(queue, i);
//     }

//     for (i = 5; i < 15; ++i) {
//         queue = dequeue(queue, &n);
//         TEST(n == i);
//     }
// })

// TEST_SUITE(record_vec_init, {

// })

// TEST_SUITE(record_vec_free, {

// })

// TEST_SUITE(record_vec_expand, {

// })

// TEST_SUITE(record_vec_append, {

// })

// TEST_SUITE(height, {

// })

// TEST_SUITE(path_to_root, {

// })

// TEST_SUITE(cut, {

// })

// TEST_SUITE(find_leaf, {

// })

// TEST_SUITE(find_key_from_leaf, {

// })

// TEST_SUITE(bpt_find, {

// })

// TEST_SUITE(bpt_find_range, {

// })

// TEST_SUITE(print_leaves, {

// })

// TEST_SUITE(print_tree, {

// })

// TEST_SUITE(find_and_print, {

// })

// TEST_SUITE(find_and_print_range, {

// })

// TEST_SUITE(make_record, {

// })

// TEST_SUITE(make_node, {

// })

// TEST_SUITE(get_index, {

// })

// TEST_SUITE(insert_into_leaf, {

// })

// TEST_SUITE(insert_into_leaf_after_splitting, {

// })

// TEST_SUITE(insert_into_node, {

// })

// TEST_SUITE(insert_into_node_after_splitting, {

// })

// TEST_SUITE(insert_into_parent, {

// })

// TEST_SUITE(insert_into_new_root, {

// })

// TEST_SUITE(start_new_tree, {

// })

// TEST_SUITE(bpt_insert, {

// })

// TEST_SUITE(shrink_root, {

// })

// TEST_SUITE(merge_nodes, {

// })

// TEST_SUITE(redistribute_nodes, {

// })

// TEST_SUITE(delete_entry, {

// })

// TEST_SUITE(bpt_delete, {

// })

// TEST_SUITE(destroy_tree, {
    
// })

// int bpt_test() {
//     return swap_ubuffer_test()
//         && queue_test()
//         && record_vec_init_test()
//         && record_vec_free_test()
//         && record_vec_expand_test()
//         && record_vec_append_test()
//         && height_test()
//         && path_to_root_test()
//         && cut_test()
//         && find_leaf_test()
//         && find_key_from_leaf_test()
//         && bpt_find_test()
//         && bpt_find_range_test()
//         && print_leaves_test()
//         && print_tree_test()
//         && find_and_print_test()
//         && find_and_print_range_test()
//         && make_record_test()
//         && make_node_test()
//         && get_index_test()
//         && insert_into_leaf_test()
//         && insert_into_leaf_after_splitting_test()
//         && insert_into_node_test()
//         && insert_into_node_after_splitting_test()
//         && insert_into_parent_test()
//         && insert_into_new_root_test()
//         && start_new_tree_test()
//         && bpt_insert_test()
//         && shrink_root_test()
//         && merge_nodes_test()
//         && redistribute_nodes_test()
//         && delete_entry_test()
//         && bpt_delete_test()
//         && destroy_tree_test();
// }
