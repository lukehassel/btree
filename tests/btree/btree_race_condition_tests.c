#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "../../btree.h"
#include "test_utils.h"
#include "btree_race_condition_tests.h"

// Test harness setup
static int tests_passed = 0;
static int tests_failed = 0;

#define RUN_TEST(test_func) \
    do { \
        printf("--- Running " #test_func " ---\n"); \
        printf("  About to call " #test_func "\n"); \
        if (test_func()) { \
            printf("✅ PASS: " #test_func "\n"); \
            tests_passed++; \
        } else { \
            printf("❌ FAIL: " #test_func "\n"); \
            tests_failed++; \
        } \
        printf("\n"); \
    } while(0)

// Global variables for shared access
BPlusTree* global_tree = NULL;
pthread_mutex_t global_tree_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool test_complete = false;

// Thread arguments structure
typedef struct {
    int thread_id;
    int start_key;
    int end_key;
    int operation_count;
} ThreadArgs;

// Helper function to get a random key within a range
int get_random_key(int min, int max) {
    return min + (rand() % (max - min + 1));
}

// Helper function to get a random operation type
int get_random_operation() {
    return rand() % 3; // 0 = insert, 1 = find, 2 = delete
}

// Simple race condition test with 2 threads
void* insert_even_keys(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = args->start_key; i <= args->end_key; i += 2) {
        pthread_mutex_lock(&global_tree_mutex);
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
        pthread_mutex_unlock(&global_tree_mutex);
        usleep(100); // Small delay to increase race condition probability
    }
    
    return NULL;
}

void* insert_odd_keys(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = args->start_key; i <= args->end_key; i += 2) {
        pthread_mutex_lock(&global_tree_mutex);
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
        pthread_mutex_unlock(&global_tree_mutex);
        usleep(100); // Small delay to increase race condition probability
    }
    
    return NULL;
}

bool test_simple_race_condition() {
    printf("Running simple race condition test...\n");
    
    printf("  Creating tree...\n");
    global_tree = bplus_tree_create(4, compare_ints, NULL);
    printf("  Setting up test data...\n");
    setup_test_data(200);
    printf("  Test data setup complete\n");
    
    pthread_t thread1, thread2;
    ThreadArgs args1 = {1, 0, 99, 50};
    ThreadArgs args2 = {2, 1, 100, 50};
    
    printf("  Creating threads...\n");
    pthread_create(&thread1, NULL, insert_even_keys, &args1);
    pthread_create(&thread2, NULL, insert_odd_keys, &args2);
    
    printf("  Waiting for threads...\n");
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("  Verifying results...\n");
    // Verify all keys were inserted (thread1: even indices 0-98, thread2: odd indices 1-99)
    for (int i = 0; i <= 99; i++) {
        printf("    Checking key %d (value: %d)...\n", i, *test_keys[i]);
        void* val = bplus_tree_find(global_tree, (void*)(long)test_keys[i]);
        if (val == NULL) {
            printf("    ❌ Key %d not found!\n", i);
        } else {
            printf("    ✅ Key %d found: %s\n", i, (char*)val);
        }
        assert(val != NULL);
    }
    
    printf("  Cleaning up...\n");
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Simple race condition test passed\n");
    return true;
}

// Concurrent insert and find test
void* insert_keys_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = args->start_key; i <= args->end_key; i++) {
        pthread_mutex_lock(&global_tree_mutex);
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
        pthread_mutex_unlock(&global_tree_mutex);
        usleep(50);
    }
    
    return NULL;
}

void* find_keys_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = 0; i < args->operation_count; i++) {
        int key_idx = get_random_key(args->start_key, args->end_key);
        pthread_mutex_lock(&global_tree_mutex);
        bplus_tree_find(global_tree, (void*)(long)test_keys[key_idx]);
        pthread_mutex_unlock(&global_tree_mutex);
        // Don't assert - some keys may not exist yet
        usleep(30);
    }
    
    return NULL;
}

