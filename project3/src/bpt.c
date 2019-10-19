#define Version "1.0.on-disk"
/*  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved, reference 3RD-PARTY.md.
 */

#include <stdlib.h>
#include <string.h>

#include "bpt.h"
#include "dbms.h"
#include "utility.h"

int load_page(pagenum_t pagenum,
              struct page_t* page,
              struct file_manager_t* manager)
{
    return page_read(pagenum, manager, page);
}

int commit_page(pagenum_t pagenum,
                struct page_t* page,
                struct file_manager_t* manager)
{
    return page_write(pagenum, manager, page);
}

void swap_page_pair(struct page_pair_t* left, struct page_pair_t* right) {
    struct page_pair_t tmp = *left;
    *left = *right;
    *right = tmp;
}

void usage_1() {
    printf("B+ Tree of leaf order %d, internal order %d.\n", LEAF_ORDER, INTERNAL_ORDER);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n");
}

void usage_2() {
    printf("Enter any of the following commands after the prompt > :\n"
    "\ti <k>  -- Insert <k> (an integer) as both key and value).\n"
    "\tf <k>  -- Find the value under key <k>.\n"
    "\tr <k1> <k2> -- Print the keys and values found in the range "
            "[<k1>, <k2>\n"
    "\td <k>  -- Delete key <k> and its associated value.\n"
    "\tx -- Destroy the whole tree.  Start again with an empty tree of the "
           "same order.\n"
    "\tt -- Print the B+ tree.\n"
    "\tl -- Print the keys of the leaves (bottom row of the tree).\n"
    "\tq -- Quit. (Or use Ctl-D.)\n"
    "\t? -- Print this help message.\n");
}

struct queue_t* enqueue(struct queue_t* queue, pagenum_t pagenum) {
    struct queue_t* tmp;
    struct queue_t* new_node = malloc(sizeof(struct queue_t));
    if (new_node == NULL) {
        return NULL;
    }

    new_node->pagenum = pagenum;
    new_node->next = NULL;

