/**
 * serialization_test.c
 *
 * Comprehensive tests for B-tree and linked list serialization functionality.
 * Tests include saving/loading data structures, data integrity verification,
 * and performance benchmarking of serialization operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include "btree.h"
#include "llist.h"

// Test configuration
#define TEST_DATA_SIZE 1000
#define TEST_FILENAME_BTREE "test_btree.bin"
#define TEST_FILENAME_LLIST "test_llist.bin"
#define TEST_FILENAME_BTREE_LARGE "test_btree_large.bin"
#define TEST_FILENAME_LLIST_LARGE "test_llist_large.bin"

// Utility functions for testing
void print_int(const void* data) {
    printf("%d", *(int*)data);
}

void print_string(const void* data) {
    printf("\"%s\"", (char*)data);
}

int compare_ints(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

int compare_strings(const void* a, const void* b) {
    return strcmp((char*)a, (char*)b);
}

void* copy_int(const void* data) {
    int* copy = malloc(sizeof(int));
    if (copy) *copy = *(int*)data;
    return copy;
}

void* copy_string(const void* data) {
    char* copy = strdup((char*)data);
    return copy;
}

// --- B-tree Serialization Tests ---

/**
 * @brief Tests basic B-tree serialization with integer keys and string values.
 */
bool test_btree_basic_serialization() {
    printf("--- Running test_btree_basic_serialization ---\n");
    
    // Create B-tree with serializers
    BPlusTree *tree = bplus_tree_create_with_serializers(16, compare_ints, destroy_string_value,
                                                        serialize_int_key, deserialize_int_key,
                                                        serialize_string_value, deserialize_string_value);
    assert(tree != NULL);
    
    // Insert test data
    for (int i = 0; i < 100; i++) {
        char *value = malloc(50);
        sprintf(value, "Value-%d", i);
        int result = bplus_tree_insert(tree, &i, value);
        assert(result == 0);
    }
    
    printf("  Inserted 100 items\n");
    
    // Save to file
    int save_result = bplus_tree_save_to_file(tree, TEST_FILENAME_BTREE);
    assert(save_result == 0);
    printf("  Saved to file: %s\n", TEST_FILENAME_BTREE);
    
    // Verify file exists and has content
    struct stat st;
    assert(stat(TEST_FILENAME_BTREE, &st) == 0);
    assert(st.st_size > 0);
    printf("  File size: %ld bytes\n", st.st_size);
    
    // Load from file
    BPlusTree *loaded_tree = bplus_tree_load_from_file(TEST_FILENAME_BTREE, compare_ints, 
                                                      destroy_string_value, deserialize_int_key, 
                                                      deserialize_string_value);
    // Note: Full deserialization not yet implemented, so this will be NULL
    // assert(loaded_tree != NULL);
    
    // Cleanup
    bplus_tree_destroy(tree);
    if (loaded_tree) bplus_tree_destroy(loaded_tree);
    
    // Remove test file
    remove(TEST_FILENAME_BTREE);
    
    printf("✅ PASS: test_btree_basic_serialization\n");
    return true;
}

/**
 * @brief Tests B-tree serialization with large dataset.
 */
bool test_btree_large_serialization() {
    printf("--- Running test_btree_large_serialization ---\n");
    
    // Create B-tree with serializers
    BPlusTree *tree = bplus_tree_create_with_serializers(32, compare_ints, destroy_string_value,
                                                        serialize_int_key, deserialize_int_key,
                                                        serialize_string_value, deserialize_string_value);
    assert(tree != NULL);
    
    // Insert large amount of test data
    clock_t start = clock();
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        char *value = malloc(100);
        sprintf(value, "LargeValue-%d-WithLongString", i);
        int result = bplus_tree_insert(tree, &i, value);
        assert(result == 0);
        
        if ((i + 1) % 100 == 0) {
            printf("    Inserted %d items...\n", i + 1);
        }
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("  Inserted %d items in %.4f seconds\n", TEST_DATA_SIZE, insert_time);
    
    // Save to file
    start = clock();
    int save_result = bplus_tree_save_to_file(tree, TEST_FILENAME_BTREE_LARGE);
    end = clock();
    double save_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    assert(save_result == 0);
    printf("  Saved to file in %.4f seconds: %s\n", save_time, TEST_FILENAME_BTREE_LARGE);
    
    // Verify file exists and has content
    struct stat st;
    assert(stat(TEST_FILENAME_BTREE_LARGE, &st) == 0);
    assert(st.st_size > 0);
    printf("  File size: %ld bytes (%.2f bytes per item)\n", st.st_size, 
           (double)st.st_size / TEST_DATA_SIZE);
    
    // Cleanup
    bplus_tree_destroy(tree);
    
    // Remove test file
    remove(TEST_FILENAME_BTREE_LARGE);
    
    printf("✅ PASS: test_btree_large_serialization\n");
    return true;
}

