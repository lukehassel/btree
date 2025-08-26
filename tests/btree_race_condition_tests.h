#ifndef BTREE_RACE_CONDITION_TESTS_H
#define BTREE_RACE_CONDITION_TESTS_H

#include <stdbool.h>
#include <pthread.h>

// Test functions
bool test_simple_race_condition(void);
bool test_concurrent_insert_find(void);
bool test_moderate_race_conditions(void);
bool test_extreme_race_conditions(void);
bool test_advanced_race_conditions(void);

// Thread functions
void* insert_even_keys(void* arg);
void* insert_odd_keys(void* arg);
void* insert_keys_thread(void* arg);
void* find_keys_thread(void* arg);
void* mixed_operations_thread(void* arg);
void* extreme_operations_thread(void* arg);
void* delete_keys_thread(void* arg);

// Phase functions
void run_insert_phase(int thread_count, int operations_per_thread);
void run_mixed_phase(int thread_count, int operations_per_thread);
void run_stress_phase(int thread_count, int operations_per_thread);
void run_cleanup_phase(int thread_count, int operations_per_thread);

// Utility functions
void verify_tree_integrity(void);

#endif // BTREE_RACE_CONDITION_TESTS_H
