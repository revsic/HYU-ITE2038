#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>

#include "buffer_manager.h"
#include "disk_manager.h"
#include "headers.h"

// TYPE DEFINITION

/// Queue with page ID.
struct queue_t {
    pagenum_t pagenum;      /// page ID.
    struct queue_t* next;   /// next queue node.
};

/// Record vector for range search.
struct record_vec_t {
    int size;               /// size of the vector.
    int capacity;           /// capacity of the vector.
    struct record_t* rec;   /// record array.
};

/// B+Tree configuration.
struct bpt_t {
    int leaf_order;
    int internal_order;
    int verbose_output;
    int delayed_merge;
    struct file_manager_t* file;
    struct buffer_manager_t* buffers;
};

// PROCEDURE DEFINTION

// PAGE RELATIVE

/// Swap page pair.
/// \param left struct ubuffer_t*, left page.
/// \param right struct ubuffer_t*, right page.
void swap_ubuffer(struct ubuffer_t* left, struct ubuffer_t* right);

/// Buffering.
/// \param bpt struct bpt_t*, B+Tree configuartion.
/// \param pagenum pagenum_t, target page ID.
/// \return struct ubuffer_t, buffer.
struct ubuffer_t bpt_buffering(struct bpt_t* bpt, pagenum_t pagenum);

/// Create new page from buffer manager.
/// \param bpt struct bpt_t*, B+Tree configuartion.
/// \return struct ubuffer_t, buffer.
struct ubuffer_t bpt_create_page(struct bpt_t* bpt);

/// Release page to free page list.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param pagenum pagenum_t, target page ID.
/// \return int, whether success or not.
int bpt_free_page(struct bpt_t* bpt, pagenum_t pagenum);


// UTILITY

/// Print usage1.
/// \param bpt struct bpt_t*, B+Tree configuration.
void usage_1(struct bpt_t* bpt);
/// Print usage2.
/// \param bpt struct bpt_t*, B+Tree configuration.
void usage_2(struct bpt_t* bpt);

/// Initialize bpt configuration.
/// \param config struct bpt_t*, B+Tree configuration.
/// \param file struct file_manager_t*, disk structure.
/// \param buffers struct buffer_manager_t*, buffer manager.
/// \return whether success to write configuration or not.
int bpt_init(struct bpt_t* config,
             struct file_manager_t* file,
             struct buffer_manager_t* buffers);
/// Release bpt configuration.
/// \param config struct bpt_t*, B+Tree configuration.
/// \return whether success release configuration or not.
int bpt_release(struct bpt_t* config);
/// Set default bpt configuartion.
/// \param config struct bpt_t*, B+Tree configuration.
/// \return whether success to write configuration or not.
int bpt_default_config(struct bpt_t* config);
/// Set test mode bpt configuartion.
/// \param config struct bpt_t*, B+Tree configuration.
/// \param leaf_order int, leaf order.
/// \param internal_order int, internal order.
/// \return whether success to write configuration or not.
int bpt_test_config(struct bpt_t* config,
                    int leaf_order,
                    int internal_order);

/// Enqueue page ID.
/// \param queue struct queue_t*, target queue.
/// \param pagenum pagenum_t, target page ID.
/// \return updated queue.
struct queue_t* enqueue(struct queue_t* queue, pagenum_t pagenum);
/// Dequeue page ID.
/// \param queue struct queue_t*, target queue.
/// \param retval pagenum_t*, returned page ID.
/// \return updated queue.
struct queue_t* dequeue(struct queue_t* queue, pagenum_t* retval);

/// Initialize record vector.
/// \param vec struct record_vec_t*, record vector.
/// \param cap int, initial capacity.
/// \return int, whether initialization success or not.
int record_vec_init(struct record_vec_t* vec, int cap);
/// Deallocate record vector.
/// \param vec struct record_vec_t*, record vector.
/// \return int, whether deallocation success or not.
int record_vec_free(struct record_vec_t* vec);
/// Expand capacity of vector.
/// \param vec struct record_vec_t*, record vector.
/// \return int, whether expansion success or not.
int record_vec_expand(struct record_vec_t* vec);
/// Append record to the vector.
/// \param vec struct record_vec_t*, record vector.
/// \return int, whether appension success or not.
int record_vec_append(struct record_vec_t* vec, struct record_t* rec);

/// Length of path to root.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param pagenum pagenum_t, page ID.
/// \return int, length of path to root.
int path_to_root(struct bpt_t* bpt, pagenum_t pagenum);

/// Return split position with given length, ceil(length / 2).
/// \param length length of the key array.
/// \return int, split position.
int cut(int length);