    if (queue == NULL) {
        queue = new_node;
    } else {
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

int record_vec_init(struct record_vec_t* vec) {
    vec->size = 0;
    vec->capacity = DEFAULT_RECORD_VEC_CAP;
    vec->rec = malloc(sizeof(struct record_t) * DEFAULT_RECORD_VEC_CAP);
    if (vec->rec == NULL) {
        vec->capacity = 0;
        return FAILURE;
    }
    return SUCCESS;
}

int record_vec_free(struct record_vec_t* vec) {
    free(vec->rec);
    return SUCCESS;
}

int record_vec_expand(struct record_vec_t* vec) {
    vec->capacity *= 2;
    struct record_t* new_records =
        malloc(sizeof(struct record_t) * vec->capacity);
    if (new_records == NULL) {
        vec->capacity /= 2;
        return FAILURE;
    }
    
    memcpy(new_records, vec->rec, sizeof(struct record_t) * vec->size);
    
    free(vec->rec);
    vec->rec = new_records;

    return SUCCESS;
}

int record_vec_append(struct record_vec_t* vec, struct record_t* rec) {
    if (vec->size >= vec->capacity) {
        CHECK_SUCCESS(record_vec_expand(vec));
    }
    memcpy(&vec->rec[vec->size++], rec, sizeof(struct record_t));
    return SUCCESS;
}

int height(struct dbms_table_t* table, pagenum_t pagenum) {
    int h, is_leaf;
    struct ubuffer_t buffer;

    for (h = 0, is_leaf = FALSE; !is_leaf; ++h) {
        buffer = dbms_buffering_from_table(table, pagenum);
        BUFFER_READ(buffer, {
            is_leaf = page_header(from_ubuffer(&buffer))->is_leaf;
            pagenum = page_header(from_ubuffer(&buffer))->special_page_number;
        })
    }

    return h;
}

int path_to_root(struct dbms_table_t* table, pagenum_t pagenum) {
    int length;
    pagenum_t root;
    struct ubuffer_t buffer;
    buffer = dbms_buffering_from_table(table, FILE_HEADER_PAGENUM);
    BUFFER_READ(buffer, {
        root = file_header(from_ubuffer(&buffer))->root_page_number;
    })

    for (length = 0; root != pagenum; ++length) {
        buffer = dbms_buffering_from_table(table, pagenum);
        BUFFER_READ(buffer, {
            pagenum = page_header(from_ubuffer(&buffer))->parent_page_number;
        })
    }

    return length;
}

int cut(int length) {
    if (length % 2 == 0) {
        return length / 2;
    } else {
        return length / 2 + 1;
    }
}


// Search.

pagenum_t find_leaf(prikey_t key,
                    struct ubuffer_t* buffer,
                    struct dbms_table_t* table)
{
    int i = 0;
    pagenum_t root, c;
    struct ubuffer_t tmp;
    struct record_t* rec;
    struct internal_t* ent;
    struct page_header_t* header;
    if (buffer == NULL) {
        buffer = &tmp;
    }

    *buffer = dbms_buffering_from_table(table, FILE_HEADER_PAGENUM);
    BUFFER_READ(*buffer, {
        root = file_header(from_ubuffer(buffer))->root_page_number;
    })

    if (root == INVALID_PAGENUM) {
        if (VERBOSE_OUTPUT) {
            printf("Empty tree.\n");
        }
        return root;
    }

    c = root;
    while (TRUE) {
        *buffer = dbms_buffering_from_table(table, c);
        BUFFER_READ(*buffer, {
            ent = entries(from_ubuffer(buffer));
            header = page_header(from_ubuffer(buffer));

            if (!header->is_leaf) {
                BUFFER_INTERCEPT_READ(*buffer, break);
            }

            if (VERBOSE_OUTPUT) {
                printf("[");
                for (i = 0; i < header->number_of_keys - 1; ++i) {
                    printf("%ld ", ent[i].key);
                }
                printf("%ld] ", ent[i].key);
            }

            for (i = 0; i < header->number_of_keys && key >= ent[i].key; ++i)
                {}
            
            --i;
            if (VERBOSE_OUTPUT) {
                printf("%d ->\n", i);
            }

            if (i < 0) {
                c = header->special_page_number;
            } else {
                c = ent[i].pagenum;
            }
        })
    }

    if (VERBOSE_OUTPUT) {
        BUFFER_READ(*buffer, {
            header = page_header(from_ubuffer(buffer));
            rec = records(header);

            printf("Leaf [");
            for (i = 0; i < header->number_of_keys - 1; ++i) {
                printf("%ld ", rec[i].key);
            }
            printf("%ld]\n", rec[i].key);
        })
    }

    return c;
}

int find_key_from_leaf(prikey_t key,
                       struct ubuffer_t buffer,
                       struct record_t* record)
{
    int i, num_key, is_leaf;
    struct page_t* page;
    BUFFER_READ(buffer, {
        page = from_ubuffer(&buffer);
        if (!page_header(page)->is_leaf) {
            BUFFER_INTERCEPT_READ(buffer, return FAILURE);
        }

        num_key = page_header(page)->number_of_keys;
        for (i = 0; i < num_key && records(page)[i].key != key; ++i)
            {}

        if (i < num_key && record != NULL) {
            memcpy(record, &records(page)[i], sizeof(struct record_t));
        }
    })

    if (i == num_key) {
        return FAILURE;
    } else {
        return SUCCESS;
    }
}

int bpt_find(prikey_t key,
             struct record_t* record,
             struct dbms_table_t* table)
{
    int i = 0;
    struct ubuffer_t buffer;
    pagenum_t c = find_leaf(key, &buffer, table);
    if (c == INVALID_PAGENUM) {
        return FAILURE;
    }

    return find_key_from_leaf(key, buffer, record);
}

int bpt_find_range(prikey_t start,
                   prikey_t end,
                   struct record_vec_t* retval,
                   struct dbms_table_t* table)
{
    int i, num_key;
    struct record_t* rec;
    struct ubuffer_t buffer, tmp;
    pagenum_t n = find_leaf(start, &buffer, table);
    if (n == INVALID_PAGENUM) {
        return FAILURE;
    }

    BUFFER_READ(buffer, {
        rec = records(from_ubuffer(&buffer));
        num_key = page_header(from_ubuffer(&buffer))->number_of_keys;
        for (i = 0; i < num_key && rec[i].key < start; ++i)
            {}
    })

    if (i == num_key) {
        return 0;
    }

    while (TRUE) {
        BUFFER_READ(buffer, {
            rec = records(from_ubuffer(&buffer));
            num_key = page_header(from_ubuffer(&buffer))->number_of_keys;

            for (; i < num_key && rec[i].key <= end; i++) {
                BUFFER_READ_CHECK_SUCCESS(buffer, record_vec_append(retval, &rec[i]));
            }

            n = page_header(from_ubuffer(&buffer))->special_page_number;
            if ((i < num_key && rec[i].key > end) || n == INVALID_PAGENUM) {
                BUFFER_INTERCEPT_READ(buffer, break);
            }

            tmp = dbms_buffering_from_table(table, n);
            BUFFER_READ_CHECK_NULL(buffer, tmp.buf);
        })
        i = 0;
        buffer = tmp;
    }

    return retval->size;
}


// Output.

void print_leaves(struct dbms_table_t* table) {
    int i, is_leaf;
    pagenum_t pagenum, root;
    struct ubuffer_t buffer = dbms_buffering_from_table(table, FILE_HEADER_PAGENUM);
    BUFFER_READ(buffer, {
        root = file_header(from_ubuffer(&buffer))->root_page_number;
    })

    if (root == INVALID_PAGENUM) {
        printf("Empty tree.\n");
        return;
    }

    is_leaf = FALSE;
    pagenum = root;
    while (!is_leaf) {
        buffer = dbms_buffering_from_table(table, pagenum);
        BUFFER_READ(buffer, {
            is_leaf = page_header(from_ubuffer(&buffer))->is_leaf;
            pagenum = page_header(from_ubuffer(&buffer))->special_page_number;
        })
    }

    int num_key;
    struct record_t* rec;
    while (1) {
        BUFFER_READ(buffer, {
            rec = records(from_ubuffer(&buffer));
            num_key = page_header(from_ubuffer(&buffer))->number_of_keys;
            pagenum = page_header(from_ubuffer(&buffer))->special_page_number;

            for (i = 0; i < num_key; ++i) {
                printf("%ld ", rec[i].key);
                if (VERBOSE_OUTPUT) {
                    printf("{v: %s} ", rec[i].value);
                }
            }
        })

        if (VERBOSE_OUTPUT) {
            printf("(next %lu) ", pagenum);
        }

        if (pagenum != INVALID_PAGENUM) {
            printf(" | ");
            buffer = dbms_buffering_from_table(table, pagenum);
        } else {
            break;
        }
    }
    printf("\n");
}

void print_tree(struct dbms_table_t* table) {
    pagenum_t root, n;
    int i, new_rank, rank = 0;
    struct page_header_t* pheader;

    struct record_t* rec;
    struct internal_t* ent;

    struct ubuffer_t buffer, tmp;
    buffer = dbms_buffering_from_table(table, FILE_HEADER_PAGENUM);
    BUFFER_READ(buffer, {
        root = file_header(from_ubuffer(&buffer))->root_page_number;
    })

    if (root == INVALID_PAGENUM) {
        printf("Empty tree.\n");
        return;
    }

    struct queue_t* queue = NULL;
    queue = enqueue(queue, root);
    while (queue != NULL) {
        queue = dequeue(queue, &n);

        buffer = dbms_buffering_from_table(table, n);
        BUFFER_READ(buffer, {
            pheader = page_header(from_ubuffer(&buffer));
            if (pheader->parent_page_number != INVALID_PAGENUM) {
                tmp = dbms_buffering_from_table(
                    table,
                    pheader->parent_page_number));
                BUFFER_READ(tmp, {
                    if (n == page_header(from_ubuffer(&tmp))->special_page_number) {
                        new_rank = path_to_root(table, n);
                        if (new_rank != rank) {
                            rank = new_rank;
                            printf("\n");
                        }
                    }
                })
            }

            if (VERBOSE_OUTPUT) {
                printf("(page %lu) ", n);
                if (!pheader->is_leaf) {
                    printf("{v: %lu} ", pheader->special_page_number);
                }
            }

            rec = records(from_ubuffer(&buffer));
            ent = entries(from_ubuffer(&buffer));
            for (i = 0; i < pheader->number_of_keys; i++) {
                if (pheader->is_leaf) {
                    printf("%ld ", rec[i].key);
                    if (VERBOSE_OUTPUT) {
                        printf("{v: %s} ", rec[i].value);
                    }
                } else {
                    printf("%ld ", ent[i].key);
                    if (VERBOSE_OUTPUT) {
                        printf("{v: %ld} ", ent[i].pagenum);
                    }
                }
            }

            if (!pheader->is_leaf) {
                if (pheader->special_page_number != INVALID_PAGENUM) {
                    queue = enqueue(queue, pheader->special_page_number);
                    for (i = 0; i < pheader->number_of_keys; i++) {
                        queue = enqueue(queue, ent[i].pagenum);
                    }
                }
            }

            if (VERBOSE_OUTPUT && pheader->is_leaf) {
                printf("(parent %lu, next %lu) ",
                    pheader->parent_page_number, pheader->special_page_number);
            }
            printf("| ");
        })
    }
    printf("\n");
}

void find_and_print(prikey_t key, struct dbms_table_t* table) {
    struct record_t r;
    int retval = bpt_find(key, &r, table);
    if (retval == FAILURE) {
        printf("Record not found under key %ld.\n", key);
    } else {
        printf("Record -- key %ld, value %s.\n", key, r.value);
    }
}

void find_and_print_range(prikey_t range1,
                          prikey_t range2,
                          struct dbms_table_t* table)
{
    int i;
    struct record_vec_t retval;
    EXIT_ON_FAILURE(record_vec_init(&retval));

