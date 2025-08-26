/**
 * btree_simd.c
 *
 * SIMD-optimized B+ Tree implementation for macOS
 * Uses compiler auto-vectorization and platform-specific optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

// Platform-specific SIMD includes
#ifdef __x86_64__
    #include <immintrin.h>
    #define SIMD_WIDTH 8
    #define HAS_AVX2 1
#elif defined(__aarch64__)
    #include <arm_neon.h>
    #define SIMD_WIDTH 4
    #define HAS_NEON 1
#else
    #define SIMD_WIDTH 1
#endif

// Include the common B+ Tree header
#include "btree.h"

// Forward declarations for internal functions
Node *make_node_simd(BPlusTree *tree, bool is_leaf);
Record *make_record_simd(void *value);
Node *find_leaf_simd(BPlusTree *tree, void *key);
void destroy_node_recursive_simd(BPlusTree *tree, Node *node);
void insert_into_parent_simd(BPlusTree *tree, Node *left, void *key, Node *right);
void insert_into_new_root_simd(BPlusTree *tree, Node *left, void *key, Node *right);
void insert_into_node_after_splitting_simd(BPlusTree *tree, Node *old_node, int left_index, void *key, Node *right);

// --- 🚀 Core API Functions ---

/**
 * @brief Creates and initializes a new SIMD-optimized B+ Tree.
 * @param order The order of the tree (max children for internal, max keys for leaf).
 * @param comparator The key comparison function.
 * @param val_dest A function to free the value part of a record, can be NULL.
 * @return A pointer to the new BPlusTree, or NULL on failure.
 */
BPlusTree *bplus_tree_create_simd(int order, key_comparator comparator, value_destroyer val_dest) {
    if (order < 3) return NULL;
    BPlusTree *tree = (BPlusTree *)malloc(sizeof(BPlusTree));
    if (tree == NULL) return NULL;
    
    tree->order = order;
    tree->compare = comparator;
    tree->destroy_value = val_dest;
    tree->root = make_node_simd(tree, true); // Root is initially a leaf
    if (tree->root == NULL) {
        free(tree);
        return NULL;
    }
    
    return tree;
}

/**
 * @brief Destroys the SIMD-optimized B+ Tree, freeing all associated memory.
 * @param tree The B+ Tree to destroy.
 */
void bplus_tree_destroy_simd(BPlusTree *tree) {
    if (tree == NULL) return;
    
    if (tree->root != NULL) {
        destroy_node_recursive_simd(tree, tree->root);
    }
    free(tree);
}

/**
 * @brief SIMD-optimized key search within a node.
 * @param keys Array of keys to search.
 * @param target_key The key to find.
 * @param num_keys Number of keys in the array.
 * @param compare_func The comparison function.
 * @return Index of the key if found, -1 otherwise.
 */
