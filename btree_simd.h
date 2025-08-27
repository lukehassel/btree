/**
 * btree_simd.h
 *
 * Header file for SIMD-optimized B+ Tree implementation.
 * Provides platform-specific SIMD optimizations for macOS.
 */

#ifndef BTREE_SIMD_H
#define BTREE_SIMD_H

#include <stdbool.h>
#include "btree.h" // Reuse common types, macros, and public structs

// --- 🚀 Core API Functions ---

/**
 * @brief Creates and initializes a new SIMD-optimized B+ Tree.
 * @param order The order of the tree (max children for internal, max keys for leaf).
 * @param comparator The key comparison function.
 * @param val_dest A function to free the value part of a record, can be NULL.
 * @return A pointer to the new BPlusTree, or NULL on failure.
 */
BPlusTree *bplus_tree_create_simd(int order, key_comparator comparator, value_destroyer val_dest);

/**
 * @brief Destroys the SIMD-optimized B+ Tree, freeing all associated memory.
 * @param tree The B+ Tree to destroy.
 */
void bplus_tree_destroy_simd(BPlusTree *tree);

/**
 * @brief Finds a record with a given key (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param key The key to search for.
 * @return A pointer to the value if found, otherwise NULL.
 */
void *bplus_tree_find_simd(BPlusTree *tree, void *key);

/**
 * @brief Inserts a new key-value pair into the B+ Tree (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param key The key to insert.
 * @param value The value associated with the key.
 * @return 0 on success, -1 on failure.
 */
int bplus_tree_insert_simd(BPlusTree *tree, void *key, void *value);

/**
 * @brief Deletes a key-value pair from the B+ Tree (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param key The key to delete.
 * @return 0 on success, -1 if key not found.
 */
int bplus_tree_delete_simd(BPlusTree *tree, void *key);

/**
 * @brief Finds all records with keys in a given range [start_key, end_key] (SIMD-optimized).
 * @param tree The B+ Tree.
 * @param start_key The start of the range (inclusive).
 * @param end_key The end of the range (inclusive).
 * @param result_values An array to store pointers to the found values.
 * @param max_results The maximum size of the result_values array.
 * @return The number of records found.
 */
int bplus_tree_find_range_simd(BPlusTree *tree, void* start_key, void* end_key, void** result_values, int max_results);

// --- 🛠️ SIMD Utility Functions ---

/**
 * @brief SIMD-optimized key search within a node.
 * @param keys Array of keys to search.
 * @param target_key The key to find.
 * @param num_keys Number of keys in the array.
 * @param compare_func The comparison function.
 * @return Index of the key if found, -1 otherwise.
 */
int simd_search_keys(void **keys, void *target_key, int num_keys, key_comparator compare_func);

/**
 * @brief SIMD-optimized insertion point search.
 * @param keys Array of keys.
 * @param target_key The key to insert.
 * @param num_keys Number of keys in the array.
 * @param compare_func The comparison function.
 * @return The insertion point for the new key.
 */
int simd_find_insertion_point(void **keys, void *target_key, int num_keys, key_comparator compare_func);

/**
 * @brief SIMD-optimized array shifting for insertions.
 * @param keys Array of keys.
 * @param pointers Array of pointers.
 * @param num_keys Number of keys.
 * @param insert_pos Position to insert at.
 */
void simd_shift_elements(void **keys, void **pointers, int num_keys, int insert_pos);

// --- 🛠️ Utility Functions ---

/**
 * @brief Comparator for integer keys.
 * @param a First integer key.
 * @param b Second integer key.
 * @return -1 if a < b, 0 if a == b, 1 if a > b.
 */
int compare_ints(const void* a, const void* b);

/**
 * @brief Destroys string values (frees the memory).
 * @param value The string value to destroy.
 */
void destroy_string_value(void* value);

#endif // BTREE_SIMD_H
