#include "test_utils.h"
#include <stdbool.h>

// Shared test data arrays
int* test_keys[10000];
char* test_values[10000];

// Initialize arrays to NULL
static void init_test_arrays() {
    for (int i = 0; i < 10000; i++) {
        test_keys[i] = NULL;
        test_values[i] = NULL;
    }
}

void setup_test_data(int count) {
    // Initialize arrays if this is the first call
    static bool initialized = false;
    if (!initialized) {
        init_test_arrays();
        initialized = true;
    }
    
    for (int i = 0; i < count; i++) {
        test_keys[i] = malloc(sizeof(int));
        *test_keys[i] = i;
        
        test_values[i] = malloc(20 * sizeof(char));
        sprintf(test_values[i], "Value-%d", i);
    }
}

void teardown_test_data(int count) {
    for (int i = 0; i < count; i++) {
        if (test_keys[i] != NULL) {
            free(test_keys[i]);
            test_keys[i] = NULL;
        }
        // Values are freed by the tree's destroyer
    }
}
