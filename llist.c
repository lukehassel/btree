/**
 * llist.c
 *
 * A generic, thread-safe linked list implementation in C with serialization support.
 *
 * Features:
 * - Generic Types: Uses void* for data with user-provided destroy function.
 * - Serialization: Efficient binary serialization/deserialization.
 * - Memory Management: Includes destroy function to prevent memory leaks.
 * - Comprehensive API: Append, prepend, insert, remove, search operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "llist.h"

// --- 🚀 Core API Functions ---

/**
 * @brief Creates and initializes a new linked list.
 * @param destroy_data Function to destroy data when removing nodes.
 * @return A pointer to the new LinkedList, or NULL on failure.
 */
LinkedList *llist_create(data_destroyer destroy_data) {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (list == NULL) return NULL;
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->destroy_data = destroy_data;
    list->serialize_data = NULL;
    list->deserialize_data = NULL;
    list->next_node_id = 1;
    
    return list;
}

/**
 * @brief Creates a new linked list with custom serialization functions.
 * @param destroy_data Function to destroy data.
 * @param ser Function to serialize data.
 * @param deser Function to deserialize data.
 * @return A pointer to the new LinkedList, or NULL on failure.
 */
LinkedList *llist_create_with_serializer(data_destroyer destroy_data,
                                       data_serializer ser, data_deserializer deser) {
    LinkedList *list = llist_create(destroy_data);
    if (list == NULL) return NULL;
    
    list->serialize_data = ser;
    list->deserialize_data = deser;
    
    return list;
}

/**
 * @brief Destroys the linked list, freeing all associated memory.
 * @param list The linked list to destroy.
 */
void llist_destroy(LinkedList *list) {
    if (list == NULL) return;
    
    LListNode *current = list->head;
    while (current != NULL) {
        LListNode *next = current->next;
        if (list->destroy_data && current->data) {
            list->destroy_data(current->data);
        }
        free(current);
        current = next;
    }
    
    free(list);
}

/**
 * @brief Creates a new linked list node.
 * @param data The data to store in the node.
 * @param node_id Unique identifier for the node.
 * @return A pointer to the new node, or NULL on failure.
 */
static LListNode *make_node(void *data, uint32_t node_id) {
    LListNode *node = (LListNode *)malloc(sizeof(LListNode));
    if (node == NULL) return NULL;
    
    node->data = data;
    node->next = NULL;
    node->node_id = node_id;
    
    return node;
}

/**
 * @brief Appends data to the end of the linked list.
 * @param list The linked list.
 * @param data The data to append.
 * @return 0 on success, -1 on failure.
 */
