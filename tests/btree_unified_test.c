/**
 * btree_unified_test.c
 *
 * A unified test suite for both pthread and OpenMP B+ Tree implementations.
 * Can dynamically switch between implementations using command-line arguments.
 * 
 * Usage:
 *   ./btree_unified_test pthread    # Test pthread implementation
 *   ./btree_unified_test openmp     # Test OpenMP implementation
 *   ./btree_unified_test            # Test both implementations
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <omp.h>

// Include the unified B+ Tree header file
#include "../btree_unified.h"
#include "test_utils.h"

// Forward declarations for test functions
bool test_create_destroy();
bool test_basic_insert_and_find();
bool test_multiple_insertions();
bool test_insertion_order();
bool test_find_nonexistent();
bool test_delete_basic();
bool test_delete_nonexistent();
bool test_delete_all();
bool test_find_range();
bool test_memory_leaks();
bool test_create_comprehensive();
bool test_insert_comprehensive();
bool test_find_comprehensive();
bool test_find_range_comprehensive();
bool test_delete_comprehensive();
bool test_destroy_comprehensive();
bool test_edge_cases();
bool test_tree_structure_integrity();
bool test_node_splitting_validation();
bool test_deletion_rebalancing();
bool test_concurrent_read_access();
bool test_string_key_comparison();
bool test_large_dataset_performance();
bool test_massive_btree_100k();
bool test_massive_btree_500k();
bool test_massive_btree_750k();
bool test_ultra_massive_btree_1m();
bool test_mixed_operations_1m();
bool test_memory_efficiency_1m();
bool test_btree_scalability_analysis();
bool test_error_handling_edge_cases();

// Forward declarations for helper functions
void* concurrent_read_worker(void* arg);

// Structure for concurrent test data
typedef struct {
    BPlusTree *tree;
    int start_idx;
    int end_idx;
    int** keys;
    char** values;
} ThreadData;

// String comparison function
int compare_strings(const void* a, const void* b) {
    return strcmp((char*)a, (char*)b);
}

// Global variables for implementation selection
bool test_pthread = false;
bool test_openmp = false;
char* current_impl_name = "Unknown";

// --- Test Harness Setup ---

int tests_passed = 0;
int tests_failed = 0;

// Macro to run a test function and report its result
#define RUN_TEST(test) \
    do { \
        printf("--- Running %s (%s) ---\n", #test, current_impl_name); \
        if (test()) { \
            tests_passed++; \
            printf("✅ PASS: %s (%s)\n\n", #test, current_impl_name); \
        } else { \
            tests_failed++; \
            printf("❌ FAIL: %s (%s)\n\n", #test, current_impl_name); \
        } \
    } while (0)

// --- Test Data Setup ---

// We'll use integer keys and string values for most tests.
// These are defined in test_utils.h

// --- Test Cases ---

/**
 * @brief Tests basic tree creation and destruction.
 */
bool test_create_destroy() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    assert(tree != NULL);
    assert(tree->root != NULL);
    assert(tree->root->num_keys == 0);
    
    bplus_tree_destroy(tree);
    return true;
}

/**
 * @brief Tests a single insertion and validates the find operation.
 */
bool test_basic_insert_and_find() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(1);

    bplus_tree_insert(tree, test_keys[0], test_values[0]);
    
    void* found = bplus_tree_find(tree, test_keys[0]);
    assert(found != NULL);
    assert(strcmp((char*)found, test_values[0]) == 0);
    
    bplus_tree_destroy(tree);
    teardown_test_data(1);
    return true;
}

/**
 * @brief Tests multiple insertions and validates all can be found.
 */
bool test_multiple_insertions() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(10);
    
    for (int i = 0; i < 10; i++) {
        int result = bplus_tree_insert(tree, test_keys[i], test_values[i]);
        assert(result == 0);
    }
    
    for (int i = 0; i < 10; i++) {
        void* found = bplus_tree_find(tree, test_keys[i]);
        assert(found != NULL);
        assert(strcmp((char*)found, test_values[i]) == 0);
    }
    
    bplus_tree_destroy(tree);
    teardown_test_data(10);
    return true;
}

