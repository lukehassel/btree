#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>

// Default order of the B+ Tree
#define DEFAULT_ORDER 4

// Serialization constants
#define BTREE_MAGIC_NUMBER 0x42545245  // "BTRE" in hex (32-bit compatible)
#define BTREE_VERSION 1
#define MAX_FILENAME_LENGTH 256

// Forward declarations
typedef struct Node Node;
typedef struct BPlusTree BPlusTree;

// Serialization metadata structures
typedef struct {
    uint32_t magic;           // Magic number to identify file format
    uint32_t version;         // Format version
    uint32_t order;           // Tree order
    uint32_t total_nodes;     // Total number of nodes
    uint32_t total_records;   // Total number of records
    uint64_t checksum;        // Data integrity checksum
} BTreeHeader;

typedef struct {
    uint32_t node_id;         // Unique node identifier
    uint32_t parent_id;       // Parent node ID (0 for root)
    uint32_t num_keys;        // Number of keys in this node
    bool is_leaf;             // Whether this is a leaf node
    uint32_t next_leaf_id;    // Next leaf node ID (for leaf nodes)
    uint32_t data_size;       // Size of node data in bytes
} NodeHeader;

// Function pointer for comparing keys.
// Returns:
//   < 0 if key1 < key2
//   = 0 if key1 == key2
//   > 0 if key1 > key2
typedef int (*key_comparator)(const void* key1, const void* key2);

// Function pointer for destroying user-provided value data.
typedef void (*value_destroyer)(void* value);

// Function pointer for serializing key data to binary format
typedef size_t (*key_serializer)(const void* key, void* buffer, size_t buffer_size);

// Function pointer for deserializing key data from binary format
typedef void* (*key_deserializer)(const void* buffer, size_t buffer_size);

// Function pointer for serializing value data to binary format
typedef size_t (*value_serializer)(const void* value, void* buffer, size_t buffer_size);

// Function pointer for deserializing value data from binary format
typedef void* (*value_deserializer)(const void* buffer, size_t buffer_size);

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
    uint32_t node_id; // Unique identifier for serialization
};

// Structure for the B+ Tree itself
struct BPlusTree {
    Node *root;
    int order;
    key_comparator compare;
    value_destroyer destroy_value;
    key_serializer serialize_key;
    key_deserializer deserialize_key;
    value_serializer serialize_value;
    value_deserializer deserialize_value;
    uint32_t next_node_id; // Counter for unique node IDs
};

// Public API functions
BPlusTree *bplus_tree_create(int order, key_comparator compare, value_destroyer destroy_value);
BPlusTree *bplus_tree_create_with_serializers(int order, key_comparator compare, value_destroyer destroy_value,
                                             key_serializer k_ser, key_deserializer k_deser,
                                             value_serializer v_ser, value_deserializer v_deser);
int bplus_tree_insert(BPlusTree *tree, void *key, void *value);
void *bplus_tree_find(BPlusTree *tree, void *key);
int bplus_tree_find_range(BPlusTree *tree, void *start_key, void *end_key, void **results, int max_results);
int bplus_tree_delete(BPlusTree *tree, void *key);
void bplus_tree_destroy(BPlusTree *tree);

// Serialization functions
int bplus_tree_save_to_file(BPlusTree *tree, const char *filename);
BPlusTree *bplus_tree_load_from_file(const char *filename, key_comparator compare, value_destroyer destroy_value,
                                    key_deserializer k_deser, value_deserializer v_deser);
int bplus_tree_save_to_json(BPlusTree *tree, const char *filename);

// Utility functions
Record *make_record(void *value);
int compare_ints(const void* a, const void* b);
void destroy_string_value(void* value);

// Built-in serializers for common types
size_t serialize_int_key(const void* key, void* buffer, size_t buffer_size);
void* deserialize_int_key(const void* buffer, size_t buffer_size);
size_t serialize_string_key(const void* key, void* buffer, size_t buffer_size);
void* deserialize_string_key(const void* buffer, size_t buffer_size);
size_t serialize_string_value(const void* value, void* buffer, size_t buffer_size);
void* deserialize_string_value(const void* buffer, size_t buffer_size);

#endif // BTREE_H