    bpt_find_range(range1, range2, &retval, table);
    if (retval.size == 0) {
        printf("None found.\n");
    } else {
        for (i = 0; i < retval.size; i++) {
            printf("Key: %ld   Value: %s\n", retval.rec[i].key, retval.rec[i].value);
        }
    }

    record_vec_free(&retval);
}


// INSERTION

int make_record(struct record_t* record, prikey_t key, uint8_t* value, int value_size) {
    record->key = key;    
    value_size = max(value_size, sizeof(struct record_t) - sizeof(prikey_t));
    memcpy(record->value, value, value_size);
    return SUCCESS;
}

struct buffer_t* make_node(struct dbms_table_t* table, uint32_t leaf) {
    struct buffer_t* buffer = dbms_new_page_from_table(table);
    if (buffer == NULL) {
        return NULL;
    }

    struct page_t* page = from_ubuffer(&buffer);
    BUFFER_WRITE(buffer, {
        EXIT_ON_FAILURE(page_init(page, leaf));
    })

    return buffer;
}

int get_index(struct page_t* parent, pagenum_t pagenum) {
    int index, num_key;
    if (page_header(parent)->special_page_number == pagenum) {
        return -1;
    }

    num_key = page_header(parent)->number_of_keys;
    for (index = 0;
         index < num_key && entries(parent)[index].pagenum != pagenum;
         ++index)
        {}
    return index;
}

int insert_into_leaf(struct page_pair_t* leaf,
                     struct record_t* pointer,
                     struct file_manager_t* manager)
{
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

