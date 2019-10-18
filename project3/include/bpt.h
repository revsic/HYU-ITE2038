#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>

#include "dbms.h"
#include "headers.h"

// GLOBAL CONSTANT

/// Order of leaf node.
#define LEAF_ORDER 32
/// Order of internal node.
#define INTERNAL_ORDER 249

/// Verbose output.
#define VERBOSE_OUTPUT 0
/// Enable delayed merge.
#define DELAYED_MERGE 1
/// Default capacity for record vector.
#define DEFAULT_RECORD_VEC_CAP 4


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

/// Page and page ID pair.
struct page_pair_t {
    pagenum_t pagenum;      /// page ID.
    struct page_t* page;    /// page.
};


// PROCEDURE DEFINTION

// PAGE RELATIVE

/// Load page with page ID.
/// \param pagenum pagenum_t, page ID.
/// \param page struct page_t*, memory for writing read page.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to load page or not.
int load_page(pagenum_t pagenum,
              struct page_t* page,
              struct file_manager_t* manager);
/// Commit page with page ID.
/// \param pagenum pagenum_t, page ID.
/// \param page struct page_t*, page structure.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to commit page or not.
int commit_page(pagenum_t pagenum,
                struct page_t* page,
                struct file_manager_t* manager);

/// Swap page pair.
/// \param left struct page_pair_t*, left page.
/// \param right struct page_pair_t*, right page.
void swap_page_pair(struct page_pair_t* left, struct page_pair_t* right);


// UTILITY

/// Print usage1.
void usage_1();
/// Print usage2.
void usage_2();

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
/// \return int, whether initialization success or not.
int record_vec_init(struct record_vec_t* vec);
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

/// Height of the node.
/// \param node struct page_uri_t, target node.
/// \param dbms struct dbms_t*, database manager.
/// \return int, height of the node.
int height(struct page_uri_t node, struct dbms_t* dbms);
/// Length of path to root.
/// \param node struct page_uri_t, target node.
/// \param dbms struct dbms_t*, database manager.
/// \return int, length of path to root.
int path_to_root(struct page_uri_t node, struct dbms_t* dbms);

/// Return split position with given length.
/// \param length length of the key array.
/// \return int, split position.
int cut(int length);


// SEARCH

/// Find leaf page where given key can exist.
/// \param key prikey_t, searching key.
/// \param page struct page_t*, returned page, nullable.
/// \param manager struct manager_t*, file manager.
/// \return pagenum_t, page ID of leaf page.
pagenum_t find_leaf(prikey_t key, struct page_t* page, struct file_manager_t* manager);
/// Find key from leaf page.
/// \param key prikey_t, searching key.
/// \param page struct page_t*, target leaf page.
/// \param record struct record_t*, returned record.
/// \return int, whether key exists or not.
int find_key_from_leaf(prikey_t key, struct page_t* page, struct record_t* record);
/// Find key from tree.
/// \param key prikey_t, searching key.
/// \param record struct record_t*, returned record.
/// \param manager struct manager_t*, file manager.
/// \return int, whether key exists or not.
int bpt_find(prikey_t key, struct record_t* record, struct file_manager_t* manager);
/// Range based searching from tree.
/// \param start prikey_t, start point.
/// \param end prikey_t, end point.
/// \param record struct record_t*, record vector for returning sequence.
/// \param manager struct manager_t*, file manager.
/// \return int, size of the return sequence.
int bpt_find_range(prikey_t start,
                   prikey_t end,
                   struct record_vec_t* retval,
                   struct file_manager_t* manager);


// OUTPUT

/// Print leaves.
/// \param manager struct manager_t*, file manager.
void print_leaves(struct file_manager_t* manager);
/// Print tree.
/// \param manager struct manager_t*, file manager.
void print_tree(struct file_manager_t* manager);

/// Find key and print result.
/// \param key prikey_t, searching key.
/// \param manager struct manager_t*, file manager.
void find_and_print(prikey_t key, struct file_manager_t* manager); 
/// Find key range and print result sequence.
/// \param range1 prikey_t, start point.
/// \param range2 prikey_t, end point.
/// \param manager struct manager_t*, file manager.
void find_and_print_range(prikey_t range1, prikey_t range2, struct file_manager_t* manager);


// INSERTION

/// Write record with given key and value.
/// \param record struct record_t*, target record.
/// \param key prikey_t, key.
/// \param value uint8_t*, value data.
/// \param value_size int, size of the value data.
/// \return int, whether success to write record or not.
int make_record(struct record_t* record, prikey_t key, uint8_t* value, int value_size);
/// Create new page.
/// \param manager struct manager_t*, file manager.
/// \param leaf int, whether leaf page or not.
/// \return pagenum_t, created page ID.
pagenum_t make_node(struct file_manager_t* manager, uint32_t leaf);

/// Get index of given page ID from parent key array.
/// \param parent struct page_t*, parent page.
/// \param pagenum pagenum_t, target page ID.
/// \return int, index of the page ID, if failed, return -1.
int get_index(struct page_t* parent, pagenum_t pagenum);