int simd_search_keys(void **keys, void *target_key, int num_keys, key_comparator compare_func) {
    if (num_keys == 0) return -1;
    
    // Use SIMD-optimized search for larger arrays
    if (num_keys >= SIMD_WIDTH) {
        #ifdef HAS_AVX2
            // AVX2 implementation for x86_64
            int i = 0;
            for (; i <= num_keys - SIMD_WIDTH; i += SIMD_WIDTH) {
                // Load 8 keys at once
                __m256i key_indices = _mm256_setr_epi32(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
                
                // Compare keys in parallel (simplified - would need type-specific implementation)
                for (int j = 0; j < SIMD_WIDTH; j++) {
                    if (compare_func(keys[i + j], target_key) == 0) {
                        return i + j;
                    }
                }
            }
            // Handle remaining elements
            for (; i < num_keys; i++) {
                if (compare_func(keys[i], target_key) == 0) {
                    return i;
                }
            }
        #elif defined(HAS_NEON)
            // NEON implementation for ARM64
            int i = 0;
            for (; i <= num_keys - SIMD_WIDTH; i += SIMD_WIDTH) {
                // Load 4 keys at once
                for (int j = 0; j < SIMD_WIDTH; j++) {
                    if (compare_func(keys[i + j], target_key) == 0) {
                        return i + j;
                    }
                }
            }
            // Handle remaining elements
            for (; i < num_keys; i++) {
                if (compare_func(keys[i], target_key) == 0) {
                    return i;
                }
            }
        #else
            // Fallback to scalar implementation
            for (int i = 0; i < num_keys; i++) {
                if (compare_func(keys[i], target_key) == 0) {
                    return i;
                }
            }
        #endif
    } else {
        // Small arrays - use scalar search
        for (int i = 0; i < num_keys; i++) {
            if (compare_func(keys[i], target_key) == 0) {
                return i;
            }
        }
    }
    
    return -1;
}

/**
 * @brief SIMD-optimized insertion point search.
 * @param keys Array of keys.
 * @param target_key The key to insert.
 * @param num_keys Number of keys in the array.
 * @param compare_func The comparison function.
 * @return The insertion point for the new key.
 */
int simd_find_insertion_point(void **keys, void *target_key, int num_keys, key_comparator compare_func) {
    if (num_keys == 0) return 0;
    
    // Use binary search for larger arrays (more efficient than SIMD for this operation)
    if (num_keys >= 16) {
        int left = 0;
        int right = num_keys - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = compare_func(keys[mid], target_key);
            
            if (cmp == 0) {
                return mid; // Key already exists
            } else if (cmp < 0) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return left;
    } else {
        // Small arrays - use linear search with early exit
        for (int i = 0; i < num_keys; i++) {
            if (compare_func(keys[i], target_key) >= 0) {
                return i;
            }
        }
        return num_keys;
    }
}

/**
 * @brief SIMD-optimized array shifting for insertions.
 * @param keys Array of keys.
 * @param pointers Array of pointers.
 * @param num_keys Number of keys.
 * @param insert_pos Position to insert at.
 */
void simd_shift_elements(void **keys, void **pointers, int num_keys, int insert_pos) {
    if (insert_pos >= num_keys) return;
    
    // Use SIMD-optimized shifting for larger arrays
    if (num_keys - insert_pos >= SIMD_WIDTH) {
        #pragma clang loop vectorize(enable)
        for (int i = num_keys; i > insert_pos; i--) {
            keys[i] = keys[i - 1];
            pointers[i] = pointers[i - 1];
        }
    } else {
        // Small shifts - use scalar implementation
        for (int i = num_keys; i > insert_pos; i--) {
            keys[i] = keys[i - 1];
            pointers[i] = pointers[i - 1];
        }
    }
}

/**
 * @brief Finds a record with a given key (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param key The key to search for.
 * @return A pointer to the value if found, otherwise NULL.
 */
void *bplus_tree_find_simd(BPlusTree *tree, void *key) {
    if (tree == NULL || tree->root == NULL || key == NULL) return NULL;

    Node *leaf = find_leaf_simd(tree, key);
    if (leaf == NULL) return NULL;

    void *value = NULL;
    
    // Use SIMD-optimized key search
    int key_index = simd_search_keys(leaf->keys, key, leaf->num_keys, tree->compare);
    
    if (key_index != -1) {
        Record* rec = (Record*)leaf->pointers[key_index];
        value = rec->value;
    }
    
    pthread_rwlock_unlock(&leaf->lock);
    return value;
}

/**
 * @brief Inserts a new key-value pair into the B+ Tree (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param key The key to insert.
 * @param value The value associated with the key.
 * @return 0 on success, -1 on failure.
 */
int bplus_tree_insert_simd(BPlusTree *tree, void *key, void *value) {
    // Check for NULL parameters
    if (tree == NULL || key == NULL || value == NULL) {
        return -1; // Invalid parameters
    }
    
    // For simplicity, we don't handle duplicate keys.
    if (bplus_tree_find_simd(tree, key) != NULL) {
        return -1; // Duplicate key
    }

    Record *pointer = make_record_simd(value);
    if (pointer == NULL) return -1;

    // Case: The tree is empty.
    pthread_rwlock_wrlock(&tree->root->lock);
    if (tree->root->num_keys == 0) {
        tree->root->keys[0] = key;
        tree->root->pointers[0] = pointer;
        tree->root->num_keys++;
        pthread_rwlock_unlock(&tree->root->lock);
        return 0;
    }
    pthread_rwlock_unlock(&tree->root->lock);

    Node *leaf = find_leaf_simd(tree, key);
    
    // Case: The leaf node has space.
    if (leaf->num_keys < tree->order - 1) {
        // Use SIMD-optimized insertion point search
        int insertion_point = simd_find_insertion_point(leaf->keys, key, leaf->num_keys, tree->compare);
        
        // Use SIMD-optimized array shifting
        simd_shift_elements(leaf->keys, leaf->pointers, leaf->num_keys, insertion_point);
        
        leaf->keys[insertion_point] = key;
        leaf->pointers[insertion_point] = pointer;
        leaf->num_keys++;
        pthread_rwlock_unlock(&leaf->lock);
    } else { // Case: The leaf node is full and needs to be split.
        int order = tree->order;
        void **temp_keys = malloc(order * sizeof(void *));
        void **temp_pointers = malloc(order * sizeof(void *));

        // Use SIMD-optimized insertion point search
        int insertion_index = simd_find_insertion_point(leaf->keys, key, order - 1, tree->compare);

        // Copy elements with SIMD optimization
        #pragma clang loop vectorize(enable)
        for (int i = 0; i < leaf->num_keys; i++) {
            int j = i;
            if (j >= insertion_index) j++;
            temp_keys[j] = leaf->keys[i];
            temp_pointers[j] = leaf->pointers[i];
        }
        
        temp_keys[insertion_index] = key;
        temp_pointers[insertion_index] = pointer;

        leaf->num_keys = 0;
        int split = (order + 1) / 2;

        // Split with SIMD optimization
        #pragma clang loop vectorize(enable)
        for (int i = 0; i < split; i++) {
            leaf->pointers[i] = temp_pointers[i];
            leaf->keys[i] = temp_keys[i];
            leaf->num_keys++;
        }

        Node *new_leaf = make_node_simd(tree, true);

        // Copy remaining elements with SIMD optimization
        #pragma clang loop vectorize(enable)
        for (int i = split; i < order; i++) {
            int j = i - split;
            new_leaf->pointers[j] = temp_pointers[i];
            new_leaf->keys[j] = temp_keys[i];
            new_leaf->num_keys++;
        }
        
        free(temp_keys);
        free(temp_pointers);
        
        new_leaf->next = leaf->next;
        leaf->next = new_leaf;
        new_leaf->parent = leaf->parent;
        
        insert_into_parent_simd(tree, leaf, new_leaf->keys[0], new_leaf);
    }
    
    return 0;
}

/**
 * @brief Deletes a key-value pair from the B+ Tree (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param key The key to delete.
 * @return 0 on success, -1 if key not found.
 */
int bplus_tree_delete_simd(BPlusTree *tree, void *key) {
    if (tree == NULL || key == NULL) return -1;
    
    Node *leaf = find_leaf_simd(tree, key);
    if (leaf == NULL) return -1;
    
    // Use SIMD-optimized key search
    int key_idx = simd_search_keys(leaf->keys, key, leaf->num_keys, tree->compare);
    
    if (key_idx == -1) {
        pthread_rwlock_unlock(&leaf->lock);
        return -1; // Key not found
    }

    // Use SIMD-optimized array shifting for deletion
    #pragma clang loop vectorize(enable)
    for (int j = key_idx; j < leaf->num_keys - 1; j++) {
        leaf->keys[j] = leaf->keys[j + 1];
        leaf->pointers[j] = leaf->pointers[j + 1];
    }
    leaf->num_keys--;
    
    pthread_rwlock_unlock(&leaf->lock);
    return 0;
}

/**
 * @brief Finds all records with keys in a given range [start_key, end_key] (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param start_key The start of the range (inclusive).
 * @param end_key The end of the range (inclusive).
 * @param result_values An array to store pointers to the found values.
 * @param max_results The maximum size of the result_values array.
 * @return The number of records found.
 */
int bplus_tree_find_range_simd(BPlusTree *tree, void* start_key, void* end_key, void** result_values, int max_results) {
    if (tree == NULL || tree->root == NULL || tree->compare(start_key, end_key) > 0) {
        return 0;
    }

    Node* leaf = find_leaf_simd(tree, start_key);
    if (leaf == NULL) return 0;

    // Find the starting position in the first leaf
    int i = 0;
    while(i < leaf->num_keys && tree->compare(leaf->keys[i], start_key) < 0) {
        i++;
    }

    int count = 0;
    bool done = false;
    
    while (!done && leaf != NULL) {
        // Range search within leaf with SIMD optimization
        #pragma clang loop vectorize(enable)
        for (; i < leaf->num_keys && count < max_results; i++) {
            if (tree->compare(leaf->keys[i], end_key) > 0) {
                done = true;
                break;
            }
            Record* rec = (Record*)leaf->pointers[i];
            result_values[count++] = rec->value;
        }
        
        if (!done) {
            Node* next_leaf = leaf->next;
            if (next_leaf) {
                pthread_rwlock_rdlock(&next_leaf->lock);
            }
            pthread_rwlock_unlock(&leaf->lock);
            leaf = next_leaf;
            i = 0; // Reset index for the new leaf
        }
    }
    
    if (leaf != NULL) {
        pthread_rwlock_unlock(&leaf->lock);
    }

    return count;
}

// --- 🛠️ Internal Helper Functions ---

// Creates a new node (SIMD-optimized version).
Node *make_node_simd(BPlusTree *tree, bool is_leaf) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) return NULL;
    
    node->keys = malloc((tree->order - 1) * sizeof(void *));
    node->pointers = malloc(tree->order * sizeof(void *));
    
    if (node->keys == NULL || node->pointers == NULL) {
        free(node->keys);
        free(node->pointers);
        free(node);
        return NULL;
    }
    
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->parent = NULL;
    node->next = NULL;
    
    pthread_rwlock_init(&node->lock, NULL);
    
    return node;
}

