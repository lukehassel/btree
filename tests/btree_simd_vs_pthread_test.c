/**
 * btree_simd_vs_pthread_test.c
 *
 * Performance comparison between SIMD and pthread B+ Tree implementations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>

// Include both implementations
#include "../btree.h"
#include "../btree_simd.h"
#include "test_utils.h"

// Forward declarations for test functions
void benchmark_pthread_implementation(int test_size);
void benchmark_simd_implementation(int test_size);
void compare_performance(int test_size);

// --- Performance Benchmarking Functions ---

/**
 * @brief Benchmarks the pthread B+ Tree implementation.
 */
void benchmark_pthread_implementation(int test_size) {
    printf("🧵 Pthread B+ Tree Implementation:\n");
    printf("  =================================\n");
    
    BPlusTree *tree = bplus_tree_create(16, compare_ints, NULL);
    if (tree == NULL) {
        printf("  ❌ Failed to create pthread tree\n");
        return;
    }
    
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "PthreadValue-%d", i);
    }
    
    // Benchmark insertion
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Benchmark find
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double find_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Find: %.4f seconds (%.1f items/sec)\n", 
           find_time, test_size / find_time);
    
    // Benchmark range query
    void* results[100];
    start = clock();
    int count = bplus_tree_find_range(tree, &keys[100], &keys[199], results, 100);
    end = clock();
    double range_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Range query: %.4f seconds, found %d items\n", range_time, count);
    
    // Cleanup
    bplus_tree_destroy(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    printf("\n");
}

/**
 * @brief Benchmarks the SIMD B+ Tree implementation.
 */
void benchmark_simd_implementation(int test_size) {
    printf("🚀 SIMD B+ Tree Implementation:\n");
    printf("  =============================\n");
    
    BPlusTree *tree = bplus_tree_create_simd(16, compare_ints, NULL);
    if (tree == NULL) {
        printf("  ❌ Failed to create SIMD tree\n");
        return;
    }
    
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    // Generate test data
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "SIMDValue-%d", i);
    }
    
    // Benchmark insertion
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert_simd(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Insertion: %.4f seconds (%.1f items/sec)\n", 
           insert_time, test_size / insert_time);
    
    // Benchmark find
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find_simd(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double find_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Find: %.4f seconds (%.1f items/sec)\n", 
           find_time, test_size / find_time);
    
    // Benchmark range query
    void* results[100];
    start = clock();
    int count = bplus_tree_find_range_simd(tree, &keys[100], &keys[199], results, 100);
    end = clock();
    double range_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Range query: %.4f seconds, found %d items\n", range_time, count);
    
    // Cleanup
    bplus_tree_destroy_simd(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    printf("\n");
}

/**
 * @brief Compares performance between both implementations.
 */
void compare_performance(int test_size) {
    printf("📊 Performance Comparison Summary:\n");
    printf("==================================\n");
    printf("Test size: %d items\n\n", test_size);
    
    // Run both benchmarks
    benchmark_pthread_implementation(test_size);
    benchmark_simd_implementation(test_size);
    
    printf("🎯 Comparison complete!\n");
    printf("Note: Performance may vary based on:\n");
    printf("- Hardware architecture (x86_64 vs ARM64)\n");
    printf("- Compiler optimizations\n");
    printf("- Cache locality and memory access patterns\n");
    printf("- SIMD instruction set support\n\n");
}

// --- Main Test Runner ---

int main() {
    printf("🚀 SIMD vs Pthread B+ Tree Performance Comparison\n");
    printf("=================================================\n\n");
    
    // Test different sizes
    int test_sizes[] = {1000, 5000, 10000};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("🔍 Running performance comparison with %d items:\n", test_sizes[i]);
        printf("=");
        for (int j = 0; j < 50; j++) printf("=");
        printf("\n\n");
        
        compare_performance(test_sizes[i]);
        
        if (i < num_tests - 1) {
            printf("\n");
        }
    }
    
    printf("🎯 All performance comparisons completed!\n");
    return 0;
}
