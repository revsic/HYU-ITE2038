#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>

#include "headers.h"

// global constant
#define ORDER 32

#define VERBOSE_OUTPUT 1

// STRUCTURE DEFINITION
struct queue_t {
    pagenum_t pagenum;
    struct queue_t* next;
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

pagenum_t find_leaf(key_t key, struct file_manager_t* manager);
struct record_t* find(key_t key, struct file_manager_t* manager);
int find_range(key_t start,
               key_t end,
               struct record_t* retval[],
               struct file_manager_t* manager);

void find_and_print(key_t key, struct file_manager_t* manager); 
void find_and_print_range(key_t range1, key_t range2, struct file_manager_t* manager);

int cut( int length );

// Insertion.

record * make_record(int value);
node * make_node( void );
node * make_leaf( void );
int get_left_index(node * parent, node * left);
node * insert_into_leaf( node * leaf, int key, record * pointer );
node * insert_into_leaf_after_splitting(node * root, node * leaf, int key,
                                        record * pointer);
node * insert_into_node(node * root, node * parent, 
        int left_index, int key, node * right);
node * insert_into_node_after_splitting(node * root, node * parent,
                                        int left_index,
        int key, node * right);
node * insert_into_parent(node * root, node * left, int key, node * right);
node * insert_into_new_root(node * left, int key, node * right);
node * start_new_tree(int key, record * pointer);
node * insert( node * root, int key, int value );

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