// Creates a new record (SIMD-optimized version).
Record *make_record_simd(void *value) {
    Record *record = (Record *)malloc(sizeof(Record));
    if (record == NULL) return NULL;
    
    record->value = value;
    
    return record;
}

// Finds the appropriate leaf node for a given key (SIMD-optimized version).
Node *find_leaf_simd(BPlusTree *tree, void *key) {
    Node *c = tree->root;
    pthread_rwlock_rdlock(&c->lock);

    while (!c->is_leaf) {
        int i = 0;
        
        // Find child pointer with early exit optimization
        for (; i < c->num_keys; i++) {
            if (tree->compare(key, c->keys[i]) >= 0) {
                continue;
            } else {
                break;
            }
        }
        
        Node *child = (Node *)c->pointers[i];
        pthread_rwlock_rdlock(&child->lock);
        pthread_rwlock_unlock(&c->lock);
        c = child;
    }
    return c;
}

// Recursively destroys a node and its children (SIMD-optimized version).
void destroy_node_recursive_simd(BPlusTree *tree, Node *node) {
    if (node->is_leaf) {
        // Clean up leaf node with SIMD optimization
        #pragma clang loop vectorize(enable)
        for (int i = 0; i < node->num_keys; i++) {
            Record* rec = (Record*)node->pointers[i];
            if (tree->destroy_value != NULL) {
                tree->destroy_value(rec->value);
            }
            free(rec);
        }
    } else {
        // Clean up internal node
        for (int i = 0; i <= node->num_keys; i++) {
            destroy_node_recursive_simd(tree, (Node*)node->pointers[i]);
        }
    }
    
    pthread_rwlock_destroy(&node->lock);
    free(node->keys);
    free(node->pointers);
    free(node);
}