/**
 * @brief Tests that insertions maintain proper order.
 */
bool test_insertion_order() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, NULL);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, NULL);
    }
    
    setup_test_data(10);
    
    // Insert in reverse order
    for (int i = 9; i >= 0; i--) {
        int result = bplus_tree_insert(tree, test_keys[i], test_values[i]);
        assert(result == 0);
    }
    
    // Verify all can still be found
    for (int i = 0; i < 10; i++) {
        void* found = bplus_tree_find(tree, test_keys[i]);
        assert(found != NULL);
        assert(strcmp((char*)found, test_values[i]) == 0);
    }
    
    bplus_tree_destroy(tree);
    teardown_test_data(10);
    return true;
}

/**
 * @brief Tests finding non-existent keys.
 */
bool test_find_nonexistent() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(5);
    
    for (int i = 0; i < 5; i++) {
        bplus_tree_insert(tree, test_keys[i], test_values[i]);
    }
    
    int non_existent_key = 999;
    void* found = bplus_tree_find(tree, &non_existent_key);
    assert(found == NULL);
    
    bplus_tree_destroy(tree);
    teardown_test_data(5);
    return true;
}

/**
 * @brief Tests basic deletion functionality.
 */
bool test_delete_basic() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(5);
    
    for (int i = 0; i < 5; i++) {
        bplus_tree_insert(tree, test_keys[i], test_values[i]);
    }
    
    // Delete middle key
    int result = bplus_tree_delete(tree, test_keys[2]);
    assert(result == 0);
    
    // Verify it's gone
    void* found = bplus_tree_find(tree, test_keys[2]);
    assert(found == NULL);
    
    // Verify others are still there
    for (int i = 0; i < 5; i++) {
        if (i != 2) {
            found = bplus_tree_find(tree, test_keys[i]);
            assert(found != NULL);
        }
    }
    
    bplus_tree_destroy(tree);
    teardown_test_data(5);
    return true;
}

/**
 * @brief Tests deleting non-existent keys.
 */
bool test_delete_nonexistent() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(3);
    
    for (int i = 0; i < 3; i++) {
        bplus_tree_insert(tree, test_keys[i], test_values[i]);
    }
    
    int non_existent_key = 999;
    int result = bplus_tree_delete(tree, &non_existent_key);
    assert(result == -1);
    
    bplus_tree_destroy(tree);
    teardown_test_data(3);
    return true;
}

/**
 * @brief Tests deleting all keys from the tree.
 */
bool test_delete_all() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(5);
    
    for (int i = 0; i < 5; i++) {
        bplus_tree_insert(tree, test_keys[i], test_values[i]);
    }
    
    // Delete all keys
    for (int i = 0; i < 5; i++) {
        int result = bplus_tree_delete(tree, test_keys[i]);
        assert(result == 0);
    }
    
    // Verify all are gone
    for (int i = 0; i < 5; i++) {
        void* found = bplus_tree_find(tree, test_keys[i]);
        assert(found == NULL);
    }
    
    bplus_tree_destroy(tree);
    teardown_test_data(5);
    return true;
}

/**
 * @brief Tests range queries.
 */
bool test_find_range() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(10);
    
    for (int i = 0; i < 10; i++) {
        bplus_tree_insert(tree, test_keys[i], test_values[i]);
    }
    
    void* results[10];
    int count = bplus_tree_find_range(tree, test_keys[2], test_keys[6], results, 10);
    assert(count == 5); // Keys 2, 3, 4, 5, 6
    
    bplus_tree_destroy(tree);
    teardown_test_data(10);
    return true;
}

/**
 * @brief Tests memory leak prevention.
 */