    CHECK_SUCCESS(commit_page(leaf->pagenum, leaf->page, manager));
    return SUCCESS;
}

int insert_into_leaf_after_splitting(struct page_pair_t* leaf,
                                     struct record_t* record,
                                     struct file_manager_t* manager)
{
    int insertion_index, split_index, i, j;
    pagenum_t new_leaf = make_node(manager, TRUE);

    struct page_t new_page;
    CHECK_SUCCESS(page_init(&new_page, TRUE));

    struct record_t temp_record[LEAF_ORDER];

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
    split_index = cut(LEAF_ORDER - 1);

    for (i = 0; i < split_index; i++) {
        memcpy(&leaf_rec[i], &temp_record[i], sizeof(struct record_t));
        leaf_header->number_of_keys++;
    }

    struct record_t* new_rec = records(&new_page);
    struct page_header_t* new_header = page_header(&new_page);
    for (i = split_index, j = 0; i < LEAF_ORDER; i++, j++) {
        memcpy(&new_rec[j], &temp_record[i], sizeof(struct record_t));
        new_header->number_of_keys++;
    }

    new_header->special_page_number = leaf_header->special_page_number;
    leaf_header->special_page_number = new_leaf;

    new_header->parent_page_number = leaf_header->parent_page_number;

    CHECK_SUCCESS(commit_page(leaf->pagenum, leaf->page, manager));
    CHECK_SUCCESS(commit_page(new_leaf, &new_page, manager));

    struct page_pair_t right = { new_leaf, &new_page };
    return insert_into_parent(leaf, new_rec[0].key, &right, manager);
}

int insert_into_node(struct page_pair_t* node,
                     int index,
                     struct internal_t* entry,
                     struct file_manager_t* manager)
{
    int i;
    struct internal_t* ent = entries(node->page);
    struct page_header_t* header = page_header(node->page);
    for (i = header->number_of_keys; i > index; --i) {
        ent[i] = ent[i - 1];
    }
    ent[index] = *entry;
    header->number_of_keys++;
    CHECK_SUCCESS(commit_page(node->pagenum, node->page, manager));
    return SUCCESS;
}

int insert_into_node_after_splitting(struct page_pair_t* old_node,
                                     int index, 
                                     struct internal_t* entry,
                                     struct file_manager_t* manager)
{
    int i, j, split_index, k_prime;
    struct internal_t temp[INTERNAL_ORDER];