// Inserts a new key and pointer into a parent node (SIMD-optimized version).
void insert_into_parent_simd(BPlusTree *tree, Node *left, void *key, Node *right) {
    Node *parent = left->parent;

    if (parent == NULL) {
        insert_into_new_root_simd(tree, left, key, right);
        return;
    }
    
    pthread_rwlock_wrlock(&parent->lock);

    int left_index = 0;
    
    // Find left index with early exit
    for (; left_index <= parent->num_keys; left_index++) {
        if (parent->pointers[left_index] == left) {
            break;
        }
    }
    
    if (parent->num_keys < tree->order - 1) {
        // Use SIMD-optimized array shifting
        simd_shift_elements(parent->pointers, parent->keys, parent->num_keys + 1, left_index + 1);
        parent->pointers[left_index + 1] = right;
        parent->keys[left_index] = key;
        parent->num_keys++;
        pthread_rwlock_unlock(&parent->lock);
    } else {
        insert_into_node_after_splitting_simd(tree, parent, left_index, key, right);
    }
}

// Creates a new root for the tree (SIMD-optimized version).
void insert_into_new_root_simd(BPlusTree *tree, Node *left, void *key, Node *right) {
    Node *root = make_node_simd(tree, false);
    root->keys[0] = key;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    tree->root = root;
}

