/**
 * btree_performance_comparison.c
 *
 * Performance comparison between pthread and OpenMP B+ Tree implementations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <omp.h>

// Include both implementations
#include "../btree.h"
#include "../btree_openmp.h"
#include "test_utils.h"

// Performance test parameters
#define SMALL_TEST_SIZE 1000
#define MEDIUM_TEST_SIZE 10000
#define LARGE_TEST_SIZE 100000

// Performance test function for pthread
double test_pthread_performance(int test_size) {
    BPlusTree *tree = bplus_tree_create(8, compare_ints, NULL);
    
    // Generate test data
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "PthreadValue-%d", i);
    }
    
    // Measure insertion performance
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Measure search performance
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double search_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Measure deletion performance
    start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_delete(tree, &keys[i]);
    }
    end = clock();
    double delete_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Cleanup
    bplus_tree_destroy(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    printf("  Pthread: Insert=%.4fs (%.1f items/sec), Search=%.4fs (%.1f items/sec), Delete=%.4fs (%.1f items/sec)\n",
           insert_time, test_size / insert_time,
           search_time, test_size / search_time,
           delete_time, test_size / delete_time);
    
    return insert_time + search_time + delete_time;
}

// Performance test function for OpenMP
double test_openmp_performance(int test_size) {
    BPlusTreeOpenMP *tree = bplus_tree_create_openmp(8, compare_ints, NULL);
    
    // Generate test data
    int* keys = malloc(test_size * sizeof(int));
    char** values = malloc(test_size * sizeof(char*));
    
    for (int i = 0; i < test_size; i++) {
        keys[i] = i;
        values[i] = malloc(20 * sizeof(char));
        sprintf(values[i], "OpenMPValue-%d", i);
    }
    
    // Measure insertion performance
    clock_t start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_insert_openmp(tree, &keys[i], values[i]);
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Measure search performance
    start = clock();
    for (int i = 0; i < test_size; i++) {
        void* found = bplus_tree_find_openmp(tree, &keys[i]);
        assert(found != NULL);
    }
    end = clock();
    double search_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Measure deletion performance
    start = clock();
    for (int i = 0; i < test_size; i++) {
        bplus_tree_delete_openmp(tree, &keys[i]);
    }
    end = clock();
    double delete_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Cleanup
    bplus_tree_destroy_openmp(tree);
    for (int i = 0; i < test_size; i++) {
        free(values[i]);
    }
    free(keys);
    free(values);
    
    printf("  OpenMP:  Insert=%.4fs (%.1f items/sec), Search=%.4fs (%.1f items/sec), Delete=%.4fs (%.1f items/sec)\n",
           insert_time, test_size / insert_time,
           search_time, test_size / search_time,
           delete_time, test_size / delete_time);
    
    return insert_time + search_time + delete_time;
}

// Main performance comparison
int main() {
    printf("🚀 B+ Tree Performance Comparison\n");
    printf("==================================\n\n");
    
    // Set OpenMP threads
    omp_set_num_threads(4);
    printf("Using %d OpenMP threads\n\n", omp_get_max_threads());
    
    // Test small dataset
    printf("📊 Small Dataset (%d items):\n", SMALL_TEST_SIZE);
    double pthread_small = test_pthread_performance(SMALL_TEST_SIZE);
    double openmp_small = test_openmp_performance(SMALL_TEST_SIZE);
    printf("  Total time - Pthread: %.4fs, OpenMP: %.4fs\n", pthread_small, openmp_small);
    printf("  Speedup: %.2fx\n\n", pthread_small / openmp_small);
    
    // Test medium dataset
    printf("📊 Medium Dataset (%d items):\n", MEDIUM_TEST_SIZE);
    double pthread_medium = test_pthread_performance(MEDIUM_TEST_SIZE);
    double openmp_medium = test_openmp_performance(MEDIUM_TEST_SIZE);
    printf("  Total time - Pthread: %.4fs, OpenMP: %.4fs\n", pthread_medium, openmp_medium);
    printf("  Speedup: %.2fx\n\n", pthread_medium / openmp_medium);
    
    // Test large dataset
    printf("📊 Large Dataset (%d items):\n", LARGE_TEST_SIZE);
    double pthread_large = test_pthread_performance(LARGE_TEST_SIZE);
    double openmp_large = test_openmp_performance(LARGE_TEST_SIZE);
    printf("  Total time - Pthread: %.4fs, OpenMP: %.4fs\n", pthread_large, openmp_large);
    printf("  Speedup: %.2fx\n\n", pthread_large / openmp_large);
    
    // Summary
    printf("🎯 Performance Summary:\n");
    printf("  Small dataset:  OpenMP is %.2fx %s\n", 
           pthread_small / openmp_small, 
           openmp_small < pthread_small ? "faster" : "slower");
    printf("  Medium dataset: OpenMP is %.2fx %s\n", 
           pthread_medium / openmp_medium, 
           openmp_medium < pthread_medium ? "faster" : "slower");
    printf("  Large dataset:  OpenMP is %.2fx %s\n", 
           pthread_large / openmp_large, 
           openmp_large < pthread_large ? "faster" : "slower");
    
    return 0;
}