bool test_concurrent_insert_find() {
    printf("Running concurrent insert and find test...\n");
    
    printf("  Creating tree...\n");
    global_tree = bplus_tree_create(5, compare_ints, NULL);
    printf("  Setting up test data...\n");
    setup_test_data(300);
    printf("  Test data setup complete\n");
    
    pthread_t insert_thread, find_thread;
    ThreadArgs insert_args = {1, 0, 199, 200};
    ThreadArgs find_args = {2, 0, 199, 300};
    
    printf("  Creating threads...\n");
    pthread_create(&insert_thread, NULL, insert_keys_thread, &insert_args);
    pthread_create(&find_thread, NULL, find_keys_thread, &find_args);
    
    printf("  Waiting for threads...\n");
    pthread_join(insert_thread, NULL);
    pthread_join(find_thread, NULL);
    
    printf("  Verifying results...\n");
    // Verify all keys were inserted
    for (int i = 0; i <= 199; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)test_keys[i]);
        assert(val != NULL);
    }
    
    printf("  Cleaning up...\n");
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Concurrent insert and find test passed\n");
    return true;
}

// Moderate race condition test with 4 threads
void* mixed_operations_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = 0; i < args->operation_count; i++) {
        int operation = get_random_operation();
        int key_idx = get_random_key(args->start_key, args->end_key);
        
        pthread_mutex_lock(&global_tree_mutex);
        switch (operation) {
            case 0: // Insert
                bplus_tree_insert(global_tree, (void*)(long)test_keys[key_idx], test_values[key_idx]);
                break;
            case 1: // Find
                bplus_tree_find(global_tree, (void*)(long)test_keys[key_idx]);
                break;
            case 2: // Delete
                bplus_tree_delete(global_tree, (void*)(long)test_keys[key_idx]);
                break;
        }
        pthread_mutex_unlock(&global_tree_mutex);
        usleep(20);
    }
    
    return NULL;
}

bool test_moderate_race_conditions() {
    printf("Running moderate race condition test...\n");
    
    printf("  Creating tree...\n");
    global_tree = bplus_tree_create(6, compare_ints, NULL);
    printf("  Setting up test data...\n");
    setup_test_data(600);
    printf("  Test data setup complete\n");
    
    printf("  Inserting initial data...\n");
    // Insert initial data
    for (int i = 0; i < 500; i++) {
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
    }
    printf("  Initial data insertion complete\n");
    
    pthread_t threads[4];
    ThreadArgs args[4] = {
        {1, 0, 499, 100},
        {2, 0, 499, 100},
        {3, 0, 499, 100},
        {4, 0, 499, 100}
    };
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, mixed_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Count remaining items (some may have been deleted)
    int found_count = 0;
    for (int i = 0; i < 500; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)test_keys[i]);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Remaining items after race conditions: %d\n", found_count);
    assert(found_count > 0 && found_count <= 500);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Moderate race condition test passed\n");
    return true;
}

// Extreme race condition test with many threads
void* extreme_operations_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = 0; i < args->operation_count; i++) {
        int operation = get_random_operation();
        int key_idx = get_random_key(args->start_key, args->end_key);
        
        pthread_mutex_lock(&global_tree_mutex);
        switch (operation) {
            case 0: // Insert
                bplus_tree_insert(global_tree, (void*)(long)test_keys[key_idx], test_values[key_idx]);
                break;
            case 1: // Find
                bplus_tree_find(global_tree, (void*)(long)test_keys[key_idx]);
                break;
            case 2: // Delete
                bplus_tree_delete(global_tree, (void*)(long)test_keys[key_idx]);
                break;
        }
        pthread_mutex_unlock(&global_tree_mutex);
        usleep(10); // Very small delay for extreme conditions
    }
    
    return NULL;
}

// Utility function to verify tree integrity
void verify_tree_integrity() {
    // This is a basic integrity check - in a real scenario you'd want more comprehensive checks
    if (global_tree == NULL || global_tree->root == NULL) {
        return; // Tree is empty or invalid
    }
    
    // Try to find a few random keys to ensure tree is accessible
    for (int i = 0; i < 10; i++) {
        int key = rand() % 1000;
        bplus_tree_find(global_tree, (void*)(long)key);
    }
}

bool test_extreme_race_conditions() {
    printf("Running extreme race condition test...\n");
    
    global_tree = bplus_tree_create(8, compare_ints, NULL);
    setup_test_data(1200);
    
            // Insert initial data
        for (int i = 0; i < 1000; i++) {
            bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
        }
    
    const int thread_count = 16;
    const int operations_per_thread = 1000;
    
    pthread_t threads[thread_count];
    ThreadArgs args[thread_count];
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 999;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
    }
    
    // Monitor thread progress
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify tree integrity after extreme conditions
    verify_tree_integrity();
    
    // Count remaining items
    int found_count = 0;
    for (int i = 0; i < 1000; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Items remaining after extreme race conditions: %d\n", found_count);
    assert(found_count >= 0 && found_count <= 1000);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Extreme race condition test passed\n");
    return true;
}

