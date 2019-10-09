#define Version "1.0.on-disk"
/*  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved, reference 3RD-PARTY.md.
 */

#include <string.h>

#include "bpt.h"
#include "disk_manager.h"
#include "utility.h"

#define EXIT_FAILURE 1

#define TRUE 1
#define FALSE 0

void usage_1() {
    printf("B+ Tree of Order %d.\n", ORDER);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n");
}

void usage_2() {
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tp <k> -- Print the path from the root to key k and its associated "
           "value.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tv -- Toggle output of pointer addresses (\"verbose\") in tree and "
           "leaves.\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
}

struct queue_t* enqueue(struct queue_t* queue, pagenum_t pagenum) {
    struct queue_t* tmp;

    struct queue_t* new_node = malloc(sizeof(struct queue_t));
    new_node->pagenum = pagenum;
    new_node->next = NULL;

    if (queue == NULL) {
        queue = new_node;
    }
    else {
        tmp = queue;
        while(tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = new_node;
    }
    return queue;
}

struct queue_t* dequeue(struct queue_t* queue, pagenum_t* retval) {
    struct queue_t* tmp = queue->next;
    *retval = queue->pagenum;
    
    free(queue);
    return tmp;
}

int height(pagenum_t node, struct file_manager_t* manager) {
    int h = 0;
    struct page_t page;
    page_read(node, manager, &page);

    while (!page_header(&page)->is_leaf) {
        node = page_header(&page)->special_page_number;
        page_read(node, manager, &page);
        h++;
    }
    return h;
}

int path_to_root(pagenum_t node, struct file_manager_t* manager) {
    int length = 0;
    struct page_t page;
    pagenum_t root = manager->file_header.root_page_number;

    while (node != root) {
        page_read(node, manager, &page);
        node = page_header(&page)->parent_page_number;
        length++;
    }
    return length;
}

void print_leaves(struct file_manager_t* manager) {
    int i;
    pagenum_t root = manager->file_header.root_page_number;

    if (root == INVALID_PAGENUM) {
        printf("Empty tree.\n");
        return;
    }

    struct page_t page;
    page_read(root, manager, &page);    

    while (!page_header(&page)->is_leaf) {
        root = page_header(&page)->special_page_number;
        page_read(root, manager, &page);
    }

    struct page_header_t* pheader = page_header(&page);
    struct record_t* rec = records(&page);
    while (1) {
        for (i = 0; i < pheader->number_of_keys; ++i) {
            if (VERBOSE_OUTPUT) {
                printf("%lx ", *(int*)rec[i].value);
            }
            printf("%d ", rec[i].key);
        }
        if (VERBOSE_OUTPUT) {
            printf("%lx ", pheader->special_page_number);
        }

        if (pheader->special_page_number != INVALID_PAGENUM) {
            printf(" | ");
            root = pheader->special_page_number;
            page_read(root, manager, &page);
        } else {
            break;
        }
    }
    printf("\n");
}

void print_tree(struct file_manager_t* manager) {
    int i = 0;
    int rank = 0;
    int new_rank = 0;
    pagenum_t n = INVALID_PAGENUM;
    pagenum_t root = manager->file_header.root_page_number;

    if (root == INVALID_PAGENUM) {
        printf("Empty tree.\n");
        return;
    }

    struct page_t page, tmp;
    struct page_header_t* pheader = page_header(&page);

    struct queue_t* queue = NULL;
    queue = enqueue(queue, root);
    while (queue != NULL) {
        queue = dequeue(queue, &n);
        page_read(n, manager, &page);

        if (pheader->parent_page_number != INVALID_PAGENUM) {
            page_read(pheader->parent_page_number, manager, &tmp);
            if (n == page_header(&tmp)->special_page_number) {
                new_rank = path_to_root(n, manager);
                if (new_rank != rank) {
                    rank = new_rank;
                    printf("\n");
                }
            }
        }

        if (VERBOSE_OUTPUT) {
            printf("(%lx)", n);
            if (!pheader->is_leaf) {
                printf("%lx ", pheader->special_page_number);
            }
        }

        for (i = 0; i < pheader->number_of_keys; i++) {
            if (pheader->is_leaf) {
                printf("%d ", records(&page)[i].key);
                if (VERBOSE_OUTPUT) {
                    printf("%lx ", *(int*)records(&page)[i].value);
                }
            } else {
                printf("%d ", entries(&page)[i].key);
                if (VERBOSE_OUTPUT) {
                    printf("%lx ", entries(&page)[i].pagenum);
                }
            }
        }

        if (!pheader->is_leaf) {
            queue = enqueue(queue, pheader->special_page_number);
            for (i = 0; i < pheader->number_of_keys; i++) {
                queue = enqueue(queue, entries(&page)[i].pagenum);
            }
        }

        if (VERBOSE_OUTPUT && pheader->is_leaf) {
            printf("%lx ", pheader->special_page_number);
        }
        printf("| ");
    }
    printf("\n");
}

pagenum_t find_leaf(key_t key, struct page_t* page, struct file_manager_t* manager) {
    int i = 0;
    struct page_t tmp;
    if (page == NULL) {
        page = &tmp;
    }

    struct internal_t* internal = entries(page);
    struct page_header_t* pheader = page_header(page);
    pagenum_t root = manager->file_header.root_page_number;
    if (root == INVALID_PAGENUM) {
        if (VERBOSE_OUTPUT) {
            printf("Empty tree.\n");
        }
        return root;
    }

    pagenum_t c = root;
    page_read(c, manager, page);
    while (!pheader->is_leaf) {
        if (VERBOSE_OUTPUT) {
            printf("[");
            for (i = 0; i < pheader->number_of_keys - 1; ++i)
                printf("%d ", internal[i].key);
            printf("%d] ", internal[i].key);
        }

        i = 0;
        for (i = 0; i < pheader->number_of_keys && key >= internal[i].key; ++i)
            {}

        if (VERBOSE_OUTPUT) {
            printf("%d ->\n", i);
        }

        c = internal[i].pagenum;
        page_read(c, manager, page);
    }

    if (VERBOSE_OUTPUT) {
        printf("Leaf [");
        for (i = 0; i < pheader->number_of_keys - 1; ++i) {
            printf("%d ", internal[i].key);
        }
        printf("%d] ->\n", internal[i].key);
    }

    return c;
}

int find(key_t key, struct record_t* record, struct file_manager_t* manager) {
    int i = 0;
    struct page_t page;
    pagenum_t c = find_leaf(key, &page, manager);
    if (c == INVALID_PAGENUM) {
        return FAILURE;
    }

    for (i = 0; i < page_header(&page)->number_of_keys; ++i) {
        if (records(&page)[i].key == key) {
            break;
        }
    }

    if (i == page_header(&page)->number_of_keys) {
        return FAILURE;
    } else {
        memcpy(record, &records(&page)[i], sizeof(struct record_t));
        return SUCCESS;
    }
}

int find_range(key_t start,
               key_t end,
               struct record_t* retval,
               struct file_manager_t* manager)
{
    int i, num_found = 0;
    struct page_t page;
    pagenum_t n = find_leaf(start, &page, manager);
    if (n == INVALID_PAGENUM) {
        return 0;
    }