/// Insert record into the leaf page without any balancing policy.
/// \param leaf struct page_pair_t*, leaf page.
/// \param pointer struct record_t*, target record.
/// \param manager struct manager_t*, file manager.
/// \return int, whether success to insert record or not.
int insert_into_leaf(struct page_pair_t* leaf,
                     struct record_t* pointer,
                     struct file_manager_t* manager);
/// Insert record into the leaf page with splitting given leaf node.
/// \param leaf struct page_pair_t*, leaf page.
/// \param pointer struct record_t*, target record.
/// \param manager struct manager_t*, file manager.
/// \return int, whether success to insert record or not.
int insert_into_leaf_after_splitting(struct page_pair_t* leaf,
                                     struct record_t* record,
                                     struct file_manager_t* manager);

/// Insert entry into the internal page without any balancing policy.
/// \param node struct page_pair_t*, target internal node.
/// \param index int, insertion point.
/// \param entry struct internal_t*, target entry.
/// \param manager struct manager_t*, file manager.
/// \return int whether success to insert entry or not.
int insert_into_node(struct page_pair_t* node,
                     int index,
                     struct internal_t* entry,
                     struct file_manager_t* manager);

/// Insert entry into the internal page with splitting given internal node.
/// \param node struct page_pair_t*, target internal node.
/// \param index int, insertion point.
/// \param entry struct internal_t*, target entry.
/// \param manager struct manager_t*, file manager.
/// \return int whether success to insert entry or not.
int insert_into_node_after_splitting(struct page_pair_t* old_node,
                                     int index,
                                     struct internal_t* entry,
                                     struct file_manager_t* manager);

/// Insert key into the parent page with given left, right child.
/// \param left struct page_pair_t*, lett child.
/// \param key prikey_t, target key.
/// \param right struct page_pair_t*, right child.
/// \param manager struct manager_t*, file manager.
/// \return int whether success to insert key or not.
int insert_into_parent(struct page_pair_t* left,
                       prikey_t key,
                       struct page_pair_t* right,
                       struct file_manager_t* manager);
/// Create new root to insert key and left, right child pages.
/// \param left struct page_pair_t*, lett child.
/// \param key prikey_t, target key.
/// \param right struct page_pair_t*, right child.
/// \param manager struct manager_t*, file manager.
/// \return int whether success to insert key or not.
int insert_into_new_root(struct page_pair_t* left,
                         prikey_t key,
                         struct page_pair_t* right,
                         struct file_manager_t* manager);

/// Create new tree.
/// \param pointer struct record_t*, first record of the new root.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to create new tree or not.
int start_new_tree(struct record_t* pointer,
                   struct file_manager_t* manager);
/// Mother method, insert key and value to the tree.
/// \param key prikey_t, target key.
/// \param value uint8_t*, target value.
/// \param value_size int, size of the value data.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to insert key or not.
int bpt_insert(prikey_t key,
               uint8_t* value,
               int value_size,
               struct file_manager_t* manager);


// DELETION

/// Shrink root page if it is blank.
/// \param manager struct file_manager_t*, file mangaer.
/// \return int, whether success to shrink root or not.
int shrink_root(struct file_manager_t* manager);
/// Merge two nodes into one node.
/// \param left struct page_pair_t*, lett child.
/// \param key_prime prikey_t, parent key for merging two nodes.
/// \param right struct page_pair_t*, right child.
/// \param parent struct page_pair_t*, parent page.
/// \param manager struct file_manager_t*, file mangaer.
/// \return int, whether success to merge nodes or not.
int merge_nodes(struct page_pair_t* left,
                prikey_t k_prime,
                struct page_pair_t* right,
                struct page_pair_t* parent,
                struct file_manager_t* manager);
/// Redistribute records or entries from one node to other node.
/// \param left struct page_pair_t*, lett child.
/// \param key_prime prikey_t, parent key for merging two nodes.
/// \param key_prim_index int, index of the `key_prime` from parent node.
/// \param right struct page_pair_t*, right child.
/// \param parent struct page_pair_t*, parent page.
/// \param manager struct file_manager_t*, file mangaer.
/// \return int, whether success to redistribute key or not.
int redistribute_nodes(struct page_pair_t* left,
                       prikey_t k_prime,
                       int k_prime_index,
                       struct page_pair_t* right,
                       struct page_pair_t* parent,
                       struct file_manager_t* manager);
/// Delete key from given page with several rebalancing policy.
/// \param key prikey_t, target key.
/// \param page struct page_pair_t*, target page.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to delete key or not.
int delete_entry(prikey_t key,
                 struct page_pair_t* page,
                 struct file_manager_t* manager);
/// Mother function, delete key from tree.
/// \param key prikey_t, target key.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to delete key or not.
int bpt_delete(prikey_t key, struct file_manager_t* manager);

/// Free all tree pages to make empty tree.
/// \param manager struct file_manager_t*, file manager.
/// \return int, whether success to destroy tree or not.
int destroy_tree(struct file_manager_t* manager);

#endif /* __BPT_H__*/