// SEARCH

/// Find leaf page where given key can exist.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param key prikey_t, searching key.
/// \param buffer struct ubuffer_t*, returned buffer, nullable.
/// \return pagenum_t, page ID of leaf page.
pagenum_t find_leaf(struct bpt_t* bpt,
                    prikey_t key,
                    struct ubuffer_t* buffer);
/// Find key from leaf page.
/// \param key prikey_t, searching key.
/// \param buffer struct ubuffer_t, target leaf page.
/// \param record struct record_t*, returned record.
/// \return int, whether key exists or not.
int find_key_from_leaf(prikey_t key,
                       struct ubuffer_t buffer,
                       struct record_t* record);
/// Find key from tree.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param key prikey_t, searching key.
/// \param record struct record_t*, returned record.
/// \return int, whether key exists or not.
int bpt_find(struct bpt_t* bpt,
             prikey_t key,
             struct record_t* record);
/// Range based searching from tree.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param start prikey_t, start point.
/// \param end prikey_t, end point.
/// \param retval struct record_t*, record vector for returning sequence.
/// \return int, size of the return sequence.
int bpt_find_range(struct bpt_t* bpt,
                   prikey_t start,
                   prikey_t end,
                   struct record_vec_t* retval);


// OUTPUT

/// Print leaves.
/// \param bpt struct bpt_t*, B+Tree configuration.
void print_leaves(struct bpt_t* bpt);
/// Print tree.
/// \param bpt struct bpt_t*, B+Tree configuration.
void print_tree(struct bpt_t* bpt);

/// Find key and print result.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param key prikey_t, searching key.
void find_and_print(struct bpt_t* bpt, prikey_t key); 
/// Find key range and print result sequence.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param range1 prikey_t, start point.
/// \param range2 prikey_t, end point.
void find_and_print_range(struct bpt_t* bpt, prikey_t range1, prikey_t range2);


// INSERTION

/// Write record with given key and value.
/// \param record struct record_t*, target record.
/// \param key prikey_t, key.
/// \param value const uint8_t*, value data.
/// \param value_size int, size of the value data.
/// \return int, whether success to write record or not.
int make_record(struct record_t* record, prikey_t key, const uint8_t* value, int value_size);
/// Create new page.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param leaf int, whether leaf page or not.
/// \return struct ubuffer_t, created page buffer.
struct ubuffer_t make_node(struct bpt_t* bpt, uint32_t leaf);

/// Get index of given page ID from parent key array.
/// \param parent struct ubuffer_t*, parent page.
/// \param pagenum pagenum_t, target page ID.
/// \return int, index of the page ID, if failed, return -1.
int get_index(struct ubuffer_t* parent, pagenum_t pagenum);

/// Insert record into the leaf page without any balancing policy.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param leaf struct ubuffer_t*, leaf page.
/// \param pointer struct record_t*, target record.
/// \return int, whether success to insert record or not.
int insert_into_leaf(struct bpt_t* bpt,
                     struct ubuffer_t* leaf,
                     struct record_t* pointer);
/// Insert record into the leaf page with splitting given leaf node.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param leaf struct ubuffer_t*, leaf page.
/// \param pointer struct record_t*, target record.
/// \return int, whether success to insert record or not.
int insert_into_leaf_after_splitting(struct bpt_t* bpt,
                                     struct ubuffer_t* leaf,
                                     struct record_t* record);

/// Insert entry into the internal page without any balancing policy.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param node struct ubuffer_t*, target internal node.
/// \param index int, insertion point.
/// \param entry struct internal_t*, target entry.
/// \return int, whether success to insert entry or not.
int insert_into_node(struct bpt_t* bpt,
                     struct ubuffer_t* node,
                     int index,
                     struct internal_t* entry);

/// Insert entry into the internal page with splitting given internal node.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param old_node struct ubuffer_t*, target internal node.
/// \param index int, insertion point.
/// \param entry struct internal_t*, target entry.
/// \return int, whether success to insert entry or not.
int insert_into_node_after_splitting(struct bpt_t* bpt,
                                     struct ubuffer_t* old_node,
                                     int index,
                                     struct internal_t* entry);

/// Insert key into the parent page with given left, right child.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param left struct ubuffer_t*, lett child.
/// \param key prikey_t, target key.
/// \param right struct ubuffer_t*, right child.
/// \return int, whether success to insert key or not.
int insert_into_parent(struct bpt_t* bpt,
                       struct ubuffer_t* left,
                       prikey_t key,
                       struct ubuffer_t* right);
