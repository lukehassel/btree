#ifndef BTREE_UNIFIED_H
#define BTREE_UNIFIED_H

#include <stdbool.h>

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
    // Lock is implementation-specific (pthread_rwlock_t or omp_lock_t)
    void *lock_data; // Opaque pointer to lock data
};

// Structure for the B+ Tree itself
struct BPlusTree {
    Node *root;
    int order;
    key_comparator compare;
    value_destroyer destroy_value;
    bool is_openmp; // Flag to indicate which implementation to use
};

// Public API functions (unified interface)
BPlusTree *bplus_tree_create(int order, key_comparator compare, value_destroyer destroy_value, bool use_openmp);
int bplus_tree_insert(BPlusTree *tree, void *key, void *value);
void *bplus_tree_find(BPlusTree *tree, void *key);
int bplus_tree_find_range(BPlusTree *tree, void *start_key, void *end_key, void **results, int max_results);
int bplus_tree_delete(BPlusTree *tree, void *key);
void bplus_tree_destroy(BPlusTree *tree);

// Utility functions
Record *make_record(void *value);
int compare_ints(const void* a, const void* b);
void destroy_string_value(void* value);

// Implementation selection
#define BTREE_USE_PTHREAD 0
#define BTREE_USE_OPENMP  1

// Macro to create tree with specific implementation
#define bplus_tree_create_pthread(order, compare, destroy_value) \
    bplus_tree_create(order, compare, destroy_value, BTREE_USE_PTHREAD)

#define bplus_tree_create_openmp(order, compare, destroy_value) \
    bplus_tree_create(order, compare, destroy_value, BTREE_USE_OPENMP)

#endif // BTREE_UNIFIED_H
