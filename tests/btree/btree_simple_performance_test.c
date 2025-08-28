/**
 * btree_simple_performance_test.c
 *
 * A simple performance test for the pthread B+ Tree implementation
 * to demonstrate working performance characteristics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

// Include the pthread B+ Tree header file
#include "../btree.h"
#include "test_utils.h"

// Forward declarations for test functions
bool test_small_dataset();
bool test_medium_dataset();
bool test_large_dataset();
bool test_mixed_operations();

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
 * @brief Tests performance with a small dataset (1000 items).
 */
bool test_small_dataset() {
    BPlusTree *tree = bplus_tree_create(8, compare_ints, destroy_string_value);
    
    const int test_size = 1000;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "SmallValue-%d", i);
    }
    
    // Test insertion performance
    printf("  Testing insertion performance...\n");
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Test find performance
    printf("  Testing find performance...\n");
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double find_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Find: %.4f seconds (%.1f items/sec)\n", 
           find_time, test_size / find_time);
    
    // Test range query performance
    printf("  Testing range query performance...\n");
    void* results[100];
    start = clock();
    int count = bplus_tree_find_range(tree, &keys[100], &keys[199], results, 100);
    end = clock();
    double range_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Range query: %.4f seconds, found %d items\n", range_time, count);
    
    assert(count == 100);
    
    // Cleanup
    bplus_tree_destroy(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    return true;
}

/**
 * @brief Tests performance with a medium dataset (10000 items).
 */
bool test_medium_dataset() {
    BPlusTree *tree = bplus_tree_create(16, compare_ints, destroy_string_value);
    
    const int test_size = 10000;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "MediumValue-%d", i);
    }
    
    // Test insertion performance
    printf("  Testing insertion performance...\n");
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Test find performance
    printf("  Testing find performance...\n");
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double find_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Find: %.4f seconds (%.1f items/sec)\n", 
           find_time, test_size / find_time);
    
    // Cleanup
    bplus_tree_destroy(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    return true;
}

/**
 * @brief Tests performance with a large dataset (100000 items).
 */
bool test_large_dataset() {
    BPlusTree *tree = bplus_tree_create(32, compare_ints, destroy_string_value);
    
    const int test_size = 100000;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "LargeValue-%d", i);
    }
    
    // Test insertion performance
    printf("  Testing insertion performance...\n");
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Test find performance
    printf("  Testing find performance...\n");
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double find_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Find: %.4f seconds (%.1f items/sec)\n", 
           find_time, test_size / find_time);
    
    // Cleanup
    bplus_tree_destroy(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    return true;
}

/**
 * @brief Tests mixed operations performance.
 */
bool test_mixed_operations() {
    BPlusTree *tree = bplus_tree_create(16, compare_ints, destroy_string_value);
    
    const int test_size = 5000;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "MixedValue-%d", i);
    }
    
    // Test mixed operations
    printf("  Testing mixed operations performance...\n");
    clock_t start = clock();
    
    // Insert half the items
    for (int i = 0; i < test_size / 2; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    
    // Find some items
    for (int i = 0; i < test_size / 4; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    
    // Insert more items
    for (int i = test_size / 2; i < test_size; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    
    // Find all items
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    
    clock_t end = clock();
    double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Mixed operations: %.4f seconds\n", total_time);
    
    // Cleanup
    bplus_tree_destroy(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    return true;
}

// --- Main Test Runner ---

int main() {
    printf("🚀 Simple B+ Tree Performance Test Suite\n");
    printf("========================================\n\n");
    
    printf("Testing pthread B+ Tree implementation performance...\n\n");
    
    // Run all tests
    RUN_TEST(test_small_dataset);
    RUN_TEST(test_medium_dataset);
    RUN_TEST(test_large_dataset);
    RUN_TEST(test_mixed_operations);
    
    printf("🎯 Performance Test Suite Complete!\n");
    printf("Tests passed: %d, Tests failed: %d\n", tests_passed, tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}
