#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>

#include "headers.h"

// global constant
#define ORDER 32
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

// Output and utility.
void usage_1();
void usage_2();

struct queue_t* enqueue(struct queue_t* queue, pagenum_t pagenum);
struct queue_t* dequeue(struct queue_t* queue, pagenum_t* retval);

int height(pagenum_t node, struct file_manager_t* manager);
int path_to_root(pagenum_t node, struct file_manager_t* manager);

void print_leaves(struct file_manager_t* manager);
void print_tree(struct file_manager_t* manager);

pagenum_t find_leaf(key_t key, struct page_t* page, struct file_manager_t* manager);
int find(key_t key, struct record_t* record, struct file_manager_t* manager);
int find_range(key_t start,
               key_t end,
               struct record_t* retval,
               struct file_manager_t* manager);

void find_and_print(key_t key, struct file_manager_t* manager); 
void find_and_print_range(key_t range1, key_t range2, struct file_manager_t* manager);

int cut(int length);

// Insertion.
int make_record(struct record_t* record, key_t key, int value);
pagenum_t make_node(struct file_manager_t* manager, uint32_t leaf);

int get_left_index(struct page_t* parent, pagenum_t left);

int insert_into_leaf(struct page_pair_t* leaf, struct record_t* pointer);
pagenum_t insert_into_leaf_after_splitting(struct page_pair_t* leaf,
                                           struct record_t* record,
                                           struct file_manager_t* manager);

int insert_into_node(struct page_pair_t* node,
                     int left_index,
                     struct internal_t* entry);
pagenum_t insert_into_node_after_splitting(struct page_pair_t* old_node,
                                           int left_index,
                                           struct internal_t* entry,
                                           struct file_manager_t* manager);

pagenum_t insert_into_parent(struct page_pair_t* left,
                             key_t key,
                             struct page_pair_t* right,
                             struct file_manager_t* manager);
pagenum_t insert_into_new_root(struct page_pair_t* left,
                               key_t key,
                               struct page_pair_t* right,
                               struct file_manager_t* manager);

pagenum_t start_new_tree(int key,
                         struct record_t* pointer,
                         struct file_manager_t* manager);
pagenum_t insert(pagenum_t root,
                 key_t key,
                 int value,
                 struct file_manager_t* manager);

// Deletion.

int get_neighbor_index( node * n );
node * adjust_root(node * root);
node * coalesce_nodes(node * root, node * n, node * neighbor,
                      int neighbor_index, int k_prime);
node * redistribute_nodes(node * root, node * n, node * neighbor,
                          int neighbor_index,
        int k_prime_index, int k_prime);
node * delete_entry( node * root, node * n, int key, void * pointer );
node * delete( node * root, int key );

void destroy_tree_nodes(node * root);
node * destroy_tree(node * root);

#endif /* __BPT_H__*/