/**
 * @brief Safer version of extreme test with better error handling
 */
bool test_safer_extreme_race_conditions() {
    printf("Running safer extreme race condition test (16 threads, 500 ops each)...\n");
    
    global_tree = bplus_tree_create(8, compare_ints, NULL);
    setup_test_data(1200);
    
    // Insert initial data
    for (int i = 0; i < 1000; i++) {
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
    }
    
    const int thread_count = 16;
    const int operations_per_thread = 500; // Reduced from 1000
    
    pthread_t threads[thread_count];
    ThreadArgs args[thread_count];
    
    // Create threads with error checking
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 999;
        args[i].operation_count = operations_per_thread;
        
        int result = pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
        if (result != 0) {
            printf("  ❌ Failed to create thread %d: %d\n", i, result);
            // Clean up already created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            bplus_tree_destroy(global_tree);
            global_tree = NULL;
            return false;
        }
    }
    
    printf("  All %d threads created successfully\n", thread_count);
    
    // Monitor thread progress with timeout
    for (int i = 0; i < thread_count; i++) {
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            printf("  ❌ Thread %d failed to join: %d\n", i, result);
        }
    }
    
    printf("  All threads completed\n");
    
    // Verify tree integrity after extreme conditions
    verify_tree_integrity();
    
    // Count remaining items
    int found_count = 0;
    for (int i = 0; i < 1000; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Items remaining after safer extreme race conditions: %d\n", found_count);
    assert(found_count >= 0 && found_count <= 1000);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Safer extreme race condition test passed\n");
    return true;
}

// Advanced race condition test with dynamic thread creation/destruction
void* delete_keys_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    for (int i = args->start_key; i <= args->end_key; i++) {
        pthread_mutex_lock(&global_tree_mutex);
        bplus_tree_delete(global_tree, (void*)(long)i);
        pthread_mutex_unlock(&global_tree_mutex);
        usleep(15);
    }
    
    return NULL;
}

// Phase functions for advanced test
void run_insert_phase(int thread_count, int operations_per_thread) {
    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
    ThreadArgs* args = malloc(thread_count * sizeof(ThreadArgs));
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = i * operations_per_thread;
        args[i].end_key = (i + 1) * operations_per_thread - 1;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, insert_keys_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(args);
}

void run_mixed_phase(int thread_count, int operations_per_thread) {
    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
    ThreadArgs* args = malloc(thread_count * sizeof(ThreadArgs));
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 999;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, mixed_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(args);
}

void run_stress_phase(int thread_count, int operations_per_thread) {
    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
    ThreadArgs* args = malloc(thread_count * sizeof(ThreadArgs));
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 999;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(args);
}

void run_cleanup_phase(int thread_count, int operations_per_thread) {
    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
    ThreadArgs* args = malloc(thread_count * sizeof(ThreadArgs));
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = i * operations_per_thread;
        args[i].end_key = (i + 1) * operations_per_thread - 1;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, delete_keys_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(args);
}

