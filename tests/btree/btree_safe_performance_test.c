/**
 * btree_safe_performance_test.c
 *
 * A safe performance test for the pthread B+ Tree implementation
 * that avoids memory management issues.
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
bool test_basic_operations();
bool test_small_scale_performance();
bool test_tree_integrity();

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
 * @brief Tests basic tree operations with a small dataset.
 */
bool test_basic_operations() {
    // Use NULL for destroy_value to avoid double-free issues
    BPlusTree *tree = bplus_tree_create(4, compare_ints, NULL);
    
    const int test_size = 100;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "BasicValue-%d", i);
    }
    
    // Test insertion
    printf("  Testing insertion...\n");
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        int result = bplus_tree_insert(tree, &keys[i], values[i]);
        assert(result == 0);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Test find
    printf("  Testing find...\n");
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
        assert(strcmp((char*)found, values[i]) == 0);
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
 * @brief Tests small-scale performance characteristics.
 */
bool test_small_scale_performance() {
    // Use NULL for destroy_value to avoid double-free issues
    BPlusTree *tree = bplus_tree_create(8, compare_ints, NULL);
    
    const int test_size = 500;
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
 * @brief Tests tree integrity with various operations.
 */
bool test_tree_integrity() {
    // Use NULL for destroy_value to avoid double-free issues
    BPlusTree *tree = bplus_tree_create(6, compare_ints, NULL);
    
    const int test_size = 200;
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    printf("  Generating %d test items...\n", test_size);
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "IntegrityValue-%d", i);
    }
    
    // Insert items
    printf("  Inserting items...\n");
    for (int i = 0; i < test_size; i++) {
        int result = bplus_tree_insert(tree, &keys[i], values[i]);
        assert(result == 0);
    }
    
    // Verify all items can be found
    printf("  Verifying tree integrity...\n");
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
        assert(strcmp((char*)found, values[i]) == 0);
    }
    
    printf("  Tree integrity verified successfully\n");
    
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
    printf("🚀 Safe B+ Tree Performance Test Suite\n");
    printf("======================================\n\n");
    
    printf("Testing pthread B+ Tree implementation with safe memory management...\n\n");
    
    // Run all tests
    RUN_TEST(test_basic_operations);
    RUN_TEST(test_small_scale_performance);
    RUN_TEST(test_tree_integrity);
    
    printf("🎯 Safe Performance Test Suite Complete!\n");
    printf("Tests passed: %d, Tests failed: %d\n", tests_passed, tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}