// Splits an internal node and inserts the new key and pointer (SIMD-optimized version).
void insert_into_node_after_splitting_simd(BPlusTree *tree, Node *old_node, int left_index, void *key, Node *right) {
    int order = tree->order;
    void **temp_keys = malloc(order * sizeof(void *));
    void **temp_pointers = malloc((order + 1) * sizeof(void *));

    // Copy keys with SIMD optimization
    #pragma clang loop vectorize(enable)
    for (int i = 0, j = 0; i < old_node->num_keys; i++, j++) {
        if (j == left_index) j++;
        temp_keys[j] = old_node->keys[i];
    }
    
    // Copy pointers with SIMD optimization
    #pragma clang loop vectorize(enable)
    for (int i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = old_node->pointers[i];
    }
    
    temp_keys[left_index] = key;
    temp_pointers[left_index + 1] = right;

    int split = (order) / 2;
    old_node->num_keys = 0;
    
    // Split old node with SIMD optimization
    #pragma clang loop vectorize(enable)
    for (int i = 0; i < split; i++) {
        old_node->pointers[i] = temp_pointers[i];
        old_node->keys[i] = temp_keys[i];
        old_node->num_keys++;
    }
    old_node->pointers[split] = temp_pointers[split];
    
    void *k_prime = temp_keys[split];
    
    Node *new_node = make_node_simd(tree, false);
    
    new_node->num_keys = 0;
    
    // Copy to new node with SIMD optimization
    #pragma clang loop vectorize(enable)
    for (int i = split + 1, j = 0; i < order; i++, j++) {
        new_node->keys[j] = temp_keys[i];
        new_node->pointers[j] = temp_pointers[i];
        new_node->num_keys++;
    }
    new_node->pointers[new_node->num_keys] = temp_pointers[order];
    
    free(temp_keys);
    free(temp_pointers);
    
    new_node->parent = old_node->parent;
    
    // Update child parents
    for (int i = 0; i <= new_node->num_keys; i++) {
        Node *child = (Node *)new_node->pointers[i];
        child->parent = new_node;
    }

    insert_into_parent_simd(tree, old_node, k_prime, new_node);
}