    struct internal_t* ent = entries(old_node->page);
    struct page_header_t* header = page_header(old_node->page);

    for (i = 0, j = 0; j < header->number_of_keys; ++i, ++j) {
        if (i == index) {
            i++;
        }
        temp[i] = ent[j];
    }
    temp[index] = *entry;

    split_index = cut(INTERNAL_ORDER);
    pagenum_t new_node = make_node(manager, FALSE);

    struct page_t new_page;
    CHECK_SUCCESS(page_init(&new_page, FALSE));

    struct internal_t* new_entries = entries(&new_page);
    struct page_header_t* new_header = page_header(&new_page);

    header->number_of_keys = 0;
    for (i = 0; i < split_index - 1; ++i) {
        ent[i] = temp[i];
        header->number_of_keys++;
    }

    k_prime = temp[split_index - 1].key;
    new_header->special_page_number = temp[split_index - 1].pagenum;
    for (++i, j = 0; i < INTERNAL_ORDER; ++i, ++j) {
        new_entries[j] = temp[i];
        new_header->number_of_keys++;
    }

    pagenum_t temp_pagenum;
    struct page_t temp_page;
    new_header->parent_page_number = header->parent_page_number;
    for (i = -1; i < (int)new_header->number_of_keys; ++i) {
        if (i == -1) {
            temp_pagenum = new_header->special_page_number;
        } else {
            temp_pagenum = new_entries[i].pagenum;
        }
        CHECK_SUCCESS(load_page(temp_pagenum, &temp_page, manager));
        page_header(&temp_page)->parent_page_number = new_node;
        CHECK_SUCCESS(commit_page(temp_pagenum, &temp_page, manager));
    }

    CHECK_SUCCESS(commit_page(old_node->pagenum, old_node->page, manager));
    CHECK_SUCCESS(commit_page(new_node, &new_page, manager));

    struct page_pair_t right = { new_node, &new_page };
    return insert_into_parent(old_node, k_prime, &right, manager);
}

int insert_into_parent(struct page_pair_t* left,
                       prikey_t key,
                       struct page_pair_t* right,
                       struct file_manager_t* manager)
{
    pagenum_t parent = page_header(left->page)->parent_page_number;

    /* Case: new root. */
    if (parent == INVALID_PAGENUM) {
        return insert_into_new_root(left, key, right, manager);
    }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */
    struct page_t parent_page;
    load_page(parent, &parent_page, manager);
    int index = get_index(&parent_page, left->pagenum) + 1;

    /* Simple case: the new key fits into the node. 
     */
    struct internal_t entry = { key, right->pagenum };
    struct page_pair_t parent_pair = { parent, &parent_page };
    if (page_header(&parent_page)->number_of_keys < INTERNAL_ORDER - 1) {
        return insert_into_node(&parent_pair, index, &entry, manager);
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */
    return insert_into_node_after_splitting(&parent_pair, index, &entry, manager);
}

int insert_into_new_root(struct page_pair_t* left,
                         prikey_t key,
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

    CHECK_SUCCESS(commit_page(root, &root_page, manager));
    CHECK_SUCCESS(commit_page(left->pagenum, left->page, manager));
    CHECK_SUCCESS(commit_page(right->pagenum, right->page, manager));

    struct page_t file_page;
    CHECK_SUCCESS(load_page(FILE_HEADER_PAGENUM, &file_page, manager));

    file_header(&file_page)->root_page_number = root;
    CHECK_SUCCESS(commit_page(FILE_HEADER_PAGENUM, &file_page, manager));

    return SUCCESS;
}

int start_new_tree(struct record_t* pointer,
                   struct file_manager_t* manager)
{
    pagenum_t root = make_node(manager, TRUE);

    struct page_t root_page;
    CHECK_SUCCESS(page_init(&root_page, TRUE));

    struct page_header_t* header = page_header(&root_page);
    header->number_of_keys++;

    struct record_t* rec = &records(&root_page)[0];
    memcpy(rec, pointer, sizeof(struct record_t));

    CHECK_SUCCESS(commit_page(root, &root_page, manager));

    struct page_t file_page;
    CHECK_SUCCESS(load_page(FILE_HEADER_PAGENUM, &file_page, manager));

    file_header(&file_page)->root_page_number = root;
    CHECK_SUCCESS(commit_page(FILE_HEADER_PAGENUM, &file_page, manager));

    return SUCCESS;
}

int bpt_insert(prikey_t key,
               uint8_t* value,
               int value_size,
               struct file_manager_t* manager)
{
    struct page_t leaf_page;
    pagenum_t leaf = find_leaf(key, &leaf_page, manager);