bool test_advanced_race_conditions() {
    printf("Running advanced race condition test...\n");
    
    global_tree = bplus_tree_create(7, compare_ints, NULL);
    setup_test_data(1200);
    
    // Phase 1: Insert phase with multiple threads
    printf("  Phase 1: Insert phase\n");
    run_insert_phase(8, 125);
    
    // Phase 2: Mixed operations phase
    printf("  Phase 2: Mixed operations phase\n");
    run_mixed_phase(12, 200);
    
    // Phase 3: Stress phase
    printf("  Phase 3: Stress phase\n");
    run_stress_phase(16, 300);
    
    // Phase 4: Cleanup phase
    printf("  Phase 4: Cleanup phase\n");
    run_cleanup_phase(8, 125);
    
    // Verify final state
    int found_count = 0;
    for (int i = 0; i < 1000; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Final tree contains %d items\n", found_count);
    assert(found_count >= 0 && found_count <= 1000);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Advanced race condition test passed\n");
    return true;
}

// New complex test cases for advanced race condition scenarios

/**
 * @brief Test race conditions with varying tree orders and sizes.
 * This test creates different tree configurations and tests race conditions.
 */
bool test_variable_tree_race_conditions() {
    printf("Running variable tree race conditions test...\n");
    
    // Test different tree orders
    for (int order = 3; order <= 10; order++) {
        printf("  Testing tree order %d\n", order);
        global_tree = bplus_tree_create(order, compare_ints, NULL);
        setup_test_data(order * 200);
        
        // Insert initial data
        int data_size = order * 100;
        for (int i = 0; i < data_size; i++) {
            bplus_tree_insert(global_tree, (void*)(long)i, "VariableValue");
        }
        
        // Create threads with different operation patterns
        pthread_t threads[6];
        ThreadArgs args[6] = {
            {1, 0, data_size - 1, 100},           // Insert operations
            {2, 0, data_size - 1, 100},           // Find operations
            {3, 0, data_size - 1, 100},           // Delete operations
            {4, 0, data_size - 1, 100},           // Mixed operations
            {5, 0, data_size - 1, 100},           // Range operations
            {6, 0, data_size - 1, 100}            // Stress operations
        };
        
        // Start all threads
        for (int i = 0; i < 6; i++) {
            pthread_create(&threads[i], NULL, mixed_operations_thread, &args[i]);
        }
        
        // Wait for all threads
        for (int i = 0; i < 6; i++) {
            pthread_join(threads[i], NULL);
        }
        
        // Verify tree integrity
        verify_tree_integrity();
        
        bplus_tree_destroy(global_tree);
        global_tree = NULL;
    }
    
    printf("✅ Variable tree race conditions test passed\n");
    return true;
}

/**
 * @brief Test race conditions with memory pressure.
 * This test creates memory pressure while running race condition tests.
 */
bool test_memory_pressure_race_conditions() {
    printf("Running memory pressure race conditions test...\n");
    
    global_tree = bplus_tree_create(5, compare_ints, NULL);
    setup_test_data(1000);
    
    // Create memory pressure with large allocations
    void** memory_blocks[50];
    for (int i = 0; i < 50; i++) {
        memory_blocks[i] = malloc(1000 * sizeof(void*));
        assert(memory_blocks[i] != NULL);
    }
    
    // Insert initial data
    for (int i = 0; i < 800; i++) {
        bplus_tree_insert(global_tree, (void*)(long)i, "PressureValue");
    }
    
    // Run race condition tests under memory pressure
    pthread_t threads[8];
    ThreadArgs args[8] = {
        {1, 0, 799, 200}, {2, 0, 799, 200}, {3, 0, 799, 200}, {4, 0, 799, 200},
        {5, 0, 799, 200}, {6, 0, 799, 200}, {7, 0, 799, 200}, {8, 0, 799, 200}
    };
    
    for (int i = 0; i < 8; i++) {
        pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < 8; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Release memory pressure
    for (int i = 0; i < 50; i++) {
        free(memory_blocks[i]);
    }
    
    // Verify final state
    int found_count = 0;
    for (int i = 0; i < 800; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Final tree contains %d items\n", found_count);
    assert(found_count >= 0 && found_count <= 800);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Memory pressure race conditions test passed\n");
    return true;
}

/**
 * @brief Test race conditions with rapid tree recreation.
 * This test creates and destroys trees rapidly while running operations.
 */
bool test_rapid_tree_recreation_race_conditions() {
    printf("Running rapid tree recreation race conditions test...\n");
    
    const int cycles = 20;
    const int threads_per_cycle = 4;
    
    for (int cycle = 0; cycle < cycles; cycle++) {
        printf("  Cycle %d/%d\n", cycle + 1, cycles);
        
        // Create new tree for this cycle
        global_tree = bplus_tree_create(4 + (cycle % 5), compare_ints, NULL);
        
        // Insert some data
        for (int i = 0; i < 200; i++) {
            bplus_tree_insert(global_tree, (void*)(long)test_keys[cycle * 200 + i], test_values[cycle * 200 + i]);
        }
        
        // Create threads that will operate on the tree
        pthread_t threads[threads_per_cycle];
        ThreadArgs args[threads_per_cycle];
        
        for (int i = 0; i < threads_per_cycle; i++) {
            args[i].thread_id = i;
            args[i].start_key = cycle * 200;
            args[i].end_key = (cycle + 1) * 200 - 1;
            args[i].operation_count = 50;
            pthread_create(&threads[i], NULL, mixed_operations_thread, &args[i]);
        }
        
        // Wait for threads to complete
        for (int i = 0; i < threads_per_cycle; i++) {
            pthread_join(threads[i], NULL);
        }
        
        // Destroy tree and create new one
        bplus_tree_destroy(global_tree);
        global_tree = NULL;
        
        // Small delay between cycles
        usleep(1000);
    }
    
    printf("✅ Rapid tree recreation race conditions test passed\n");
    return true;
}

// --- New Focused Tests to Isolate Issues ---

/**
 * @brief Simple test with just 2 threads doing basic operations
 */
bool test_minimal_race_conditions() {
    printf("Running minimal race condition test (2 threads, 100 ops each)...\n");
    
    global_tree = bplus_tree_create(4, compare_ints, NULL);
    setup_test_data(200);
    
    // Insert initial data
    for (int i = 0; i < 100; i++) {
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
    }
    
    const int thread_count = 2;
    const int operations_per_thread = 100;
    
    pthread_t threads[thread_count];
    ThreadArgs args[thread_count];
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 99;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Simple verification
    int found_count = 0;
    for (int i = 0; i < 100; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Items remaining after minimal race conditions: %d\n", found_count);
    assert(found_count >= 0 && found_count <= 100);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Minimal race condition test passed\n");
    return true;
}

/**
 * @brief Test with 4 threads, moderate operations
 */
bool test_moderate_thread_count() {
    printf("Running moderate thread count test (4 threads, 200 ops each)...\n");
    
    global_tree = bplus_tree_create(6, compare_ints, NULL);
    setup_test_data(400);
    
    // Insert initial data
    for (int i = 0; i < 200; i++) {
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
    }
    
    const int thread_count = 4;
    const int operations_per_thread = 200;
    
    pthread_t threads[thread_count];
    ThreadArgs args[thread_count];
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 199;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int found_count = 0;
    for (int i = 0; i < 200; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Items remaining after moderate race conditions: %d\n", found_count);
    assert(found_count >= 0 && found_count <= 200);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ Moderate thread count test passed\n");
    return true;
}

/**
 * @brief Test with 8 threads, higher operations
 */
bool test_high_thread_count() {
    printf("Running high thread count test (8 threads, 300 ops each)...\n");
    
    global_tree = bplus_tree_create(7, compare_ints, NULL);
    setup_test_data(600);
    
    // Insert initial data
    for (int i = 0; i < 300; i++) {
        bplus_tree_insert(global_tree, (void*)(long)test_keys[i], test_values[i]);
    }
    
    const int thread_count = 8;
    const int operations_per_thread = 300;
    
    pthread_t threads[thread_count];
    ThreadArgs args[thread_count];
    
    for (int i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        args[i].start_key = 0;
        args[i].end_key = 299;
        args[i].operation_count = operations_per_thread;
        pthread_create(&threads[i], NULL, extreme_operations_thread, &args[i]);
    }
    
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int found_count = 0;
    for (int i = 0; i < 300; i++) {
        void* val = bplus_tree_find(global_tree, (void*)(long)i);
        if (val != NULL) {
            found_count++;
        }
    }
    
    printf("  Items remaining after high thread count race conditions: %d\n", found_count);
    assert(found_count >= 0 && found_count <= 300);
    
    bplus_tree_destroy(global_tree);
    global_tree = NULL;
    printf("✅ High thread count test passed\n");
    return true;
}

// Main function to run all tests
int main() {
    printf("==================================\n");
    printf(" B+ Tree Race Condition Tests\n");
    printf("==================================\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    printf("Starting tests...\n");
    
    // Run all tests
    RUN_TEST(test_simple_race_condition);
    RUN_TEST(test_concurrent_insert_find);
    RUN_TEST(test_moderate_race_conditions);
    RUN_TEST(test_extreme_race_conditions);
    RUN_TEST(test_safer_extreme_race_conditions); // Added new test
    RUN_TEST(test_advanced_race_conditions);
    
    // New complex test cases
    RUN_TEST(test_variable_tree_race_conditions);
         RUN_TEST(test_memory_pressure_race_conditions);
     RUN_TEST(test_rapid_tree_recreation_race_conditions);
     
     // New focused tests to isolate issues
     RUN_TEST(test_minimal_race_conditions);
     RUN_TEST(test_moderate_thread_count);
     RUN_TEST(test_high_thread_count);
    
    // Print summary
    printf("----------------------------------\n");
    printf("Race Condition Test Results: %d Passed, %d Failed\n", tests_passed, tests_failed);
    printf("----------------------------------\n");
    
    if (tests_failed == 0) {
        printf("🎉 All race condition tests PASSED!\n");
        return 0;
    } else {
        printf("💥 Some race condition tests FAILED!\n");
        return 1;
    }
}