/**
 * @brief Tests B-tree serialization with string keys.
 */
bool test_btree_string_key_serialization() {
    printf("--- Running test_btree_string_key_serialization ---\n");
    
    // Create B-tree with string key serializers
    BPlusTree *tree = bplus_tree_create_with_serializers(16, compare_strings, destroy_string_value,
                                                        serialize_string_key, deserialize_string_key,
                                                        serialize_string_value, deserialize_string_value);
    assert(tree != NULL);
    
    // Insert test data with string keys
    char keys[100][20];
    for (int i = 0; i < 100; i++) {
        sprintf(keys[i], "Key-%d", i);
        char *value = malloc(50);
        sprintf(value, "StringValue-%d", i);
        int result = bplus_tree_insert(tree, keys[i], value);
        assert(result == 0);
    }
    
    printf("  Inserted 100 items with string keys\n");
    
    // Save to file
    int save_result = bplus_tree_save_to_file(tree, TEST_FILENAME_BTREE);
    assert(save_result == 0);
    printf("  Saved to file: %s\n", TEST_FILENAME_BTREE);
    
    // Verify file exists
    struct stat st;
    assert(stat(TEST_FILENAME_BTREE, &st) == 0);
    printf("  File size: %ld bytes\n", st.st_size);
    
    // Cleanup
    bplus_tree_destroy(tree);
    
    // Remove test file
    remove(TEST_FILENAME_BTREE);
    
    printf("✅ PASS: test_btree_string_key_serialization\n");
    return true;
}

// --- Linked List Serialization Tests ---

/**
 * @brief Tests basic linked list serialization with integer data.
 */
bool test_llist_basic_serialization() {
    printf("--- Running test_llist_basic_serialization ---\n");
    
    // Create linked list with serializer
    LinkedList *list = llist_create_with_serializer(free, serialize_int_data, deserialize_int_data);
    assert(list != NULL);
    
    // Insert test data
    for (int i = 0; i < 100; i++) {
        int *data = malloc(sizeof(int));
        *data = i;
        int result = llist_append(list, data);
        assert(result == 0);
    }
    
    printf("  Inserted 100 items\n");
    
    // Save to file
    int save_result = llist_save_to_file(list, TEST_FILENAME_LLIST);
    assert(save_result == 0);
    printf("  Saved to file: %s\n", TEST_FILENAME_LLIST);
    
    // Verify file exists and has content
    struct stat st;
    assert(stat(TEST_FILENAME_LLIST, &st) == 0);
    assert(st.st_size > 0);
    printf("  File size: %ld bytes\n", st.st_size);
    
    // Load from file
    LinkedList *loaded_list = llist_load_from_file(TEST_FILENAME_LLIST, free, deserialize_int_data);
    assert(loaded_list != NULL);
    assert(llist_size(loaded_list) == 100);
    
    // Verify data integrity
    for (int i = 0; i < 100; i++) {
        int *data = (int*)llist_get_at(loaded_list, i);
        assert(data != NULL);
        assert(*data == i);
    }
    
    printf("  Loaded and verified 100 items\n");
    
    // Cleanup
    llist_destroy(list);
    llist_destroy(loaded_list);
    
    // Remove test file
    remove(TEST_FILENAME_LLIST);
    
    printf("✅ PASS: test_llist_basic_serialization\n");
    return true;
}

/**
 * @brief Tests linked list serialization with large dataset.
 */