    /* The current implementation ignores
     * duplicates.
     */
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, &leaf_page, NULL) == SUCCESS)
    {
        return FAILURE;
    }

    /* Create a new record for the
     * value.
     */
    struct record_t record;
    CHECK_SUCCESS(make_record(&record, key, value, value_size));

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */
    struct page_t file_page;
    CHECK_SUCCESS(load_page(FILE_HEADER_PAGENUM, &file_page, manager));
    pagenum_t root = file_header(&file_page)->root_page_number;
    if (root == INVALID_PAGENUM) {
        return start_new_tree(&record, manager);
    }

    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    /* Case: leaf has room for key and pointer.
     */
    struct page_pair_t pair = { leaf, &leaf_page };
    if (page_header(&leaf_page)->number_of_keys < LEAF_ORDER - 1) {
        return insert_into_leaf(&pair, &record, manager);
    }

    /* Case:  leaf must be split.
     */
    return insert_into_leaf_after_splitting(&pair, &record, manager);
}


// DELETION.

int remove_record_from_leaf(prikey_t key, struct page_t* node) {
    struct record_t* rec = records(node);
    int i, num_key = page_header(node)->number_of_keys;
    for (i = 0; i < num_key && rec[i].key != key; ++i)
        {}
    
    if (i == num_key) {
        return FAILURE;
    }

    for (++i; i < num_key; ++i) {
        rec[i - 1] = rec[i];
    }

    --page_header(node)->number_of_keys;
    return SUCCESS;
}

int remove_entry_from_internal(prikey_t key, struct page_t* node) {
    struct internal_t* ent = entries(node);
    int i, num_key = page_header(node)->number_of_keys;
    for (i = 0; i < num_key && ent[i].key != key; ++i)
        {}
    
    if (i == num_key) {
        return FAILURE;
    }

    for (++i; i < num_key; ++i) {
        ent[i - 1] = ent[i];
    }

    --page_header(node)->number_of_keys;
    return SUCCESS;
}

int shrink_root(struct file_manager_t* manager) {
    struct page_t file_page;
    struct file_header_t* header = file_header(&file_page);
    CHECK_SUCCESS(load_page(FILE_HEADER_PAGENUM, &file_page, manager));
    pagenum_t root = header->root_page_number;
    
    struct page_t root_page;
    CHECK_SUCCESS(load_page(root, &root_page, manager));

    if (page_header(&root_page)->number_of_keys > 0) {
        return SUCCESS;
    }

    pagenum_t child;
    struct page_t child_page;

    if (!page_header(&root_page)->is_leaf) {
        child = page_header(&root_page)->special_page_number;
        header->root_page_number = child;

        CHECK_SUCCESS(load_page(child, &child_page, manager));
        page_header(&child_page)->parent_page_number = INVALID_PAGENUM;
        CHECK_SUCCESS(commit_page(child, &child_page, manager));        
    } else {
        header->root_page_number = INVALID_PAGENUM;
    }

    header->number_of_pages++;
    CHECK_SUCCESS(commit_page(FILE_HEADER_PAGENUM, &file_page, manager));

    CHECK_SUCCESS(page_free(root, manager));
    return SUCCESS;
}

int merge_nodes(struct page_pair_t* left,
                prikey_t k_prime,
                struct page_pair_t* right,
                struct page_pair_t* parent,
                struct file_manager_t* manager)
{
    int i, insertion_index = page_header(left->page)->number_of_keys;
    uint32_t* left_num_key = &page_header(left->page)->number_of_keys;
    int* right_num_key = (int*)&page_header(right->page)->number_of_keys;

    struct page_t temp;
    struct internal_t *left_entries, *right_entries;
    struct record_t *left_records, *right_records;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */
    if (!page_header(left->page)->is_leaf) {
        left_entries = entries(left->page);
        right_entries = entries(right->page);

        for (i = -1; *right_num_key >= 0; ++i, ++insertion_index) {
            if (i == -1) {
                left_entries[insertion_index].key = k_prime;
                left_entries[insertion_index].pagenum =
                    page_header(right->page)->special_page_number;
            } else {
                left_entries[insertion_index] = right_entries[i];
            }

            CHECK_SUCCESS(
                load_page(left_entries[insertion_index].pagenum, &temp, manager));
            page_header(&temp)->parent_page_number = left->pagenum;
            CHECK_SUCCESS(
                commit_page(left_entries[insertion_index].pagenum, &temp, manager));

            *right_num_key -= 1;
            *left_num_key += 1;
        }
    } else {
        left_records = records(left->page);
        right_records = records(right->page);
        for (i = 0; *right_num_key > 0; ++i, ++insertion_index) {
            left_records[insertion_index] = right_records[i];
            *right_num_key -= 1;
            *left_num_key += 1;
        }

        page_header(left->page)->special_page_number =
            page_header(right->page)->special_page_number;
    }