    uint32_t nkey = page_header(&page)->number_of_keys;
    struct record_t* rec = records(&page);

    for (i = 0; i < nkey && rec[i].key < start; ++i)
        {}

    if (i == nkey) {
        return 0;
    }

    while (1) {
        for (; i < nkey && rec[i].key <= end; i++) {
            memcpy(&retval[num_found++], &rec[i], sizeof(struct record_t));
        }

        i = 0;
        n = page_header(&page)->special_page_number;
        if (n != INVALID_PAGENUM) {
            break;
        }

        page_read(n, manager, &page);
        nkey = page_header(&page)->number_of_keys;
        rec = records(&page);
    }

    return num_found;
}

void find_and_print(key_t key, struct file_manager_t* manager) {
    struct record_t r;
    int retval = find(key, &r, manager);
    if (retval == FAILURE)
        printf("Record not found under key %d.\n", key);
    else 
        printf("Record -- key %d, value %d.\n",
               key, *(int*)r.value);
}

void find_and_print_range(key_t key_start, key_t key_end, struct file_manager_t* manager) {
    int i;
    int array_size = key_end - key_start + 1;
    struct record_t* retval = malloc(sizeof(struct record_t) * array_size);

    int num_found = find_range(key_start, key_end, retval, manager);
    if (!num_found) {
        printf("None found.\n");
    } else {
        for (i = 0; i < num_found; i++)
            printf("Key: %d   Value: %d\n",
                   retval[i].key,
                   *(int*)retval[i].value);
    }

    free(retval);
}

int cut(int length) {
    if (length % 2 == 0) {
        return length / 2;
    } else {
        return length / 2 + 1;
    }
}


// INSERTION

int make_record(struct record_t* record, key_t key, int value) {
    record->key = key;
    *(int*)record->value = value;
    return SUCCESS;
}

pagenum_t make_node(struct file_manager_t* manager, uint32_t leaf) {
    pagenum_t new_node = page_create(manager);
    if (new_node == INVALID_PAGENUM) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }

    struct page_t page;
    page_init(&page, leaf);
    page_write(new_node, manager, &page);
    return new_node;
}

int get_left_index(struct page_t* parent, pagenum_t left) {
    int left_index = 0;
    int num_key = page_header(parent)->number_of_keys;
    for (;
         left_index < num_key && entries(parent)[left_index].pagenum != left;
         ++left_index)
        {}
    return left_index;
}

int insert_into_leaf(struct page_pair_t* leaf, struct record_t* pointer) {
    int i, insertion_point;
    int num_key = page_header(leaf->page)->number_of_keys;
    struct record_t* rec = records(leaf->page);
    for (insertion_point = 0;
         insertion_point < num_key && rec[insertion_point].key < pointer->key;
         ++insertion_point)
         {}

    for (i = num_key; i > insertion_point; --i) {
        memcpy(&rec[i], &rec[i - 1], sizeof(struct record_t));
    }

    page_header(leaf->page)->number_of_keys += 1;
    memcpy(&rec[insertion_point], pointer, sizeof(struct record_t));
    return SUCCESS;
}

pagenum_t insert_into_leaf_after_splitting(struct page_pair_t* leaf,
                                           struct record_t* record,
                                           struct file_manager_t* manager)
{
    int insertion_index, split_index, i, j;
    pagenum_t new_leaf = make_node(manager, TRUE);

    struct page_t new_page;
    page_init(&new_page, TRUE);

    struct record_t* temp_record = malloc(ORDER * sizeof(struct record_t));
    if (temp_record == NULL) {
        perror("Temporary records array.");
        exit(EXIT_FAILURE);
    }

    struct record_t* leaf_rec = records(leaf->page);
    struct page_header_t* leaf_header = page_header(leaf->page);
    for (insertion_index = 0;
         insertion_index < leaf_header->number_of_keys
            && leaf_rec[insertion_index].key < record->key;
         ++insertion_index)
        {}

    for (i = 0, j = 0; i < leaf_header->number_of_keys; i++, j++) {
        if (j == insertion_index) {
            ++j;
        }
        memcpy(&temp_record[j], &leaf_rec[i], sizeof(struct record_t));
    }

    memcpy(&temp_record[insertion_index], record, sizeof(struct record_t));

    leaf_header->number_of_keys = 0;
    split_index = cut(ORDER - 1);

    for (i = 0; i < split_index; i++) {
        memcpy(&leaf_rec[i], &temp_record[i], sizeof(struct record_t));
        leaf_header->number_of_keys++;
    }

    struct record_t* new_rec = records(&new_page);
    struct page_header_t* new_header = page_header(&new_page);
    for (i = split_index, j = 0; i < ORDER; i++, j++) {
        memcpy(&new_rec[j], &temp_record[i], sizeof(struct record_t));
        new_header->number_of_keys++;
    }

    free(temp_record);

    new_header->special_page_number = leaf_header->special_page_number;
    leaf_header->special_page_number = new_leaf;

    new_header->parent_page_number = leaf_header->parent_page_number;

    struct page_pair_t right = { new_leaf, &new_page };
    return insert_into_parent(leaf, new_rec[0].key, &right, manager);
}

int insert_into_node(struct page_pair_t* node,
                     int left_index,
                     struct internal_t* entry)
{
    int i;
    struct internal_t* ent = entries(node->page);
    struct page_header_t* header = page_header(node->page);
    for (i = header->number_of_keys; i > left_index; --i) {
        ent[i] = ent[i - 1];
    }
    ent[left_index] = *entry;
    header->number_of_keys++;
    return SUCCESS;
}

pagenum_t insert_into_node_after_splitting(struct page_pair_t* old_node,
                                           int left_index, 
                                           struct internal_t* entry,
                                           struct file_manager_t* manager)
{
    int i, j, split_index, k_prime;
    struct internal_t* temp = malloc(ORDER * sizeof(key_t));
    if (temp == NULL) {
        perror("Temporary array for splitting nodes.");
        exit(EXIT_FAILURE);
    }

    struct internal_t* ent = entries(old_node->page);
    struct page_header_t* header = page_header(old_node->page);