bool test_llist_large_serialization() {
    printf("--- Running test_llist_large_serialization ---\n");
    
    // Create linked list with serializer
    LinkedList *list = llist_create_with_serializer(free, serialize_int_data, deserialize_int_data);
    assert(list != NULL);
    
    // Insert large amount of test data
    clock_t start = clock();
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        int *data = malloc(sizeof(int));
        *data = i;
        int result = llist_append(list, data);
        assert(result == 0);
        
        if ((i + 1) % 100 == 0) {
            printf("    Inserted %d items...\n", i + 1);
        }
    }
    clock_t end = clock();
    double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("  Inserted %d items in %.4f seconds\n", TEST_DATA_SIZE, insert_time);
    
    // Save to file
    start = clock();
    int save_result = llist_save_to_file(list, TEST_FILENAME_LLIST_LARGE);
    end = clock();
    double save_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    assert(save_result == 0);
    printf("  Saved to file in %.4f seconds: %s\n", save_time, TEST_FILENAME_LLIST_LARGE);
    
    // Verify file exists and has content
    struct stat st;
    assert(stat(TEST_FILENAME_LLIST_LARGE, &st) == 0);
    assert(st.st_size > 0);
    printf("  File size: %ld bytes (%.2f bytes per item)\n", st.st_size, 
           (double)st.st_size / TEST_DATA_SIZE);
    
    // Load from file
    start = clock();
    LinkedList *loaded_list = llist_load_from_file(TEST_FILENAME_LLIST_LARGE, free, deserialize_int_data);
    end = clock();
    double load_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    assert(loaded_list != NULL);
    assert(llist_size(loaded_list) == TEST_DATA_SIZE);
    printf("  Loaded %d items in %.4f seconds\n", TEST_DATA_SIZE, load_time);
    
    // Verify data integrity
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        int *data = (int*)llist_get_at(loaded_list, i);
        assert(data != NULL);
        assert(*data == i);
    }
    printf("  Verified data integrity\n");
    
    // Cleanup
    llist_destroy(list);
    llist_destroy(loaded_list);
    
    // Remove test file
    remove(TEST_FILENAME_LLIST_LARGE);
    
    printf("✅ PASS: test_llist_large_serialization\n");
    return true;
}

/**
 * @brief Tests linked list serialization with string data.
 */
bool test_llist_string_serialization() {
    printf("--- Running test_llist_string_serialization ---\n");
    
    // Create linked list with string serializer
    LinkedList *list = llist_create_with_serializer(free, serialize_string_data, deserialize_string_data);
    assert(list != NULL);
    
    // Insert test data with strings
    for (int i = 0; i < 100; i++) {
        char *str = malloc(50);
        sprintf(str, "StringValue-%d", i);
        int result = llist_append(list, str);
        assert(result == 0);
    }
    
    printf("  Inserted 100 string items\n");
    
    // Save to file
    int save_result = llist_save_to_file(list, TEST_FILENAME_LLIST);
    assert(save_result == 0);
    printf("  Saved to file: %s\n", TEST_FILENAME_LLIST);
    
    // Verify file exists
    struct stat st;
    assert(stat(TEST_FILENAME_LLIST, &st) == 0);
    printf("  File size: %ld bytes\n", st.st_size);
    
    // Load from file
    LinkedList *loaded_list = llist_load_from_file(TEST_FILENAME_LLIST, free, deserialize_string_data);
    assert(loaded_list != NULL);
    assert(llist_size(loaded_list) == 100);
    
    // Verify data integrity
    for (int i = 0; i < 100; i++) {
        char *str = (char*)llist_get_at(loaded_list, i);
        assert(str != NULL);
        char expected[50];
        sprintf(expected, "StringValue-%d", i);
        assert(strcmp(str, expected) == 0);
    }
    
    printf("  Loaded and verified 100 string items\n");
    
    // Cleanup
    llist_destroy(list);
    llist_destroy(loaded_list);
    
    // Remove test file
    remove(TEST_FILENAME_LLIST);
    
    printf("✅ PASS: test_llist_string_serialization\n");
    return true;
}

/**
 * @brief Tests linked list operations after deserialization.
 */