    CHECK_SUCCESS(commit_page(left->pagenum, left->page, manager));
    CHECK_SUCCESS(delete_entry(k_prime, parent, manager));
    CHECK_SUCCESS(page_free(right->pagenum, manager));

    return SUCCESS;
}

int rotate_to_right(struct page_pair_t* left,
                    prikey_t k_prime,
                    int k_prime_index,
                    struct page_pair_t* right,
                    struct page_pair_t* parent,
                    struct file_manager_t* manager)
{
    int i, num_key = page_header(right->page)->number_of_keys;
    struct page_t temp_page;
    struct record_t *right_record, *left_record;
    struct internal_t *right_internal, *left_internal, tmp;

    if (page_header(left->page)->is_leaf) {
        right_record = records(right->page);
        left_record = records(left->page);

        for (i = num_key; i > 0; --i) {
            right_record[i] = right_record[i - 1];
        }

        right_record[0] = left_record[page_header(left->page)->number_of_keys - 1];
        entries(parent->page)[k_prime_index].key = right_record[0].key;
    } else {
        right_internal = entries(right->page);
        left_internal = entries(left->page);

        for (i = num_key; i > 0; --i) {
            right_internal[i] = right_internal[i - 1];
        }

        right_internal[0].key = k_prime;
        right_internal[0].pagenum = page_header(right->page)->special_page_number;

        tmp = left_internal[page_header(left->page)->number_of_keys - 1];
        entries(parent->page)[k_prime_index].key = tmp.key;
        page_header(right->page)->special_page_number = tmp.pagenum;

        CHECK_SUCCESS(load_page(tmp.pagenum, &temp_page, manager));
        page_header(&temp_page)->parent_page_number = right->pagenum;
        CHECK_SUCCESS(commit_page(tmp.pagenum, &temp_page, manager));
    }

    page_header(left->page)->number_of_keys -= 1;
    page_header(right->page)->number_of_keys += 1;

    return SUCCESS;
}

int rotate_to_left(struct page_pair_t* left,
                   prikey_t k_prime,
                   int k_prime_index,
                   struct page_pair_t* right,
                   struct page_pair_t* parent,
                   struct file_manager_t* manager)
{
    int i, num_key = page_header(left->page)->number_of_keys;
    struct page_t temp_page;
    struct record_t *left_record, *right_record;
    struct internal_t* left_internal, *right_internal;

    if (page_header(left->page)->is_leaf) {
        right_record = records(right->page);
        left_record = records(left->page);

        left_record[num_key] = right_record[0];
        entries(parent->page)[k_prime_index].key = right_record[1].key;

        num_key = page_header(right->page)->number_of_keys;
        for (i = 0; i < num_key - 1; ++i) {
            right_record[i] = right_record[i + 1];
        }
    } else {
        right_internal = entries(right->page);
        left_internal = entries(left->page);

        left_internal[num_key].key = k_prime;
        left_internal[num_key].pagenum = page_header(right->page)->special_page_number;

        entries(parent->page)[k_prime_index].key = right_internal[0].key;
        page_header(right->page)->special_page_number = right_internal[0].pagenum;

        CHECK_SUCCESS(
            load_page(left_internal[num_key].pagenum, &temp_page, manager));
        page_header(&temp_page)->parent_page_number = left->pagenum;
        CHECK_SUCCESS(
            commit_page(left_internal[num_key].pagenum, &temp_page, manager));

        num_key = page_header(right->page)->number_of_keys;
        for (i = 0; i < num_key -1; ++i) {
            right_internal[i] = right_internal[i + 1];
        }
    }

    page_header(left->page)->number_of_keys += 1;
    page_header(right->page)->number_of_keys -= 1;

    return SUCCESS;
}

int redistribute_nodes(struct page_pair_t* left,
                       prikey_t k_prime,
                       int k_prime_index,
                       struct page_pair_t* right,
                       struct page_pair_t* parent,
                       struct file_manager_t* manager)
{
    if (page_header(left->page)->number_of_keys 
        < page_header(right->page)->number_of_keys)
    {
        CHECK_SUCCESS(
            rotate_to_left(left, k_prime, k_prime_index, right, parent, manager));
    } else {
        CHECK_SUCCESS(
            rotate_to_right(left, k_prime, k_prime_index, right, parent, manager));
    }