/// Create new root to insert key and left, right child pages.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param left struct ubuffer_t*, lett child.
/// \param key prikey_t, target key.
/// \param right struct ubuffer_t*, right child.
/// \return int, whether success to insert key or not.
int insert_into_new_root(struct bpt_t* bpt,
                         struct ubuffer_t* left,
                         prikey_t key,
                         struct ubuffer_t* right);

/// Create new tree.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param pointer struct record_t*, first record of the new root.
/// \return int, whether success to create new tree or not.
int start_new_tree(struct bpt_t* bpt, struct record_t* pointer);
/// Mother method, insert key and value to the tree.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param key prikey_t, target key.
/// \param value uint8_t*, target value.
/// \param value_size int, size of the value data.
/// \return int, whether success to insert key or not.
int bpt_insert(struct bpt_t* bpt,
               prikey_t key,
               uint8_t* value,
               int value_size);


// DELETION

/// Simply remove record from leaf without any other manipulation policy.
/// \param key prikey_t, the key to remove.
/// \param node struct ubuffer_t*, buffered node.
/// \return int, whether success to remove or not.
int remove_record_from_leaf(prikey_t key, struct ubuffer_t* node);

/// Simply remove entry from internal node without any other manipulation policy.
/// \param key prikey_t, the key to remove.
/// \param node struct ubuffer_t*, buffered node.
/// \return int, whether success to remove or not.
int remove_entry_from_internal(prikey_t key, struct ubuffer_t* node);

/// Move key from right node to left node (right rotation).
/// \param bpt struct bpt_t*, B+ Tree structure.
/// \param left struct ubuffer_t*, left node.
/// \param k_prime prikey_t, median value from parent node.
/// \param k_prime_index int, index of the k_prime on parent node.
/// \param right struct ubuffer_t*, right node.
/// \param parent struct ubuffer_t*, parent node.
int rotate_to_right(struct bpt_t* bpt,
                    struct ubuffer_t* left,
                    prikey_t k_prime,
                    int k_prime_index,
                    struct ubuffer_t* right,
                    struct ubuffer_t* parent);

/// Move key from left node to right node (left rotation).
/// \param bpt struct bpt_t*, B+ Tree structure.
/// \param left struct ubuffer_t*, left node.
/// \param k_prime prikey_t, median value from parent node.
/// \param k_prime_index int, index of the k_prime on parent node.
/// \param right struct ubuffer_t*, right node.
/// \param parent struct ubuffer_t*, parent node.
int rotate_to_left(struct bpt_t* bpt,
                   struct ubuffer_t* left,
                   prikey_t k_prime,
                   int k_prime_index,
                   struct ubuffer_t* right,
                   struct ubuffer_t* parent);

/// Shrink root page if it is blank.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \return int, whether success to shrink root or not.
int shrink_root(struct bpt_t* bpt);
/// Merge two nodes into one node.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param left struct ubuffer_t*, lett child.
/// \param key_prime prikey_t, parent key for merging two nodes.
/// \param right struct ubuffer_t*, right child.
/// \param parent struct ubuffer_t*, parent page.
/// \return int, whether success to merge nodes or not.
int merge_nodes(struct bpt_t* bpt,
                struct ubuffer_t* left,
                prikey_t k_prime,
                struct ubuffer_t* right,
                struct ubuffer_t* parent);
/// Redistribute records or entries from one node to other node.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param left struct ubuffer_t*, lett child.
/// \param key_prime prikey_t, parent key for merging two nodes.
/// \param key_prim_index int, index of the `key_prime` from parent node.
/// \param right struct ubuffer_t*, right child.
/// \param parent struct ubuffer_t*, parent page.
/// \return int, whether success to redistribute key or not.
int redistribute_nodes(struct bpt_t* bpt,
                       struct ubuffer_t* left,
                       prikey_t k_prime,
                       int k_prime_index,
                       struct ubuffer_t* right,
                       struct ubuffer_t* parent);
/// Delete key from given page with several rebalancing policy.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param key prikey_t, target key.
/// \param page struct ubuffer_t*, target page.
/// \return int, whether success to delete key or not.
int delete_entry(struct bpt_t* bpt,
                 prikey_t key,
                 struct ubuffer_t* page);
/// Mother function, delete key from tree.
/// \param bpt struct bpt_t*, B+Tree configuration.
/// \param key prikey_t, target key.
/// \return int, whether success to delete key or not.
int bpt_delete(struct bpt_t* bpt, prikey_t key);

/// Free all tree pages to make empty tree.
/// \param table struct dbms_table_t*, table accessor.
/// \return int, whether success to destroy tree or not.
int destroy_tree(struct bpt_t* bpt);

#endif /* __BPT_H__*/