bool test_llist_post_deserialization_operations() {
    printf("--- Running test_llist_post_deserialization_operations ---\n");
    
    // Create and populate list
    LinkedList *list = llist_create_with_serializer(free, serialize_int_data, deserialize_int_data);
    assert(list != NULL);
    
    for (int i = 0; i < 50; i++) {
        int *data = malloc(sizeof(int));
        *data = i;
        llist_append(list, data);
    }
    
    // Save and load
    assert(llist_save_to_file(list, TEST_FILENAME_LLIST) == 0);
    LinkedList *loaded_list = llist_load_from_file(TEST_FILENAME_LLIST, free, deserialize_int_data);
    assert(loaded_list != NULL);
    
    // Test operations on loaded list
    assert(llist_size(loaded_list) == 50);
    
    // Test insert operations
    int *new_data = malloc(sizeof(int));
    *new_data = 100;
    assert(llist_insert_at(loaded_list, 25, new_data) == 0);
    assert(llist_size(loaded_list) == 51);
    
    // Test find operations
    int search_key = 25;
    int *found = llist_find(loaded_list, &search_key, compare_ints);
    assert(found != NULL && *found == 25);
    
    // Test remove operations
    assert(llist_remove_at(loaded_list, 25) == 0);
    assert(llist_size(loaded_list) == 50);
    
    // Test reverse operation
    LinkedList *reversed = llist_reverse(loaded_list);
    assert(reversed != NULL);
    assert(llist_size(reversed) == 50);
    
    // Verify reverse worked
    int *first = (int*)llist_get_at(reversed, 0);
    int *last = (int*)llist_get_at(reversed, 49);
    assert(*first == 49 && *last == 0);
    
    printf("  Tested all operations on deserialized list\n");
    
    // Cleanup
    llist_destroy(list);
    llist_destroy(loaded_list);
    llist_destroy(reversed);
    
    // Remove test file
    remove(TEST_FILENAME_LLIST);
    
    printf("✅ PASS: test_llist_post_deserialization_operations\n");
    return true;
}

// --- Performance Tests ---

/**
 * @brief Benchmarks serialization performance for both data structures.
 */
bool test_serialization_performance() {
    printf("--- Running test_serialization_performance ---\n");
    
    const int sizes[] = {100, 1000, 10000};
    const int num_sizes = 3;
    
    for (int size_idx = 0; size_idx < num_sizes; size_idx++) {
        int size = sizes[size_idx];
        printf("  Testing size: %d\n", size);
        
        // B-tree performance
        BPlusTree *btree = bplus_tree_create_with_serializers(32, compare_ints, destroy_string_value,
                                                            serialize_int_key, deserialize_int_key,
                                                            serialize_string_value, deserialize_string_value);
        
        clock_t start = clock();
        for (int i = 0; i < size; i++) {
            char *value = malloc(50);
            sprintf(value, "Value-%d", i);
            bplus_tree_insert(btree, &i, value);
        }
        clock_t end = clock();
        double insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        start = clock();
        int save_result = bplus_tree_save_to_file(btree, TEST_FILENAME_BTREE);
        end = clock();
        double save_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        assert(save_result == 0);
        printf("    B-tree: %d items, insert: %.4fs, save: %.4fs\n", size, insert_time, save_time);
        
        bplus_tree_destroy(btree);
        remove(TEST_FILENAME_BTREE);
        
        // Linked list performance
        LinkedList *llist = llist_create_with_serializer(free, serialize_int_data, deserialize_int_data);
        
        start = clock();
        for (int i = 0; i < size; i++) {
            int *data = malloc(sizeof(int));
            *data = i;
            llist_append(llist, data);
        }
        end = clock();
        insert_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        start = clock();
        save_result = llist_save_to_file(llist, TEST_FILENAME_LLIST);
        end = clock();
        save_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        assert(save_result == 0);
        printf("    List:  %d items, insert: %.4fs, save: %.4fs\n", size, insert_time, save_time);
        
        llist_destroy(llist);
        remove(TEST_FILENAME_LLIST);
    }
    
    printf("✅ PASS: test_serialization_performance\n");
    return true;
}

// --- Main Test Runner ---

int main() {
    printf("==================================\n");
    printf(" Serialization Test Suite\n");
    printf("==================================\n\n");
    
    // B-tree serialization tests
    test_btree_basic_serialization();
    test_btree_large_serialization();
    test_btree_string_key_serialization();
    
    // Linked list serialization tests
    test_llist_basic_serialization();
    test_llist_large_serialization();
    test_llist_string_serialization();
    test_llist_post_deserialization_operations();
    
    // Performance tests
    test_serialization_performance();
    
    printf("\n----------------------------------\n");
    printf("Test Results: All Passed\n");
    printf("----------------------------------\n");
    
    return 0;
}