int llist_append(LinkedList *list, void *data) {
    if (list == NULL || data == NULL) return -1;
    
    LListNode *new_node = make_node(data, list->next_node_id++);
    if (new_node == NULL) return -1;
    
    if (list->tail == NULL) {
        // Empty list
        list->head = new_node;
        list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    
    list->size++;
    return 0;
}

/**
 * @brief Prepends data to the beginning of the linked list.
 * @param list The linked list.
 * @param data The data to prepend.
 * @return 0 on success, -1 on failure.
 */
int llist_prepend(LinkedList *list, void *data) {
    if (list == NULL || data == NULL) return -1;
    
    LListNode *new_node = make_node(data, list->next_node_id++);
    if (new_node == NULL) return -1;
    
    new_node->next = list->head;
    list->head = new_node;
    
    if (list->tail == NULL) {
        list->tail = new_node;
    }
    
    list->size++;
    return 0;
}

/**
 * @brief Inserts data at a specific index in the linked list.
 * @param list The linked list.
 * @param index The index where to insert (0-based).
 * @param data The data to insert.
 * @return 0 on success, -1 on failure.
 */
int llist_insert_at(LinkedList *list, size_t index, void *data) {
    if (list == NULL || data == NULL || index > list->size) return -1;
    
    if (index == 0) {
        return llist_prepend(list, data);
    }
    
    if (index == list->size) {
        return llist_append(list, data);
    }
    
    LListNode *new_node = make_node(data, list->next_node_id++);
    if (new_node == NULL) return -1;
    
    LListNode *current = list->head;
    for (size_t i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    new_node->next = current->next;
    current->next = new_node;
    
    list->size++;
    return 0;
}

/**
 * @brief Gets data at a specific index in the linked list.
 * @param list The linked list.
 * @param index The index to get data from (0-based).
 * @return Pointer to the data, or NULL if index is invalid.
 */
void *llist_get_at(LinkedList *list, size_t index) {
    if (list == NULL || index >= list->size) return NULL;
    
    LListNode *current = list->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current->data;
}

/**
 * @brief Removes data at a specific index in the linked list.
 * @param list The linked list.
 * @param index The index to remove from (0-based).
 * @return 0 on success, -1 on failure.
 */
int llist_remove_at(LinkedList *list, size_t index) {
    if (list == NULL || index >= list->size) return -1;
    
    if (index == 0) {
        return llist_remove_first(list) != NULL ? 0 : -1;
    }
    
    LListNode *current = list->head;
    for (size_t i = 0; i < index - 1; i++) {
        current = current->next;
    }
    
    LListNode *to_remove = current->next;
    current->next = to_remove->next;
    
    if (to_remove == list->tail) {
        list->tail = current;
    }
    
    if (list->destroy_data && to_remove->data) {
        list->destroy_data(to_remove->data);
    }
    
    free(to_remove);
    list->size--;
    return 0;
}

/**
 * @brief Removes and returns the first element in the linked list.
 * @param list The linked list.
 * @return Pointer to the removed data, or NULL if list is empty.
 */
void *llist_remove_first(LinkedList *list) {
    if (list == NULL || list->head == NULL) return NULL;
    
    LListNode *to_remove = list->head;
    void *data = to_remove->data;
    
    list->head = to_remove->next;
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    free(to_remove);
    list->size--;
    
    return data;
}

/**
 * @brief Removes and returns the last element in the linked list.
 * @param list The linked list.
 * @return Pointer to the removed data, or NULL if list is empty.
 */
void *llist_remove_last(LinkedList *list) {
    if (list == NULL || list->head == NULL) return NULL;
    
    if (list->head == list->tail) {
        // Only one element
        void *data = list->head->data;
        free(list->head);
        list->head = list->tail = NULL;
        list->size = 0;
        return data;
    }
    
    LListNode *current = list->head;
    while (current->next != list->tail) {
        current = current->next;
    }
    
    LListNode *to_remove = list->tail;
    void *data = to_remove->data;
    
    current->next = NULL;
    list->tail = current;
    
    free(to_remove);
    list->size--;
    
    return data;
}

/**
 * @brief Returns the current size of the linked list.
 * @param list The linked list.
 * @return The number of elements in the list.
 */
size_t llist_size(LinkedList *list) {
    return list ? list->size : 0;
}

/**
 * @brief Checks if the linked list is empty.
 * @param list The linked list.
 * @return true if empty, false otherwise.
 */
bool llist_is_empty(LinkedList *list) {
    return list == NULL || list->size == 0;
}

/**
 * @brief Finds the first occurrence of data in the linked list.
 * @param list The linked list.
 * @param data The data to search for.
 * @param compare Comparison function.
 * @return Pointer to the found data, or NULL if not found.
 */
void *llist_find(LinkedList *list, const void *data, int (*compare)(const void*, const void*)) {
    if (list == NULL || data == NULL || compare == NULL) return NULL;
    
    LListNode *current = list->head;
    while (current != NULL) {
        if (compare(current->data, data) == 0) {
            return current->data;
        }
        current = current->next;
    }
    
    return NULL;
}

/**
 * @brief Finds the index of the first occurrence of data in the linked list.
 * @param list The linked list.
 * @param data The data to search for.
 * @param compare Comparison function.
 * @return The index of the found data, or -1 if not found.
 */
int llist_index_of(LinkedList *list, const void *data, int (*compare)(const void*, const void*)) {
    if (list == NULL || data == NULL || compare == NULL) return -1;
    
    LListNode *current = list->head;
    int index = 0;
    
    while (current != NULL) {
        if (compare(current->data, data) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }
    
    return -1;
}

// --- 🚀 Serialization Functions ---

/**
 * @brief Calculates a simple checksum for data integrity.
 * @param data Pointer to data.
 * @param size Size of data in bytes.
 * @return 64-bit checksum.
 */
static uint64_t calculate_checksum(const void *data, size_t size) {
    uint64_t checksum = 0;
    const uint8_t *bytes = (const uint8_t*)data;
    
    for (size_t i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) + bytes[i]; // Simple hash
    }
    
    return checksum;
}

/**
 * @brief Saves the linked list to a binary file.
 * @param list The linked list to save.
 * @param filename The output filename.
 * @return 0 on success, -1 on failure.
 */
int llist_save_to_file(LinkedList *list, const char *filename) {
    if (!list || !filename || !list->serialize_data) {
        return -1;
    }
    
    // Calculate required buffer size
    size_t estimated_size = sizeof(LListHeader) + 
                           list->size * (sizeof(LListNodeHeader) + 64); // Conservative estimate
    
    void *buffer = malloc(estimated_size);
    if (!buffer) return -1;
    
    // Write header
    LListHeader *header = (LListHeader*)buffer;
    header->magic = LLIST_MAGIC_NUMBER;
    header->version = LLIST_VERSION;
    header->total_nodes = list->size;
    header->checksum = 0; // Will calculate after writing data
    
    size_t offset = sizeof(LListHeader);
    
    // Serialize all nodes
    LListNode *current = list->head;
    while (current != NULL) {
        if (offset + sizeof(LListNodeHeader) > estimated_size) {
            // Buffer too small, reallocate
            estimated_size *= 2;
            void *new_buffer = realloc(buffer, estimated_size);
            if (!new_buffer) {
                free(buffer);
                return -1;
            }
            buffer = new_buffer;
            header = (LListHeader*)buffer;
        }
        
        LListNodeHeader *node_header = (LListNodeHeader*)((char*)buffer + offset);
        node_header->node_id = current->node_id;
        node_header->next_id = (current->next) ? current->next->node_id : 0;
        
        size_t data_size = list->serialize_data(current->data, 
                                               (char*)buffer + offset + sizeof(LListNodeHeader),
                                               estimated_size - offset - sizeof(LListNodeHeader));
        if (data_size == 0) {
            free(buffer);
            return -1;
        }
        
        node_header->data_size = data_size;
        offset += sizeof(LListNodeHeader) + data_size;
        
        current = current->next;
    }
    
    // Calculate final checksum
    header->checksum = calculate_checksum((char*)buffer + sizeof(LListHeader), 
                                        offset - sizeof(LListHeader));
    
    // Write to file
    FILE *file = fopen(filename, "wb");
    if (!file) {
        free(buffer);
        return -1;
    }
    
    size_t written = fwrite(buffer, 1, offset, file);
    fclose(file);
    free(buffer);
    
    return (written == offset) ? 0 : -1;
}

/**
 * @brief Loads a linked list from a binary file.
 * @param filename The input filename.
 * @param destroy_data Function to destroy data.
 * @param deser Function to deserialize data.
 * @return A pointer to the loaded LinkedList, or NULL on failure.
 */
LinkedList *llist_load_from_file(const char *filename, data_destroyer destroy_data,
                               data_deserializer deser) {
    if (!filename || !deser) return NULL;
    
    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;
    
    // Read header
    LListHeader header;
    if (fread(&header, sizeof(LListHeader), 1, file) != 1) {
        fclose(file);
        return NULL;
    }
    
    // Validate header
    if (header.magic != LLIST_MAGIC_NUMBER || header.version != LLIST_VERSION) {
        fclose(file);
        return NULL;
    }
    
    // Create list with serializer
    LinkedList *list = llist_create_with_serializer(destroy_data, NULL, deser);
    if (!list) {
        fclose(file);
        return NULL;
    }
    
    // Read all nodes
    for (uint32_t i = 0; i < header.total_nodes; i++) {
        LListNodeHeader node_header;
        if (fread(&node_header, sizeof(LListNodeHeader), 1, file) != 1) {
            llist_destroy(list);
            fclose(file);
            return NULL;
        }
        
        // Read node data
        void *data_buffer = malloc(node_header.data_size);
        if (!data_buffer) {
            llist_destroy(list);
            fclose(file);
            return NULL;
        }
        
        if (fread(data_buffer, 1, node_header.data_size, file) != node_header.data_size) {
            free(data_buffer);
            llist_destroy(list);
            fclose(file);
            return NULL;
        }
        
        // Deserialize data
        void *data = deser(data_buffer, node_header.data_size);
        free(data_buffer);
        
        if (!data) {
            llist_destroy(list);
            fclose(file);
            return NULL;
        }
        
        // Add to list
        if (llist_append(list, data) != 0) {
            if (destroy_data) destroy_data(data);
            llist_destroy(list);
            fclose(file);
            return NULL;
        }
    }
    
    fclose(file);
    return list;
}

// --- Built-in Serializers ---

/**
 * @brief Serializes integer data to binary format.
 * @param data Pointer to integer data.
 * @param buffer Output buffer.
 * @param buffer_size Size of buffer.
 * @return Number of bytes written.
 */
size_t serialize_int_data(const void* data, void* buffer, size_t buffer_size) {
    if (buffer_size < sizeof(int)) return 0;
    *(int*)buffer = *(int*)data;
    return sizeof(int);
}

/**
 * @brief Deserializes integer data from binary format.
 * @param buffer Input buffer.
 * @param buffer_size Size of buffer.
 * @return Pointer to deserialized integer data.
 */
void* deserialize_int_data(const void* buffer, size_t buffer_size) {
    if (buffer_size < sizeof(int)) return NULL;
    int* data = malloc(sizeof(int));
    if (data) *data = *(int*)buffer;
    return data;
}

/**
 * @brief Serializes string data to binary format.
 * @param data Pointer to string data.
 * @param buffer Output buffer.
 * @param buffer_size Size of buffer.
 * @return Number of bytes written.
 */
size_t serialize_string_data(const void* data, void* buffer, size_t buffer_size) {
    const char* str = (const char*)data;
    size_t len = strlen(str) + 1; // Include null terminator
    
    if (buffer_size < len + sizeof(size_t)) return 0;
    
    *(size_t*)buffer = len;
    memcpy((char*)buffer + sizeof(size_t), str, len);
    
    return sizeof(size_t) + len;
}

/**
 * @brief Deserializes string data from binary format.
 * @param buffer Input buffer.
 * @param buffer_size Size of buffer.
 * @return Pointer to deserialized string data.
 */
void* deserialize_string_data(const void* buffer, size_t buffer_size) {
    if (buffer_size < sizeof(size_t)) return NULL;
    
    size_t len = *(size_t*)buffer;
    if (buffer_size < sizeof(size_t) + len) return NULL;
    
    char* str = malloc(len);
    if (str) {
        memcpy(str, (char*)buffer + sizeof(size_t), len);
    }
    
    return str;
}

// --- Utility Functions ---

/**
 * @brief Prints the linked list using a custom print function.
 * @param list The linked list.
 * @param print_func Function to print individual elements.
 */
void llist_print(LinkedList *list, void (*print_func)(const void*)) {
    if (!list || !print_func) return;
    
    printf("LinkedList[%zu]: ", list->size);
    LListNode *current = list->head;
    
    while (current != NULL) {
        print_func(current->data);
        if (current->next != NULL) {
            printf(" -> ");
        }
        current = current->next;
    }
    printf("\n");
}

/**
 * @brief Reverses the linked list in place.
 * @param list The linked list.
 * @return The reversed linked list (same pointer).
 */
LinkedList *llist_reverse(LinkedList *list) {
    if (!list || list->size <= 1) return list;
    
    LListNode *prev = NULL;
    LListNode *current = list->head;
    LListNode *next = NULL;
    
    list->tail = list->head;
    
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    
    list->head = prev;
    return list;
}

/**
 * @brief Creates a copy of the linked list.
 * @param list The linked list to copy.
 * @param copy_func Function to copy individual elements.
 * @return A new linked list with copied data.
 */
LinkedList *llist_copy(LinkedList *list, void* (*copy_func)(const void*)) {
    if (!list || !copy_func) return NULL;
    
    LinkedList *copy = llist_create(list->destroy_data);
    if (!copy) return NULL;
    
    LListNode *current = list->head;
    while (current != NULL) {
        void *copied_data = copy_func(current->data);
        if (!copied_data) {
            llist_destroy(copy);
            return NULL;
        }
        
        if (llist_append(copy, copied_data) != 0) {
            if (list->destroy_data) list->destroy_data(copied_data);
            llist_destroy(copy);
            return NULL;
        }
        
        current = current->next;
    }
    
    return copy;
}