    for (i = 0; i < header->number_of_keys; ++i) {
        if (i == left_index) {
            i++;
        }
        temp[i] = ent[i];
    }
    temp[left_index] = *entry;

    split_index = cut(ORDER);
    pagenum_t new_node = make_node(manager, FALSE);

    struct page_t new_page;
    page_init(&new_page, FALSE);

    struct internal_t* new_entries = entries(&new_page);
    struct page_header_t* new_header = page_header(&new_page);

    header->number_of_keys = 0;
    for (i = 0; i < split_index - 1; i++) {
        ent[i] = temp[i];
        header->number_of_keys++;
    }

    k_prime = temp[split_index - 1].key;
    new_header->special_page_number = temp[split_index - 1].pagenum;
    for (++i, j = 0; i < ORDER; ++i, ++j) {
        new_entries[j] = temp[i];
        new_header->number_of_keys++;
    }

    free(temp);

    pagenum_t temp_pagenum;
    struct page_t temp_page;
    new_header->parent_page_number = header->parent_page_number;
    for (i = -1; i < new_header->number_of_keys; ++i) {
        if (i == -1) {
            temp_pagenum = new_header->special_page_number;
        } else {
            temp_pagenum = new_entries[i].pagenum;
        }
        page_read(temp_pagenum, manager, &temp_page);
        page_header(&temp_page)->parent_page_number = new_node;
        page_write(temp_pagenum, manager, &temp_page);
    }

    struct page_pair_t right = { new_node, &new_page };
    return insert_into_parent(old_node, k_prime, &right, manager);
}

pagenum_t insert_into_parent(struct page_pair_t* left,
                             key_t key,
                             struct page_pair_t* right,
                             struct file_manager_t* manager) {
    int left_index;
    pagenum_t parent = page_header(left)->parent_page_number;

    /* Case: new root. */
    if (parent == INVALID_PAGENUM)
        return insert_into_new_root(left, key, right);

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */
    struct page_t parent_page;
    page_read(parent, manager, &parent_page);
    left_index = get_left_index(&parent, left->pagenum);

    /* Simple case: the new key fits into the node. 
     */
    struct internal_t entry = { key, right->pagenum };
    struct page_pair_t parent_pair = { parent, &parent_page };
    if (page_header(&parent_page)->number_of_keys < ORDER - 1)
        return insert_into_node(&parent_pair, left_index, &entry);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */
    return insert_into_node_after_splitting(&parent_pair, left_index, &entry, manager);
}

pagenum_t insert_into_new_root(struct page_pair_t* left,
                               key_t key,
                               struct page_pair_t* right,
                               struct file_manager_t* manager)
{
    pagenum_t root = make_node(manager, FALSE);

    struct page_t root_page;
    page_init(&root_page, FALSE);

    struct page_header_t* header = page_header(&root_page);
    header->number_of_keys++;
    header->special_page_number = left->pagenum;

    page_header(left->page)->parent_page_number = root;
    page_header(right->page)->parent_page_number = root;

    struct internal_t* ent = entries(&root_page);
    ent[0].key = key;
    ent[0].pagenum = right->pagenum;

    return root;
}

pagenum_t start_new_tree(int key,
                         struct record_t* pointer,
                         struct file_manager_t* manager)
{
    pagenum_t root = make_node(manager, FALSE);

    struct page_t root_page;
    page_init(&root_page, FALSE);

    struct page_header_t* header = page_header(&root_page);
    header->number_of_keys++;
    header->special_page_number = key;

    struct internal_t* entry = &entries(&root_page)[0];
    entry->key = key;
    entry->pagenum = INVALID_PAGENUM;

    return root;
}

pagenum_t insert(key_t key,
                 int value,
                 struct file_manager_t* manager)
{
    struct record_t record;
    pagenum_t root = manager->file_header.root_page_number;

    /* The current implementation ignores
     * duplicates.
     */
    if (find(key, &record, manager) == SUCCESS)
        return root;

    /* Create a new record for the
     * value.
     */
    make_record(&record, key, value);

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */
    if (root == INVALID_PAGENUM) 
        return start_new_tree(key, &record, manager);


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    struct page_t leaf_page;
    pagenum_t leaf = find_leaf(key, &leaf_page, manager);

    /* Case: leaf has room for key and pointer.
     */
    struct page_pair_t pair = { leaf, &leaf_page };
    if (page_header(&leaf_page)->number_of_keys < ORDER - 1) {
        insert_into_leaf(&pair, &record);
        return root;
    }

