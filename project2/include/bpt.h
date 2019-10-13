#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>

#include "disk_manager.h"
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
/// \param node pagenum_t, target node.
/// \param manager struct file_manager_t*, file manager.
/// \return int, height of the node.
int height(pagenum_t node, struct file_manager_t* manager);
/// Length of path to root.
/// \param node pagenum_t, target node.
/// \param manager struct file_manager_t*, file manager.
/// \return int, length of path to root.
int path_to_root(pagenum_t node, struct file_manager_t* manager);

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
int find(prikey_t key, struct record_t* record, struct file_manager_t* manager);
/// Range based searching from tree.
/// \param start prikey_t, start point.
/// \param end prikey_t, end point.
/// \param record struct record_t*, record vector for returning sequence.
/// \param manager struct manager_t*, file manager.
/// \return int, size of the return sequence.
int find_range(prikey_t start,
               prikey_t end,
               struct record_vec_t* retval,
               struct file_manager_t* manager);


// OUTPUT

void print_leaves(struct file_manager_t* manager);
void print_tree(struct file_manager_t* manager);

void find_and_print(prikey_t key, struct file_manager_t* manager); 
void find_and_print_range(prikey_t range1, prikey_t range2, struct file_manager_t* manager);


// INSERTION

int make_record(struct record_t* record, prikey_t key, uint8_t* value, int value_size);
pagenum_t make_node(struct file_manager_t* manager, uint32_t leaf);

int get_index(struct page_t* parent, pagenum_t pagenum);

int insert_into_leaf(struct page_pair_t* leaf,
                     struct record_t* pointer,
                     struct file_manager_t* manager);
int insert_into_leaf_after_splitting(struct page_pair_t* leaf,
                                     struct record_t* record,
                                     struct file_manager_t* manager);

int insert_into_node(struct page_pair_t* node,
                     int index,
                     struct internal_t* entry,
                     struct file_manager_t* manager);
int insert_into_node_after_splitting(struct page_pair_t* old_node,
                                     int index,
                                     struct internal_t* entry,
                                     struct file_manager_t* manager);

int insert_into_parent(struct page_pair_t* left,
                       prikey_t key,
                       struct page_pair_t* right,
                       struct file_manager_t* manager);
int insert_into_new_root(struct page_pair_t* left,
                         prikey_t key,
                         struct page_pair_t* right,
                         struct file_manager_t* manager);

int start_new_tree(struct record_t* pointer,
                   struct file_manager_t* manager);
int insert(prikey_t key,
           uint8_t* value,
           int value_size,
           struct file_manager_t* manager);


// DELETION

int shrink_root(struct file_manager_t* manager);
int merge_nodes(struct page_pair_t* left,
                prikey_t k_prime,
                struct page_pair_t* right,
                struct page_pair_t* parent,
                struct file_manager_t* manager);
int redistribute_nodes(struct page_pair_t* left,
                       prikey_t k_prime,
                       int k_prime_index,
                       struct page_pair_t* right,
                       struct page_pair_t* parent,
                       struct file_manager_t* manager);
int delete_entry(prikey_t key,
                 struct page_pair_t* leaf_page,
                 struct file_manager_t* manager);
int delete(prikey_t key, struct file_manager_t* manager);

int destroy_tree(struct file_manager_t* manager);

#endif /* __BPT_H__*/
