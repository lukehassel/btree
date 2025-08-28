#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shared test data
extern int* test_keys[10000];
extern char* test_values[10000];

// Test data setup and teardown functions
void setup_test_data(int count);
void teardown_test_data(int count);

#endif // TEST_UTILS_H