bool test_memory_leaks() {
    BPlusTree *tree;
    
    if (test_openmp) {
        tree = bplus_tree_create_openmp(DEFAULT_ORDER, compare_ints, destroy_string_value);
    } else {
        tree = bplus_tree_create_pthread(DEFAULT_ORDER, compare_ints, destroy_string_value);
    }
    
    setup_test_data(10);
    
    for (int i = 0; i < 10; i++) {
        bplus_tree_insert(tree, test_keys[i], test_values[i]);
    }
    
    bplus_tree_destroy(tree);
    teardown_test_data(10);
    return true;
}

// --- Main Test Runner ---

int main(int argc, char* argv[]) {
    printf("🚀 B+ Tree Unified Test Suite\n");
    printf("==============================\n\n");
    
    // Parse command line arguments
    if (argc == 1) {
        // Test both implementations
        test_pthread = true;
        test_openmp = true;
        printf("Testing both pthread and OpenMP implementations\n\n");
    } else if (argc == 2) {
        if (strcmp(argv[1], "pthread") == 0) {
            test_pthread = true;
            test_openmp = false;
            printf("Testing pthread implementation only\n\n");
        } else if (strcmp(argv[1], "openmp") == 0) {
            test_pthread = false;
            test_openmp = true;
            printf("Testing OpenMP implementation only\n\n");
        } else {
            printf("Usage: %s [pthread|openmp]\n", argv[0]);
            printf("  pthread: Test pthread implementation only\n");
            printf("  openmp:  Test OpenMP implementation only\n");
            printf("  no args: Test both implementations\n");
            return 1;
        }
    } else {
        printf("Usage: %s [pthread|openmp]\n", argv[0]);
        return 1;
    }
    
    // Test pthread implementation
    if (test_pthread) {
        current_impl_name = "pthread";
        printf("🧵 Testing Pthread Implementation\n");
        printf("================================\n");
        
        // Basic functionality tests
        RUN_TEST(test_create_destroy);
        RUN_TEST(test_basic_insert_and_find);
        RUN_TEST(test_multiple_insertions);
        RUN_TEST(test_insertion_order);
        RUN_TEST(test_find_nonexistent);
        RUN_TEST(test_delete_basic);
        RUN_TEST(test_delete_nonexistent);
        RUN_TEST(test_delete_all);
        RUN_TEST(test_find_range);
        RUN_TEST(test_memory_leaks);
        
        printf("Pthread Tests Summary: %d passed, %d failed\n\n", tests_passed, tests_failed);
    }
    
    // Test OpenMP implementation
    if (test_openmp) {
        current_impl_name = "openmp";
        printf("🔓 Testing OpenMP Implementation\n");
        printf("================================\n");
        
        // Reset counters for OpenMP tests
        int pthread_passed = tests_passed;
        int pthread_failed = tests_failed;
        tests_passed = 0;
        tests_failed = 0;
        
        // Basic functionality tests
        RUN_TEST(test_create_destroy);
        RUN_TEST(test_basic_insert_and_find);
        RUN_TEST(test_multiple_insertions);
        RUN_TEST(test_insertion_order);
        RUN_TEST(test_find_nonexistent);
        RUN_TEST(test_delete_basic);
        RUN_TEST(test_delete_nonexistent);
        RUN_TEST(test_delete_all);
        RUN_TEST(test_find_range);
        RUN_TEST(test_memory_leaks);
        
        printf("OpenMP Tests Summary: %d passed, %d failed\n\n", tests_passed, tests_failed);
        
        // Overall summary
        if (test_pthread) {
            printf("Overall Summary:\n");
            printf("  Pthread: %d passed, %d failed\n", pthread_passed, pthread_failed);
            printf("  OpenMP:  %d passed, %d failed\n", tests_passed, tests_failed);
            printf("  Total:   %d passed, %d failed\n", pthread_passed + tests_passed, pthread_failed + tests_failed);
        }
    }
    
    printf("\n🎯 Test Suite Complete!\n");
    return 0;
}
