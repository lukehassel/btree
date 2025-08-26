/**
 * btree_simd_test.c
 *
 * Test suite for the SIMD-optimized B+ Tree implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>

// Include the SIMD B+ Tree header file
#include "../btree_simd.h"
#include "test_utils.h"

// Forward declarations for test functions
bool test_create_destroy();
bool test_basic_insert_find();
bool test_multiple_inserts();
bool test_range_queries();
bool test_simd_performance();

// --- Test Harness Setup ---

int tests_passed = 0;
int tests_failed = 0;

// Macro to run a test function and report its result
#define RUN_TEST(test) \
    do { \
        printf("--- Running %s ---\n", #test); \
        if (test()) { \
            tests_passed++; \
            printf("✅ PASS: %s\n\n", #test); \
        } else { \
            tests_failed++; \
            printf("❌ FAIL: %s\n\n", #test); \
        } \
    } while (0)

// --- Test Cases ---

/**
 * @brief Tests basic tree creation and destruction.
 */
bool test_create_destroy() {
    printf("  Creating SIMD B+ Tree...\n");
    BPlusTree *tree = bplus_tree_create_simd(4, compare_ints, NULL);
    
    if (tree == NULL) {
        printf("  ❌ Failed to create tree\n");
        return false;
    }
    
    printf("  ✅ Tree created successfully\n");
    
    printf("  Destroying tree...\n");
    bplus_tree_destroy_simd(tree);
    printf("  ✅ Tree destroyed successfully\n");
    
    return true;
}

/**
 * @brief Tests basic insert and find operations.
 */
bool test_basic_insert_find() {
    printf("  Creating SIMD B+ Tree...\n");
    BPlusTree *tree = bplus_tree_create_simd(4, compare_ints, NULL);
    
    if (tree == NULL) {
        printf("  ❌ Failed to create tree\n");
        return false;
    }
    
    printf("  ✅ Tree created successfully\n");
    
    // Test single insert
    int key = 42;
    char *value = "test_value";
    
    printf("  Inserting key %d...\n", key);
    int insert_result = bplus_tree_insert_simd(tree, &key, value);
    
    if (insert_result != 0) {
        printf("  ❌ Insert failed with result %d\n", insert_result);
        bplus_tree_destroy_simd(tree);
        return false;
    }
    
    printf("  ✅ Insert successful\n");
    
    // Test single find
    printf("  Finding key %d...\n", key);
    void *found_value = bplus_tree_find_simd(tree, &key);
    
    if (found_value == NULL) {
        printf("  ❌ Find failed - value not found\n");
        bplus_tree_destroy_simd(tree);
        return false;
    }
    
    if (strcmp((char*)found_value, value) != 0) {
        printf("  ❌ Find failed - wrong value found\n");
        bplus_tree_destroy_simd(tree);
        return false;
    }
    
    printf("  ✅ Find successful - correct value found\n");
    
    // Cleanup
    bplus_tree_destroy_simd(tree);
    printf("  ✅ Tree destroyed successfully\n");
    
    return true;
}

/**
 * @brief Tests multiple insert operations.
 */
bool test_multiple_inserts() {
    printf("  Creating SIMD B+ Tree...\n");
    BPlusTree *tree = bplus_tree_create_simd(8, compare_ints, NULL);
    
    if (tree == NULL) {
        printf("  ❌ Failed to create tree\n");
        return false;
    }
    
    printf("  ✅ Tree created successfully\n");
    
    const int test_size = 100;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "Value-%d", i);
    }
    
    // Insert all items
    printf("  Inserting %d items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        int result = bplus_tree_insert_simd(tree, &keys[i], values[i]);
        if (result != 0) {
            printf("  ❌ Insert failed for key %d\n", keys[i]);
            bplus_tree_destroy_simd(tree);
            for (int j = 0; j < test_size; j++) {
                free(values[j]);
            }
            free(keys);
            free(values);
            return false;
        }
    }
    
    printf("  ✅ All inserts successful\n");
    
    // Verify all items can be found
    printf("  Verifying all items...\n");
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find_simd(tree, &keys[i]);
        if (found == NULL || strcmp((char*)found, values[i]) != 0) {
            printf("  ❌ Verification failed for key %d\n", keys[i]);
            bplus_tree_destroy_simd(tree);
            for (int j = 0; j < test_size; j++) {
                free(values[j]);
            }
            free(keys);
            free(values);
            return false;
        }
    }
    
    printf("  ✅ All items verified successfully\n");
    
    // Cleanup
    bplus_tree_destroy_simd(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    printf("  ✅ Tree destroyed successfully\n");
    
    return true;
}

/**
 * @brief Tests range query operations.
 */
bool test_range_queries() {
    printf("  Creating SIMD B+ Tree...\n");
    BPlusTree *tree = bplus_tree_create_simd(8, compare_ints, NULL);
    
    if (tree == NULL) {
        printf("  ❌ Failed to create tree\n");
        return false;
    }
    
    printf("  ✅ Tree created successfully\n");
    
    const int test_size = 50;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i * 2; // Even numbers: 0, 2, 4, 6, ...
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "Value-%d", keys[i]);
    }
    
    // Insert all items
    printf("  Inserting %d items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert_simd(tree, &keys[i], values[i]);
    }
    
    printf("  ✅ All inserts successful\n");
    
    // Test range query
    int start_key = 10;
    int end_key = 30;
    void* results[20];
    
    printf("  Testing range query from %d to %d...\n", start_key, end_key);
    int count = bplus_tree_find_range_simd(tree, &start_key, &end_key, results, 20);
    
    if (count != 11) { // Should find 11 items: 10, 12, 14, ..., 30
        printf("  ❌ Range query failed - expected 11 items, got %d\n", count);
        bplus_tree_destroy_simd(tree);
        for (int j = 0; j < test_size; j++) {
            free(values[j]);
        }
        free(keys);
        free(values);
        return false;
    }
    
    printf("  ✅ Range query successful - found %d items\n", count);
    
    // Cleanup
    bplus_tree_destroy_simd(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    printf("  ✅ Tree destroyed successfully\n");
    
    return true;
}

/**
 * @brief Tests SIMD performance characteristics.
 */
bool test_simd_performance() {
    printf("  Creating SIMD B+ Tree...\n");
    BPlusTree *tree = bplus_tree_create_simd(16, compare_ints, NULL);
    
    if (tree == NULL) {
        printf("  ❌ Failed to create tree\n");
        return false;
    }
    
    printf("  ✅ Tree created successfully\n");
    
    const int test_size = 1000;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "PerfValue-%d", i);
    }
    
    // Test insertion performance
    printf("  Testing insertion performance...\n");
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert_simd(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Test find performance
    printf("  Testing find performance...\n");
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find_simd(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double find_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Find: %.4f seconds (%.1f items/sec)\n", 
           find_time, test_size / find_time);
    
    printf("  ✅ Performance test completed successfully\n");
    
    // Cleanup
    bplus_tree_destroy_simd(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    printf("  ✅ Tree destroyed successfully\n");
    
    return true;
}

// --- Main Test Runner ---

int main() {
    printf("🚀 SIMD B+ Tree Test Suite\n");
    printf("===========================\n\n");
    
    printf("Testing SIMD-optimized B+ Tree implementation...\n\n");
    
    // Run all tests
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_basic_insert_find);
    RUN_TEST(test_multiple_inserts);
    RUN_TEST(test_range_queries);
    RUN_TEST(test_simd_performance);
    
    printf("🎯 SIMD Test Suite Complete!\n");
    printf("Tests passed: %d, Tests failed: %d\n", tests_passed, tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}
