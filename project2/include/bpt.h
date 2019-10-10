#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>

#include "disk_manager.h"
#include "headers.h"

// global constant
// #define ORDER 32
#define ORDER 5
#define VERBOSE_OUTPUT 1

#define SUCCESS 0
#define FAILURE 1

// STRUCTURE DEFINITION
struct queue_t {
    pagenum_t pagenum;
    struct queue_t* next;
};

struct page_pair_t {
    pagenum_t pagenum;
    struct page_t* page;
};

// FUNCTION PROTOTYPES.

// Page relative.
int load_page(pagenum_t pagenum,
              struct page_t* page,
              struct file_manager_t* manager);
int commit_page(pagenum_t pagenum,
                struct page_t* page,
                struct file_manager_t* manager);

void swap_page_pair(struct page_pair_t* left, struct page_pair_t* right);

// Utility.
void usage_1();
void usage_2();

struct queue_t* enqueue(struct queue_t* queue, pagenum_t pagenum);
struct queue_t* dequeue(struct queue_t* queue, pagenum_t* retval);

int height(pagenum_t node, struct file_manager_t* manager);
int path_to_root(pagenum_t node, struct file_manager_t* manager);

int cut(int length);

// Search.
pagenum_t find_leaf(prikey_t key, struct page_t* page, struct file_manager_t* manager);
int find_key_from_leaf(prikey_t key, struct page_t* page, struct record_t* record);
int find(prikey_t key, struct record_t* record, struct file_manager_t* manager);
int find_range(prikey_t start,
               prikey_t end,
               struct record_t* retval,
               struct file_manager_t* manager);

// Output.
void print_leaves(struct file_manager_t* manager);
void print_tree(struct file_manager_t* manager);

void find_and_print(prikey_t key, struct file_manager_t* manager); 
void find_and_print_range(prikey_t range1, prikey_t range2, struct file_manager_t* manager);

// Insertion.
int make_record(struct record_t* record, prikey_t key, int value);
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
           int value,
           struct file_manager_t* manager);

// Deletion.
int shrink_root(struct file_manager_t* manager);
// node * coalesce_nodes(node * root, node * n, node * neighbor,
//                       int neighbor_index, int k_prime);
// node * redistribute_nodes(node * root, node * n, node * neighbor,
//                           int neighbor_index,
//         int k_prime_index, int k_prime);
int delete_entry(prikey_t key,
                 struct page_pair_t* leaf_page,
                 struct file_manager_t* manager);
int delete(prikey_t key, struct file_manager_t* manager);

int destroy_tree(struct file_manager_t* manager);

#endif /* __BPT_H__*/
