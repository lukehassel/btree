#ifndef LLIST_H
#define LLIST_H

#include <stdbool.h>
#include <stdint.h>

// Serialization constants
#define LLIST_MAGIC_NUMBER 0x4C4C4953  // "LLIS" in hex (32-bit compatible)
#define LLIST_VERSION 1
#define MAX_FILENAME_LENGTH 256

// Forward declarations
typedef struct LListNode LListNode;
typedef struct LinkedList LinkedList;

// Serialization metadata structures
typedef struct {
    uint32_t magic;           // Magic number to identify file format
    uint32_t version;         // Format version
    uint32_t total_nodes;     // Total number of nodes
    uint64_t checksum;        // Data integrity checksum
} LListHeader;

typedef struct {
    uint32_t node_id;         // Unique node identifier
    uint32_t next_id;         // Next node ID (0 for last node)
    uint32_t data_size;       // Size of node data in bytes
} LListNodeHeader;

// Function pointer for destroying user-provided data
typedef void (*data_destroyer)(void* data);

// Function pointer for serializing data to binary format
typedef size_t (*data_serializer)(const void* data, void* buffer, size_t buffer_size);

// Function pointer for deserializing data from binary format
typedef void* (*data_deserializer)(const void* buffer, size_t buffer_size);

// Structure for a linked list node
struct LListNode {
    void *data;
    LListNode *next;
    uint32_t node_id; // Unique identifier for serialization
};

// Structure for the linked list itself
struct LinkedList {
    LListNode *head;
    LListNode *tail;
    size_t size;
    data_destroyer destroy_data;
    data_serializer serialize_data;
    data_deserializer deserialize_data;
    uint32_t next_node_id; // Counter for unique node IDs
};

// Public API functions
LinkedList *llist_create(data_destroyer destroy_data);
LinkedList *llist_create_with_serializer(data_destroyer destroy_data,
                                       data_serializer ser, data_deserializer deser);
void llist_destroy(LinkedList *list);

// Basic operations
int llist_append(LinkedList *list, void *data);
int llist_prepend(LinkedList *list, void *data);
int llist_insert_at(LinkedList *list, size_t index, void *data);
void *llist_get_at(LinkedList *list, size_t index);
int llist_remove_at(LinkedList *list, size_t index);
void *llist_remove_first(LinkedList *list);
void *llist_remove_last(LinkedList *list);
size_t llist_size(LinkedList *list);
bool llist_is_empty(LinkedList *list);

// Search operations
void *llist_find(LinkedList *list, const void *data, int (*compare)(const void*, const void*));
int llist_index_of(LinkedList *list, const void *data, int (*compare)(const void*, const void*));

// Serialization functions
int llist_save_to_file(LinkedList *list, const char *filename);
LinkedList *llist_load_from_file(const char *filename, data_destroyer destroy_data,
                               data_deserializer deser);

// Utility functions
void llist_print(LinkedList *list, void (*print_func)(const void*));
LinkedList *llist_reverse(LinkedList *list);
LinkedList *llist_copy(LinkedList *list, void* (*copy_func)(const void*));

// Built-in serializers for common types
size_t serialize_int_data(const void* data, void* buffer, size_t buffer_size);
void* deserialize_int_data(const void* buffer, size_t buffer_size);
size_t serialize_string_data(const void* data, void* buffer, size_t buffer_size);
void* deserialize_string_data(const void* buffer, size_t buffer_size);

#endif // LLIST_H


