#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>
#include <pthread.h>

// Default order of the B+ Tree
#define DEFAULT_ORDER 4

// Forward declarations
typedef struct Node Node;
typedef struct BPlusTree BPlusTree;

// Function pointer for comparing keys.
// Returns:
//   < 0 if key1 < key2
//   = 0 if key1 == key2
//   > 0 if key1 > key2
typedef int (*key_comparator)(const void* key1, const void* key2);

// Function pointer for destroying user-provided value data.
typedef void (*value_destroyer)(void* value);

// Structure for a record (stores a pointer to the value)
typedef struct Record {
    void *value;
} Record;

// Structure for a B+ Tree node
struct Node {
    void **pointers;
    void **keys;
    Node *parent;
    bool is_leaf;
    int num_keys;
    Node *next; // Used for linking leaf nodes
    pthread_rwlock_t lock; // Read-Write lock for this node
};

// Structure for the B+ Tree itself
struct BPlusTree {
    Node *root;
    int order;
    key_comparator compare;
    value_destroyer destroy_value;
};

// Public API functions
BPlusTree *bplus_tree_create(int order, key_comparator compare, value_destroyer destroy_value);
int bplus_tree_insert(BPlusTree *tree, void *key, void *value);
void *bplus_tree_find(BPlusTree *tree, void *key);
int bplus_tree_find_range(BPlusTree *tree, void *start_key, void *end_key, void **results, int max_results);
int bplus_tree_delete(BPlusTree *tree, void *key);
void bplus_tree_destroy(BPlusTree *tree);
int bplus_tree_save_to_json(BPlusTree *tree, const char *filename);

// Utility functions
Record *make_record(void *value);
int compare_ints(const void* a, const void* b);
void destroy_string_value(void* value);

#endif // BTREE_H
