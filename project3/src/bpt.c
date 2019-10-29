#define Version "1.0.on-disk"
/*  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved, reference 3RD-PARTY.md.
 */

#include <stdlib.h>
#include <string.h>

#include "bpt.h"
#include "utility.h"

void swap_ubuffer(struct ubuffer_t* left, struct ubuffer_t* right) {
    struct ubuffer_t tmp = *left;
    *left = *right;
    *right = tmp;
}

struct ubuffer_t bpt_buffering(struct bpt_t* bpt, pagenum_t pagenum) {
    return buffer_manager_buffering(bpt->buffers, bpt->file, pagenum);
}

struct ubuffer_t bpt_create_page(struct bpt_t* bpt) {
    return buffer_manager_new_page(bpt->buffers, bpt->file);
}

int bpt_free_page(struct bpt_t* bpt, pagenum_t pagenum) {
    return buffer_manager_free_page(bpt->buffers, bpt->file, pagenum);
}

void usage_1(struct bpt_t* bpt) {
    printf("B+ Tree of leaf order %d, internal order %d.\n", bpt->leaf_order, bpt->internal_order);
    printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
           "5th ed.\n\n");
}

void usage_2(struct bpt_t* bpt) {
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

int bpt_init(struct bpt_t* config,
             struct file_manager_t* file,
             struct buffer_manager_t* buffers)
{
    bpt_default_config(config);
    config->file = file;
    config->buffers = buffers;
    return SUCCESS;
}

int bpt_release(struct bpt_t* config) {
    memset(config, 0, sizeof(struct bpt_t));
    return SUCCESS;
}

int bpt_default_config(struct bpt_t* config) {
    config->leaf_order = 32;
    config->internal_order = 249;
    config->verbose_output = FALSE;
    config->delayed_merge = TRUE;
    return SUCCESS;
}

int bpt_test_config(struct bpt_t* config,
                    int leaf_order,
                    int internal_order)
{
    config->leaf_order = leaf_order;
    config->internal_order = internal_order;
    config->verbose_output = TRUE;
    config->delayed_merge = TRUE;
    return SUCCESS;
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

int record_vec_init(struct record_vec_t* vec, int cap) {
    vec->size = 0;
    vec->capacity = cap;
    vec->rec = malloc(sizeof(struct record_t) * cap);
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

int height(struct bpt_t* bpt, pagenum_t pagenum) {
    int h, is_leaf;
    struct ubuffer_t buffer;

    for (h = 0, is_leaf = FALSE; !is_leaf; ++h) {
        buffer = bpt_buffering(bpt, pagenum);
        BUFFER(buffer, READ_FLAG, {
            is_leaf = page_header(from_ubuffer(&buffer))->is_leaf;
            pagenum = page_header(from_ubuffer(&buffer))->special_page_number;
        })
    }

    return h;
}

int path_to_root(struct bpt_t* bpt, pagenum_t pagenum) {
    int length;
    pagenum_t root;
    struct ubuffer_t buffer;
    buffer = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(buffer, READ_FLAG, {
        root = file_header(from_ubuffer(&buffer))->root_page_number;
    })

    for (length = 0; root != pagenum; ++length) {
        buffer = bpt_buffering(bpt, pagenum);
        BUFFER(buffer, READ_FLAG, {
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

pagenum_t find_leaf(struct bpt_t* bpt,
                    prikey_t key,
                    struct ubuffer_t* buffer)
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

    *buffer = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(*buffer, READ_FLAG, {
        root = file_header(from_ubuffer(buffer))->root_page_number;
    })

    if (root == INVALID_PAGENUM) {
        if (bpt->verbose_output) {
            printf("Empty tree.\n");
        }
        return root;
    }

    c = root;
    while (TRUE) {
        *buffer = bpt_buffering(bpt, c);
        BUFFER(*buffer, READ_FLAG, {
            ent = entries(from_ubuffer(buffer));
            header = page_header(from_ubuffer(buffer));

            if (header->is_leaf) {
                BUFFER_INTERCEPT(*buffer, READ_FLAG, break);
            }

            if (bpt->verbose_output) {
                printf("[");
                for (i = 0; i < header->number_of_keys - 1; ++i) {
                    printf("%ld ", ent[i].key);
                }
                printf("%ld] ", ent[i].key);
            }

            for (i = 0; i < header->number_of_keys && key >= ent[i].key; ++i)
                {}
            
            --i;
            if (bpt->verbose_output) {
                printf("%d ->\n", i);
            }

            if (i < 0) {
                c = header->special_page_number;
            } else {
                c = ent[i].pagenum;
            }
        })
    }

    if (bpt->verbose_output) {
        BUFFER(*buffer, READ_FLAG, {
            header = page_header(from_ubuffer(buffer));
            rec = records(from_ubuffer(buffer));

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
    BUFFER(buffer, READ_FLAG, {
        page = from_ubuffer(&buffer);
        if (!page_header(page)->is_leaf) {
            BUFFER_INTERCEPT(buffer, READ_FLAG, return FAILURE);
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

int bpt_find(struct bpt_t* bpt,
             prikey_t key,
             struct record_t* record)
{
    int i = 0;
    struct ubuffer_t buffer;
    pagenum_t c = find_leaf(bpt, key, &buffer);
    if (c == INVALID_PAGENUM) {
        return FAILURE;
    }

    return find_key_from_leaf(key, buffer, record);
}

int bpt_find_range(struct bpt_t* bpt,
                   prikey_t start,
                   prikey_t end,
                   struct record_vec_t* retval)
{
    int i, num_key;
    struct record_t* rec;
    struct ubuffer_t buffer, tmp;
    pagenum_t n = find_leaf(bpt, start, &buffer);
    if (n == INVALID_PAGENUM) {
        return FAILURE;
    }

    BUFFER(buffer, READ_FLAG, {
        rec = records(from_ubuffer(&buffer));
        num_key = page_header(from_ubuffer(&buffer))->number_of_keys;
        for (i = 0; i < num_key && rec[i].key < start; ++i)
            {}
    })

    if (i == num_key) {
        return 0;
    }

    while (TRUE) {
        BUFFER(buffer, READ_FLAG, {
            rec = records(from_ubuffer(&buffer));
            num_key = page_header(from_ubuffer(&buffer))->number_of_keys;

            for (; i < num_key && rec[i].key <= end; i++) {
                BUFFER_CHECK_SUCCESS(buffer, READ_FLAG, record_vec_append(retval, &rec[i]));
            }

            n = page_header(from_ubuffer(&buffer))->special_page_number;
            if ((i < num_key && rec[i].key > end) || n == INVALID_PAGENUM) {
                BUFFER_INTERCEPT(buffer, READ_FLAG, break);
            }

            tmp = bpt_buffering(bpt, n);
            BUFFER_CHECK_NULL(buffer, READ_FLAG, tmp.buf);
        })
        i = 0;
        buffer = tmp;
    }

    return retval->size;
}


// Output.

void print_leaves(struct bpt_t* bpt) {
    int i, is_leaf;
    pagenum_t pagenum, root;
    struct ubuffer_t buffer = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(buffer, READ_FLAG, {
        root = file_header(from_ubuffer(&buffer))->root_page_number;
    })

    if (root == INVALID_PAGENUM) {
        printf("Empty tree.\n");
        return;
    }

    is_leaf = FALSE;
    pagenum = root;
    while (!is_leaf) {
        buffer = bpt_buffering(bpt, pagenum);
        BUFFER(buffer, READ_FLAG, {
            is_leaf = page_header(from_ubuffer(&buffer))->is_leaf;
            pagenum = page_header(from_ubuffer(&buffer))->special_page_number;
        })
    }

    int num_key;
    struct record_t* rec;
    while (1) {
        BUFFER(buffer, READ_FLAG, {
            rec = records(from_ubuffer(&buffer));
            num_key = page_header(from_ubuffer(&buffer))->number_of_keys;
            pagenum = page_header(from_ubuffer(&buffer))->special_page_number;

            for (i = 0; i < num_key; ++i) {
                printf("%ld ", rec[i].key);
                if (bpt->verbose_output) {
                    printf("{v: %s} ", rec[i].value);
                }
            }
        })

        if (bpt->verbose_output) {
            printf("(next %lu) ", pagenum);
        }

        if (pagenum != INVALID_PAGENUM) {
            printf(" | ");
            buffer = bpt_buffering(bpt, pagenum);
        } else {
            break;
        }
    }
    printf("\n");
}

void print_tree(struct bpt_t* bpt) {
    pagenum_t root, n;
    int i, new_rank, rank = 0;
    struct page_header_t* pheader;

    struct record_t* rec;
    struct internal_t* ent;

    struct ubuffer_t buffer, tmp;
    buffer = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(buffer, READ_FLAG, {
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

        buffer = bpt_buffering(bpt, n);
        BUFFER(buffer, READ_FLAG, {
            pheader = page_header(from_ubuffer(&buffer));
            if (pheader->parent_page_number != INVALID_PAGENUM) {
                tmp = bpt_buffering(
                    bpt,
                    pheader->parent_page_number);
                BUFFER(tmp, READ_FLAG, {
                    if (n == page_header(from_ubuffer(&tmp))->special_page_number) {
                        new_rank = path_to_root(bpt, n);
                        if (new_rank != rank) {
                            rank = new_rank;
                            printf("\n");
                        }
                    }
                })
            }

            if (bpt->verbose_output) {
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
                    if (bpt->verbose_output) {
                        printf("{v: %s} ", rec[i].value);
                    }
                } else {
                    printf("%ld ", ent[i].key);
                    if (bpt->verbose_output) {
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

            if (bpt->verbose_output && pheader->is_leaf) {
                printf("(parent %lu, next %lu) ",
                    pheader->parent_page_number, pheader->special_page_number);
            }
            printf("| ");
        })
    }
    printf("\n");
}

void find_and_print(struct bpt_t* bpt, prikey_t key) {
    struct record_t r;
    int retval = bpt_find(bpt, key, &r);
    if (retval == FAILURE) {
        printf("Record not found under key %ld.\n", key);
    } else {
        printf("Record -- key %ld, value %s.\n", key, r.value);
    }
}

void find_and_print_range(struct bpt_t* bpt, prikey_t range1, prikey_t range2) {
    int i;
    struct record_vec_t retval;
    EXIT_ON_FAILURE(record_vec_init(&retval, 4));

    bpt_find_range(bpt, range1, range2, &retval);
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

struct ubuffer_t make_node(struct bpt_t* bpt, uint32_t leaf) {
    struct ubuffer_t buffer = bpt_create_page(bpt);
    if (buffer.buf == NULL) {
        return buffer;
    }

    struct page_t* page = from_ubuffer(&buffer);
    BUFFER(buffer, WRITE_FLAG, {
        EXIT_ON_FAILURE(page_init(page, leaf));
    })

    return buffer;
}

int get_index(struct ubuffer_t* parent, pagenum_t pagenum) {
    int index, num_key;
    struct page_t* page;
    BUFFER(*parent, READ_FLAG, {
        page = from_ubuffer(parent);
        if (page_header(page)->special_page_number == pagenum) {
            return -1;
        }

        num_key = page_header(page)->number_of_keys;
        for (index = 0;
             index < num_key && entries(page)[index].pagenum != pagenum;
             ++index)
            {}
    })
    return index;
}

int insert_into_leaf(struct ubuffer_t* leaf,
                     struct record_t* pointer)
{
    int i, insertion_point;
    int num_key; 
    struct record_t* rec; 
    BUFFER(*leaf, WRITE_FLAG, {
        num_key = page_header(from_ubuffer(leaf))->number_of_keys;
        rec = records(from_ubuffer(leaf));

        for (insertion_point = 0;
            insertion_point < num_key && rec[insertion_point].key < pointer->key;
            ++insertion_point)
            {}

        for (i = num_key; i > insertion_point; --i) {
            memcpy(&rec[i], &rec[i - 1], sizeof(struct record_t));
        }

        page_header(from_ubuffer(leaf))->number_of_keys += 1;
        memcpy(&rec[insertion_point], pointer, sizeof(struct record_t));
    })
    return SUCCESS;
}

int insert_into_leaf_after_splitting(struct bpt_t* bpt,
                                     struct ubuffer_t* leaf,
                                     struct record_t* record)
{
    prikey_t key;
    int insertion_index, split_index, i, j;
    pagenum_t next_node, parent_node;
    struct record_t *leaf_rec, *new_rec;
    struct record_t temp_record[bpt->leaf_order];
    struct page_header_t *leaf_header, *new_header;
    struct ubuffer_t new_page = make_node(bpt, TRUE);
    BUFFER(*leaf, READ_FLAG, {
        leaf_rec = records(from_ubuffer(leaf));
        leaf_header = page_header(from_ubuffer(leaf));

        next_node = leaf_header->special_page_number;
        parent_node = leaf_header->parent_page_number;

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
    })
 
    BUFFER(*leaf, WRITE_FLAG, {
        leaf_rec = records(from_ubuffer(leaf));
        leaf_header = page_header(from_ubuffer(leaf));

        leaf_header->number_of_keys = 0;
        split_index = cut(bpt->leaf_order - 1);

        for (i = 0; i < split_index; i++) {
            memcpy(&leaf_rec[i], &temp_record[i], sizeof(struct record_t));
            leaf_header->number_of_keys++;
        }

        leaf_header->special_page_number = new_page.buf->pagenum;
    })

    BUFFER(new_page, WRITE_FLAG, {
        new_rec = records(from_ubuffer(&new_page));
        new_header = page_header(from_ubuffer(&new_page));

        for (i = split_index, j = 0; i < bpt->leaf_order; i++, j++) {
            memcpy(&new_rec[j], &temp_record[i], sizeof(struct record_t));
            new_header->number_of_keys++;
        }

        new_header->special_page_number = next_node;
        new_header->parent_page_number = parent_node;

        key = new_rec[0].key;
    })

    return insert_into_parent(bpt, leaf, key, &new_page);
}

int insert_into_node(struct ubuffer_t* node,
                     int index,
                     struct internal_t* entry)
{
    int i;
    struct internal_t* ent;
    struct page_header_t* header;
    BUFFER(*node, WRITE_FLAG, {
        ent = entries(from_ubuffer(node));
        header = page_header(from_ubuffer(node));
        for (i = header->number_of_keys; i > index; --i) {
            ent[i] = ent[i - 1];
        }
        ent[index] = *entry;
        header->number_of_keys++;
    })
    return SUCCESS;
}

int insert_into_node_after_splitting(struct bpt_t* bpt,
                                     struct ubuffer_t* old_node,
                                     int index,
                                     struct internal_t* entry)
{
    int i, j, split_index, k_prime;
    pagenum_t temp_pagenum, parent_num;
    struct internal_t *ent, *new_entries;
    struct internal_t temp[bpt->internal_order];
    struct page_header_t *header, *new_header;
    struct ubuffer_t temp_page, new_node = make_node(bpt, FALSE);

    BUFFER(*old_node, READ_FLAG, {
        ent = entries(from_ubuffer(old_node));
        header = page_header(from_ubuffer(old_node));

        parent_num = header->parent_page_number;
        for (i = 0, j = 0; j < header->number_of_keys; ++i, ++j) {
            if (i == index) {
                i++;
            }
            temp[i] = ent[j];
        }
        temp[index] = *entry;
    })

    split_index = cut(bpt->internal_order);
    BUFFER(*old_node, WRITE_FLAG, {
        ent = entries(from_ubuffer(old_node));
        header = page_header(from_ubuffer(old_node));

        header->number_of_keys = 0;
        for (i = 0; i < split_index - 1; ++i) {
            ent[i] = temp[i];
            header->number_of_keys++;
        }
    })

    BUFFER(new_node, WRITE_FLAG, {
        new_entries = entries(from_ubuffer(&new_node));
        new_header = page_header(from_ubuffer(&new_node));

        k_prime = temp[split_index - 1].key;
        new_header->special_page_number = temp[split_index - 1].pagenum;
        for (++i, j = 0; i < bpt->internal_order; ++i, ++j) {
            new_entries[j] = temp[i];
            new_header->number_of_keys++;
        }

        new_header->parent_page_number = parent_num;
    })

    BUFFER(new_node, READ_FLAG, {
        new_entries = entries(from_ubuffer(&new_node));
        new_header = page_header(from_ubuffer(&new_node));

        for (i = -1; i < (int)new_header->number_of_keys; ++i) {
            if (i == -1) {
                temp_pagenum = new_header->special_page_number;
            } else {
                temp_pagenum = new_entries[i].pagenum;
            }

            temp_page = bpt_buffering(bpt, temp_pagenum);
            BUFFER(temp_page, WRITE_FLAG, {
                page_header(from_ubuffer(&temp_page))->parent_page_number = new_node.buf->pagenum;
            })
        }
    })

    return insert_into_parent(bpt, old_node, k_prime, &new_node);
}

int insert_into_parent(struct bpt_t* bpt,
                       struct ubuffer_t* left,
                       prikey_t key,
                       struct ubuffer_t* right)
{
    int index, num_key;
    struct ubuffer_t parent_page;
    pagenum_t parent, left_pagenum, right_pagenum;
    BUFFER(*left, READ_FLAG, {
        left_pagenum = left->buf->pagenum;
        parent = page_header(from_ubuffer(left))->parent_page_number;
    })
 
    /* Case: new root. */
    if (parent == INVALID_PAGENUM) {
        return insert_into_new_root(bpt, left, key, right);
    }

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */
    parent_page = bpt_buffering(bpt, parent);
    index = get_index(&parent_page, left_pagenum) + 1;

    /* Simple case: the new key fits into the node. 
     */
    BUFFER(*right, READ_FLAG, {
        right_pagenum = right->buf->pagenum;
        num_key = page_header(from_ubuffer(&parent_page))->number_of_keys;
    })

    struct internal_t entry = { key, right_pagenum };
    if (num_key < bpt->internal_order - 1) {
        return insert_into_node(&parent_page, index, &entry);
    }

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */
    return insert_into_node_after_splitting(bpt, &parent_page, index, &entry);
}

int insert_into_new_root(struct bpt_t* bpt,
                         struct ubuffer_t* left,
                         prikey_t key,
                         struct ubuffer_t* right)
{
    pagenum_t root_pagenum;
    struct internal_t* ent;
    struct page_header_t* header;
    struct ubuffer_t fileheader, root = make_node(bpt, FALSE);

    BUFFER(root, WRITE_FLAG, {
        root_pagenum = root.buf->pagenum;

        header = page_header(from_ubuffer(&root));
        header->number_of_keys++;

        BUFFER(*left, READ_FLAG, {
            header->special_page_number = left->buf->pagenum;
        })

        ent = entries(from_ubuffer(&root));
        ent[0].key = key;
        BUFFER(*right, READ_FLAG, {
            ent[0].pagenum = right->buf->pagenum;
        })
    })

    BUFFER(*left, WRITE_FLAG, {
        page_header(from_ubuffer(left))->parent_page_number = root_pagenum;
    })

    BUFFER(*right, WRITE_FLAG, {
        page_header(from_ubuffer(right))->parent_page_number = root_pagenum;
    })

    fileheader = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(fileheader, WRITE_FLAG, {
        file_header(from_ubuffer(&fileheader))->root_page_number = root_pagenum;
    })

    return SUCCESS;
}

int start_new_tree(struct bpt_t* bpt, struct record_t* pointer)
{
    pagenum_t root_pagenum;
    struct record_t* rec;
    struct page_header_t* header;
    struct ubuffer_t fileheader, root = make_node(bpt, TRUE);

    BUFFER(root, WRITE_FLAG, {
        root_pagenum = root.buf->pagenum;

        header = page_header(from_ubuffer(&root));
        header->number_of_keys++;

        rec = &records(from_ubuffer(&root))[0];
        memcpy(rec, pointer, sizeof(struct record_t));
    })

    fileheader = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(fileheader, WRITE_FLAG, {
        file_header(from_ubuffer(&fileheader))->root_page_number = root_pagenum;
    })

    return SUCCESS;
}

int bpt_insert(struct bpt_t* bpt,
               prikey_t key,
               uint8_t* value,
               int value_size)
{
    struct ubuffer_t leaf_page;
    pagenum_t leaf = find_leaf(bpt, key, &leaf_page);

    /* The current implementation ignores
     * duplicates.
     */
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, leaf_page, NULL) == SUCCESS)
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
    struct ubuffer_t file_page = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    
    pagenum_t root;
    BUFFER(file_page, READ_FLAG, {
        root = file_header(from_ubuffer(&file_page))->root_page_number;
    })

    if (root == INVALID_PAGENUM) {
        return start_new_tree(bpt, &record);
    }

    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    /* Case: leaf has room for key and pointer.
     */
    int num_key;
    BUFFER(leaf_page, READ_FLAG, {
        num_key = page_header(from_ubuffer(&leaf_page))->number_of_keys;
    })

    if (num_key < bpt->leaf_order - 1) {
        return insert_into_leaf(&leaf_page, &record);
    }

    /* Case:  leaf must be split.
     */
    return insert_into_leaf_after_splitting(bpt, &leaf_page, &record);
}


// DELETION.

int remove_record_from_leaf(prikey_t key, struct ubuffer_t* node) {
    int i, num_key;
    struct record_t* rec;
    BUFFER(*node, WRITE_FLAG, {
        rec = records(from_ubuffer(node));
        num_key = page_header(from_ubuffer(node))->number_of_keys;
        for (i = 0; i < num_key && rec[i].key != key; ++i)
            {}

        if (i == num_key) {
            return FAILURE;
        }

        for (++i; i < num_key; ++i) {
            rec[i - 1] = rec[i];
        }

        --page_header(from_ubuffer(node))->number_of_keys;
    })
    return SUCCESS;
}

int remove_entry_from_internal(prikey_t key, struct ubuffer_t* node) {
    int i, num_key;
    struct internal_t* ent;
    BUFFER(*node, WRITE_FLAG, {
        ent = entries(from_ubuffer(node));
        num_key = page_header(from_ubuffer(node))->number_of_keys;
        for (i = 0; i < num_key && ent[i].key != key; ++i)
            {}
        
        if (i == num_key) {
            return FAILURE;
        }

        for (++i; i < num_key; ++i) {
            ent[i - 1] = ent[i];
        }

        --page_header(from_ubuffer(node))->number_of_keys;
    })    
    return SUCCESS;
}

int shrink_root(struct bpt_t* bpt) {
    pagenum_t root;
    struct ubuffer_t file_page = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(file_page, READ_FLAG, {
        root = file_header(from_ubuffer(&file_page))->root_page_number;
    })

    int num_key;
    struct ubuffer_t root_page = bpt_buffering(bpt, root);
    BUFFER(root_page, READ_FLAG, {
        num_key = page_header(from_ubuffer(&root_page))->number_of_keys;
    })

    if (num_key > 0) {
        return SUCCESS;
    }

    struct ubuffer_t child;
    pagenum_t child_num = INVALID_PAGENUM;
    BUFFER(root_page, READ_FLAG, {
        if (!page_header(from_ubuffer(&root_page))->is_leaf) {
            child_num = page_header(from_ubuffer(&root_page))->special_page_number;
            child = bpt_buffering(bpt, child_num);
            BUFFER(child, WRITE_FLAG, {
                page_header(from_ubuffer(&child))->parent_page_number = INVALID_PAGENUM;
            })
        }
    })

    BUFFER(file_page, WRITE_FLAG, {
        file_header(from_ubuffer(&file_page))->root_page_number = child_num;
    })

    CHECK_SUCCESS(bpt_free_page(bpt, root));
    return SUCCESS;
}

int merge_nodes(struct bpt_t* bpt,
                struct ubuffer_t* left,
                prikey_t k_prime,
                struct ubuffer_t* right,
                struct ubuffer_t* parent)
{
    pagenum_t pagenum;
    int i, is_leaf, insertion_index;
    int *left_num_key, *right_num_key;

    struct ubuffer_t temp;
    struct internal_t *left_entries, *right_entries;
    struct record_t *left_records, *right_records;

    BUFFER(*left, READ_FLAG, {
        is_leaf = page_header(from_ubuffer(left))->is_leaf;
        insertion_index = page_header(from_ubuffer(left))->number_of_keys;
    })

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */
    if (!is_leaf) {
        BUFFER(*left, WRITE_FLAG, {
            left_entries = entries(from_ubuffer(left));
            left_num_key = (int*)&page_header(from_ubuffer(left))->number_of_keys;
            BUFFER(*right, READ_FLAG, {
                right_entries = entries(from_ubuffer(right));
                right_num_key = (int*)&page_header(from_ubuffer(right))->number_of_keys;

                for (i = -1; *right_num_key >= 0; ++i, ++insertion_index) {
                    if (i == -1) {
                        left_entries[insertion_index].key = k_prime;
                        left_entries[insertion_index].pagenum =
                            page_header(from_ubuffer(right))->special_page_number;
                    } else {
                        left_entries[insertion_index] = right_entries[i];
                    }

                    pagenum = left_entries[insertion_index].pagenum;
                    temp = bpt_buffering(bpt, pagenum);
                    BUFFER(temp, WRITE_FLAG, {
                        page_header(from_ubuffer(&temp))->parent_page_number =
                            left->buf->pagenum;
                    })

                    *right_num_key -= 1;
                    *left_num_key += 1;
                }
            })
        })
    } else {
        BUFFER(*left, WRITE_FLAG, {
            left_records = records(from_ubuffer(left));
            left_num_key = (int*)&page_header(from_ubuffer(left))->number_of_keys;
            BUFFER(*right, READ_FLAG, {
                right_records = records(from_ubuffer(right));
                right_num_key = (int*)&page_header(from_ubuffer(right))->number_of_keys;
                for (i = 0; *right_num_key > 0; ++i, ++insertion_index) {
                    left_records[insertion_index] = right_records[i];
                    *right_num_key -= 1;
                    *left_num_key += 1;
                }

                page_header(from_ubuffer(left))->special_page_number =
                    page_header(from_ubuffer(right))->special_page_number;
            })
        })
    }

    CHECK_SUCCESS(delete_entry(bpt, k_prime, parent));
    BUFFER(*right, READ_FLAG, {
        pagenum = right->buf->pagenum;
    })
    bpt_free_page(bpt, pagenum);

    return SUCCESS;
}

int rotate_to_right(struct bpt_t* bpt,
                    struct ubuffer_t* left,
                    prikey_t k_prime,
                    int k_prime_index,
                    struct ubuffer_t* right,
                    struct ubuffer_t* parent)
{
    int i, num_key, is_leaf;
    struct ubuffer_t temp_page;
    struct record_t *right_record, *left_record;
    struct internal_t *right_internal, *left_internal, tmp;

    BUFFER(*right, READ_FLAG, {
        num_key = page_header(from_ubuffer(right))->number_of_keys;
    })
    BUFFER(*left, READ_FLAG, {
        is_leaf = page_header(from_ubuffer(left))->is_leaf;
    })

    if (is_leaf) {
        BUFFER(*right, WRITE_FLAG, {
            right_record = records(from_ubuffer(right));
            for (i = num_key; i > 0; --i) {
                right_record[i] = right_record[i - 1];
            }
            BUFFER(*left, READ_FLAG, {
                left_record = records(from_ubuffer(left));
                num_key = page_header(from_ubuffer(left))->number_of_keys;
                right_record[0] = left_record[num_key - 1];
            })
            BUFFER(*parent, WRITE_FLAG, {
                entries(from_ubuffer(parent))[k_prime_index].key =
                    right_record[0].key;
            })
        })
    } else {
        BUFFER(*right, WRITE_FLAG, {
            right_internal = entries(from_ubuffer(right));
            for (i = num_key; i > 0; --i) {
                right_internal[i] = right_internal[i - 1];
            }
            right_internal[0].key = k_prime;
            right_internal[0].pagenum =
                page_header(from_ubuffer(right))->special_page_number;
        })
        BUFFER(*left, READ_FLAG, {
            left_internal = entries(from_ubuffer(left));
            num_key = page_header(from_ubuffer(left))->number_of_keys;

            tmp = left_internal[num_key - 1];
            temp_page = bpt_buffering(bpt, tmp.pagenum);
            BUFFER(*parent, WRITE_FLAG, {
                entries(from_ubuffer(parent))[k_prime_index].key = tmp.key;
            })
            BUFFER(*right, WRITE_FLAG, {
                page_header(from_ubuffer(right))->special_page_number = tmp.pagenum;
                page_header(from_ubuffer(&temp_page))->parent_page_number =
                    right->buf->pagenum;
            })
        })
    }

    BUFFER(*left, WRITE_FLAG, {
        page_header(from_ubuffer(left))->number_of_keys -= 1;
    })
    BUFFER(*right, WRITE_FLAG, {
        page_header(from_ubuffer(right))->number_of_keys += 1;
    })

    return SUCCESS;
}

int rotate_to_left(struct bpt_t* bpt,
                   struct ubuffer_t* left,
                   prikey_t k_prime,
                   int k_prime_index,
                   struct ubuffer_t* right,
                   struct ubuffer_t* parent)
{
    int i, num_key, is_leaf;
    pagenum_t left_pagenum, child_pagenum;
    struct ubuffer_t tmp;
    struct record_t *left_record, *right_record;
    struct internal_t* left_internal, *right_internal;

    BUFFER(*left, READ_FLAG, {
        is_leaf = page_header(from_ubuffer(left))->is_leaf;
        num_key = page_header(from_ubuffer(left))->number_of_keys;
    })

    if (is_leaf) {
        BUFFER(*right, READ_FLAG, {
            right_record = records(from_ubuffer(right));
            BUFFER(*left, WRITE_FLAG, {
                left_record = records(from_ubuffer(left));
                left_record[num_key] = right_record[0];
            })
            BUFFER(*parent, WRITE_FLAG, {
                entries(from_ubuffer(parent))[k_prime_index].key = right_record[1].key;
            })
        })

        BUFFER(*right, WRITE_FLAG, {
            right_record = records(from_ubuffer(right));
            num_key = page_header(from_ubuffer(right))->number_of_keys;
            for (i = 0; i < num_key - 1; ++i) {
                right_record[i] = right_record[i + 1];
            }
        })
    } else {
        BUFFER(*right, READ_FLAG, {
            right_internal = entries(from_ubuffer(right));
            BUFFER(*left, WRITE_FLAG, {
                left_internal = entries(from_ubuffer(left));
                left_internal[num_key].key = k_prime;
                left_internal[num_key].pagenum =
                    page_header(from_ubuffer(right))->special_page_number;
                
                left_pagenum = left->buf->pagenum;
                child_pagenum = left_internal[num_key].pagenum;
            })

            tmp = bpt_buffering(bpt, child_pagenum);
            BUFFER(tmp, WRITE_FLAG, {
                page_header(from_ubuffer(&tmp))->parent_page_number = left->buf->pagenum;
            })

            BUFFER(*parent, WRITE_FLAG, {
                entries(from_ubuffer(parent))[k_prime_index].key = right_internal[0].key;
            })
        })

        BUFFER(*right, WRITE_FLAG, {
            right_internal = entries(from_ubuffer(right));
            page_header(from_ubuffer(right))->special_page_number
                = right_internal[0].pagenum;
            
            num_key = page_header(from_ubuffer(right))->number_of_keys;
            for (i = 0; i < num_key - 1; ++i) {
                right_internal[i] = right_internal[i + 1];
            }
        })
    }

    BUFFER(*right, WRITE_FLAG, {
        page_header(from_ubuffer(right))->number_of_keys -= 1;
    })
    BUFFER(*left, WRITE_FLAG, {
        page_header(from_ubuffer(left))->number_of_keys += 1;
    })

    return SUCCESS;
}

int redistribute_nodes(struct bpt_t* bpt,
                       struct ubuffer_t* left,
                       prikey_t k_prime,
                       int k_prime_index,
                       struct ubuffer_t* right,
                       struct ubuffer_t* parent)
{
    int left_num_key, right_num_key;
    BUFFER(*left, READ_FLAG, {
        left_num_key = page_header(from_ubuffer(left))->number_of_keys;
    })
    BUFFER(*right, READ_FLAG, {
        right_num_key = page_header(from_ubuffer(right))->number_of_keys;
    })

    if (left_num_key < right_num_key) {
        CHECK_SUCCESS(
            rotate_to_left(bpt, left, k_prime, k_prime_index, right, parent));
    } else {
        CHECK_SUCCESS(
            rotate_to_right(bpt, left, k_prime, k_prime_index, right, parent));
    }
    return SUCCESS;
}

int delete_entry(struct bpt_t* bpt,
                 prikey_t key,
                 struct ubuffer_t* page)
{
    int is_leaf, num_key;
    struct page_header_t* header;
    pagenum_t pagenum, parent_num, root_num;
    BUFFER(*page, READ_FLAG, {
        pagenum = page->buf->pagenum;
        header = page_header(from_ubuffer(page));
        is_leaf = header->is_leaf;
        num_key = header->number_of_keys;
        parent_num = header->parent_page_number;
    })

    if (is_leaf) {
        CHECK_SUCCESS(remove_record_from_leaf(key, page));
    } else {
        CHECK_SUCCESS(remove_entry_from_internal(key, page));
    }

    /* Case:  deletion from the root. 
     */
    struct ubuffer_t file_page = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(file_page, READ_FLAG, {
        root_num = file_header(from_ubuffer(&file_page))->root_page_number;
    })

    if (pagenum == root_num) {
        return shrink_root(bpt);
    }

    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */
    int min_keys;
    if (bpt->delayed_merge) {
        min_keys = 1;
    } else {
        min_keys = is_leaf ? cut(bpt->leaf_order - 1) : cut(bpt->internal_order) - 1;
    }

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */
    if (num_key >= min_keys) {
        return SUCCESS;
    }

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */
    struct ubuffer_t parent = bpt_buffering(bpt, parent_num);

    int index = get_index(&parent, pagenum);
    int k_prime_index = index == -1 ? 0 : index;

    prikey_t k_prime;
    BUFFER(parent, READ_FLAG, {
        k_prime = entries(from_ubuffer(&parent))[k_prime_index].key;
    })

    pagenum_t left_num;
    struct ubuffer_t left, right;

    right = *page;
    left_num = index == -1
        ? entries(from_ubuffer(&parent))[0].pagenum
        : index ==  0 ? page_header(from_ubuffer(&parent))->special_page_number
                      : entries(from_ubuffer(&parent))[index - 1].pagenum;
    left = bpt_buffering(bpt, left_num);

    if (index == -1) {
        swap_ubuffer(&left, &right);
    }

    int right_num_key, left_num_key;
    BUFFER(left, READ_FLAG, {
        left_num_key = page_header(from_ubuffer(&left))->number_of_keys;
    })
    BUFFER(right, READ_FLAG, {
        right_num_key = page_header(from_ubuffer(&right))->number_of_keys;
    })

    int capacity = is_leaf ? bpt->leaf_order : bpt->internal_order - 1;
    if (left_num_key + right_num_key < capacity) {
        return merge_nodes(bpt, &left, k_prime, &right, &parent);
    } else {
        return redistribute_nodes(bpt, &left, k_prime, k_prime_index, &right, &parent);
    }
}

int bpt_delete(struct bpt_t* bpt, prikey_t key) {
    pagenum_t leaf;
    struct ubuffer_t leaf_page;

    leaf = find_leaf(bpt, key, &leaf_page);
    if (leaf != INVALID_PAGENUM
        && find_key_from_leaf(key, leaf_page, NULL) == SUCCESS)
    {
        return delete_entry(bpt, key, &leaf_page);
    }
    return FAILURE;
}

int destroy_tree(struct bpt_t* bpt) {
    int i;
    pagenum_t pagenum, root;
    struct page_t* page_ptr;
    struct queue_t* queue;
    struct ubuffer_t page = bpt_buffering(bpt, FILE_HEADER_PAGENUM);
    BUFFER(page, WRITE_FLAG, {
        page_ptr = from_ubuffer(&page);
        root = file_header(page_ptr)->root_page_number;
        file_header(page_ptr)->root_page_number = INVALID_PAGENUM;
    })

    if (root == INVALID_PAGENUM) {
        return SUCCESS;
    }

    queue = enqueue(NULL, root);
    while (queue != NULL) {
        queue = dequeue(queue, &pagenum);
        page = bpt_buffering(bpt, pagenum);
        BUFFER(page, READ_FLAG, {
            page_ptr = from_ubuffer(&page);
            if (!page_header(page_ptr)->is_leaf) {
                queue = enqueue(
                    queue,
                    page_header(page_ptr)->special_page_number);
                for (i = 0; i < page_header(page_ptr)->number_of_keys; ++i) {
                    queue = enqueue(queue, entries(page_ptr)[i].pagenum);
                }
            }
        })
        CHECK_SUCCESS(bpt_free_page(bpt, pagenum));
    }
    return SUCCESS;
}