    /* Case:  leaf must be split.
     */
    return insert_into_leaf_after_splitting(&leaf, &record, manager);
}


// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int get_neighbor_index( node * n ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= n->parent->num_keys; i++)
        if (n->parent->pointers[i] == n)
            return i - 1;

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}


node * remove_entry_from_node(node * n, int key, node * pointer) {

    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    i = 0;
    while (n->keys[i] != key)
        i++;
    for (++i; i < n->num_keys; i++)
        n->keys[i - 1] = n->keys[i];

    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
    i = 0;
    while (n->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        n->pointers[i - 1] = n->pointers[i];


    // One key fewer.
    n->num_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (n->is_leaf)
        for (i = n->num_keys; i < order - 1; i++)
            n->pointers[i] = NULL;
    else
        for (i = n->num_keys + 1; i < order; i++)
            n->pointers[i] = NULL;

    return n;
}


node * adjust_root(node * root) {

    node * new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->num_keys > 0)
        return root;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    if (!root->is_leaf) {
        new_root = root->pointers[0];
        new_root->parent = NULL;
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else
        new_root = NULL;

    free(root->keys);
    free(root->pointers);
    free(root);

    return new_root;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
node * coalesce_nodes(node * root, node * n, node * neighbor, int neighbor_index, int k_prime) {

    int i, j, neighbor_insertion_index, n_end;
    node * tmp;

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = neighbor->num_keys;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (!n->is_leaf) {

        /* Append k_prime.
         */

        neighbor->keys[neighbor_insertion_index] = k_prime;
        neighbor->num_keys++;


        n_end = n->num_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
            n->num_keys--;
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        neighbor->pointers[i] = n->pointers[j];

        /* All children must now point up to the same parent.
         */

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            tmp = (node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
        }
        neighbor->pointers[order - 1] = n->pointers[order - 1];
    }

    root = delete_entry(root, n->parent, k_prime, n);
    free(n->keys);
    free(n->pointers);
    free(n); 
    return root;
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
node * redistribute_nodes(node * root, node * n, node * neighbor, int neighbor_index, 
        int k_prime_index, int k_prime) {  

    int i;
    node * tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if (neighbor_index != -1) {
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node *)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = NULL;
            n->keys[0] = k_prime;
            n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            n->parent->keys[k_prime_index] = n->keys[0];
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if (n->is_leaf) {
            n->keys[n->num_keys] = neighbor->keys[0];
            n->pointers[n->num_keys] = neighbor->pointers[0];
            n->parent->keys[k_prime_index] = neighbor->keys[1];
        }
        else {
            n->keys[n->num_keys] = k_prime;
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (node *)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->num_keys++;
    neighbor->num_keys--;

    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
node * delete_entry( node * root, node * n, int key, void * pointer ) {

    int min_keys;
    node * neighbor;
    int neighbor_index;
    int k_prime_index, k_prime;
    int capacity;

    // Remove key and pointer from node.

    n = remove_entry_from_node(n, key, pointer);

    /* Case:  deletion from the root. 
     */

    if (n == root) 
        return adjust_root(root);


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    min_keys = n->is_leaf ? cut(order - 1) : cut(order) - 1;

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (n->num_keys >= min_keys)
        return root;

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = n->parent->keys[k_prime_index];
    neighbor = neighbor_index == -1 ? n->parent->pointers[1] : 
        n->parent->pointers[neighbor_index];

    capacity = n->is_leaf ? order : order - 1;

    /* Coalescence. */

    if (neighbor->num_keys + n->num_keys < capacity)
        return coalesce_nodes(root, n, neighbor, neighbor_index, k_prime);

    /* Redistribution. */

    else
        return redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
}



/* Master deletion function.
 */
node * delete(node * root, int key) {

    node * key_leaf;
    record * key_record;

    key_record = find(root, key, false);
    key_leaf = find_leaf(root, key, false);
    if (key_record != NULL && key_leaf != NULL) {
        root = delete_entry(root, key_leaf, key, key_record);
        free(key_record);
    }
    return root;
}


void destroy_tree_nodes(node * root) {
    int i;
    if (root->is_leaf)
        for (i = 0; i < root->num_keys; i++)
            free(root->pointers[i]);
    else
        for (i = 0; i < root->num_keys + 1; i++)
            destroy_tree_nodes(root->pointers[i]);
    free(root->pointers);
    free(root->keys);
    free(root);
}


node * destroy_tree(node * root) {
    destroy_tree_nodes(root);
    return NULL;
}