    CHECK_SUCCESS(commit_page(left->pagenum, left->page, manager));
    CHECK_SUCCESS(commit_page(right->pagenum, right->page, manager));
    CHECK_SUCCESS(commit_page(parent->pagenum, parent->page, manager));

    return SUCCESS;
}

int delete_entry(prikey_t key,
                 struct page_pair_t* pair,
                 struct file_manager_t* manager)
{
    struct page_header_t* header = page_header(pair->page);
    if (header->is_leaf) {
        CHECK_SUCCESS(remove_record_from_leaf(key, pair->page));
    } else {
        CHECK_SUCCESS(remove_entry_from_internal(key, pair->page));
    }
    CHECK_SUCCESS(commit_page(pair->pagenum, pair->page, manager));

    /* Case:  deletion from the root. 
     */
    struct page_t file_page;
    CHECK_SUCCESS(load_page(FILE_HEADER_PAGENUM, &file_page, manager));
    if (pair->pagenum == file_header(&file_page)->root_page_number) {
        return shrink_root(manager);
    }

    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */
    int min_keys;
    if (DELAYED_MERGE) {
        min_keys = 1;
    } else {
        min_keys = header->is_leaf ? cut(LEAF_ORDER - 1) : cut(INTERNAL_ORDER) - 1;
    }

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */
    if (header->number_of_keys >= min_keys) {
        return SUCCESS;
    }

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */
    struct page_t parent;
    CHECK_SUCCESS(
        load_page(header->parent_page_number, &parent, manager));

    int index = get_index(&parent, pair->pagenum);
    int k_prime_index = index == -1 ? 0 : index;
    prikey_t k_prime = entries(&parent)[k_prime_index].key;

    struct page_t tmp;
    struct page_pair_t left, right;
    
    right = *pair;
    left.page = &tmp;
    left.pagenum = index == -1
        ? entries(&parent)[0].pagenum
        : index ==  0 ? page_header(&parent)->special_page_number
                      : entries(&parent)[index - 1].pagenum;
    CHECK_SUCCESS(load_page(left.pagenum, left.page, manager));

    if (index == -1) {
        swap_page_pair(&left, &right);
    }

    struct page_pair_t parent_pair = { header->parent_page_number, &parent };

    int capacity = header->is_leaf ? LEAF_ORDER : INTERNAL_ORDER - 1;
    if (page_header(left.page)->number_of_keys
        + page_header(right.page)->number_of_keys < capacity)
    {
        return merge_nodes(&left, k_prime, &right, &parent_pair, manager);
    } else {
        return redistribute_nodes(&left, k_prime, k_prime_index, &right, &parent_pair, manager);
    }
}

int bpt_delete(prikey_t key, struct file_manager_t* manager) {
    pagenum_t leaf;
    struct page_t leaf_page;
    struct page_pair_t pair;

    leaf = find_leaf(key, &leaf_page, manager);
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, &leaf_page, NULL) == SUCCESS)
    {
        pair.pagenum = leaf;
        pair.page = &leaf_page;
        return delete_entry(key, &pair, manager);
    }
    return FAILURE;
}

int destroy_tree(struct file_manager_t* manager) {
    int i;
    pagenum_t pagenum;
    struct page_t page;
    struct queue_t* queue;
    struct page_t file_page;
    CHECK_SUCCESS(load_page(FILE_HEADER_PAGENUM, &file_page, manager));

    pagenum_t root = file_header(&file_page)->root_page_number;
    if (root == INVALID_PAGENUM) {
        return SUCCESS;
    }

    queue = enqueue(NULL, root);
    while (queue != NULL) {
        queue = dequeue(queue, &pagenum);
        CHECK_SUCCESS(load_page(pagenum, &page, manager));

        if (!page_header(&page)->is_leaf) {
            queue = enqueue(queue, page_header(&page)->special_page_number);
            for (i = 0; i < page_header(&page)->number_of_keys; ++i) {
                queue = enqueue(queue, entries(&page)[i].pagenum);
            }
        }

        CHECK_SUCCESS(page_free(pagenum, manager));
    }

    file_header(&file_page)->root_page_number = INVALID_PAGENUM;
    CHECK_SUCCESS(commit_page(FILE_HEADER_PAGENUM, &file_page, manager));
    return SUCCESS;
}
